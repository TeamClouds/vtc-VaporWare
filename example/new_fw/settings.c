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

void setupButtons();

int load_settings(void) {
    s.mode = 0;
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

void buildMenu() {
    char buff[8];

    Display_Clear();
    getSelector(buff);
    Display_PutText(0, selectorY, buff, FONT_DEJAVU_8PT);

    buildHeaderAndChildren(0, buff);
    buildHeaderAndChildren(20, buff);
    buildHeaderAndChildren(40, buff);
    buildHeaderAndChildren(60, buff);
    buildHeaderAndChildren(80, buff);
    buildHeaderAndChildren(100, buff);

    Display_Update();
}

void exitSettings(uint32_t counterIndex) {
    if(g.buttonCnt < 5) {
        if(Button_GetState() & BUTTON_MASK_FIRE) {
            // TODO change things
            uint8_t currIndex = settings[selectorY];
            if (selectorX == 2) {
                currIndex = 0;
            } else {
                currIndex++;
            }
            settings[selectorY] = currIndex;
            buildMenu();
         }
    } else {
        disableButtons();
        setupButtons();
        updateScreen(&g);
        g.buttonCnt = 0;
    }
}

void buttonSettingFire(uint8_t state) {
   // To things
   if (state & BUTTON_MASK_FIRE) {
       if(g.fireTimer)
           Timer_DeleteTimer(g.fireTimer);
       g.fireTimer = Timer_CreateTimeout(200, 0, exitSettings, 5);
       g.buttonCnt++;
   } else {
       // debounce
       fireButtonPressed = 0;
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
    disableButtons();
    setupSettingsButtons();
    buildMenu();
}
