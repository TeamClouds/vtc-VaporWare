#include <stdint.h>

#include <Atomizer.h>
#include <Display.h>

#include "display.h"
#include "font/font_vaporware.h"
#include "globals.h"
#include "helper.h"
#include "images/temperature.h"


void voltInit() {
	// set this initial value because we may be switching
	// from another mode that changes our volts.
    g.watts = 15000;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void voltFire() {
    g.vapeCnt++;
    while (gv.fireButtonPressed) {
	// Handle fire button
	if (!Atomizer_IsOn() && g.atomInfo.resistance != 0
	    && Atomizer_GetError() == OK) {
	    // Power on
	    Atomizer_Control(1);
	}
	// Update info
	// If resistance is zero voltage will be zero
	Atomizer_ReadInfo(&g.atomInfo);

	Atomizer_SetOutputVoltage(g.volts);

	g.vapeCnt++;
	updateScreen(&g);
    }

    if (Atomizer_IsOn())
	Atomizer_Control(0);
    g.vapeCnt = 0;
}

void voltUp() {
    g.newVolts = g.volts + 100;
    if (g.newVolts <= ATOMIZER_MAX_VOLTS) {
	g.volts = g.newVolts;
    } else {
	g.volts = ATOMIZER_MAX_VOLTS;
    }
    g.watts = voltsToWatts(g.volts, g.atomInfo.resistance);
}

void voltDown() {
    g.newVolts = g.volts - 100;
    if (g.volts >= 0) {
	g.volts = g.newVolts;
    } else {
	g.volts = 0;
    }
    g.watts = voltsToWatts(g.volts, g.atomInfo.resistance);
}

void voltDisplay(uint8_t atomizerOn) {
    char buff[9];
	getFloatingTenth(buff, g.volts);
	Display_PutText(0, 5, buff, FONT_LARGE);
	getString(buff, "V");
	Display_PutText(48, 2, buff, FONT_SMALL);
}

void voltBottomDisplay(uint8_t atomizerOn) {
    char buff[9];
	Display_PutPixels(0, 100, tempImage, tempImage_width, tempImage_height);

	printNumber(buff, CToDisplay(g.atomInfo.temperature));
	Display_PutText(24, 107, buff, FONT_MEDIUM);
}
