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
#include <Font.h>
#include <TimerUtils.h>
#include <Button.h>
#include <USB_VirtualCOM.h>
#include <Dataflash.h>

#include "button.h"
#include "display.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"

void saveDefaultSettings();

uint16_t selectorY = 0;
uint16_t selectorX = 0;
uint16_t toggleItem = 0;
uint16_t showItemToggle = 0;
volatile uint8_t currentItem = 0;
volatile uint8_t viewingInfo = 0;
const char *strings[] = { "one", "two", "three" };

uint8_t settings[100];

const char *headers[] =
    { "Type", "Mode", "Scale", "Reboot", "Exit", "Info" };
struct tempScale tempScaleType[] = {
    {
      .display = "C",
      .max = 320,
      .min = 0,
      .def = 200,
    },
    {
      .display = "F",
      .max = 600,
      .min = 0,
      .def = 400,
    },
    {
      .display = "K",
      .max = 880,
      .min = 280,
      .def = 680,
    },
};
#define SCALE_CNT 3

uint8_t ITEM_COUNT = 6;
// Array of selections
uint8_t items[6] = { 0, 28, 56, 90, 100, 110 };

int load_settings(void) {
    s.fromRom = 0;
    s.mode = 2;
    s.screenTimeout = 30;   // 100s of s
    s.materialIndex = 1;
    s.tempScaleTypeIndex = 1;
    s.displayTemperature = tempScaleType[s.tempScaleTypeIndex].def;
    s.targetTemperature = displayToC(s.displayTemperature);
    s.pidP = 17000;
    s.pidI = 5500;
    s.pidD = 0;
    s.initWatts = 15000;
    s.pidSwitch = 600;
    s.dumpPids = 0;
    return 1;
}

int8_t parseUInt32(char *V, const char *C, char *R, uint32_t M, uint32_t m, uint32_t *o) {
    char *endptr;
    char buff[63];
    errno = 0;
    uint32_t val32 = strtoul(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        siprintf(buff, "INFO,%s not valid %s\r\n", V, C);
        USB_VirtualCOM_SendString(buff);
        return 1;
    }
    
    *o = val32;
    R[0] = '$';
    return 0;
}

int8_t parseInt32(char *V, const char *C, char *R, int32_t M, int32_t m, int32_t *o) {
    char *endptr;
    char buff[63];
    errno = 0;
    int32_t val32 = strtol(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        siprintf(buff, "INFO,%s not valid %s\r\n", V, C);
        USB_VirtualCOM_SendString(buff);
        return 1;
    }
    
    *o = val32;
    R[0] = '$';
    return 0;
}

int8_t parseUInt16(char *V, const char *C, char *R, uint16_t M, uint16_t m, uint16_t *o) {
    uint32_t val32;
    if (parseUInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFFFF;
    return 0;
}

int8_t parseInt16(char *V, const char *C, char *R, int16_t M, int16_t m, int16_t *o) {
    int32_t val32;
    if (parseInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFFFF;
    return 0;
}

int8_t parseUInt8(char *V, const char *C, char *R, uint8_t M, uint8_t m, uint8_t *o) {
    uint32_t val32;
    if (parseUInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFF;
    return 0;
}

int8_t parseInt8(char *V, const char *C, char *R, int8_t M, int8_t m, int8_t *o) {
    int32_t val32;
    if (parseInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFF;
    return 0;
}

void updateSettings(char *buffer, char *response) {
    char buff[63];
    char *setting;
    char *value;
    const char delim = ',';

    strtok(buffer, &delim); // eat the 'S'
    setting = strtok(NULL, &delim);
    value = strtok(NULL, &delim);
    if (!setting || !value) {
        response[0] = '~';
        return;
    }

    if (strncmp(setting,"mode",4) == 0) {
        if (parseUInt8(value, "mode", response, MAX_CONTROL, 0, &s.mode))
            return;
    } else if (strncmp(setting, "screenTimeout", 13) == 0) {
        if (parseUInt16(value, "screenTimeout", response, 600, 0, &s.screenTimeout))
            return;
    } else if (strncmp(setting, "targetTemperature", 17) == 0) {
        struct tempScale *t = &tempScaleType[s.tempScaleTypeIndex];
        uint32_t ttemp;
        if (parseUInt32(value, "targetTemperature", response, t->max, t->min, &ttemp))
            return;
        s.displayTemperature = ttemp;
        s.targetTemperature = displayToC(s.displayTemperature);
    } else if (strncmp(setting, "materialIndex", 13) == 0) {
        uint8_t tindex = 0;
        if (parseUInt8(value, "materialIndex", response, MATERIAL_COUNT, 0, &tindex))
            return;
        setVapeMaterial(tindex);
    } else if (strncmp(setting, "tempScaleTypeIndex", 13) == 0) {
        uint8_t tindex;
        if (parseUInt8(value, "tempScaleTypeIndex", response, SCALE_CNT - 1, 0, &tindex))
            return;
        s.tempScaleTypeIndex = tindex;
        s.targetTemperature = tempScaleType[s.tempScaleTypeIndex].def;
    } else if (strncmp(setting, "pidP", 4) == 0) {
        if (parseUInt32(value, "pidP", response, 0xFFFFFFFF, 0, &s.pidP))
            return;
    } else if (strncmp(setting, "pidI", 4) == 0) {
        if (parseUInt32(value, "pidI", response, 0xFFFFFFFF, 0, &s.pidI))
            return;
    } else if (strncmp(setting, "pidD", 4) == 0) {
        if (parseUInt32(value, "pidD", response, 0xFFFFFFFF, 0, &s.pidD))
            return;
    } else if (strncmp(setting, "initWatts", 9) == 0) {
        if (parseInt32(value, "initWatts", response, 60000, 0, &s.initWatts))
            return;
    } else if (strncmp(setting, "pidSwitch", 8) == 0) {
        if (parseInt32(value, "pidSwitch", response, 600, -600, &s.pidSwitch))
            return;
    } else if (strncmp(setting, "dumpPids", 8) == 0) {
        if (parseUInt8(value, "dumpPids", response, 1, 0, &s.dumpPids))
            return;
    }

    if (response[0] == '$') {
        siprintf(buff, "INFO,setting %s to %s\r\n", setting, value);
        USB_VirtualCOM_SendString(buff);
    }
}


void dumpSettings(char *buffer, char *response) {
    char buff[63];
    USB_VirtualCOM_SendString("INFO,dumpSettings\r\n");
    siprintf(buff, "setting,%s,%i\r\n","fromRom",s.fromRom);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","mode",s.mode);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","screenTimeout",s.screenTimeout);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","targetTemperature",s.targetTemperature);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","materialIndex",s.materialIndex);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","tempScaleTypeIndex",s.tempScaleTypeIndex);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidP",s.pidP);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidI",s.pidI);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidD",s.pidD);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","initWatts",s.initWatts);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","dumpPids",s.dumpPids);
    USB_VirtualCOM_SendString(buff);
}

void updateAtomizer(char *buffer, char *response) {
    char buff[63];
    char *setting;
    char *value;
    const char delim = ',';

    strtok(buffer, &delim); // eat the 'S'
    setting = strtok(NULL, &delim);
    value = strtok(NULL, &delim);
    if (!setting || !value) {
        response[0] = '~';
        return;
    }

    if (strncmp(setting,"base_resistance",15) == 0) {
        if (parseUInt16(value, "base_resistance", response, 3500, 0, &g.atomInfo.base_resistance))
            return;
    } else if (strncmp(setting,"base_temperature", 16) == 0) {
        if (parseUInt32(value, "base_temperature", response, 200, 0, &g.atomInfo.base_temperature))
            return;
    } else if (strncmp(setting,"tcr", 3) == 0) {
        if (parseUInt16(value, "tcr", response, 1000, 10, &g.atomInfo.tcr))
            return;
    }
    if (response[0] == '$') {
        siprintf(buff, "INFO,setting atomInfo.%s to %s\r\n", setting, value);
        USB_VirtualCOM_SendString(buff);
    }
}

void dumpAtomizer(char *buffer, char *response) {
    char buff[63];
    Atomizer_Info_t *a = &g.atomInfo;
    USB_VirtualCOM_SendString("INFO,dumpSettings\r\n");
    siprintf(buff, "atomInfo,%s,%i\r\n","voltage",a->voltage);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","resistance",a->resistance);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","base_resistance",a->base_resistance);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%ld\r\n","base_temperature",a->base_temperature);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","current",a->current);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%ld\r\n","temperature",a->temperature);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","tcr",a->tcr);
    USB_VirtualCOM_SendString(buff);
 
}

void reboot() {
    /* Unlock protected registers */
    SYS_UnlockReg();
    SYS_ResetChip();
}

inline void getMenuToggle(char *buff, char *data) {
    siprintf(buff, "%s", data);
}

inline void getSelector(char *buff) {
    siprintf(buff, "*");
}

void
printSettingsItem(uint8_t starting, char *buff, char *header,
          char *string) {

    getString(buff, header);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);

    getString(buff, string);
    Display_PutText(15, starting + 12, buff, FONT_DEJAVU_8PT);
}

void
printInfoItem(uint8_t starting, char *buff, char *header,
          char *string) {

    getString(buff, header);
    Display_PutText(0, starting, buff, FONT_DEJAVU_8PT);

    getString(buff, string);
    Display_PutText(10, starting + 15, buff, FONT_DEJAVU_8PT);
}

void printHeader(uint8_t starting, char *buff, char *text) {
    getString(buff, text);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);
}

void printHWVersion(uint8_t starting, char *buff) {
    getString(buff, "HW Ver");
    Display_PutText(0, starting, buff, FONT_DEJAVU_8PT);

    uint8_t hwVerMajor, hwVerMinor;
    hwVerMajor = 0; //Dataflash_info.hwVersion / 100;
    hwVerMinor = 0; //Dataflash_info.hwVersion % 100;
    siprintf(buff, "%d.%02d", hwVerMajor, hwVerMinor);
    Display_PutText(10, starting + 15, buff, FONT_DEJAVU_8PT);
}

void buildMenu() {
    char buff[8];

    Display_Clear();

    if (viewingInfo) {
        // TODO: read this value from a datastruct
        printInfoItem(0, buff, "FW Ver", "-0.01");
        printHWVersion(40, buff);
        printInfoItem(80, buff, "Display",
                Display_GetType() == DISPLAY_SSD1327 ? "1327" : "1306");
    } else {

    Display_PutLine(0, 80, 63, 80);

    getSelector(buff);
    Display_PutText(0, items[currentItem], buff, FONT_DEJAVU_8PT);

    printSettingsItem(0, buff, (char *) headers[0],
              vapeMaterialList[s.materialIndex].name);
    printSettingsItem(28, buff, (char *) headers[1],
              g.vapeModes[s.mode]->name);
    printSettingsItem(56, buff, (char *) headers[2],
              (char *) tempScaleType[s.tempScaleTypeIndex].display);

    // Print reboot and standard stuff
    printHeader(90, buff, (char *) headers[5]);
    printHeader(100, buff, (char *) headers[3]);
    printHeader(110, buff, (char *) headers[4]);

    }

    Display_Update();
}

void buttonSettingRight(uint8_t state, uint32_t time) {
    if (gv.fireButtonPressed)
        return;

    if (state == BUTTON_REL) {
        if (currentItem + 1 > ITEM_COUNT - 1) {
            currentItem = 0;
        } else {
            currentItem++;
        }
    }
}

void buttonSettingLeft(uint8_t state, uint32_t time) {
    if (gv.fireButtonPressed)
        return;
    if (state == BUTTON_REL) {
        if (currentItem - 1 < 0) {
            currentItem = ITEM_COUNT - 1;
        } else {
            currentItem--;
        }
    }
}

void handleFireButton();
void buttonSettingFire(uint8_t state, uint32_t time) {
    // To things
    if (state == BUTTON_PRESS)
        handleFireButton();
    else {
        viewingInfo = 0;

    }

}

struct buttonHandler settingsButtons = {
    .name = "settingsButtons",

    .fire_handler = &buttonSettingFire,
    .left_handler = &buttonSettingLeft,
    .right_handler = &buttonSettingRight,
};

void registerSettingsButtons() {
    switchHandler(&settingsButtons);
}

void deregisterSettingsButtons() {
    returnHandler();
}



void handleFireButton() {
    
    switch (currentItem) {
    case 0:
    if (s.materialIndex == 3) {
        s.materialIndex = 0;
    } else {
        s.materialIndex++;
    }
    setVapeMaterial(s.materialIndex);
    break;
    case 1:
    s.mode++;
    if (!g.vapeModes[s.mode]) {
        s.mode = 0;
    }
    if (!
        (vapeMaterialList[s.materialIndex].typeMask & g.
         vapeModes[s.mode]->supportedMaterials)) {
        s.mode++;
    }
    if (!g.vapeModes[s.mode]) {
        s.mode = 0;
    }
    setVapeMode(s.mode);
    break;
    case 2:
    if (s.tempScaleTypeIndex == 2) {
        s.tempScaleTypeIndex = 0;
    } else {
        s.tempScaleTypeIndex++;
    }
    break;
    case 3:
    if (!viewingInfo) {
        viewingInfo = 1;
    }
    break;
    case 4:
    reboot();
    break;
    case 5:
    
    deregisterSettingsButtons();
    updateScreen(&g);
    gv.buttonCnt = 0;
    gv.shouldShowMenu = 0;
    currentItem = 0;
    return;
    break;
    }
}

void showMenu() {
    gv.buttonCnt = 0;
    gv.shouldShowMenu = 1;
    registerSettingsButtons();
    
    while (gv.shouldShowMenu) {
        if (gv.buttonEvent) {
            handleButtonEvents();
            gv.buttonEvent = 0;
        }
        buildMenu();
    }
}
