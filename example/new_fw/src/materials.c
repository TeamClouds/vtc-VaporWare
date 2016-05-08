#include <stdint.h>
#include "materials.h"

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
     {
        .name = "\0",
     }
};

/* -1 for the 'sentinel' */
uint8_t vapeMaterialsCount = sizeof(vapeMaterialList)/sizeof(struct vapeMaterials) - 1;