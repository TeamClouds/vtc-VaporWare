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
#include <Button.h>
#include <TimerUtils.h>
#include <Display.h>
#include <System.h>
#include <USB_VirtualCOM.h>

#include "button.h"
#include "communication.h"
#include "dataflash.h"
#include "debug.h"
#include "display.h"
#include "globals.h"
#include "materials.h"
#include "settings.h"

#include "mode.h"
#include "mode_watt.h"
#include "mode_volt.h"
#include "mode_temp.h"


inline void screenOn() {
    if (!s.stealthMode)
        Display_SetOn(1);

    g.sysSleepAt = 0;
    g.screenOffTime = gv.uptime + s.screenTimeout * 10;
    g.pauseScreenOff = 1;
}

inline void screenOff() {
    g.pauseScreenOff = 0;
}

void uptime(uint32_t param) {
    gv.uptime++;
    if (!gv.sleeping && (
        (buttonTimeout && *buttonTimeout > gv.uptime) || gv.buttonEvent)
        )
        buttonTimer(param);
}

void fire(uint8_t status, uint32_t held) {

    screenOn();
    if (Button_GetState() & BUTTON_MASK_RIGHT) {
        stealthModeSet(!s.stealthMode);
    } else if (Button_GetState() & BUTTON_MASK_LEFT) {
        vsetLockSet(!s.vsetLock);
    } else if (status & BUTTON_PRESS) {
        __vape();
    }
    screenOn();
    screenOff();
    
}

void left(uint8_t status, uint32_t held) {


    screenOn();
    if (!s.vsetLock && ((status & BUTTON_PRESS) ||
        ((held > 30) && status & BUTTON_HELD)))
        __down();
    else 
        screenOff();
}

void right(uint8_t status, uint32_t held) {
    screenOn();
    if (!s.vsetLock && ((status & BUTTON_PRESS) ||
        ((held > 30) && status & BUTTON_HELD)))
        __up();
    else
        screenOff();
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

void attyPromptFire(uint8_t status, uint32_t held) {

}

void attyPromptLeft(uint8_t status, uint32_t held) {
    baseFromUserSet(USERSET);
    g.askUser = 0;
}

void attyPromptRight(uint8_t status, uint32_t held) {
    baseResSet(g.newBaseRes);
    baseTempSet(g.newBaseTemp);
    g.askUser = 0;
}

struct buttonHandler attyPromptHandler = {
    .name = "attyPrompt",
    .flags = 0,
    .fire_handler = &attyPromptFire,
    .left_handler = &attyPromptLeft,
    .right_handler = &attyPromptRight,
};

void drawPrompt() {
    Display_Clear();
    char buff[10];
    Display_PutText(0, 0,  "Atomizer", FONT_SMALL);
    Display_PutText(0, 10, "Changed", FONT_SMALL);

    Display_PutText(0, 30, "<Use Old", FONT_SMALL);
    siprintf(buff, "%01u.%02u", g.baseRes / 1000, (g.baseRes % 1000)/10);
    Display_PutText(8, 40, buff, FONT_SMALL);

    Display_PutText(0, 60, "Use New>", FONT_SMALL);
    siprintf(buff, "%01u.%02u", g.newBaseRes / 1000, (g.newBaseRes % 1000)/10);
    Display_PutText(8, 70, buff, FONT_SMALL);

    Display_Update();
}

void askUserAboutTheAttomizer() {
    switchHandler(&attyPromptHandler);
    do {;} while (Button_GetState());
    while (g.askUser) {
        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }
        drawPrompt();
    }
    do {;} while (Button_GetState());
    g.newBaseRes = 0;
    g.newBaseTemp = 0;
    returnHandler();
}

uint8_t newReading(uint16_t oldRes, uint8_t oldTemp, uint16_t *newRes, uint8_t *newTemp) {
    uint16_t lowRes;

    if (oldRes == 0 && g.baseFromUser == USERSET) {
        g.baseFromUser = USERLOCK;
    }

    switch(g.baseFromUser) {
        case AUTORES:
            if (((*newRes < g.baseRes) && (*newRes > 0)) ||
                  g.baseRes == 0) {
                    baseResSet(*newRes);
                    baseTempSet(*newTemp);
            }
            break;
        case USERSET:
            break;
        case USERLOCK:
            lowRes = (100 - BRESDIFFPCT) * g.baseRes / 100;

            if (*newRes < lowRes) {
                gv.fireButtonPressed = 0;
                g.newBaseRes = *newRes;
                g.newBaseTemp = *newTemp;
                g.askUser = 1;
            }
            break;
    }
    return 1;
}

int main() {
    int i = 0;
    gv.uptimeTimer = Timer_CreateTimer(100, 1, uptime, 3);
    Communication_Init();

    initHandlers();
    setHandler(&mainButtonHandler);
    Atomizer_SetBaseUpdateCallback(newReading);

    addVapeMode(&variableVoltage);
    addVapeMode(&variableWattage);
    addVapeMode(&variableTemp);

    load_settings();

    struct vapeMode THEMAX = {
        .name = "\0",
        .index = MAX_CONTROL,
    };

    addVapeMode(&THEMAX);

    setVapeMode(s.mode);
    
    screenOn();
    screenOff();

    if (!s.fromRom)
        gv.shouldShowMenu = 1;

    uint8_t rcmd[63];
    i = 0;
    while (1) {

    g.charging = Battery_IsCharging();
    Atomizer_ReadInfo(&g.atomInfo);

    if ((s.dumpPids || s.tunePids) && !g.charging)
                s.dumpPids = s.tunePids = 0;

    if (g.askUser) {
        screenOn();
        askUserAboutTheAttomizer();
        screenOn();
        screenOff();
    }

    if (gv.buttonEvent) {
        
        handleButtonEvents();
        gv.buttonEvent = 0;
    }
    if (g.pauseScreenOff)
        screenOn();

    if (g.settingsChanged && gv.uptime > g.writeSettingsAt) {
        writeSettings();
        g.writeSettingsAt = 0;
    }

    if (gv.shouldShowMenu) {
        showMenu();
        gv.shouldShowMenu = 0;
    } else if (s.stealthMode) {
        Display_Clear();
        Display_SetOn(0);
    } else if (!s.stealthMode && (g.nextRefresh < gv.uptime) && ((g.screenOffTime >= gv.uptime) || g.charging)) {
        g.nextRefresh = gv.uptime + 6;
        Display_SetOn(1);
        updateScreen(&g);
    } else if (gv.sleeping) {
        gv.sleeping = 0;
    } else if ((g.screenOffTime < gv.uptime) && !g.charging) {

        if (g.settingsChanged && !g.writeSettingsAt)
            g.writeSettingsAt = gv.uptime + SETTINGSWRITEDEFAULT;

        if (g.sysSleepAt == 0)
            g.sysSleepAt = gv.uptime + SYSSLEEPDEFAULT;

        g.screenFadeInTime = 0;
        Display_Clear();
        Display_SetOn(0);
        if (!g.settingsChanged) {
            if (gv.uptime > g.sysSleepAt) {
                gv.sleeping = 1;
                Sys_Sleep();
                if (!s.stealthMode) {
                    Display_SetOn(1);
                    screenOn();
                    screenOff();
                    updateScreen(&g);
                }
            }
        }
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
