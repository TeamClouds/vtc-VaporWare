#include <stdint.h>
#include <stdio.h>
#include "globals.h"
#include "debug.h"
#include "materials.h"
#include "settings.h"

void materialIndexSet(uint32_t materialIndex) {
    if (materialIndex < 0) {
        s.materialIndex = DEFAULTMATERIAL;
    } else if (materialIndex >= vapeMaterialsCount) {
        s.materialIndex = DEFAULTMATERIAL;
    } else if (vapeMaterialList[materialIndex].name[0] == '\0') {
        /* Somehow we got to the sentinal, things are probably about to get bad */
        s.materialIndex = DEFAULTMATERIAL;
    } else {
        s.materialIndex = materialIndex;
    }

    g.atomInfo.tcr = vapeMaterialList[materialIndex].tcr;
}

void modeSet(uint8_t mode) {
    if (mode < 0) {
        s.mode = DEFAULTMODE;
    } else if (mode >= modeCount) {
        /* This is currently 'not perfect' as vapemodes is a larger 
           array than we use.  Point to cleanup */
        s.mode = DEFAULTMODE;
    } else if (g.vapeModes[mode] == NULL) {
        s.mode = DEFAULTMODE;
    } else if (g.vapeModes[mode]->supportedMaterials &
        vapeMaterialList[s.materialIndex].typeMask) {
        // TODO: Extract this out as its own helper function in modes
        s.mode = DEFAULTMODE;
    } else {
        s.mode = mode;
    }
    setVapeMode(mode);
}

void screenTimeoutSet(uint16_t screenTimeout) {
    if (screenTimeout < SCREENMINTIMEOUT) {
        s.screenTimeout = SCREENDEFAUTLTIMEOUT;
    } else if (screenTimeout > SCREENMAXTIMEOUT) {
        s.screenTimeout = SCREENDEFAUTLTIMEOUT;
    } else {
        s.screenTimeout = screenTimeout;
    }
}

void tempScaleTypeIndexSet(uint8_t tempScaleTypeIndex) {
    // tempScales currently live in the settings struct.
    if (tempScaleTypeIndex < 0) {
        s.tempScaleTypeIndex = DEFAULTTEMPSCALE;
    } else if (tempScaleTypeIndex >= tempScaleCount) {
        s.tempScaleTypeIndex = DEFAULTTEMPSCALE;
    } else if (tempScaleType[tempScaleTypeIndex].display[0] == '\0') {
        s.tempScaleTypeIndex = DEFAULTTEMPSCALE;   
    } else {
        s.tempScaleTypeIndex = tempScaleTypeIndex;
    }
}

void displayTemperatureSet(uint32_t displayTemperature) {
    if (displayTemperature > tempScaleType[s.tempScaleTypeIndex].max) {
        s.displayTemperature = tempScaleType[s.tempScaleTypeIndex].def;
    } else if (displayTemperature < tempScaleType[s.tempScaleTypeIndex].min) {
        s.displayTemperature = tempScaleType[s.tempScaleTypeIndex].def;
    } else {
        s.displayTemperature = displayTemperature;
    }
    // Todo set target temp based on display?
}

void targetTemperatureSet(uint32_t targetTemperature) {
    // 0 is C and the unit all the low levels use
    if (targetTemperature > tempScaleType[0].max) {
        s.targetTemperature = tempScaleType[0].def;
    } else if (targetTemperature < tempScaleType[0].min) {
        s.targetTemperature = tempScaleType[0].def;
    } else {
        s.targetTemperature = targetTemperature;
    }
}

void pidPSet(uint32_t pidP) {
    if (pidP > MAXPID) {
        s.pidP = DEFPIDP;
    } else if(pidP < MINPID) {
        s.pidP = DEFPIDP;
    } else {
        s.pidP = pidP;
    }
}

void pidISet(uint32_t pidI){
    if (pidI > MAXPID) {
        s.pidI = DEFPIDI;
    } else if(pidI < MINPID) {
        s.pidI = DEFPIDI;
    } else {
        s.pidI = pidI;
    }
}

void pidDSet(uint32_t pidD) {
    if (pidD > MAXPID) {
        s.pidD = DEFPIDD;
    } else if(pidD < MINPID) {
        s.pidD = DEFPIDD;
    } else {
        s.pidD = pidD;
    }
}

void initWattsSet(int32_t initWatts) {
    if (initWatts < MINWATTS) {
        s.initWatts = DEFWATTS;
    } else if (initWatts > MAXWATTS) {
        s.initWatts = DEFWATTS;
    } else {
        s.initWatts = initWatts;
    }
}

void pidSwitchSet(int32_t pidSwitch) {
    if (pidSwitch < STEMPMIN) {
        s.pidSwitch = STEMPDEF;
    } else if (pidSwitch > STEMPMAX) {
        s.pidSwitch = STEMPDEF;
    } else {
        s.pidSwitch = pidSwitch;
    }
}

void invertDisplaySet(uint8_t invertDisplay) {
    if (invertDisplay != 0 &&
        invertDisplay != 1) {
        s.invertDisplay = INVERTDEF;
    } else {
        s.invertDisplay = invertDisplay;
    }
}

void flipOnVapeSet(uint8_t flipOnVape) {
    if (flipOnVape !=0  &&
        flipOnVape != 1) {
        s.flipOnVape = FLIPDEF;
    } else {
        s.flipOnVape = flipOnVape;
    }
}