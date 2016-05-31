#ifndef __DATFLASH_H
#define __DATFLASH_H

#include <stdint.h>

/*

    I cannot express the importance of knowing what you're
    doing when you change these values.  If you alter these
    struct, or the values below, and don't get it right, bad
    bad bad things will happen.

    If you've forked another rom off of this and reach the
    point that you're changing this stuff, PLEASE incriment
    one of the LEFT 24 BITS.  I will maintain a list of known
    prefixes here:

    0x001xxx - Team Clouds















  */



// 24-bit structure magic number. Upper 8 bits must be zero.
// NEVER CHANGE THIS NUMBER UNLESS YOU'RE FORKING A NEW ROM,
// and if you are, only  vvv  <~Those are the bytes to change
#define MYSTRUCT_MAGIC 0x001000
// 2nd set of magic for stuff that changes all the time.
// NEVER CHANGE THIS NUMBER
#define MYSTRUCT_FREQ_MAGIC 0x500

#define BASE_VER 3
#define FREQ_VER 3

#define BASE_UNSUPPORTED 2
#define FREQ_UNSUPPORTED 2
/*
   TODO for v3:
   screenBrightness: uint8_t
   displayTemperature: int16_t
   targetTemperature: int16_t
 */
/*
 * NEVER EVER CHANGE THESE.  If something needs changed, copy/paste, re-name
 * and add an upgrade path.
 */
#define SETTINGS_V3 (MYSTRUCT_MAGIC + 3)
struct baseSettings_3 {
    int32_t pidP;
    int32_t pidI;
    int32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint16_t screenTimeout;
    uint16_t fadeInTime;
    uint16_t fadeOutTime;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
    uint16_t tcr;
    uint8_t flipOnVape;
    uint8_t invertDisplay;
    uint8_t screenBrightness;
};



#define FREQ_SETTINGS_V3 (MYSTRUCT_MAGIC + MYSTRUCT_FREQ_MAGIC + 3)
struct freqSettings_3 {
    int16_t displayTemperature;
    int16_t targetTemperature;
    uint32_t targetWatts;
    uint16_t targetVolts;
    uint8_t mode;
    uint8_t baseFromUser;
    int16_t baseTemp;
    uint16_t baseRes;
};


#define SETTINGS_MAX SETTINGS_V3
#define SETTINGS_VCNT (BASE_VER - BASE_UNSUPPORTED)

#define FREQ_SETTINGS_MAX FREQ_SETTINGS_V3
#define FREQ_SETTINGS_VCNT (FREQ_VER - FREQ_UNSUPPORTED)

int readSettings();
int writeSettings();
int defaultSettings();

#ifdef WITHFLASHDAMAGESUPPORT
void makeDFInvalid();
void eraseDF();
#endif

#endif

#if 0
/*
 * Versions Older than 2 suffered a dataflash bug.  The defines are left here
 * only for posterity.  Upgrade paths have been removed
 */
#define SETTINGS_V2 (MYSTRUCT_MAGIC + 2)
struct DONOTUSE_baseSettings_2 {
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint16_t screenTimeout;
    uint8_t fadeInTime;
    uint8_t fadeOutTime;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
    uint16_t tcr;
    uint8_t flipOnVape;
    uint8_t invertDisplay;
    uint32_t screenBrightness;
};

#define SETTINGS_V1 (MYSTRUCT_MAGIC + 1)
struct DONOTUSE_baseSettings_1 {
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint16_t screenTimeout;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
};

#define FREQ_SETTINGS_V2 (MYSTRUCT_MAGIC + MYSTRUCT_FREQ_MAGIC + 2)
struct DONOTUSE_freqSettings_2 {
    uint32_t displayTemperature;
    uint32_t targetTemperature;
    uint32_t targetWatts;
    uint16_t targetVolts;
    uint8_t mode;
    uint8_t baseFromUser;
    int16_t baseTemp;
    uint16_t baseRes;
};

#define FREQ_SETTINGS_V1 (MYSTRUCT_MAGIC + MYSTRUCT_FREQ_MAGIC + 1)
struct DONOTUSE_freqSettings_1 {
    uint32_t displayTemperature;
    uint32_t targetTemperature;
    uint8_t mode;
};
#endif