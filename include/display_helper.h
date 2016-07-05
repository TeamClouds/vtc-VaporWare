#ifndef __DISPLAYHELPER_H
#define __DISPLAYHELPER_H

#include <stdint.h>

void printNumber(char *buff, int32_t temperature);
void getPercent(char *buff, int32_t percent);
void getString(char *buff, char *state);
void getFloating(char *buff, int32_t floating);
void getFloatingTenth(char *buff, uint32_t floating);
void formatThousandths(char *formatted, int32_t value);

void buildRow(uint8_t y, uint8_t* icon, void (*parsingCallback)(char* buff, int32_t value), uint32_t value);
void buildItem(uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint8_t* icon, void (*parsingCallback)(char* buff, int32_t value), uint32_t value);


#endif
