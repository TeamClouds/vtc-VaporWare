#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <Atomizer.h>

struct globals {
	uint16_t volts;
	uint16_t newVolts;
	uint32_t watts;
	Atomizer_Info_t atomInfo;
	uint8_t fire;
	uint8_t fireTimer;
	uint8_t minus;
	uint8_t plus;
	uint8_t buttonCnt;
	uint8_t vapeCnt;
	uint8_t whatever;
};

struct settings {
	uint8_t mode;
};

extern struct settings s;

int load_settings(void);

#endif
