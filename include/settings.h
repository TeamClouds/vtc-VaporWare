#ifndef __SETTINGS_H
#define __SETTINGS_H
#include "defaults.h"

enum {
    AUTORES = 0,
    USERSET = 1,
    USERLOCK = 2,
    MAXFROMROM = USERLOCK,
};


struct settings {
    uint8_t fromRom;
    uint8_t mode;
    uint16_t screenTimeout;
    uint8_t fadeInTime;
    uint8_t fadeOutTime;
    volatile int32_t displayTemperature;
    volatile int32_t targetTemperature;
    uint32_t targetWatts;
    uint16_t targetVolts;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint8_t invertDisplay;
    uint8_t flipOnVape;
    uint16_t tcr;
    uint8_t baseFromUser;
    int16_t baseTemp;
    uint16_t baseRes;
    uint32_t screenBrightness;


    /* Runtime settings that don't need to  be persisted */
    uint8_t stealthMode;
    uint8_t vsetLock;
    uint8_t dumpPids;
    uint8_t tunePids;
};

extern struct settings s;

struct tempScale {
    char display[2];
    int32_t max;
    int32_t min;
    int32_t def;
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
void displayTemperatureSet(int32_t displayTemperature);
void targetTemperatureSet(int32_t targetTemperature);
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
void baseFromUserSet(uint8_t baseFromuser);
void baseTempSet(int16_t baseTemp);
void baseResSet(uint16_t baseRes);

void screenBrightnessSet(uint8_t brightness);
void stealthModeSet(uint8_t stealthMode);
void vsetLockSet(uint8_t vsetLock);

void fadeInTimeSet(uint8_t fadeInTime);
void fadeOutTimeSet(uint8_t fadeOutTime);
void targetWattsSet(uint32_t targetWatts);
void targetVoltsSet(uint16_t targetVolts);
#endif
