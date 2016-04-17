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
#include <Display.h>
#include <Font.h>
#include <Atomizer.h>
#include <TimerUtils.h>
#include <Battery.h>
#include <Button.h>
#include "main.h"
extern struct globals g;

// To whom it may concern
// Calling things like Button state here are really bad
// You should not do that
// Vapers need to vape, potatos need to potate
// If you get button state in this file you wont be able to vape
// Horrible things will happen to your house.

// NOTES:
// Evic VTC mini X-MAX = 116

void sleepDisplay(uint32_t counterIndex) {
}

inline void printNumber(char *buff, uint32_t temperature) {
    siprintf(buff, "%lu", temperature);
}

inline void getPercent(char *buff, uint8_t percent) {
    siprintf(buff, "%d%%", percent);
}

void getString(char *buff, char *state) {
    siprintf(buff, "%s", state);
}

inline void getFloating(char *buff, uint32_t floating) {
	siprintf(buff, "%3lu.%lu",
		floating / 1000,
		floating % 1000 / 10);
}

void updateScreen(struct globals *g) {
	char *atomState;
	uint16_t battVolts;
    uint8_t battPerc;

    if (!gv.screenState)
        return;

    if (Atomizer_IsOn()) {
        if (!Display_IsFlipped()) {
            Display_Flip();
        }
    } else {
        if (Display_IsFlipped()) {
            Display_Flip();
        }
    }


    // Get battery voltage and charge
	battVolts = Battery_IsPresent() ? Battery_GetVoltage() : 0;
	battPerc = Battery_VoltageToPercent(battVolts);

	// Display info
	switch(Atomizer_GetError()) {
		case SHORT:
			atomState = "SHORT";
			break;
		case OPEN:
			atomState = "NO ATOM";
			break;
		case WEAK_BATT:
			atomState = "WEAK BAT";
			break;
		case OVER_TEMP:
			atomState = "TOO HOT";
			break;
		default:
			if (g->atomInfo.temperature >= 600) {
        	    atomState = "PROTECT";
        	} else {
        		atomState = Atomizer_IsOn() ? "FIRING" : "";
        	}
			break;
	}

	//Battery_IsCharging() & Battery_IsPresent() ? "CHARGING" : ""
	Display_Clear();

    char buff[8];

    if (g->vapeModes[s.mode]->controlType == TEMP_CONTROL) {
    	if (Atomizer_IsOn()) {
    	    printNumber(buff, g->atomInfo.temperature);
    		Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);
    	} else {
            printNumber(buff, s.targetTemperature);
        	Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);
    	}
        getFloating(buff, g->watts);
    	Display_PutText(0, 20, buff, FONT_DEJAVU_8PT);
    } else if (g->vapeModes[s.mode]->controlType == WATT_CONTROL) {
        getFloating(buff, g->watts);
    	Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);

	    printNumber(buff, g->atomInfo.temperature);
		Display_PutText(0, 20, buff, FONT_DEJAVU_8PT);
    }

    getFloating(buff, g->atomInfo.resistance);
	Display_PutText(0, 30, buff, FONT_DEJAVU_8PT);

	getFloating(buff, g->atomInfo.base_resistance);
    Display_PutText(0, 40, buff, FONT_DEJAVU_8PT);

    getPercent(buff, battPerc);
	Display_PutText(0, 60, buff, FONT_DEJAVU_8PT);

    getString(buff, s.material->name);
	Display_PutText(0, 70, buff, FONT_DEJAVU_8PT);


    printNumber(buff, g->maxTemp);
	Display_PutText(0, 80, buff, FONT_DEJAVU_8PT);

    printNumber(buff, g->minTemp);
	Display_PutText(0, 90, buff, FONT_DEJAVU_8PT);

    printNumber(buff, gv.screenState);
        Display_PutText(0, 100, buff, FONT_DEJAVU_8PT);

    getString(buff, atomState);
    Display_PutText(0, 116, buff, FONT_DEJAVU_8PT);

	Display_Update();

	//Timer_CreateTimeout(1000, 0, sleepDisplay, 0);
}


