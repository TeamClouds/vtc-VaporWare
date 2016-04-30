#ifndef __MODES_H
#define __MODES_H

#define MODE_COUNT 8

enum {
    WATT_CONTROL,
    VOLT_CONTROL,
    TEMP_CONTROL,
    MAX_CONTROL
};

struct vapeMode {
    int8_t index;
    int8_t controlType;
    char name[8];
    int8_t supportedMaterials;
    uint32_t maxSetting;
    void (*init) (void);
    void (*fire) (void);
    void (*increase) (void);
    void (*decrease) (void);
    void (*display) (uint8_t atomizerOn);
    void (*bottomDisplay) (uint8_t atomizerOn);
};

void getModesByMaterial(uint8_t materialMask, int8_t * modes,
			int8_t * cnt);
void setVapeMode(int newMode);
void setVapeMaterial(int index);
#endif
