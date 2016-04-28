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
#include <Battery.h>
#include <TimerUtils.h>
#include <Display.h>
#include <USB_VirtualCOM.h>

#include "button.h"
#include "communication.h"
#include "display.h"
#include "globals.h"
#include "materials.h"
#include "settings.h"

#include "mode_watt.h"
#include "mode_volt.h"
#include "mode_temp.h"

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

#define REGISTER_MODE(X) g.vapeModes[X.index] = &X

void uptime(uint32_t param) {
    gv.uptime++;
}

void fire(uint8_t status, uint32_t held) {
    screenOn();
    if (status & BUTTON_PRESS)
        __vape();
    else
        __screenOff();
}

void left(uint8_t status, uint32_t held) {
    screenOn();
    if ((status & BUTTON_PRESS) ||
        ((held > 30) && status & BUTTON_HELD))
        __down();
    else 
        __screenOff();
}

void right(uint8_t status, uint32_t held) {
    screenOn();
    if ((status & BUTTON_PRESS) ||
        ((held > 30) && status & BUTTON_HELD))
        __up();
    else
        __screenOff();
}

struct buttonHandler mainButtonHandler = {
    .name = "mainButtons",
    .flags = LEFT_HOLD_EVENT | RIGHT_HOLD_EVENT | FIRE_REPEAT,

    .fire_handler = &fire,
    .fire_repeated = &showMenu,
    .fireRepeatCount = 3,
    .fireRepeatTimeout = 30,

    .left_handler = &left,
    .leftUpdateInterval = 10,

    .right_handler = &right,
    .rightUpdateInterval = 10,

};

int main() {
    int i = 0;
    gv.uptimeTimer = Timer_CreateTimer(100, 1, uptime, 3);
    Communication_Init();
    initHandlers();
    setHandler(&mainButtonHandler);

    load_settings();
    
    REGISTER_MODE(variableVoltage);
    REGISTER_MODE(variableWattage);
    REGISTER_MODE(variableTemp);

    setVapeMode(s.mode);
    setVapeMaterial(s.materialIndex);

    
    Atomizer_ReadInfo(&g.atomInfo);

    // Initialize atomizer info
    do {
        Atomizer_ReadInfo(&g.atomInfo);
        updateScreen(&g);
        i++;
    } while (i < 100 && g.atomInfo.resistance == 0);

    while (g.atomInfo.resistance - g.atomInfo.base_resistance > 10) {
        Atomizer_ReadInfo(&g.atomInfo);
        updateScreen(&g);
    }
    screenOn();
    __screenOff();

    if (!s.fromRom)
    gv.shouldShowMenu = 1;
    uint8_t rcmd[63];
    i = 0;
    while (1) {
    g.charging = Battery_IsCharging();

    if ((s.dumpPids || s.tunePids) && !g.charging)
                s.dumpPids = s.tunePids = 0;

    if (gv.buttonEvent) {
        
        handleButtonEvents();
        gv.buttonEvent = 0;
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
        /* This should be updatedto eat all control chars */
        if (rcmd[i - 1] == '\r') {
            // eat \r
            rcmd[i - 1] = '\0';
            i--;
        } else if (rcmd[i - 1] == '\n') {
            rcmd[i - 1] = '\0';
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
    // TODO WFI
    }
}
