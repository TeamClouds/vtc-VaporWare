#ifndef __MODE_WATT_H
#define __MODE_WATT_H
#include "main.h"

void wattInit(void);
void wattFire(void);
void wattUp(void);
void wattDown(void);

struct vapeMode variableWattage = {
    .index = 0,
    .controlType = WATT_CONTROL,
    .name = "Wattage",
    .supportedMaterials = KANTHAL | NICKEL | TITANIUM | STAINLESS,
    .init = &wattInit,
    .fire = &wattFire,
    .increase = &wattUp,
    .decrease = &wattDown,
    .maxSetting = 75000,
};
#endif
