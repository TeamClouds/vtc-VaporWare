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
#include <M451Series.h>
#include <Atomizer.h>
#include <Button.h>
#include <TimerUtils.h>

#include "main.h"
#include "mode_watt.h"
#include "mode_volt.h"

volatile int fireButtonPressed = 0;
struct globals g = {};
struct settings s = {};

// ALWAYS init it a sane mode
void (*__init)(void);
void (*__vape)(void);
void (*__up)(void);
void (*__down)(void);

void setVapeMode(int newMode) {
    if(newMode >= MODE_COUNT)
        return;

    __vape = g.vapeModes[newMode]->fire;
    __up = g.vapeModes[newMode]->increase;
    __down = g.vapeModes[newMode]->decrease;
    if(g.vapeModes[newMode]->init) {
        __init = g.vapeModes[newMode]->init;
	__init();
    }
}

void startVaping(uint32_t counterIndex) {
   if(g.buttonCnt < 3) {
       if(Button_GetState() & BUTTON_MASK_FIRE) {
          fireButtonPressed = 1;
          g.buttonCnt = 0;
       }
   } else {
       showMenu();
       g.buttonCnt = 0;
   }
}

void buttonFire(uint8_t state) {
   g.whatever++;
   if (state & BUTTON_MASK_FIRE) {
       if(g.fireTimer)
           Timer_DeleteTimer(g.fireTimer);
       g.fireTimer = Timer_CreateTimeout(200, 0, startVaping, 3);
       g.buttonCnt++;
   } else {
       fireButtonPressed = 0;
   }
}

void buttonRight(uint8_t state) {
    updateScreen(&g);
    if(state & BUTTON_MASK_RIGHT) {
        __up();
        Atomizer_SetOutputVoltage(g.volts);
    }
}

void buttonLeft(uint8_t state) {
    updateScreen(&g);
    if (state & BUTTON_MASK_LEFT) {
        __down();
	Atomizer_SetOutputVoltage(g.volts);
    }
}


void setupButtons() {
    g.fire = Button_CreateCallback(buttonFire, BUTTON_MASK_FIRE);
    g.plus = Button_CreateCallback(buttonRight, BUTTON_MASK_RIGHT);
    g.minus = Button_CreateCallback(buttonLeft, BUTTON_MASK_LEFT);
}

#define REGISTER_MODE(X) g.vapeModes[X.index] = &X

int main() {
    int i = 0;
    load_settings();
    setupButtons();

    REGISTER_MODE(variableVoltage);
    REGISTER_MODE(variableWattage);

    setVapeMode(0);

    // Let's start with 15.0W as the initial value
    // We keep g.watts as mW
    Atomizer_ReadInfo(&g.atomInfo);
    g.watts = 15000;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);

    // Initialize atomizer info
    do {
        Atomizer_ReadInfo(&g.atomInfo);
        updateScreen(&g);
        i++;
    } while(i < 100 && g.atomInfo.resistance == 0) ;

    while(1) {

        if (fireButtonPressed) {
            __vape();
        }
        while(g.atomInfo.resistance - g.atomInfo.base_resistance > 10) {
            Atomizer_ReadInfo(&g.atomInfo);
            updateScreen(&g);
        }

    }
}
