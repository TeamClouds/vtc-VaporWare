#include <stdint.h>
#include <stdio.h>

#include <inttypes.h>

#include <Atomizer.h>
#include <Display.h>

#include "display.h"
#include "drawables.h"
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

void voltGetText(char *buff, uint8_t len) {
    sniprintf(buff, len, "%d.%01dV", s.targetVolts / 1000, s.targetVolts % 1000 / 100);
}

void voltGetAltText(char *buff, uint8_t len) {
    printNumber(buff, len, CToDisplay(g.curTemp));
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
    .maxSetting = ATOMIZER_MAX_VOLTS,
    .getAltDisplayText = &voltGetAltText,
    .getDisplayText = &voltGetText,
    .altIconDrawable = TEMPICON,
};

static void __attribute__((constructor)) registerVoltMode(void) {
    addVapeMode(&variableVoltage);
}