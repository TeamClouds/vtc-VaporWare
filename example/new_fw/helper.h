#ifndef __HELPER_H
#define __HELPER_H
uint16_t wattsToVolts(uint32_t watts, uint16_t res);
uint32_t voltsToWatts(uint16_t volts, uint16_t res);
uint32_t displayToC(uint32_t T);
uint32_t CToDisplay(uint32_t T);
#endif