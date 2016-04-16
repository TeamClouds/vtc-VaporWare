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
void sleepDisplay(uint32_t counterIndex) {
	Display_SetOn(0);
}


inline void getMenuDumbText(char *buff) {
    siprintf(buff, "Herro");
}

void showMenu() {
    char buff[8];
	Display_Clear();
    getMenuDumbText(buff);
	Display_PutText(0, 60, buff, FONT_DEJAVU_8PT);
	Display_Update();
}


inline void getTemperature(char *buff, uint32_t temperature) {
    siprintf(buff, "%lu", temperature);
}

inline void getPercent(char *buff, uint8_t percent) {
    siprintf(buff, "%d%%", percent);
}

inline void getState(char *buff, char *state, uint8_t intstate) {
    siprintf(buff, "%s(%i)", state, intstate);
}

inline void getResistance(char *buff, uint32_t resistance) {
	siprintf(buff, "%3lu.%luO",
		resistance / 1000,
		resistance % 1000 / 10);
}

inline void getWatts(char *buff, uint32_t watts) {
	siprintf(buff, "%3lu.%luW",
		watts / 1000,
		watts % 1000 / 100);
}


void updateScreen(struct globals *g) {
	char *atomState;
	uint16_t battVolts;
    uint8_t battPerc;

    Display_SetOn(1);

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
    getTemperature(buff, g->atomInfo.temperature);
	Display_PutText(0, 0, buff, FONT_DEJAVU_8PT);

    getTemperature(buff, g->atomInfo.base_temperature);
	Display_PutText(0, 10, buff, FONT_DEJAVU_8PT);

    getWatts(buff, g->watts);
	Display_PutText(0, 20, buff, FONT_DEJAVU_8PT);

    getResistance(buff, g->atomInfo.resistance);
	Display_PutText(0, 30, buff, FONT_DEJAVU_8PT);

	getResistance(buff, g->atomInfo.base_resistance);
    Display_PutText(0, 40, buff, FONT_DEJAVU_8PT);

// NEVER FUCKING CALL FUNCTIONS HERE YOU FUCKING FUCK
//    getState(buff, atomState, Button_GetState());
//	Display_PutText(0, 50, buff, FONT_DEJAVU_8PT);

    getPercent(buff, battPerc);
	Display_PutText(0, 60, buff, FONT_DEJAVU_8PT);

    getTemperature(buff, g->vapeCnt);
	Display_PutText(0, 70, buff, FONT_DEJAVU_8PT);

    getTemperature(buff, g->whatever);
	Display_PutText(0, 80, buff, FONT_DEJAVU_8PT);

	Display_Update();

	//Timer_CreateTimeout(1000, 0, sleepDisplay, 0);
}


