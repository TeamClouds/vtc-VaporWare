#ifndef __DATFLASH_H
#define __DATFLASH_H

#include <stdint.h>

// 24-bit structure magic number. Upper 8 bits must be zero.
#define MYSTRUCT_MAGIC 0x001000
// 2nd set of magic for stuff that changes all the time.
#define MYSTRUCT_FREQ_MAGIC 0x500
/*
 * NEVER EVER CHANGE THESE.  If something needs changed, copy/paste, re-name
 * and add an upgrade path.  
 */

#define SETTINGS_V1 (MYSTRUCT_MAGIC + 0x0)
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

#define FREQ_SETTINGS_V1 (MYSTRUCT_MAGIC + MYSTRUCT_FREQ_MAGIC + 0x0)
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