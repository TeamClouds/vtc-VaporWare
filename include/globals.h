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
    uint8_t baseFromUser;
    int32_t m1;
    uint16_t baseRes;
    int32_t m2;
    int16_t baseTemp;
    int32_t m3;
    uint16_t tcr;

    struct vapeMode **vapeModes;
    uint8_t modeCount;

    // These represent state, not settings
    uint32_t watts;
    uint16_t volts;
    int16_t curTemp;
    
    uint8_t charging;
    
    uint8_t batteryPercent;
    uint32_t screenOffTime;
    uint32_t screenFadeInTime;
    int8_t pauseScreenOff;
    uint32_t nextRefresh;
    uint32_t currentBrightness;
    uint8_t settingsChanged;
    uint8_t baseSettingsChanged;
    uint8_t freqSettingsChanged;
    uint32_t writeSettingsAt;
    uint32_t sysSleepAt;
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
    volatile uint8_t sleeping;
};

extern volatile struct globalVols gv;

#endif
