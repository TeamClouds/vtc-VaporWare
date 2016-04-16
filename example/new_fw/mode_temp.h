#ifndef __MODE_TEMP_H
#define __MODE_TEMP_H
#include "main.h"

void tempInit(void);
void tempFire(void);
void tempUp(void);
void tempDown(void);

struct vapeMode variableTemp = {
    .index = 2,
    .controlType = TEMP_CONTROL,
    .name = "Temp",
    .supportedMaterials = NICKEL | TITANIUM | STAINLESS,
    .init = &tempInit,
    .fire = &tempFire,
    .increase = &tempUp,
    .decrease = &tempDown,
    .maxSetting = 600,
};
#endif
