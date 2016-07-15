#ifndef __DISPLAYHELPER_H
#define __DISPLAYHELPER_H

#include <stdint.h>

void printNumber(char *buff, uint8_t len, int32_t temperature);
void getPercent(char *buff, uint8_t len, int32_t percent);
void getString(char *buff, uint8_t len, char *state);
void getFloating(char *buff, uint8_t len, int32_t floating);
void getFloatingTenth(char *buff, uint8_t len, uint32_t floating);
void formatThousandths(char *formatted, uint8_t len, int32_t value);

void buildRow(uint8_t y, uint8_t* icon, void (*parsingCallback)(char* buff, uint8_t len, int32_t value), uint32_t value);

#endif
