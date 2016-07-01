#include <stdint.h>

#include <Atomizer.h>
#include <Display.h>

#include "display.h"
#include "font/font_vaporware.h"
#include "globals.h"
#include "settings.h"
#include "helper.h"
#include "images/temperature.h"

void wattInit() {
    g.volts = wattsToVolts(s.targetWatts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);
}

void wattFire() {
    uint32_t newVolts;

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

        newVolts = wattsToVolts(s.targetWatts, g.atomInfo.resistance);

        if (newVolts != g.volts || !g.volts) {
            if (Atomizer_IsOn()) {

            // Update output voltage to correct res variations:
            // If the new voltage is lower, we only correct it in
            // 10mV steps, otherwise a flake res reading might
            // make the voltage plummet to zero and stop.
            // If the new voltage is higher, we push it up by 100mV
            // to make it hit harder on TC coils, but still keep it
            // under control.
            if (newVolts < g.volts) {
                newVolts = g.volts - (g.volts >= 10 ? 10 : 0);
            } else {
                newVolts = g.volts + 100;
            }

            }

            if (newVolts > ATOMIZER_MAX_VOLTS) {
                newVolts = ATOMIZER_MAX_VOLTS;
            }

            g.volts = newVolts;

            Atomizer_SetOutputVoltage(g.volts);
        }

        updateScreen(&g);
    }

    if (Atomizer_IsOn())
        Atomizer_Control(0);
}

void wattUp() {
    uint32_t tempWatts = s.targetWatts + 100;
    if (tempWatts <= MAXWATTS) {
        targetWattsSet(tempWatts);
    } else {
        targetWattsSet(MAXWATTS);
    }
}

void wattUpFast(){
  uint32_t tempWatts = s.targetWatts + 1000;
  if (tempWatts <= MAXWATTS) {
      targetWattsSet(tempWatts);
  } else {
      targetWattsSet(MAXWATTS);
  }
}

void wattDown() {
    uint32_t tempWatts = s.targetWatts - 100;
    if (tempWatts > MINWATTS) {
        targetWattsSet(tempWatts);
    } else {
        targetWattsSet(MINWATTS);
    }
}

void wattDownFast() {
    uint32_t tempWatts = s.targetWatts - 1000;
    if (tempWatts > MINWATTS) {
        targetWattsSet(tempWatts);
    } else {
        targetWattsSet(MINWATTS);
    }
}

void wattDisplay(uint8_t atomizerOn) {
    char buff[9];
    getFloatingTenth(buff, s.targetWatts);
    Display_PutText(0, 5, buff, FONT_LARGE);
    getString(buff, "W");
    Display_PutText(48, 2, buff, FONT_SMALL);
}

void wattBottomDisplay(uint8_t atomizerOn) {
    char buff[9];
    Display_PutPixels(0, 100, tempImage, tempImage_width, tempImage_height);

    printNumber(buff, CToDisplay(g.curTemp));
    Display_PutText(24, 107, buff, FONT_MEDIUM);
}

struct vapeMode variableWattage = {
    .index = 0,
    .controlType = WATT_CONTROL,
    .name = "Wattage",
    .supportedMaterials = KANTHAL | NICKEL | TITANIUM | STAINLESS,
    .init = &wattInit,
    .fire = &wattFire,
    .increase = &wattUp,
    .increaseFast = &wattUpFast,
    .decrease = &wattDown,
    .decreaseFast = &wattDownFast,
    .display = &wattDisplay,
    .bottomDisplay = &wattBottomDisplay,
};

static void __attribute__((constructor)) registerWattMode(void) {
    addVapeMode(&variableWattage);
}
