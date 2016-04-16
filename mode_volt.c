#include <stdint.h>
#include "main.h"

void voltFire() {
    g.vapeCnt++;
    while (fireButtonPressed) {
        // Handle fire button
        if(!Atomizer_IsOn() && g.atomInfo.resistance != 0 && Atomizer_GetError() == OK) {
            // Power on
            Atomizer_Control(1);
        }

        // Update info
        // If resistance is zero voltage will be zero
        Atomizer_ReadInfo(&g.atomInfo);

        g.watts = voltsToWatts(g.volts, g.atomInfo.resistance);

        g.vapeCnt++;
        updateScreen(&g);
    }

    if(Atomizer_IsOn())
        Atomizer_Control(0);
    g.vapeCnt = 0;
}

void voltUp() {
    g.newVolts = g.volts + 100;
    if(g.newVolts <= ATOMIZER_MAX_VOLTS) {
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

