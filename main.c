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
#include <math.h>
#include <M451Series.h>
#include <Display.h>
#include <Font.h>
#include <Atomizer.h>
#include <Button.h>
#include <TimerUtils.h>
#include <Battery.h>

struct globals {
	uint16_t volts;
	uint16_t newVolts;
	uint16_t displayVolts;
	uint32_t watts;
	uint32_t maxTemp;
	uint8_t sleepTimeout;
	Atomizer_Info_t atomInfo;
	double startingResistance;
} g = {};

uint16_t wattsToVolts(uint32_t watts, uint16_t res) {
	// Units: mV, mW, mOhm
	// V = sqrt(P * R)
	// Round to nearest multiple of 10
	uint16_t volts = (sqrt(watts * res) + 5) / 10;
	return volts * 10;
}

void sleepDisplay(uint32_t counterIndex) {
	Display_SetOn(0);
}

uint32_t cToF(uint8_t temp) {
    uint32_t celTemp = temp;
    uint32_t current = g.atomInfo.resistance % 1000 / 10;
    if (current > g.startingResistance) {
        celTemp = ((current - g.startingResistance) * .01) / (0.00006 * g.startingResistance);
    }
    return (celTemp * 1.8) + 32;
}

void updateScreen() {
	const char *atomState;
	char buf[100];
	uint32_t boardTemp;
	uint16_t battVolts;
    uint8_t battPerc;

    // Get battery voltage and charge
	battVolts = Battery_IsPresent() ? Battery_GetVoltage() : 0;
	battPerc = Battery_VoltageToPercent(battVolts);

	// Get board temperature
	boardTemp = Atomizer_ReadBoardTemp();
	g.maxTemp = cToF(boardTemp);

	// Display info
	g.displayVolts = Atomizer_IsOn() ? g.atomInfo.voltage : g.volts;
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
			atomState = Atomizer_IsOn() ? "FIRING" : "";
			break;
	}
	if (g.maxTemp >= 600) {
	    atomState = "TEMP PRO";
	}
	siprintf(buf, "%3lu.%luW\n%2d.%02do\n%2d.%02dA\n%5luF\n%s\n\n\n\n\n%d%%\n%s",
		g.watts / 1000, g.watts % 1000 / 100,
		g.atomInfo.resistance / 1000, g.atomInfo.resistance % 1000 / 10,
		g.atomInfo.current / 1000, g.atomInfo.current % 1000 / 10,
		g.maxTemp,
		atomState,
		battPerc,
		Battery_IsCharging() & Battery_IsPresent() ? "CHARGING" : "");
	Display_Clear();
	Display_PutText(0, 0, buf, FONT_DEJAVU_8PT);
	Display_Update();
	if (g.sleepTimeout > 0) {
	    Timer_DeleteTimer(g.sleepTimeout);
	    g.sleepTimeout = 0;
	}
	g.sleepTimeout = Timer_CreateTimeout(1000, 0, sleepDisplay, 0);
}

void updateAtomData() {
	Atomizer_ReadInfo(&g.atomInfo);
    g.newVolts = wattsToVolts(g.watts, g.atomInfo.resistance);
}

int main() {
    uint8_t btnState;
	bool buttonsPressed = false;

	// Let's start with 10.0W as the initial value
	// We keep watts as mW
	g.watts = 10000;
	Atomizer_SetOutputVoltage(g.volts);

	while(1) {

		btnState = Button_GetState();

		if (g.startingResistance == 0) {
		    g.startingResistance = g.atomInfo.resistance % 1000 / 10;
		}

		switch(btnState) {
		    case BUTTON_MASK_FIRE:
		        if (!Atomizer_IsOn() && g.atomInfo.resistance != 0 && Atomizer_GetError() == OK) {
		        	Atomizer_Control(1);
		        }

		        buttonsPressed = true;
		        break;
		    case BUTTON_MASK_RIGHT:
		        g.newVolts = wattsToVolts(g.watts + 100, g.atomInfo.resistance);
                if(g.newVolts <= ATOMIZER_MAX_VOLTS) {
                	g.watts += 100;
                	g.volts = g.newVolts;

                	// Set voltage
                	Atomizer_SetOutputVoltage(g.volts);
                	// Slow down increment
                	Timer_DelayMs(25);
                }
                buttonsPressed = true;
                break;
		    case BUTTON_MASK_LEFT:
		        if (g.watts >= 100) {
		        	g.watts -= 100;
                	g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);

                	// Set voltage
                	Atomizer_SetOutputVoltage(g.volts);
                	// Slow down decrement
                	Timer_DelayMs(25);
		        }
		        buttonsPressed = true;
		        break;
		    default:
		        // No buttons!
		        if(Atomizer_IsOn()) {
		            Atomizer_Control(0);
		        }
		        if (buttonsPressed) {
		            updateScreen();
		        }
		        buttonsPressed = false;
		        break;
		}

		// Update info
        updateAtomData();

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
             if (g.maxTemp >= 600) {
                g.newVolts = g.volts - 100;
             }
             g.volts = g.newVolts;
             Atomizer_SetOutputVoltage(g.volts);
         }

        if (buttonsPressed || Battery_IsCharging()) {
        	Display_SetOn(1);
		    updateScreen();
		}
	}
}
