#include "globals.h"
#include "settings.h"
#include "materials.h"

struct globals g = { };

volatile struct globalVols gv = {
    .fireButtonPressed = 0,
};
struct settings s = { };

struct vapeMaterials vapeMaterialList[] = {
    {
     .typeMask = KANTHAL,
     .name = "KA",
     .tcr = 0,
     },
    {
     .typeMask = NICKEL,
     .name = "NI",
     .tcr = 620,
     },
    {
     .typeMask = TITANIUM,
     .name = "TI",
     .tcr = 350,
     },
    {
     .typeMask = STAINLESS,
     .name = "SS",
     .tcr = 105,
     },
};
