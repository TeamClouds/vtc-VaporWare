#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Atomizer.h>
#include <stdint.h>
#include "materials.h"
#include "mode.h"

#define UPTIME "%8lu.%02lu"
#define UPTIMEVAL gv.uptime / 100, gv.uptime % 100

struct globals {
    Atomizer_Info_t atomInfo;
    struct vapeMode *vapeModes[MODE_COUNT];
    uint32_t watts;
    uint16_t volts;
    uint16_t newVolts;
    uint8_t charging;
    uint8_t fire;
    uint8_t minus;
    uint8_t plus;
    uint8_t vapeCnt;
    uint32_t maxTemp;
    uint32_t minTemp;
    uint8_t batteryPercent;
    uint32_t screenState;
    int8_t pauseScreenOff;
};
extern struct globals g;

struct globalVols {
    volatile uint32_t uptime;
    volatile uint8_t uptimeTimer;
    volatile uint8_t fireTimer;
    volatile uint8_t fireButtonPressed;
    volatile uint8_t buttonCnt;
    volatile uint8_t shouldShowMenu;
    volatile uint8_t buttonRepeatTimer;
    volatile uint32_t saveSettings;
    volatile uint8_t buttonEvent;
};

extern volatile struct globalVols gv;

extern struct vapeMaterials vapeMaterialList[];

#endif