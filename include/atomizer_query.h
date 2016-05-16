#ifndef __ATOMIZERQUERY_H
#define __ATOMIZERQUERY_H

void askUserAboutTheAttomizer();
uint8_t newReading(uint16_t oldRes, uint8_t oldTemp, uint16_t *newRes, uint8_t *newTemp);

#endif
