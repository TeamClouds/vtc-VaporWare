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

#include <math.h>
#include <stdio.h>
#include <M451Series.h>
#include <Atomizer.h>
#include <Button.h>
#include <TimerUtils.h>

#include "main.h"
struct globals g = {};
struct settings s = {};

void updateScreen(struct globals *g);
void showMenu();

uint16_t wattsToVolts(uint32_t watts, uint16_t res) {
    // Units: mV, mW, mOhm
    // V = sqrt(P * R)
    // Round to nearest multiple of 10
    uint16_t volts = (sqrt(watts * res) + 5) / 10;

    return volts * 10;
}

volatile int fireButtonPressed = 0;

void vape() {
    g.vapeCnt++;
    while (fireButtonPressed) {
        // Handle fire button
        if(!Atomizer_IsOn() && g.atomInfo.resistance != 0 && Atomizer_GetError() == OK) {
            // Power on
            Atomizer_Control(1);
        }

        // Update info
        // If resistance is zero voltage will be zero
        Atomizer_ReadInfo(&g.atomInfo);

        g.newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);

        if(g.newVolts != g.volts || !g.volts) {
            if(Atomizer_IsOn()) {

                // Update output voltage to correct res variations:
                // If the new voltage is lower, we only correct it in
                // 10mV steps, otherwise a flake res reading might
                // make the voltage plummet to zero and stop.
                // If the new voltage is higher, we push it up by 100mV
                // to make it hit harder on TC coils, but still keep it
                // under control.
                if(g.newVolts < g.volts) {
                    g.newVolts = g.volts - (g.volts >= 10 ? 10 : 0);
                }
                else {
                    g.newVolts = g.volts + 100;
                }

            }

            if(g.newVolts > ATOMIZER_MAX_VOLTS) {
                g.newVolts = ATOMIZER_MAX_VOLTS;
            }

            g.volts = g.newVolts;

            Atomizer_SetOutputVoltage(g.volts);
        }
        g.vapeCnt++;
        updateScreen(&g);
    }
    if(Atomizer_IsOn())
        Atomizer_Control(0);
    g.vapeCnt = 0;
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
        g.newVolts = wattsToVolts(g.watts + 100, g.atomInfo.resistance);
        if(g.newVolts <= ATOMIZER_MAX_VOLTS) {
            g.watts += 100;
            g.volts = g.newVolts;

            // Set voltage
            Atomizer_SetOutputVoltage(g.volts);
        }
    }
}

void buttonLeft(uint8_t state) {
    updateScreen(&g);
    if (state & BUTTON_MASK_LEFT) {
        if (g.watts >= 100) {
            g.watts -= 100;
            g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);

            // Set voltage
            Atomizer_SetOutputVoltage(g.volts);
        }
    }
}


void setupButtons() {
    g.fire = Button_CreateCallback(buttonFire, BUTTON_MASK_FIRE);
    g.plus = Button_CreateCallback(buttonRight, BUTTON_MASK_RIGHT);
    g.minus = Button_CreateCallback(buttonLeft, BUTTON_MASK_LEFT);
}

void (*__vape)(void) = &vape;

int main() {
    int i = 0;
    load_settings();
    setupButtons();

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
