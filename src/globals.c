#include "globals.h"
#include "settings.h"
#include "materials.h"
#include <stdio.h>

struct globals g = {0};

volatile struct globalVols gv = {
    .fireButtonPressed = 0,
};

struct settings s = {0};