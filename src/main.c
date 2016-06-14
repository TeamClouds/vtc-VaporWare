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

#include <Display.h>
#include <System.h>
#include <USB_VirtualCOM.h>

#include <game.h>
#include "button.h"
#include "communication.h"
#include "dataflash.h"
#include "debug.h"
#include "display.h"
#include "globals.h"
#include "materials.h"
#include "settings.h"
#include "variabletimer.h"
#include "atomizer_query.h"

#include "mode.h"

inline void screenOn() {
    if (!s.stealthMode)
        Display_SetOn(1);

    g.sysSleepAt = 0;
    g.screenOffTime = uptime + s.screenTimeout;
    g.pauseScreenOff = 1;
}

inline void screenOff() {
    g.pauseScreenOff = 0;
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
        ((held > 300) && status & BUTTON_HELD)))
        __down();
    else
        screenOff();
}

void right(uint8_t status, uint32_t held) {
    screenOn();
    if (!s.vsetLock && ((status & BUTTON_PRESS) ||
        ((held > 300) && status & BUTTON_HELD)))
        __up();
    else
        screenOff();
}

void launchMenu() {
    showMenu();
    screenOn();
    screenOff();
}

struct buttonHandler mainButtonHandler = {
    .name = "mainButtons",
    .flags = LEFT_HOLD_EVENT | RIGHT_HOLD_EVENT | FIRE_REPEAT,

    .fire_handler = &fire,
    .fire_repeated = &launchMenu,
    .fireRepeatCount = 3,
    .fireRepeatTimeout = 300,

    .left_handler = &left,
    .leftUpdateInterval = 100,

    .right_handler = &right,
    .rightUpdateInterval = 100,

};

#ifdef ATYDEBUG
void drawError() {

    if (Display_IsFlipped()) {
        Display_Flip();
    }

    Display_Clear();
    char buff[10];
    switch(gv.sawError) {
        case OK: siprintf(buff, "OK"); break;
        case SHORT: siprintf(buff, "SHORT"); break;
        case OPEN: siprintf(buff, "OPEN"); break;
        case WEAK_BATT: siprintf(buff, "WEAK_BATT"); break;
        case OVER_TEMP: siprintf(buff, "OVER_TEMP"); break;
        default: siprintf(buff, "UNKNOWN"); break;
    }

    Display_PutText(0, 0,  buff, FONT_SMALL);

    Display_Update();
}

void nullButton(uint8_t status, uint32_t held) {}

void errorButton(uint8_t status, uint32_t held) {
    gv.sawError = OK;
    g.ignoreNextAttyUntil = ATOMIZERGONEAFTER + uptime;
    Atomizer_Unlock();
}

struct buttonHandler errorPromptHandler = {
    .name = "attyPrompt",
    .flags = 0,
    .fire_handler = &nullButton,
    .left_handler = &errorButton,
    .right_handler = &errorButton,
};

void showUserError() {
    do {;} while (Button_GetState());

    switchHandler(&errorPromptHandler);
    while (gv.sawError) {
        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }
        drawError();
    }
    do {;} while (Button_GetState());
    returnHandler();
}

void atomizerError(uint8_t errorNum) {
    if (errorNum == OPEN && g.ignoreNextAttyUntil) {
        Atomizer_Unlock();
        return;
    }

    gv.sawError = errorNum;
}
#endif

int main() {
    int i = 0;
    Communication_Init();

    uint8_t mainTimerSlot = requestTimerSlot();
    requestTimer(mainTimerSlot, TimerStdres);

    initHandlers();
    setHandler(&mainButtonHandler);
    Atomizer_SetBaseUpdateCallback(newReading);

#ifdef ATYDEBUG
    Atomizer_SetErrorLock(1);
    Atomizer_SetErrorCallback(&atomizerError);
#endif

    load_settings();

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
    uint8_t atomizerOn = Atomizer_IsOn();

    if (!atomizerOn) {
	    g.batteryPercent = Battery_VoltageToPercent(
            Battery_IsPresent()? Battery_GetVoltage() : 0);
    }

    if ((s.dumpPids || s.tunePids) && !g.charging)
                s.dumpPids = s.tunePids = 0;

    if (gv.buttonEvent) {
        handleButtonEvents();
        gv.buttonEvent = 0;
    }

    if (gv.spacinVaper) {
        runSpace();
        screenOff();
        screenOn();
        g.screenFadeInTime = 0;
        gv.spacinVaper = 0;
    }

#ifdef ATYDEBUG
    if (gv.sawError) {
        screenOn();
        showUserError();
        screenOn();
        screenOff();
    }
#endif

    if (g.askUser) {
        screenOn();
        askUserAboutTheAttomizer();
        screenOn();
        screenOff();
    }

    if (g.pauseScreenOff)
        screenOn();

    if (Atomizer_GetError())
        screenOn();

    if (g.settingsChanged && uptime > g.writeSettingsAt) {
        writeSettings();
        g.writeSettingsAt = 0;
    }

    if (gv.shouldShowMenu) {
        showMenu();
        g.screenFadeInTime = 0;
        gv.shouldShowMenu = 0;
    } else if (s.stealthMode) {
        Display_Clear();
        Display_SetOn(0);
    } else if (!s.stealthMode && (g.nextRefresh < uptime) && ((g.screenOffTime < uptime) && g.charging)) {
        displayCharging();
    } else if (!s.stealthMode && (g.nextRefresh < uptime) && (g.screenOffTime >= uptime)) {
        updateScreen();
    } else if (gv.sleeping) {
        gv.sleeping = 0;
    } else if ((g.screenOffTime < uptime) && !g.charging) {

        if (g.settingsChanged && !g.writeSettingsAt)
            g.writeSettingsAt = uptime + SETTINGSWRITEDEFAULT;

        if (g.sysSleepAt == 0)
            g.sysSleepAt = uptime + SYSSLEEPDEFAULT;

        g.screenFadeInTime = 0;
        Display_Clear();
        Display_SetOn(0);
        if (!g.settingsChanged) {
            if (uptime > g.sysSleepAt) {
                gv.sleeping = 1;
                Sys_Sleep();
                if (!s.stealthMode) {
                    Display_SetOn(1);
                    screenOn();
                    screenOff();
                    updateScreen();
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
