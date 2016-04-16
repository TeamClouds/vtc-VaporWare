#ifndef __MODE_WATT_H
#define __MODE_WATT_H
#include "main.h"

void wattFire(void);
void wattUp(void);
void wattDown(void);

struct vapeMode variableWattage = {
    .index = 0,
    .controlType = VARIABLE_WATTAGE,
    .fire = &wattFire,
    .increase = &wattUp,
    .decrease = &wattDown,
};
#endif
