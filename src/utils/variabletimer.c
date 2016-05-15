#include <stdint.h>
#include <math.h>

#include <TimerUtils.h>

#include "button.h"
#include "globals.h"
#include "variabletimer.h"

volatile uint32_t uptime = 0;

struct timerData {
    uint8_t timerSlots[8];
    uint8_t timerSlotsUsed;
    volatile uint16_t curTimerFreq;
    uint16_t newTimerFreq;
    volatile uint32_t curTimerStep;
    uint32_t newTimerStep;
    volatile uint16_t curTimerInt;
    volatile uint16_t newTimerInt;
    int8_t ourTimer;
    volatile uint8_t ourSlot;
} td = {
    .timerSlots = {0},
    .timerSlotsUsed = 0,
    .curTimerFreq = 0,
    .newTimerFreq = 0,
    .curTimerStep = 0,
    .newTimerStep = 0,
    .curTimerInt = 0,
    .newTimerInt = 0,
    .ourTimer = -1,
    .ourSlot = 0,
};

void uptimeTimer(uint32_t param) {
    uptime += td.curTimerStep;

    if (!gv.sleeping && (
        (buttonTimeout && *buttonTimeout > uptime) || gv.buttonEvent)
        )
        buttonTimer(param);

    if (!td.newTimerInt)
        return;

    td.curTimerFreq = td.newTimerFreq;
    td.curTimerStep = td.newTimerStep;
    td.curTimerInt = td.newTimerInt;

    td.newTimerInt = 0;

    Timer_DeleteTimer(td.ourTimer);

    td.ourTimer = Timer_CreateTimer(td.curTimerFreq, 1, uptimeTimer, 3);
}

void waitForFasterTimer(uint8_t freqTen) {
    do {;} while (freqTen > td.curTimerInt);
}

uint8_t requestTimerSlot() {
    return td.timerSlotsUsed++;
}

uint8_t requestTimer(uint8_t slot, uint8_t freqTen) {
    int i = 0;
    uint32_t maxExp = 0;


    td.timerSlots[slot] = freqTen;

    if (freqTen <= td.newTimerInt)
        return 0;

    for (i = 0; i < td.timerSlotsUsed; i++) {
        if (td.timerSlots[i] > maxExp)
            maxExp = td.timerSlots[i];
    }

    if (maxExp <= td.curTimerInt)
        return 0;

    td.newTimerFreq = pow(10,maxExp);
    td.newTimerStep = 1000 / td.newTimerFreq;
    td.newTimerInt = maxExp;

    return 1;
}

static void __attribute__((constructor)) prepareTimers(void) {
    td.ourSlot = requestTimerSlot();
    requestTimer(td.ourSlot, TimerLowres);
    td.curTimerFreq = pow(10, TimerLowres);
    td.curTimerStep = 1000 / td.curTimerFreq;
    td.curTimerInt = TimerLowres;
    
    td.ourTimer = Timer_CreateTimer(td.curTimerFreq, 1, uptimeTimer, 3);
}