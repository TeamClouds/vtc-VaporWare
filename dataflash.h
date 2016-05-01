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

#define BASE_VER 1
#define FREQ_VER 1
/*
 * NEVER EVER CHANGE THESE.  If something needs changed, copy/paste, re-name
 * and add an upgrade path.  
 */

#define SETTINGS_V1 (MYSTRUCT_MAGIC + BASE_VER)
#define SETTINGS_MAX SETTINGS_V1
#define SETTINGS_VCNT (SETTINGS_MAX - MYSTRUCT_MAGIC + 1)
struct baseSettings_1 {
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint16_t screenTimeout;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
};

#define FREQ_SETTINGS_V1 (MYSTRUCT_MAGIC + MYSTRUCT_FREQ_MAGIC + FREQ_VER)
#define FREQ_SETTINGS_MAX FREQ_SETTINGS_V1
#define FREQ_SETTINGS_VCNT (FREQ_SETTINGS_MAX - MYSTRUCT_MAGIC - MYSTRUCT_FREQ_MAGIC + 1)
struct freqSettings_1 {
    uint32_t displayTemperature;
    uint32_t targetTemperature;
    uint8_t mode;
};

int readSettings();
int writeSettings();
int defaultSettings();
#endif
