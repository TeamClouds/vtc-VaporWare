#ifndef __SETTINGS_H
#define __SETTINGS_H



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
    uint8_t invertDisplay;
    uint8_t flipOnVape;
};

extern struct settings s;

struct tempScale {
    char display[2];
    uint32_t max;
    uint32_t min;
    uint32_t def;
};
extern struct tempScale tempScaleType[];

int load_settings(void);
void saveSettings();

#endif
