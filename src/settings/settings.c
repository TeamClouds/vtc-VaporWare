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
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <M451Series.h>
#include <Display.h>
#include <TimerUtils.h>
#include <Button.h>
#include <USB_VirtualCOM.h>
#include <Dataflash.h>
#include <SysInfo.h>

#include "button.h"
#include "dataflash.h"
#include "debug.h"
#include "display.h"
#include "globals.h"
#include "helper.h"
#include "menu.h"
#include "settings.h"
#include "font/font_vaporware.h"
#include "variabletimer.h"

void saveDefaultSettings();

#define MAXOPTIONS 16

uint8_t getTypeDefault() {
    return s.materialIndex;
}

char *getTypeString(uint8_t index) {
    return vapeMaterialList[index].name;
}

void updateType(uint16_t index) {
    materialIndexSet(index);
    refreshMenu();
}

uint8_t getModeDefault() {
    return s.mode;
}

char *getModeString(uint8_t index) {
    if (g.vapeModes[index]->supportedMaterials &
        vapeMaterialList[s.materialIndex].typeMask)
        return g.vapeModes[index]->name;
    else
        return NULL;
}

void updateMode(uint16_t index) {
    modeSet(index);
}

int32_t getScreenBrightnessDefault() {
    return s.screenBrightness;
}

void formatBrightnessNumber(int32_t value, char *formatted) {
    Display_SetContrast((char*)value);
    siprintf(formatted, "%" PRId32, value);
}

void updateScreenBrightness(int32_t value) {
	screenBrightnessSet(value);
}

char *getScaleString(uint8_t index) {
    return tempScaleType[index].display;
}

uint8_t getScaleDefault() {
    return s.tempScaleTypeIndex;
}

void updateScale(uint16_t index) {
    tempScaleTypeIndexSet(index);
}

void showInfo(void) {
    char buff[63];
    uint8_t hwVerMajor, hwVerMinor;
    hwVerMajor = gSysInfo.hwVersion / 100;
    hwVerMinor = gSysInfo.hwVersion % 100;

    while(Button_GetState()){
        Display_Clear();

        Display_PutText(0, 0, "FW Ver", FONT_SMALL);
        Display_PutText(10, 10, GIT_VERSION, FONT_SMALL);

        Display_PutText(0, 25, "HW Ver", FONT_SMALL);
        siprintf(buff, "%d.%02d", hwVerMajor, hwVerMinor);
        Display_PutText(10, 35, buff, FONT_SMALL);

        Display_PutText(0, 50, "Display", FONT_SMALL);
        Display_PutText(10, 60, Display_GetType() == DISPLAY_SSD1327 ? "1327" : "1306", FONT_SMALL);

        Display_PutText(0, 75, "Uptime", FONT_SMALL);
        siprintf(buff, "%" PRIu32, uptime / 1000);
        Display_PutText(10,85, buff, FONT_SMALL);

        Display_Update();
    }
}

void reboot() {
    /* Unlock protected registers */
    SYS_UnlockReg();
    SYS_ResetChip();
}

void factoryReset() {
    defaultSettings();
    writeSettings();
    reboot();
}

#ifdef WITHFLASHDAMAGESUPPORT
void invalidateDataFlash() {
    if(!(Button_GetState() & BUTTON_MASK_RIGHT))
        return;

    makeDFInvalid();
}

void eraseDataFlash() {
    if(!(Button_GetState() & BUTTON_MASK_RIGHT))
        return;

    eraseDF();
}
#endif

void invertSet(uint8_t a){
	invertDisplaySet(a);

}

void flipSet(uint8_t a) {
	flipOnVapeSet(a);
}

struct menuItem displaySubMenuItems[] = {
	{
	    .type = SELECT,
	    .label = "Scale",
        .count = &tempScaleCount,
        .getDefaultCallback = &getScaleDefault,
	    .getValueCallback = &getScaleString,
	    .selectCallback = &updateScale,
	},
    {
        .type = EDIT,
        .label = "Brightness",
        .editMin = 0,
        .editMax = 255,
        .getEditStart = &getScreenBrightnessDefault,
        .editCallback = &updateScreenBrightness,
        .editStep = 10,
        .editFormat = &formatBrightnessNumber
    },
	{
	    .type = TOGGLE,
	    .label = "FlipVape",
	    .on = "On",
	    .off = "Off",
	    .isSet = &s.flipOnVape,
	    .toggleCallback = &flipSet,
	},
	{
	    .type = TOGGLE,
	    .label = "Invert",
	    .on = "On",
	    .off = "Off",
	    .isSet = &s.invertDisplay,
	    .toggleCallback = &invertSet,
	},
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .rows = 2,
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};

struct menuDefinition displaySettingsMenu = {
    .name = "Display Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &displaySubMenuItems,
};

void showModeSettings(struct menuItem *MI) {
	MI->subMenu = &(*g.vapeModes[s.mode]->vapeModeMenu);
}

int shouldHideMenu() {
	return g.vapeModes[s.mode]->vapeModeMenu == NULL;
}

struct menuItem advancedMenuItems[] = {
    {
        .type = ACTION,
        .label = "Reboot",
        .actionCallback = &reboot,
    },
    {
        .type = ACTION,
        .label = "F.Reset",
        .actionCallback = &factoryReset,
    },
#ifdef WITHFLASHDAMAGESUPPORT
    {
        .type = ACTION,
        .label = "Inv.Fla",
        .actionCallback = &invalidateDataFlash,
    },
    {
        .type = ACTION,
        .label = "Era.Fla",
        .actionCallback = &eraseDataFlash,
    },
#endif

    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .rows = 2,
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};


struct menuDefinition advancedMenu = {
    .name = "Advanced Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &advancedMenuItems,

};

/* Will always show 3 decimals, todo: make the '3' a param */
void formatFixedPoint(int32_t value, int32_t divisor, char *formatted) {
    if(divisor == 0)
        siprintf(formatted, "infin");
    else
        siprintf(formatted, "%"PRId32".%03"PRId32, value/divisor, value % divisor);
}

void formatThousandths(int32_t value, char *formatted) {
    formatFixedPoint(value, 1000, formatted);
}

void formatINT(int32_t value, char *formatted) {
    siprintf(formatted, "%"PRId32, value);
}

int32_t getTCRDefault() {
    return s.tcr;
}

int32_t getBaseTempDefault() {
    return s.baseTemp;
}

int32_t getBaseResDefault() {
    return s.baseRes;
}

void saveTCR(int32_t value) {
    if (value < 0) {
        /* don't set a default if it's invalid, somehow */
        return;
    }
    tcrSet(value & 0xFFFF);
}

void saveTemp(int32_t value) {
    baseFromUserSet(USERSET);
    baseTempSet(value & 0xFFFF);
}

void saveBaseRes(int32_t value) {
    baseFromUserSet(USERSET);
    baseResSet(value & 0xFFFF);
}

char *fromUserStrings[] = {
    "AutoRes",
    "UserSet",
    "UserLock",
};
uint8_t fromUserStringsCount = 3;

uint8_t getBaseFromUserDefault() {
    return s.baseFromUser;
}

char *getBaseFromUserString(uint8_t index) {
    return fromUserStrings[index];
}

void updateBaseFromUser(uint16_t index) {
    baseFromUserSet(index);
}

struct menuItem dragonMenuItems[] = {
    {
        .type = EDIT,
        .label = "TCR",
        .editMin = TCRMIN,
        .editMax = TCRMAX,
        .getEditStart = &getTCRDefault,
        .editStep = 1,
        .editFormat = &formatINT,
        .editCallback = &saveTCR,
    },
    {
        .type = SELECT,
        .label = "ResType",
        .count = &fromUserStringsCount,
        .getDefaultCallback = &getBaseFromUserDefault,
        .getValueCallback = &getBaseFromUserString,
        .selectCallback = &updateBaseFromUser,
    },
    {
        .type = EDIT,
        .label = "B.Temp",
        .editMin = BTEMPMIN,
        .editMax = BTEMPMAX,
        .getEditStart = &getBaseTempDefault,
        .editStep = 1,
        .editFormat = &formatINT,
        .editCallback = &saveTemp,
    },
    {
        .type = EDIT,
        .label = "B.Res",
        .editMin = 50,
        .editMax = 3450,
        .getEditStart = &getBaseResDefault,
        .editStep = 5,
        .editFormat = &formatThousandths,
        .editCallback = &saveBaseRes
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};

struct menuDefinition TheDragonning = {
    .name = "Dragon Mode On",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &dragonMenuItems,
};

void showAdvanced(struct menuItem *MI) {
    if (Button_GetState() & BUTTON_MASK_RIGHT) {
        MI->subMenu = &TheDragonning;
    } else {
        MI->subMenu = &advancedMenu;
    }
}


struct menuItem settingsMenuItems[] = {
    {
        .type = SELECT,
        .label = "Type",
        .count = &vapeMaterialsCount,
        .getDefaultCallback = &getTypeDefault,
        .getValueCallback = &getTypeString,
        .selectCallback = &updateType,
    },
    {
        .type = SELECT,
        .label = "Mode",
        .count = &g.modeCount,
        .getDefaultCallback = &getModeDefault,
        .getValueCallback = &getModeString,
        .selectCallback = &updateMode,
    },
	{
		.type = SUBMENU,
		.label = "Mode Settings",
		.getMenuDef = &showModeSettings,
		.hidden = &shouldHideMenu,
	},
	{
		.type = SUBMENU,
		.label = "Display",
		.subMenu = &displaySettingsMenu,
	},
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .rows = 2,
    },
    {
        .type = ACTION,
        .label = "Info",
        .actionCallback = &showInfo,
    },
    {
        .type = SUBMENU,
        .label = "Advnced",
        .getMenuDef = &showAdvanced,
    },
    {
        .type = EXITMENU,
        .label = "Exit",
    },
    {
        .type = END,
    }
};

struct menuDefinition settingsMenu = {
    .name = "Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &settingsMenuItems,
};

int load_settings(void) {
    s.fromRom = 0;
    // Should become part of globals instead of settings
    s.dumpPids = 0;
    readSettings();

    return 1;
}


void showMenu() {
    Display_SetContrast((char *) s.screenBrightness);
    runMenu(&settingsMenu);
    if (g.settingsChanged == 1) {
        writeSettings();
    }
}
