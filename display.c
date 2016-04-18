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
	siprintf(buff, "%3lu.%02lu",
		floating / 1000,
		floating % 1000 / 10);
}

void printTemperature(char *buff, uint32_t temp) {
	switch (s.tempScaleType) {
	case 0:
		temp = (temp - 32) * .5556;
		break;
	case 1:
		break;
	case 2:
		temp = (temp + 459.67) * 5/9;
		break;
	}
	printNumber(buff, temp);

}

void updateScreen(struct globals *g) {
	char *atomState;
	uint16_t battVolts;
    uint8_t battPerc;
    char buff[9];
    uint8_t atomizerOn = Atomizer_IsOn();

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
			atomState = "Short";
			break;
		case OPEN:
			atomState = "Atomizer";
			break;
		case WEAK_BATT:
			atomState = "Battery";
			break;
		case OVER_TEMP:
			atomState = "Too Hot";
			break;
		default:
			if (g->atomInfo.temperature >= s.targetTemperature) {
        	    atomState = "Temp Max";
        	} else if (Battery_IsCharging() & Battery_IsPresent()) {
        		atomState = "Charging";
        	} else if (!Battery_IsPresent()){
        		atomState = "No Bat";
        	} else {
        		atomState = "Firing";
        	}
			break;
	}
	Display_Clear();

    Display_PutLine(0, 24, 63, 24);

    if (g->vapeModes[s.mode]->controlType == TEMP_CONTROL) {
    	if (atomizerOn) {
    		printTemperature(buff, g->atomInfo.temperature);
    	} else {
    		printTemperature(buff, s.targetTemperature);
    	}
		Display_PutText(0, 0, buff, FONT_LARGE);
    	// TODO put type of temp here
		getString(buff, tempScaleType[s.tempScaleType]);
    	Display_PutText(48, 2, buff, FONT_DEJAVU_8PT);

        getString(buff, vapeMaterialList[s.materialIndex].name);
    	Display_PutText(48, 15, buff, FONT_DEJAVU_8PT);
    } else if (g->vapeModes[s.mode]->controlType == WATT_CONTROL) {
        getFloating(buff, g->watts);
    	Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);
    }

    getString(buff, atomState);
    Display_PutText(0, 35, buff, FONT_DEJAVU_8PT);


    getPercent(buff, battPerc);
	Display_PutText(0, 48, buff, FONT_DEJAVU_8PT);

	getFloating(buff, Battery_GetVoltage());
    Display_PutText(15, 48, buff, FONT_DEJAVU_8PT);

    getFloating(buff, g->watts);
	Display_PutText(0, 100, buff, FONT_DEJAVU_8PT);

    if (atomizerOn) {
		getFloating(buff, g->atomInfo.resistance);
    } else {
		getFloating(buff, g->atomInfo.base_resistance);
    }
	Display_PutText(0, 110, buff, FONT_DEJAVU_8PT);



	Display_Update();
}




