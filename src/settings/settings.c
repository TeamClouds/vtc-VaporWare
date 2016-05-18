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
#include "display_helper.h"
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

uint8_t getModeDefault() {
    return s.mode;
}

uint8_t getScaleDefault() {
    return s.tempScaleTypeIndex;
}

uint8_t getBaseFromUserDefault() {
    return s.baseFromUser;
}

int32_t getScreenBrightnessDefault() {
    return s.screenBrightness;
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

char *getTypeString(uint8_t index) {
    return vapeMaterialList[index].name;
}

void updateType(uint16_t index) {
    materialIndexSet(index);
    refreshMenu();
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

void formatBrightnessNumber(char *formatted, int32_t value) {
    Display_SetContrast(value & 0xFF);
    printNumber(formatted, value);
}

void updateScreenBrightness(int32_t value) {
	screenBrightnessSet(value);
}

char *getScaleString(uint8_t index) {
    return tempScaleType[index].display;
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

const struct menuDefinition *const showModeSettings(const struct menuItem *MI) {
	return &(*g.vapeModes[s.mode]->vapeModeMenu);
}

int shouldHideMenu() {
	return g.vapeModes[s.mode]->vapeModeMenu == NULL;
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
    "Auto",
    "Set",
    "Lock",
};
uint8_t fromUserStringsCount = 3;

char *getBaseFromUserString(uint8_t index) {
    return fromUserStrings[index];
}

void updateBaseFromUser(uint16_t index) {
    baseFromUserSet(index);
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

const struct menuItem dragonMenuItems[] = {
    {
        .type = EDIT,
        .label = "TCR",
        .editMin = TCRMIN,
        .editMax = TCRMAX,
        .getEditStart = &getTCRDefault,
        .editStep = 1,
        .editFormat = &printNumber,
        .editCallback = &saveTCR,
    },
    {
        .type = EDIT,
        .label = "B.Temp",
        .editMin = BTEMPMIN,
        .editMax = BTEMPMAX,
        .getEditStart = &getBaseTempDefault,
        .editStep = 1,
        .editFormat = &printNumber,
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

const struct menuDefinition TheDragonning = {
    .name = "Dragons",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &dragonMenuItems,
};

const struct menuItem coilMenuItems[] = {
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
        .label = "Res.",
        .count = &fromUserStringsCount,
        .getDefaultCallback = &getBaseFromUserDefault,
        .getValueCallback = &getBaseFromUserString,
        .selectCallback = &updateBaseFromUser,
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
        .type = SUBMENU,
        .label = "Dragons",
        .subMenu = &TheDragonning,
    },
    {
        .type = EXITMENU,
        .label = "Back",
    },
    {
        .type = END,
    }
};


const struct menuItem modeMenuItems[] = {
    {
        .type = SELECT,
        .label = "Mode",
        .count = &g.modeCount,
        .getDefaultCallback = &getModeDefault,
        .getValueCallback = &getModeString,
        .selectCallback = &updateMode,
    },
    {
        .type = STARTBOTTOM,
    },
	{
		.type = SUBMENU,
		.label = "Options",
		.getMenuDef = &showModeSettings,
		.hidden = &shouldHideMenu,
	},
    {
        .type = SPACE,
        .rows = 2,
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

const struct menuItem advancedMenuItems[] = {
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

const struct menuItem displaySubMenuItems[] = {
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
	    .toggleCallback = &flipOnVapeSet,
	},
	{
	    .type = TOGGLE,
	    .label = "Invert",
	    .on = "On",
	    .off = "Off",
	    .isSet = &s.invertDisplay,
	    .toggleCallback = &invertDisplaySet,
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

const struct menuDefinition displaySettingsMenu = {
    .name = "Display Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &displaySubMenuItems,
};

const struct menuDefinition modeMenu = {
    .name = "Mode",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &modeMenuItems,
};

const struct menuDefinition coilMenu = {
    .name = "Coil",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &coilMenuItems,
};

const struct menuDefinition advancedMenu = {
    .name = "Advanced Settings",
    .font = FONT_SMALL,
    .cursor = "*",
    .prev_sel = "<",
    .next_sel = ">",
    .less_sel = "-",
    .more_sel = "+",
    .menuItems = &advancedMenuItems,
};

const struct menuItem settingsMenuItems[] = {
	{
		.type = SUBMENU,
		.label = "Coil",
		.subMenu = &coilMenu,
	},
	{
		.type = SUBMENU,
		.label = "Mode",
		.subMenu = &modeMenu,
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
        .subMenu = &advancedMenu,
    },
    {
        .type = EXITMENU,
        .label = "Exit",
    },
    {
        .type = END,
    }
};

const struct menuDefinition settingsMenu = {
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
    Display_SetContrast(s.screenBrightness);
    runMenu(&settingsMenu);
    if (g.settingsChanged == 1) {
        writeSettings();
    }
}
