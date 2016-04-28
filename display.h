#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "globals.h"

void updateScreen(struct globals *g);
void getString(char *buff, char *state);
void showMenu();

#endif