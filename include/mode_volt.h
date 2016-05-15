#ifndef __MODE_VOLT_H
#define __MODE_VOLT_H

#include <stdint.h>

void voltInit(void);
void voltFire(void);
void voltUp(void);
void voltDown(void);
void voltDisplay(uint8_t atomizerOn);
void voltBottomDisplay(uint8_t atomizerOn);

struct vapeMode variableVoltage = {
    .index = 1,
    .controlType = VOLT_CONTROL,
    .name = "Voltage",
    .supportedMaterials = KANTHAL | NICKEL | TITANIUM | STAINLESS,
    .fire = &voltFire,
    .init = &voltInit,
    .increase = &voltUp,
    .decrease = &voltDown,
    .maxSetting = ATOMIZER_MAX_VOLTS,
	.display = &voltDisplay,
	.bottomDisplay = &voltBottomDisplay,
};
#endif
