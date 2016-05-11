#ifndef __MATERIALS_H
#define __MATERIALS_H

#include <stdint.h>

enum {
    KANTHAL = 1 << 0,
    NICKEL = 1 << 1,
    TITANIUM = 1 << 2,
    STAINLESS = 1 << 3,
    MAX_MATERIAL
};

struct vapeMaterials {
    int8_t typeMask;
    char name[3];
    uint16_t tcr;
};
extern struct vapeMaterials vapeMaterialList[];
extern uint8_t vapeMaterialsCount;
#endif