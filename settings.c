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
#include <M451Series.h>
#include <Display.h>
#include <Font.h>
#include <TimerUtils.h>
#include <Button.h>
#include "main.h"

uint16_t selectorY = 0;
uint16_t selectorX = 0;
uint16_t toggleItem = 0;
uint16_t showItemToggle = 0;
const char *strings[] = {"one","two","three"};
uint8_t settings[100];


const char *headers[] = {"Type", "Mode", "Exit"};


void setupButtons();

int load_settings(void) {
    s.mode = 2;
    s.materialIndex = 1;
    s.material = &vapeMaterialList[s.materialIndex];
    return 1;
}

inline void getMenuDumbText(char *buff) {
    siprintf(buff, "Herro");
}

inline void getMenuToggle(char *buff, char *data) {
    siprintf(buff, "%s", data);
}
inline void getSelector(char *buff) {
     siprintf(buff, "*");
}

void disableButtons() {
    g.buttonCnt = 0;
    Button_DeleteCallback(g.fire);
    Button_DeleteCallback(g.plus);
    Button_DeleteCallback(g.minus);
}

void buildHeaderAndChildren(uint8_t starting, char *buff) {
    getMenuDumbText(buff);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);

    char *data;
    uint8_t index = settings[starting];
    data = strings[index];

    getString(buff, data);
    Display_PutText(15, starting+10, buff, FONT_DEJAVU_8PT);
}

void printSettingsItem(uint8_t starting, char *buff, char *header, char *string) {
    getString(buff, header);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);

    getString(buff, string);
    Display_PutText(15, starting+10, buff, FONT_DEJAVU_8PT);
}


void buildExit(uint8_t starting, char *buff) {
    getString(buff, "EXIT");
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);
}

void buildMenu() {
    char buff[8];

    Display_Clear();
    getSelector(buff);
    Display_PutText(0, selectorY, buff, FONT_DEJAVU_8PT);

    printSettingsItem(0, buff, headers[0], s.material->name);
    printSettingsItem(20, buff, headers[1],  g.vapeModes[s.mode]->name);

    buildExit(100, buff);

    Display_Update();
}

void buttonSettingFire(uint8_t state) {
   // To things
   if (state & BUTTON_MASK_FIRE) {
       if(Button_GetState() & BUTTON_MASK_FIRE) {
       	if (selectorY == 0) {
       		if (s.materialIndex == 3) {
       			s.materialIndex = 0;
       		} else {
       			s.materialIndex++;
       		}
       	    setVapeMaterial(&vapeMaterialList[s.materialIndex]);
       	} else if (selectorY == 20) {
       		s.mode++;
       		if (!g.vapeModes[s.mode]) {
       			s.mode = 0;
       		}
       		if (!(s.material->typeMask & g.vapeModes[s.mode]->supportedMaterials)) {
       			s.mode++;
       		}
       		if (!g.vapeModes[s.mode]) {
       			s.mode = 0;
       		}
       		setVapeMode(s.mode);
       	} else if (selectorY == 100) {
               disableButtons();
               setupButtons();
               updateScreen(&g);
               g.buttonCnt = 0;
               return;
       	}
        buildMenu();
        }
   }
}

void buttonSettingRight(uint8_t state) {
   if (state & BUTTON_MASK_RIGHT) {
        if (selectorY+20 > 100) {
            selectorY = 0;
        } else {
            selectorY += 20;
        }
        buildMenu();
    }
}

void buttonSettingLeft(uint8_t state) {
   if (state & BUTTON_MASK_LEFT) {
        if (selectorY-20 < 0) {
            selectorY = 100;
        } else {
            selectorY -= 20;
        }
        buildMenu();
    }
}

void setupSettingsButtons() {
    g.fire = Button_CreateCallback(buttonSettingFire, BUTTON_MASK_FIRE);
    g.plus = Button_CreateCallback(buttonSettingRight, BUTTON_MASK_RIGHT);
    g.minus = Button_CreateCallback(buttonSettingLeft, BUTTON_MASK_LEFT);
}

void showMenu() {
	g.buttonCnt = 0;
    disableButtons();
    setupSettingsButtons();
    buildMenu();
}
