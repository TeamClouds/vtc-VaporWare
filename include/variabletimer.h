#ifndef __VARIABLETIMER_H
#define __VARIABLETIMER_H

#include <stdint.h>

// Hopefully our only lone global
extern volatile uint32_t uptime;

#define UPTIME "%8lu.%03lu"
#define TIMEFMT(X) (X)/1000, (X)%1000
#define UPTIMEVAL TIMEFMT(uptime)

enum {
    TimerIdle = 0,
    TimerLowres = 1,
    TimerStdres = 2,
    TimerHighRes = 3,
};

uint8_t requestTimerSlot();
uint8_t requestTimer(uint8_t slot, uint8_t freqTen);
void waitForFasterTimer(uint8_t freqTen);
void setTimer(uint8_t freqTen);

#endif