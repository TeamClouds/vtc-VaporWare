#ifndef __MATERIALS_H
#define __MATERIALS_H
enum {
    KANTHAL = 1 << 0,
    NICKEL = 1 << 1,
    TITANIUM = 1 << 2,
    STAINLESS = 1 << 3,
    MAX_MATERIAL
};
#define MATERIAL_COUNT 4

struct vapeMaterials {
    int8_t typeMask;
    char name[3];
    uint16_t tcr;
};
#endif