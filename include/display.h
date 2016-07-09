#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "globals.h"
#include "display_helper.h"

void updateScreen();
void displayCharging();
void getString(char *buff, char *state);
void showMenu();

uint8_t* getBatteryIcon();

#endif
