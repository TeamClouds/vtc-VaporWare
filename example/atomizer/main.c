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

uint16_t volts, newVolts, displayVolts;
uint32_t watts, maxTemp;
uint8_t sleepTimeout;
Atomizer_Info_t atomInfo;
double startingResistance;

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
    uint32_t current = atomInfo.resistance % 1000 / 10;
    if (current > startingResistance) {
        celTemp = ((current - startingResistance) * .01) / (0.00006 * startingResistance);
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
	maxTemp = cToF(boardTemp);

	// Display info
	displayVolts = Atomizer_IsOn() ? atomInfo.voltage : volts;
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
	if (maxTemp >= 600) {
	    atomState = "TEMP PRO";
	}
	siprintf(buf, "%3lu.%luW\n%2d.%02do\n%2d.%02dA\n%5dF\n%s\n\n\n\n\n%d%%\n%s",
		watts / 1000, watts % 1000 / 100,
		atomInfo.resistance / 1000, atomInfo.resistance % 1000 / 10,
		atomInfo.current / 1000, atomInfo.current % 1000 / 10,
		maxTemp,
		atomState,
		battPerc,
		Battery_IsCharging() & Battery_IsPresent() ? "CHARGING" : "");
	Display_Clear();
	Display_PutText(0, 0, buf, FONT_DEJAVU_8PT);
	Display_Update();
	if (sleepTimeout > 0) {
	    Timer_DeleteTimer(sleepTimeout);
	    sleepTimeout = 0;
	}
	sleepTimeout = Timer_CreateTimeout(1000, 0, sleepDisplay, 0);
}

void updateAtomData() {
	Atomizer_ReadInfo(&atomInfo);
    newVolts = wattsToVolts(watts, atomInfo.resistance);
}

int main() {
    uint8_t btnState;
	bool buttonsPressed = false;

	// Let's start with 10.0W as the initial value
	// We keep watts as mW
	watts = 10000;
	Atomizer_SetOutputVoltage(volts);

	while(1) {

		btnState = Button_GetState();

		if (startingResistance == 0) {
		    startingResistance = atomInfo.resistance % 1000 / 10;
		}

		switch(btnState) {
		    case BUTTON_MASK_FIRE:
		        if (!Atomizer_IsOn() && atomInfo.resistance != 0 && Atomizer_GetError() == OK) {
		        	Atomizer_Control(1);
		        }

		        buttonsPressed = true;
		        break;
		    case BUTTON_MASK_RIGHT:
		        newVolts = wattsToVolts(watts + 100, atomInfo.resistance);
                if(newVolts <= ATOMIZER_MAX_VOLTS) {
                	watts += 100;
                	volts = newVolts;

                	// Set voltage
                	Atomizer_SetOutputVoltage(volts);
                	// Slow down increment
                	Timer_DelayMs(25);
                }
                buttonsPressed = true;
                break;
		    case BUTTON_MASK_LEFT:
		        if (watts >= 100) {
		        	watts -= 100;
                	volts = wattsToVolts(watts, atomInfo.resistance);

                	// Set voltage
                	Atomizer_SetOutputVoltage(volts);
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

		 if(newVolts != volts) {
             if(Atomizer_IsOn()) {
                 // Update output voltage to correct res variations:
                 // If the new voltage is lower, we only correct it in
                 // 10mV steps, otherwise a flake res reading might
                 // make the voltage plummet to zero and stop.
                 // If the new voltage is higher, we push it up by 100mV
                 // to make it hit harder on TC coils, but still keep it
                 // under control.
                 if(newVolts < volts) {
                     newVolts = volts - (volts >= 10 ? 10 : 0);
                 }
                 else {
                     newVolts = volts + 100;
                 }
             }

             if(newVolts > ATOMIZER_MAX_VOLTS) {
                 newVolts = ATOMIZER_MAX_VOLTS;
             }
             if (maxTemp >= 600) {
                newVolts = volts - 100;
             }
             volts = newVolts;
             Atomizer_SetOutputVoltage(volts);
         }

        if (buttonsPressed || Battery_IsCharging()) {
        	Display_SetOn(1);
		    updateScreen();
		}
	}
}
