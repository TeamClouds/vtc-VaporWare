#include <stdint.h>

#include <Atomizer.h>
#include <Display.h>

#include "display.h"
#include "font/font_vaporware.h"
#include "globals.h"
#include "settings.h"
#include "helper.h"
#include "images/temperature.h"


void voltInit() {
    // set this initial value because we may be switching
    // from another mode that changes our volts.
    Atomizer_SetOutputVoltage(s.targetVolts);
}

void voltFire() {
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

        Atomizer_SetOutputVoltage(s.targetVolts);

        updateScreen(&g);
    }

    if (Atomizer_IsOn())
        Atomizer_Control(0);
}

void voltUp() {
    uint16_t newVolts = s.targetVolts + 100;
    if (newVolts <= MAXVOLTS) {
        targetVoltsSet(newVolts);
    } else {
        targetVoltsSet(MAXVOLTS);
    }
}

void voltDown() {
    uint16_t newVolts = s.targetVolts - 100;
    if (g.volts >= MINVOLTS) {
        targetVoltsSet(newVolts);
    } else {
        targetVoltsSet(MINVOLTS);
    }
}

void voltDisplay(uint8_t atomizerOn) {
    char buff[9];
    getFloatingTenth(buff, s.targetVolts);
    Display_PutText(0, 5, buff, FONT_LARGE);
    getString(buff, "V");
    Display_PutText(48, 2, buff, FONT_SMALL);
}

void voltBottomDisplay(uint8_t atomizerOn) {
    char buff[9];
    Display_PutPixels(0, 100, tempImage, tempImage_width, tempImage_height);

    printNumber(buff, CToDisplay(g.curTemp));
    Display_PutText(24, 107, buff, FONT_MEDIUM);
}

struct vapeMode variableVoltage = {
    .index = 1,
    .controlType = VOLT_CONTROL,
    .name = "Voltage",
    .supportedMaterials = KANTHAL | NICKEL | TITANIUM | STAINLESS,
    .fire = &voltFire,
    .init = &voltInit,
    .increase = &voltUp,
    .decrease = &voltDown,
    .display = &voltDisplay,
    .bottomDisplay = &voltBottomDisplay,
};

static void __attribute__((constructor)) registerVoltMode(void) {
    addVapeMode(&variableVoltage);
}
