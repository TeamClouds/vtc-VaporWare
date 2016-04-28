#include <stdint.h>
#include "globals.h"


void getModesByMaterial(uint8_t materialMask, int8_t * modes, int8_t * cnt) {
    int i = 0;
    int j = 0;
    for (;;) {
	if (g.vapeModes[i]
	    && g.vapeModes[i]->supportedMaterials & materialMask) {
	    modes[j] = i;
	    j++;
	    *cnt = j;
	}

	if (!g.vapeModes[i])
	    return;
	i++;
    }
}
