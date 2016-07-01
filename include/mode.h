#ifndef __MODES_H
#define __MODES_H

#include <stdint.h>
#include "menu.h"

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
    void (*increaseFast) (void);
    void (*decreaseFast) (void);
    void (*display) (uint8_t atomizerOn);
    void (*bottomDisplay) (uint8_t atomizerOn);
    const struct menuDefinition *vapeModeMenu;
};

void addVapeMode(struct vapeMode *vm);
void setVapeMode(int newMode);
void setVapeMaterial(int index);

void (*__init) (void);
void (*__vape) (void);
void (*__up) (void);
void (*__down) (void);
void (*__upFast) (void);
void (*__downFast) (void);
#endif
