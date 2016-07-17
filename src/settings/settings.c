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
#include <menu.h>

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

#include "vaptris.h"

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

void formatBrightnessNumber(char *formatted, uint8_t len, int32_t value) {
    Display_SetContrast(value & 0xFF);
    printNumber(formatted, len, value);
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
    char *buff;
    uint8_t hwVerMajor, hwVerMinor;
    hwVerMajor = gSysInfo.hwVersion / 100;
    hwVerMinor = gSysInfo.hwVersion % 100;

    while(Button_GetState()){
        Display_Clear();

        Display_PutText(0, 0, "FW Ver", FONT_SMALL);
        Display_PutText(10, 10, GIT_VERSION, FONT_SMALL);

        Display_PutText(0, 25, "HW Ver", FONT_SMALL);
        asiprintf(&buff, "%d.%02d", hwVerMajor, hwVerMinor);
        Display_PutText(10, 35, buff, FONT_SMALL);
        free(buff);

        Display_PutText(0, 50, "Display", FONT_SMALL);
        Display_PutText(10, 60, Display_GetType() == DISPLAY_SSD1327 ? "1327" : "1306", FONT_SMALL);

        Display_PutText(0, 75, "Uptime", FONT_SMALL);

        asiprintf(&buff, "%" PRIu32, uptime / 1000);
        Display_PutText(10,85, buff, FONT_SMALL);
        free(buff);

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

void spacinVaper(void) {
    Display_SetOn(1);
    gv.spacinVaper = 1;
    exitMenu();
}

void runVaptris(void) {
    runvaptris();
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

void forceCrash() {
    *((volatile uint8_t *) 0x2000FFFF) = 0;
}
#endif

void invertSet(uint8_t a){
	invertDisplaySet(a);

}

void flipSet(uint8_t a) {
	flipOnVapeSet(a);
}

void bordersSet(uint8_t a) {
    g.showBorders = a;
}

const struct menuItem dragonMenuItems[] = {
    {
        .type = EDIT,
        .label = "TCR",
        .Item.edit.editMin = TCRMIN,
        .Item.edit.editMax = TCRMAX,
        .Item.edit.getEditStart = &getTCRDefault,
        .Item.edit.editStep = 1,
        .Item.edit.editFormat = &printNumber,
        .Item.edit.editCallback = &saveTCR,
    },
    {
        .type = EDIT,
        .label = "B.Temp",
        .Item.edit.editMin = BTEMPMIN,
        .Item.edit.editMax = BTEMPMAX,
        .Item.edit.getEditStart = &getBaseTempDefault,
        .Item.edit.editStep = 1,
        .Item.edit.editFormat = &printNumber,
        .Item.edit.editCallback = &saveTemp,
    },
    {
        .type = EDIT,
        .label = "B.Res",
        .Item.edit.editMin = 50,
        .Item.edit.editMax = 3450,
        .Item.edit.getEditStart = &getBaseResDefault,
        .Item.edit.editStep = 5,
        .Item.edit.editFormat = &formatThousandths,
        .Item.edit.editCallback = &saveBaseRes
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .Item.space.rows = 2,
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
        .Item.select.count = &vapeMaterialsCount,
        .Item.select.getDefaultCallback = &getTypeDefault,
        .Item.select.getValueCallback = &getTypeString,
        .Item.select.selectCallback = &updateType,
    },
    {
        .type = SELECT,
        .label = "Res.",
        .Item.select.count = &fromUserStringsCount,
        .Item.select.getDefaultCallback = &getBaseFromUserDefault,
        .Item.select.getValueCallback = &getBaseFromUserString,
        .Item.select.selectCallback = &updateBaseFromUser,
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .Item.space.rows = 2,
    },
    {
        .type = SUBMENU,
        .label = "Dragons",
        .Item.submenu.subMenu = &TheDragonning,
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
        .Item.select.count = &g.modeCount,
        .Item.select.getDefaultCallback = &getModeDefault,
        .Item.select.getValueCallback = &getModeString,
        .Item.select.selectCallback = &updateMode,
    },
    {
        .type = STARTBOTTOM,
    },
	{
		.type = SUBMENU,
		.label = "Options",
		.Item.submenu.getMenuDef = &showModeSettings,
		.hidden = &shouldHideMenu,
	},
    {
        .type = SPACE,
        .Item.space.rows = 2,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .Item.space.rows = 2,
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
        .Item.action.actionCallback = &reboot,
    },
    {
        .type = ACTION,
        .label = "F.Reset",
        .Item.action.actionCallback = &factoryReset,
    },
#ifdef WITHFLASHDAMAGESUPPORT
    {
        .type = ACTION,
        .label = "Inv.Fla",
        .Item.action.actionCallback = &invalidateDataFlash,
    },
    {
        .type = ACTION,
        .label = "Era.Fla",
        .Item.action.actionCallback = &eraseDataFlash,
    },
    {
        .type = ACTION,
        .label = "Crash",
        .Item.action.actionCallback = &forceCrash,
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
        .Item.space.rows = 2,
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
        .Item.select.count = &tempScaleCount,
        .Item.select.getDefaultCallback = &getScaleDefault,
	    .Item.select.getValueCallback = &getScaleString,
	    .Item.select.selectCallback = &updateScale,
	},
    {
        .type = EDIT,
        .label = "Brightness",
        .Item.edit.editMin = 0,
        .Item.edit.editMax = 255,
        .Item.edit.getEditStart = &getScreenBrightnessDefault,
        .Item.edit.editCallback = &updateScreenBrightness,
        .Item.edit.editStep = 10,
        .Item.edit.editFormat = &formatBrightnessNumber
    },
	{
	    .type = TOGGLE,
	    .label = "FlipVape",
	    .Item.toggle.on = "On",
	    .Item.toggle.off = "Off",
	    .Item.toggle.isSet = &s.flipOnVape,
	    .Item.toggle.toggleCallback = &flipOnVapeSet,
	},
	{
	    .type = TOGGLE,
	    .label = "Invert",
	    .Item.toggle.on = "On",
	    .Item.toggle.off = "Off",
	    .Item.toggle.isSet = &s.invertDisplay,
	    .Item.toggle.toggleCallback = &invertDisplaySet,
	},
    {
        .type = TOGGLE,
        .label = "Borders",
        .Item.toggle.on = "On",
        .Item.toggle.off = "Off",
        .Item.toggle.isSet = &g.showBorders,
        .Item.toggle.toggleCallback = &bordersSet,
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .Item.space.rows = 2,
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
		.Item.submenu.subMenu = &coilMenu,
	},
	{
		.type = SUBMENU,
		.label = "Mode",
		.Item.submenu.subMenu = &modeMenu,
	},
	{
		.type = SUBMENU,
		.label = "Display",
		.Item.submenu.subMenu = &displaySettingsMenu,
	},
    {
        .type = SPACE,
        .Item.space.rows = 10,
    },
    {
        .type = ACTION,
        .label = "Space",
        .Item.action.actionCallback = &spacinVaper,
    },
    {
        .type = ACTION,
        .label ="Vaptris",
        .Item.action.actionCallback = &runVaptris,
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = LINE,
    },
    {
        .type = SPACE,
        .Item.space.rows = 2,
    },
    {
        .type = ACTION,
        .label = "Info",
        .Item.action.actionCallback = &showInfo,
    },
    {
        .type = SUBMENU,
        .label = "Advnced",
        .Item.submenu.subMenu = &advancedMenu,
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
