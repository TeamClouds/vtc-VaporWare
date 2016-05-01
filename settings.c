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
#include "display.h"
#include "globals.h"
#include "helper.h"
#include "menu.h"
#include "settings.h"
#include "font/font_vaporware.h"

void saveDefaultSettings();

#define MAXOPTIONS 16

char *typeIdString[MAXOPTIONS];
int typeIdMapping[MAXOPTIONS];

void populateTypes(struct menuItem *MI) {
    uint8_t index = 0;
    struct vapeMaterials *VM;
    while ((VM = &vapeMaterialList[index])->name[0] != '\0') {
        typeIdString[index] = VM->name;
        typeIdMapping[index] = index;
        index++;
    }
    MI->items = &typeIdString;
    MI->count = index;
    MI->startAt = s.materialIndex;
}

void updateType(uint16_t index) {
    setVapeMaterial(index);
}

char *modeIdString[MAXOPTIONS];
int modeIdMapping[MAXOPTIONS];

void populateModes(struct menuItem *MI) {
    uint8_t index = 0;
    struct vapeMode *VPM;
    while ((VPM = g.vapeModes[index])->name[0] != '\0') {
        modeIdString[index] = VPM->name;
        modeIdMapping[index] = VPM->index;
        index++;
    }
    MI->items = &modeIdString;
    MI->count = index;
    MI->startAt = s.mode;
}

void updateMode(uint16_t index) {
    setVapeMode(index);
}

char *scaleIdString[MAXOPTIONS];
int scaleIdMapping[MAXOPTIONS];

void populateScales(struct menuItem *MI) {
    uint8_t index = 0;
    struct tempScale *TS;
    index = 0;
    while ((TS = &tempScaleType[index])->display[0] != '\0') {
        scaleIdString[index] = TS->display;
        scaleIdMapping[index] = index;
        index++;
    }
    MI->items = &scaleIdString;
    MI->count = index;
    MI->startAt = s.tempScaleTypeIndex;
}

void updateScale(uint16_t index) {
    s.tempScaleTypeIndex = index;
}

void showInfo(void) {
    char buff[63];
    uint8_t hwVerMajor, hwVerMinor;
    hwVerMajor = gSysInfo.hwVersion / 100;
    hwVerMinor = gSysInfo.hwVersion % 100;
    
    Display_Clear();

    Display_PutText(0, 0, "FW Ver", FONT_SMALL);
    siprintf(buff, "%s", "-0.01");
    Display_PutText(10, 15, buff, FONT_SMALL);
    
    Display_PutText(0, 40, "HW Ver", FONT_SMALL);
    siprintf(buff, "%d.%02d", hwVerMajor, hwVerMinor);
    Display_PutText(10, 55, buff, FONT_SMALL);
    
    Display_PutText(0, 80, "Display", FONT_DEJAVU_8PT);
    Display_PutText(10, 95, Display_GetType() == DISPLAY_SSD1327 ? "1327" : "1306", FONT_SMALL);

    Display_Update();

    while(Button_GetState()){;}
}

void reboot() {
    /* Unlock protected registers */
    SYS_UnlockReg();
    SYS_ResetChip();
}

void factoryReset() {
    defaultSettings();
}

void invertSet(uint8_t a){
	s.invertDisplay = a;
    Display_SetInverted(s.invertDisplay);
}

void flipSet(uint8_t a) {
	s.flipOnVape = a;
}

struct menuItem displaySubMenuItems[] = {
	{
	    .type = SELECT,
	    .label = "Scale",
	    /* .items = assigned before calling */
	    /* .startAt assinged before calling */
	    /* .count = assigned before calling */
	    .populateCallback = &populateScales,
	    .selectCallback = &updateScale,
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
        .type = EXITMENU,
        .label = "Exit",
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

int shouldShowMenu() {
	return false;
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

    {
        .type = STARTBOTTOM,
    },
    {
        .type = EXITMENU,
        .label = "Exit",
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

struct menuItem settingsMenuItems[] = {
    {
        .type = SELECT,
        .label = "Type",
        /* .items = assigned before calling */
        /* .startAt assinged before calling */
        /* .count = assigned before calling */
        .populateCallback = &populateTypes,
        .selectCallback = &updateType,
    },
    {
        .type = SELECT,
        .label = "Mode",
        /* .items = assigned before calling */
        /* .startAt assinged before calling */
        /* .count = assigned before calling */
        .populateCallback = &populateModes,
        .selectCallback = &updateMode,
    },
	{
		.type = SUBMENU,
		.label = "Mode Settings",
		.getMenuDef = &showModeSettings,
	},
	{
		.type = SUBMENU,
		.label = "Display",
		.subMenu = &displaySettingsMenu,
		.hidden = &shouldShowMenu,
	},
    {
        .type = SPACE,
        .rows = 1,
    },
    {
        .type = LINE,
    },
    {
        .type = STARTBOTTOM,
    },
    {
        .type = ACTION,
        .label = "Info",
        .actionCallback = &showInfo,
    },
    {
        .type = SUBMENU,
        .label = "Advanced",
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
    s.flipOnVape = 0;
    s.invertDisplay = 0;
    return 1;
}


void showMenu() {
    runMenu(&settingsMenu);
    writeSettings();
}
