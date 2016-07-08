#include "globals.h"
#include "settings.h"
#include "materials.h"
#include <stdio.h>

struct globals g = { };

volatile struct globalVols gv = {
    .fireButtonPressed = 0,
};

struct settings s = { };
