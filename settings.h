#ifndef __SETTINGS_H
#define __SETTINGS_H
#include "defaults.h"



struct settings {
    uint8_t fromRom;
    uint8_t mode;
    uint16_t screenTimeout;
    volatile uint32_t displayTemperature;
    volatile uint32_t targetTemperature;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint8_t dumpPids;
    uint8_t tunePids;
/* Adding in V2 of dataflash */
    uint8_t invertDisplay;
    uint8_t flipOnVape;
    uint16_t tcr;
    int16_t baseTemp;
    uint16_t baseRes;
    uint32_t screenBrightness;
    uint8_t stealthMode;
    uint8_t vsetLock;
};

extern struct settings s;

struct tempScale {
    char display[2];
    uint32_t max;
    uint32_t min;
    uint32_t def;
};
// Definition in communication.c
extern struct tempScale tempScaleType[];
extern uint8_t tempScaleCount;


int load_settings(void);
void saveSettings();

/* These all set the value passed in if validated, and make any other changes that
   this change might need.  If the passed value is not sane, it sets the default */
void modeSet(uint8_t mode);
void screenTimeoutSet(uint16_t screenTimeout);
void displayTemperatureSet(uint32_t displayTemperature);
void targetTemperatureSet(uint32_t targetTemperature);
void materialIndexSet(uint32_t materialIndex);
void tempScaleTypeIndexSet(uint8_t tempScaleTypeIndex);
void pidPSet(uint32_t pidP);
void pidISet(uint32_t pidI);
void pidDSet(uint32_t pidD);
void initWattsSet(int32_t initWatts);
void pidSwitchSet(int32_t pidSwitch);

void invertDisplaySet(uint8_t invertDisplay);
void flipOnVapeSet(uint8_t flipOnVape);

void tcrSet(uint16_t tcr);
void baseTempSet(int16_t baseTemp);
void baseResSet(uint16_t baseRes);

void screenBrightnessSet(uint8_t brightness);
void stealthModeSet(uint8_t stealthMode);
void vsetLockSet(uint8_t vsetLock);
#endif
