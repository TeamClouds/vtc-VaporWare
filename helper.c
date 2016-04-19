#include <math.h>
#include <stdint.h>

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
