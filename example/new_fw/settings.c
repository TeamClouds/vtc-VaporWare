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
uint8_t currentItem = 0;
const char *strings[] = {"one","two","three"};
uint8_t settings[100];

const char *headers[] = {"Type", "Mode", "Exit"};

uint8_t ITEM_COUNT = 4;
// Array of selections
 uint8_t items[4] = { 0, 20, 100, 110};

void setupButtons();

int load_settings(void) {
    s.mode = 2;
    s.screenTimeout = 30; // 100s of s
    s.materialIndex = 1;
    s.material = &vapeMaterialList[s.materialIndex];
    return 1;
}

inline void getMenuToggle(char *buff, char *data) {
    siprintf(buff, "%s", data);
}
inline void getSelector(char *buff) {
     siprintf(buff, "*");
}

void disableButtons() {
    gv.buttonCnt = 0;
    Button_DeleteCallback(g.fire);
    Button_DeleteCallback(g.plus);
    Button_DeleteCallback(g.minus);
}

void printSettingsItem(uint8_t starting, char *buff, char *header, char *string) {

    getString(buff, header);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);

    getString(buff, string);
    Display_PutText(15, starting+10, buff, FONT_DEJAVU_8PT);
}


void printHeader(uint8_t starting, char *buff, char *text) {
    getString(buff, text);
    Display_PutText(10, starting, buff, FONT_DEJAVU_8PT);
}

void buildMenu() {
    char buff[8];

    Display_Clear();

    Display_PutLine(0, 90, 63, 90);


    getSelector(buff);
    Display_PutText(0, items[currentItem], buff, FONT_DEJAVU_8PT);



    printSettingsItem(0, buff, headers[0], s.material->name);
    printSettingsItem(20, buff, headers[1],  g.vapeModes[s.mode]->name);

    printHeader(100, buff, "Reboot");
    printHeader(110, buff, "Exit");

    Display_Update();
}

void buttonSettingFire(uint8_t state) {
   // To things
   if (state & BUTTON_MASK_FIRE) {
       if(Button_GetState() & BUTTON_MASK_FIRE) {
    	switch(currentItem) {
    	case 0:
       		if (s.materialIndex == 3) {
       			s.materialIndex = 0;
       		} else {
       			s.materialIndex++;
       		}
       	    setVapeMaterial(&vapeMaterialList[s.materialIndex]);
    		break;
    	case 1:
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
    		break;
    	case 2:
       		reboot();
    		break;
    	case 3:
            disableButtons();
            setupButtons();
            updateScreen(&g);
            gv.buttonCnt = 0;
            gv.shouldShowMenu = 0;
            return;
    		break;
    	}
        buildMenu();
        }
   }
}

void reboot() {
    /* Unlock protected registers */
    SYS_UnlockReg();
    SYS_ResetChip();
}

void buttonSettingRight(uint8_t state) {
   if (!(state & BUTTON_MASK_RIGHT)) {
        if (currentItem+1 > ITEM_COUNT-1) {
            currentItem = 0;
        } else {
            currentItem++;
        }
        buildMenu();
    }
}

void buttonSettingLeft(uint8_t state) {
   if (!(state & BUTTON_MASK_LEFT)) {
        if (currentItem-1 < 0) {
            currentItem = ITEM_COUNT-1;
        } else {
            currentItem--;
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
	gv.buttonCnt = 0;
    disableButtons();
    setupSettingsButtons();
    buildMenu();
}
