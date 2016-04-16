#ifndef __MODE_VOLT_H
#define __MODE_VOLT_H
#include "main.h"

void voltFire(void);
void voltUp(void);
void voltDown(void);

struct vapeMode variableVoltage = {
    .index = 1,
    .controlType = VOLT_CONTROL,
    .fire = &voltFire,
    .increase = &voltUp,
    .decrease = &voltDown,
};
#endif
