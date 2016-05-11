#ifndef __DISPLAYHELPER_H
#define __DISPLAYHELPER_H

#include <stdint.h>

void printNumber(char *buff, uint32_t temperature);
void getPercent(char *buff, uint8_t percent);
void getString(char *buff, char *state);
void getFloating(char *buff, uint32_t floating);
void getFloatingTenth(char *buff, uint32_t floating);

#endif
