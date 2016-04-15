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
#include <stdbool.h>
#include <M451Series.h>
#include <Atomizer.h>
#include <Button.h>

struct globals {
	uint16_t volts;
	uint16_t newVolts;
	uint32_t watts;
	Atomizer_Info_t atomInfo;
} g = {};

uint16_t wattsToVolts(uint32_t watts, uint16_t res) {
	// Units: mV, mW, mOhm
	// V = sqrt(P * R)
	// Round to nearest multiple of 10
	uint16_t volts = (sqrt(watts * res) + 5) / 10;
	return volts * 10;
}

void startVaping(uint8_t state) {
    if (!Atomizer_IsOn() && g.atomInfo.resistance != 0 && Atomizer_GetError() == OK) {
    	Atomizer_Control(1);
    }
}

void buttonRight(uint8_t state) {
    g.newVolts = wattsToVolts(g.watts + 100, g.atomInfo.resistance);
    if(g.newVolts <= ATOMIZER_MAX_VOLTS) {
    	g.watts += 100;
    	g.volts = g.newVolts;

    	// Set voltage
    	Atomizer_SetOutputVoltage(g.volts);
    }
}

void buttonLeft(uint8_t state) {
    if (g.watts >= 100) {
    	g.watts -= 100;
    	g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);

    	// Set voltage
    	Atomizer_SetOutputVoltage(g.volts);
    }
}

uint8_t checkButtonsClear() {
    return Button_GetState() == BUTTON_MASK_NONE;
}

int main() {

	Button_CreateCallback(startVaping, BUTTON_MASK_FIRE);
	Button_CreateCallback(buttonRight, BUTTON_MASK_RIGHT);
	Button_CreateCallback(buttonLeft, BUTTON_MASK_LEFT);

	// Let's start with 15.0W as the initial value
	// We keep g.watts as mW
	g.watts = 15000;

	// Update info
    Atomizer_ReadInfo(&g.atomInfo);

	while(1) {
	    if (checkButtonsClear()) {
	        // buttons are not pressed
	        if(Atomizer_IsOn()) {
                Atomizer_Control(0);
                updateScreen(g.atomInfo, g.watts); // update to reflect atomizer off.
            }
	    } else {
	    	// Update info
        	Atomizer_ReadInfo(&g.atomInfo);
	        updateScreen(g.atomInfo, g.watts);
	    }

		g.newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);
		 if(g.newVolts != g.volts) {
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
             if (g.atomInfo.temperature >= 600) {
                g.newVolts = g.volts - 100;
             }
             g.volts = g.newVolts;
             Atomizer_SetOutputVoltage(g.volts);
         }
	}
}
