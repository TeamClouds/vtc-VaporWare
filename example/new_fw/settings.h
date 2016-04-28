#ifndef __SETTINGS_H
#define __SETTINGS_H

// 24-bit structure magic number. Upper 8 bits must be zero.
#define MYSTRUCT_MAGIC 0x001000

struct settings {
	uint32_t magic;
	uint32_t size;
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
};

extern struct settings s;

struct tempScale {
    const char display[2];
    uint32_t max;
    uint32_t min;
    uint32_t def;
};
extern struct tempScale tempScaleType[];

int load_settings(void);
void updateSettings(char *buffer, char *response);
void dumpSettings(char *buffer, char *response);

void updateAtomizer(char *buffer, char *response);
void dumpAtomizer(char *buffer, char *response);

void saveSettings();

#endif