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

void sleepDisplay(uint32_t counterIndex) {
	Display_SetOn(0);
}

void updateScreen(volatile Atomizer_Info_t atomInfo, volatile uint32_t watts) {
	const char *atomState;
	char buf[100];
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
			if (atomInfo.temperature >= 600) {
        	    atomState = "PROTECT";
        	} else {
        		atomState = Atomizer_IsOn() ? "FIRING" : "";
        	}
			break;
	}

	siprintf(buf, "%3lu.%luW\n%2d.%02do\n%2d.%02dA\n%5dF\n%s\n\n\n\n%d%%\n%s",
		watts / 1000,
		watts % 1000 / 100,
		atomInfo.resistance / 1000,
		atomInfo.resistance % 1000 / 10,
		atomInfo.current / 1000,
		atomInfo.current % 1000 / 10,
		atomInfo.temperature,
		atomState,
		battPerc,
		Battery_IsCharging() & Battery_IsPresent() ? "CHARGING" : "");

	Display_Clear();
	Display_PutText(0, 0, buf, FONT_DEJAVU_8PT);
	Display_Update();

	Timer_CreateTimeout(1000, 0, sleepDisplay, 0);
}
