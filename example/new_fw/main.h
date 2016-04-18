#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Atomizer.h>
#include <stdint.h>
#include <math.h>

// TODO: Make this match reality later
#define MODE_COUNT 8

/* Vape Mode */
enum {
	WATT_CONTROL,
	VOLT_CONTROL,
	TEMP_CONTROL,
	MAX_CONTROL
};

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

struct vapeMode {
	int8_t index;
	int8_t controlType;
	char name[8];
	int8_t supportedMaterials;
	uint32_t maxSetting;
	void (*init)(void);
	void (*fire)(void);
	void (*increase)(void);
	void (*decrease)(void);
};

void getModesByMaterial(uint8_t materialMask, int8_t *modes, int8_t *cnt);

/* Settings */
struct settings {
	uint8_t mode;
	uint16_t screenTimeout;
	uint32_t targetTemperature;
	uint8_t materialIndex;
	struct vapeMaterials *material;
	uint8_t tempScaleType;
};

extern const char *tempScaleType[];

extern struct settings s;

int load_settings(void);


/* Helpers */
uint16_t wattsToVolts(uint32_t watts, uint16_t res);
uint32_t voltsToWatts(uint16_t volts, uint16_t res);

/* Ugly Globals */
struct globals {
	Atomizer_Info_t atomInfo;
	struct vapeMode *vapeModes[MODE_COUNT];
	uint32_t watts;
	uint16_t volts;
	uint16_t newVolts;
	uint8_t charging;
	uint8_t fire;
	uint8_t fireTimer;
	uint8_t minus;
	uint8_t plus;
	uint8_t vapeCnt;
	uint8_t whatever;
	uint32_t maxTemp;
	uint32_t minTemp;
};
extern struct globals g;

struct globalVols {
	volatile uint8_t fireButtonPressed;
	volatile uint8_t screenState;
	volatile uint8_t buttonCnt;
	volatile uint8_t shouldShowMenu;
	volatile int8_t screenOffTimer;
};
extern volatile struct globalVols gv;

/* Display */
void updateScreen(struct globals *g);
void getString(char *buff, char *state);
void showMenu();

#endif
