#ifndef __MODE_TEMP_H
#define __MODE_TEMP_H

void tempInit(void);
void tempFire(void);
void tempUp(void);
void tempDown(void);
void tempDisplay(uint8_t atomizerOn);
void tempBottomDisplay(uint8_t atomizerOn);
void showTempMenu();

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
	.display = &tempDisplay,
	.bottomDisplay = &tempBottomDisplay,
	.settings = &showTempMenu,
};
#endif
