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
#include <string.h>

#include <M451Series.h>
#include <Display.h>
#include <Font.h>
#include <Atomizer.h>
#include <TimerUtils.h>
#include <Battery.h>
#include <Button.h>

#include "globals.h"
#include "helper.h"
#include "images/battery.h"
#include "settings.h"
#include "images/hot.h"
#include "images/temperature.h"
#include "images/short.h"
#include "images/ohm.h"
#include "images/watts.h"

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
    siprintf(buff, "%lu.%02lu", floating / 1000, floating % 1000 / 10);
}

inline void getFloatingTenth(char *buff, uint32_t floating) {
    siprintf(buff, "%lu.%lu", floating / 1000, floating % 1000 / 10);
}

uint8_t* getBatteryIcon(struct globals *g) {
    switch (Atomizer_GetError()) {
    case WEAK_BATT:
	    return batteryalert;
    default:
    	if (Battery_IsPresent()) {
        	if (Battery_IsCharging()) {
        	    return batterycharging;
        	} else {
        		return battery;
        	}
    	} else {
    		return batteryalert;
    	}
    }
}

void updateScreen(struct globals *g) {
    char buff[9];
    uint8_t atomizerOn = Atomizer_IsOn();

    if (!atomizerOn) {
	    g->batteryPercent = Battery_VoltageToPercent(
            Battery_IsPresent()? Battery_GetVoltage() : 0);
    }

    if (g->charging && !g->pauseScreenOff && (g->screenState < gv.uptime)) {
        Display_Clear();
        // update the battery percent all the time if
        // we are charging
        getPercent(buff, g->batteryPercent);
        uint8_t size = strlen(buff);
    	Display_PutPixels(20, 20, getBatteryIcon(&g), battery_width, battery_height);

        Display_PutText((DISPLAY_WIDTH/2)-((12*size)/2),
            (DISPLAY_HEIGHT/2)-12, buff, FONT_LARGE);
        Display_Update();
        return;
    }

    if (atomizerOn) {
	if (!Display_IsFlipped()) {
	    Display_Flip();
	}
    } else {
	if (Display_IsFlipped()) {
	    Display_Flip();
	}
    }

    Display_Clear();

    Display_PutLine(0, 24, 63, 24);

    switch (g->vapeModes[s.mode]->controlType) {
    case TEMP_CONTROL:
        if (atomizerOn) {
            printNumber(buff, CToDisplay(g->atomInfo.temperature));
        } else {
            printNumber(buff, s.displayTemperature);
        }
	    Display_PutText(0, 5, buff, FONT_LARGE);
	    getString(buff, (char *) tempScaleType[s.tempScaleTypeIndex].display);
	    Display_PutText(48, 2, buff, FONT_DEJAVU_8PT);
        break;
    case WATT_CONTROL:
        getFloatingTenth(buff, g->watts);
	    Display_PutText(0, 10, buff, FONT_DEJAVU_8PT);
	    getString(buff, "W");
	    Display_PutText(48, 2, buff, FONT_DEJAVU_8PT);
        break;
    case VOLT_CONTROL:
        getFloatingTenth(buff, g->volts);
        Display_PutText(0, 10, buff, FONT_DEJAVU_8PT);
	    getString(buff, "V");
	    Display_PutText(48, 2, buff, FONT_DEJAVU_8PT);
        break;
    }

	// Material
	getString(buff, vapeMaterialList[s.materialIndex].name);
	Display_PutText(48, 15, buff, FONT_DEJAVU_8PT);

	// battery
	Display_PutPixels(0, 34, getBatteryIcon(&g), battery_width, battery_height);

    getPercent(buff, g->batteryPercent);
    Display_PutText(24, 35, buff, FONT_DEJAVU_8PT);

	getFloating(buff, Battery_GetVoltage());
	Display_PutText(24, 47, buff, FONT_DEJAVU_8PT);

    switch (Atomizer_GetError()) {
    case SHORT:
    case OPEN:
		Display_PutPixels(20, 75, shortBIT, shortBIT_width, shortBIT_height);
    	break;
    default:
		Display_PutPixels(0, 60, ohm, ohm_width, ohm_height);

		if (atomizerOn) {
		getFloating(buff, g->atomInfo.resistance);
		} else {
		getFloating(buff, g->atomInfo.base_resistance);
		}
		Display_PutText(24, 68, buff, FONT_DEJAVU_8PT);

		Display_PutPixels(0, 85, watts, watts_width, watts_height);

		getFloating(buff, g->watts);
		Display_PutText(24, 90, buff, FONT_DEJAVU_8PT);
		break;
    }

    Display_Update();
}
