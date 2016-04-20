/*
 * This file is part of eVic SDK.
 *
 * eVic SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eVic SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eVic SDK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2016 ReservedField
 * Copyright (C) 2016 kfazz
 */
#include <stdio.h>
#include <string.h>

#include <M451Series.h>
#include <Atomizer.h>
#include <Button.h>
#include <Battery.h>
#include <TimerUtils.h>
#include <Display.h>
#include <USB_VirtualCOM.h>

#include "main.h"
#include "communication.h"
#include "mode_watt.h"
#include "mode_volt.h"
#include "mode_temp.h"

struct globals g = { };

volatile struct globalVols gv = {
    .fireButtonPressed = 0,
    .screenState = 1,
    .screenOffTimer = -1,
};
struct settings s = { };

struct vapeMaterials vapeMaterialList[] = {
    {
     .typeMask = KANTHAL,
     .name = "KA",
     .tcr = 0,
     },
    {
     .typeMask = NICKEL,
     .name = "NI",
     .tcr = 620,
     },
    {
     .typeMask = TITANIUM,
     .name = "TI",
     .tcr = 350,
     },
    {
     .typeMask = STAINLESS,
     .name = "SS",
     .tcr = 105,
     },
};

// ALWAYS init it a sane mode
void (*__init) (void);
void (*__vape) (void);
void (*__up) (void);
void (*__down) (void);

void setVapeMode(int newMode) {
    if (newMode >= MODE_COUNT)
	return;

    s.mode = newMode;

    __vape = g.vapeModes[newMode]->fire;
    __up = g.vapeModes[newMode]->increase;
    __down = g.vapeModes[newMode]->decrease;
    if (g.vapeModes[newMode]->init) {
	__init = g.vapeModes[newMode]->init;
	__init();
    }
}

void setVapeMaterial(int index) {
    struct vapeMaterials *material = &vapeMaterialList[index];
    s.materialIndex = index;
    g.atomInfo.tcr = material->tcr;
}

inline void __screenOff(void);

void screenOffTimeout(uint32_t c) {
    gv.screenState--;
    if (gv.screenState >= 1) {
	__screenOff();
    } else {
	gv.buttonCnt = 0;
    }
}

inline void screenOn() {
    gv.screenState = s.screenTimeout;
}

inline void __screenOff() {
    if (gv.screenOffTimer >= 0)
	Timer_DeleteTimer(gv.screenOffTimer);
    gv.screenOffTimer = Timer_CreateTimeout(100, 0, screenOffTimeout, 9);
}

void startVaping(uint32_t counterIndex) {
    if (gv.buttonCnt < 3) {
	if (Button_GetState() & BUTTON_MASK_FIRE) {
	    Timer_DeleteTimer(gv.screenOffTimer);
	    gv.screenOffTimer = 5;
	    gv.fireButtonPressed = 1;
	    gv.buttonCnt = 0;
	}
    } else {
	gv.shouldShowMenu = 1;
	gv.buttonCnt = 0;
    }
}

void buttonFire(uint8_t state) {
    screenOn();
    if (state & BUTTON_MASK_FIRE) {
	if (gv.fireTimer)
	    Timer_DeleteTimer(gv.fireTimer);
	gv.fireTimer = Timer_CreateTimeout(200, 0, startVaping, 3);
	gv.buttonCnt++;
    } else {
	screenOn();
	__screenOff();
	gv.fireButtonPressed = 0;
    }
}

void buttonRight(uint8_t state) {
    screenOn();
    if (state & BUTTON_MASK_RIGHT) {
	__up();
    } else {
	__screenOff();
    }
}

void buttonLeft(uint8_t state) {
    screenOn();
    if (state & BUTTON_MASK_LEFT) {
	__down();
    } else {
	__screenOff();
    }
}


void setupButtons() {
    g.fire = Button_CreateCallback(buttonFire, BUTTON_MASK_FIRE);
    g.plus = Button_CreateCallback(buttonRight, BUTTON_MASK_RIGHT);
    g.minus = Button_CreateCallback(buttonLeft, BUTTON_MASK_LEFT);
}

#define REGISTER_MODE(X) g.vapeModes[X.index] = &X

int main() {
    int i = 0;
    load_settings();
    setupButtons();

    REGISTER_MODE(variableVoltage);
    REGISTER_MODE(variableWattage);
    REGISTER_MODE(variableTemp);

    setVapeMode(s.mode);
    setVapeMaterial(s.materialIndex);

    Communication_Init();
    Atomizer_ReadInfo(&g.atomInfo);

    // Initialize atomizer info
    do {
	Atomizer_ReadInfo(&g.atomInfo);
	updateScreen(&g);
	i++;
    }
    while (i < 100 && g.atomInfo.resistance == 0);

    while (g.atomInfo.resistance - g.atomInfo.base_resistance > 10) {
	Atomizer_ReadInfo(&g.atomInfo);
	updateScreen(&g);
    }
    screenOn();
    __screenOff();

    if (!s.fromRom)
	gv.shouldShowMenu = 1;
    uint8_t rcmd[63];
    while (1) {
	g.charging = Battery_IsCharging();
	if (gv.fireButtonPressed) {
	    __vape();
	}
	if (gv.shouldShowMenu) {
	    showMenu();
	} else if (gv.screenState || g.charging) {
	    Display_SetOn(1);
	    updateScreen(&g);
	} else if (gv.screenState <= 1 && !g.charging) {
	    Timer_DelayMs(100);
	    Display_Clear();
	    Display_SetOn(0);
	}
        while(USB_VirtualCOM_GetAvailableSize() > 0) {
            uint8_t C = 0;
            i += USB_VirtualCOM_Read(&C,1);
            rcmd[i - 1] = C;
            if (rcmd[i - 1] == '\n') {
                Communication_Command((char *)rcmd);
                memset(rcmd, 0, sizeof(rcmd));
                i = 0;
            } else if (i == 62) {
                Communication_Command((char *)rcmd);
                memset(rcmd, 0, sizeof(rcmd));
                i = 0;
                break; //overflow
            }
        }
        Timer_DelayMs(66); // 15fps
    }
}
