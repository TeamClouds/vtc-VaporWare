#include <math.h>
#include <stdint.h>
#include "globals.h"
#include "settings.h"

uint16_t wattsToVolts(uint32_t watts, uint16_t res) {
    // Units: mV, mW, mOhm
    // V = sqrt(P * R)
    // Round to nearest multiple of 10
    uint16_t volts = (sqrt(watts * res) + 5) / 10;

    return volts * 10;
}


uint32_t voltsToWatts(uint16_t volts, uint16_t res) {
    if (!res)
	return -1;

    uint32_t watts = (volts * volts / res + 5) / 10;
    return watts * 10;
}

uint32_t displayToC(uint32_t T) {
    switch(s.tempScaleTypeIndex) {
    case 0: //C - Do nothing
        return T;
        break;
    case 1: //F
        return 5 * (T - 32) / 9;
        break;
    case 2: //K
        return T + 276;
        break;
    }
    // default probably won't ever reach here
    return T;
}

uint32_t CToDisplay(uint32_t T) {
    switch(s.tempScaleTypeIndex) {
    case 0: //C - Do nothing
        return T;
        break;
    case 1: //F
        return 32 + 9 * T / 5;
        break;
    case 2: //K
        return T - 276;
        break;
    }
    // Something Bad Happened
    return T;
}
