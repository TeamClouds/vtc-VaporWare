#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Atomizer.h>
#include <stdint.h>
#include <math.h>

/* Vape Mode */
enum {
	WATT_CONTROL,
	VOLT_CONTROL,
	TEMP_CONTROL,
	MAX_CONTROL
};

struct vapeMode {
	int8_t index;
	int8_t controlType;
	void (*fire)(void);
	void (*increase)(void);
	void (*decrease)(void);
};


/* Settings */
enum {
    VARIABLE_WATTAGE,
    VARIABLE_VOLTAGE,
    MAX_MODE
};

struct settings {
	uint8_t mode;
};

extern struct settings s;

int load_settings(void);


/* Helpers */
uint16_t wattsToVolts(uint32_t watts, uint16_t res);
uint32_t voltsToWatts(uint16_t volts, uint16_t res);

/* Ugly Globals */
struct globals {
	Atomizer_Info_t atomInfo;
	struct vapeMode vapeModes[MAX_MODE];
	uint32_t watts;
	uint16_t volts;
	uint16_t newVolts;
	uint8_t fire;
	uint8_t fireTimer;
	uint8_t minus;
	uint8_t plus;
	uint8_t buttonCnt;
	uint8_t vapeCnt;
	uint8_t whatever;
};

volatile extern int fireButtonPressed;
extern struct globals g;

/* Display */
void updateScreen(struct globals *g);
void showMenu();

#endif
