#include "communication.h"
#include "mode.h"
#include "globals.h"
#include "debug.h"
#include "helper.h"
#include "settings.h"
#include "materials.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void Communication_Init() {
    // We may want to only init this if the user tells us it's cool.

    // The virtual COM port is not initialized by default.
    // To initialize it, follow those steps:
    // 1) Unlock system control registers.
    SYS_UnlockReg();
    // 2) Initialize the virtual COM port.
    USB_VirtualCOM_Init();
    // 3) Lock system control registers.
    SYS_LockReg();
}

void Communication_Command(char *buffer) {
    char response[4];
    response[0] = '-';
    response[1] = buffer[0];
    response[2] = '\r';
    response[3] = '\n';
    switch(buffer[0]) {
    case '@':
        response[0] = '$';
        USB_VirtualCOM_SendString("AT HOME YOU ARE\r\n");
        break;
    case 'A':
        updateAtomizer(buffer, response);
        break;
    case 'a':
        dumpAtomizer(buffer, response);
        break;
    case 'S':
        updateSettings(buffer, response);
        break;
    case 's':
        dumpSettings(buffer, response);
        break;
    case 'U':
        SYS_UnlockReg();
        SYS_CLEAR_RST_SOURCE(SYS_RSTSTS_PORF_Msk | SYS_RSTSTS_PINRF_Msk);
        FMC_SELECT_NEXT_BOOT(1);
        NVIC_SystemReset();
        break;
    default:
        response[0] = '~';
        break;
    }
    USB_VirtualCOM_SendString(response);
}

// Lower levels assume 'C' is at INDEX 0, so don't change it
struct tempScale tempScaleType[] = {
    {
      .display = "C",
      .max = 320,
      .min = 0,
      .def = 200,
    },
    {
      .display = "F",
      .max = 600,
      .min = 0,
      .def = 400,
    },
    {
      .display = "K",
      .max = 880,
      .min = 280,
      .def = 680,
    },
    {
        .display = "\0",
    }
};
/* tempScaleType has a 'sentinel' at the end, so -1 */
uint8_t tempScaleCount = sizeof(tempScaleType)/sizeof(struct tempScale) - 1;


int8_t parseUInt32(char *V, const char *C, char *R, uint32_t M, uint32_t m, uint32_t *o) {
    char *endptr;
    errno = 0;
    uint32_t val32 = strtoul(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        writeUsb("INFO,%s not valid %s\r\n", V, C);
        return 1;
    }
    
    *o = val32;
    R[0] = '$';
    return 0;
}

int8_t parseInt32(char *V, const char *C, char *R, int32_t M, int32_t m, int32_t *o) {
    char *endptr;
    errno = 0;
    int32_t val32 = strtol(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        writeUsb("INFO,%s not valid %s\r\n", V, C);
        return 1;
    }
    
    *o = val32;
    R[0] = '$';
    return 0;
}

int8_t parseUInt16(char *V, const char *C, char *R, uint16_t M, uint16_t m, uint16_t *o) {
    uint32_t val32;
    if (parseUInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFFFF;
    return 0;
}

int8_t parseInt16(char *V, const char *C, char *R, int16_t M, int16_t m, int16_t *o) {
    int32_t val32;
    if (parseInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFFFF;
    return 0;
}

int8_t parseUInt8(char *V, const char *C, char *R, uint8_t M, uint8_t m, uint8_t *o) {
    uint32_t val32;
    if (parseUInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFF;
    return 0;
}

int8_t parseInt8(char *V, const char *C, char *R, int8_t M, int8_t m, int8_t *o) {
    int32_t val32;
    if (parseInt32(V,C,R,M,m,&val32))
        return 1;
    *o = val32 & 0xFF;
    return 0;
}

int8_t isSetting(char *value, const char *setting) {
    if (strlen(value) != strlen(setting))
        return 0;

    return strncmp(value, setting, strlen(setting)) == 0;
}

void updateSettings(char *buffer, char *response) {
    char *setting;
    char *value;
    uint8_t tu8 = 0;
    uint16_t tu16 = 0;
    uint32_t tu32 = 0;
    //int8_t t8 = 0;
    int16_t t16 = 0;
    int32_t t32 = 0;
    const char delim = ',';

    strtok(buffer, &delim); // eat the 'S'
    setting = strtok(NULL, &delim);
    value = strtok(NULL, &delim);
    if (!setting || !value) {
        response[0] = '~';
        return;
    }


    /* fromRom is useless to set directly */
    if (isSetting(setting,"mode")) {
        if (parseUInt8(value, "mode", response, MAX_CONTROL, 0, &tu8))
            return;
        modeSet(tu8);
    } else if (isSetting(setting, "screenTimeout")) {
        if (parseUInt16(value, "screenTimeout", response, SCREENMAXTIMEOUT, SCREENMINTIMEOUT, &tu16))
            return;
        screenTimeoutSet(tu16);
    } else if (isSetting(setting, "displayTemperature")) {
        struct tempScale *t = &tempScaleType[s.tempScaleTypeIndex];
        if (parseUInt32(value, "displayTemperature", response, t->max, t->min, &tu32))
            return;
        displayTemperatureSet(tu32);
        targetTemperatureSet(displayToC(tu32));
    } else if (isSetting(setting, "targetTemperature")) {
        struct tempScale *t = &tempScaleType[0];
        if (parseUInt32(value, "targetTemperature", response, t->max, t->min, &tu32))
            return;
        targetTemperatureSet(tu32);
        displayTemperatureSet(CToDisplay(tu32));
    } else if (isSetting(setting, "targetWatts")) {
        if (parseUInt32(value, "targetWatts", response, MAXWATTS, MINWATTS, &tu32))
            return;
        targetWattsSet(tu32);
    } else if (isSetting(setting, "targetVolts")) {
        if (parseUInt16(value, "targetVolts", response, MAXVOLTS, MINVOLTS, &tu16))
            return;
        targetVoltsSet(tu16);
    } else if (isSetting(setting, "materialIndex")) {
        if (parseUInt8(value, "materialIndex", response, vapeMaterialsCount, 0, &tu8))
            return;
        materialIndexSet(tu8);
    } else if (isSetting(setting, "tempScaleTypeIndex")) {
        if (parseUInt8(value, "tempScaleTypeIndex", response, tempScaleCount, 0, &tu8))
            return;
        tempScaleTypeIndexSet(tu8);
        targetTemperatureSet(tempScaleType[s.tempScaleTypeIndex].def);
    } else if (isSetting(setting, "pidP")) {
        if (parseUInt32(value, "pidP", response, MAXPID, MINPID, &tu32))
            return;
        pidPSet(tu32);
    } else if (isSetting(setting, "pidI")) {
        if (parseUInt32(value, "pidI", response, MAXPID, MINPID, &tu32))
            return;
        pidISet(tu32);
    } else if (isSetting(setting, "pidD")) {
        if (parseUInt32(value, "pidD", response, MAXPID, MINPID, &tu32))
            return;
        pidDSet(tu32);
    } else if (isSetting(setting, "initWatts")) {
        if (parseInt32(value, "initWatts", response, MAXWATTS, MINWATTS, &t32))
            return;
        initWattsSet(t32);
    } else if (isSetting(setting, "pidSwitch")) {
        if (parseInt32(value, "pidSwitch", response, STEMPMAX, STEMPMIN, &t32))
            return;
        pidSwitchSet(t32);
    } else if (isSetting(setting, "invertDisplay")) {
        if (parseUInt8(value, "invertDisplay", response, 1, 0, &tu8))
            return;
        invertDisplaySet(tu8);
    } else if (isSetting(setting, "flipOnVape")) {
        if (parseUInt8(value, "flipOnVape", response, 1, 0, &tu8))
            return;
        flipOnVapeSet(tu8);
    } else if (isSetting(setting, "tcr")) {
        if (parseUInt16(value, "tcr", response, 1, 0, &tu16))
            return;
        tcrSet(tu16);
    } else if (isSetting(setting, "baseFromUser")) {
        if (parseUInt8(value, "baseFromUser", response, MAXFROMROM, AUTORES, &tu8))
            return;
        baseFromUserSet(tu8);
    } else if (isSetting(setting, "baseTemp")) {
        if (parseInt16(value, "baseTemp", response, BTEMPMAX, BTEMPMIN, &t16))
            return;
        baseTempSet(t16);
    } else if (isSetting(setting, "baseRes")) {
        if (parseUInt16(value, "baseRes", response, BRESMIN, BRESMIN, &tu16))
            return;
        baseResSet(t16);
    } else if (isSetting(setting, "screenBrightness")) {
        if (parseUInt32(value, "screenBrightness", response, BRESMIN, BRESMIN, &tu32))
            return;
        screenBrightnessSet(tu32);


    } else if (isSetting(setting, "stealthMode")) {
        if (parseUInt8(value, "stealthMode", response, 1, 0, &s.stealthMode))
            return;
    } else if (isSetting(setting, "vsetLock")) {
        if (parseUInt8(value, "vsetLock", response, 1, 0, &s.vsetLock))
            return;
    } else if (isSetting(setting, "dumpPids")) {
        if (parseUInt8(value, "dumpPids", response, 1, 0, &s.dumpPids))
            return;
    } else if (isSetting(setting, "tunePids")) {
        if (parseUInt8(value, "tunePids", response, 1, 0, &s.tunePids))
            return;
    }

    if (response[0] == '$') {
        writeUsb("INFO,setting %s to %s\r\n", setting, value);
    }
}


void dumpSettings(char *buffer, char *response) {
    writeUsb("INFO,dumpSettings\r\n");
    writeUsb("setting,%s,%i\r\n","fromRom",s.fromRom);
    writeUsb("setting,%s,%i\r\n","mode",s.mode);
    writeUsb("setting,%s,%i\r\n","screenTimeout",s.screenTimeout);
    writeUsb("setting,%s,%i\r\n","fadeInTime",s.fadeInTime);
    writeUsb("setting,%s,%i\r\n","fadeOutTime",s.fadeOutTime);
    writeUsb("setting,%s,%ld\r\n","displayTemperature",s.displayTemperature);
    writeUsb("setting,%s,%ld\r\n","targetTemperature",s.targetTemperature);
    writeUsb("setting,%s,%ld\r\n","targetWatts",s.targetWatts);
    writeUsb("setting,%s,%ld\r\n","targetVolts",s.targetVolts);
    writeUsb("setting,%s,%i\r\n","materialIndex",s.materialIndex);
    writeUsb("setting,%s,%i\r\n","tempScaleTypeIndex",s.tempScaleTypeIndex);
    writeUsb("setting,%s,%ld\r\n","pidP",s.pidP);
    writeUsb("setting,%s,%ld\r\n","pidI",s.pidI);
    writeUsb("setting,%s,%ld\r\n","pidD",s.pidD);
    writeUsb("setting,%s,%ld\r\n","initWatts",s.initWatts);
    writeUsb("setting,%s,%ld\r\n","pidSwitch",s.pidSwitch);
    writeUsb("setting,%s,%ld\r\n","invertDisplay",s.invertDisplay);
    writeUsb("setting,%s,%ld\r\n","flipOnVape",s.flipOnVape);
    writeUsb("setting,%s,%ld\r\n","tcr",s.tcr);
    writeUsb("setting,%s,%ld\r\n","baseFromUser",s.baseFromUser);
    writeUsb("setting,%s,%ld\r\n","baseTemp",s.baseTemp);
    writeUsb("setting,%s,%ld\r\n","baseRes",s.baseRes);
    writeUsb("setting,%s,%ld\r\n","screenBrightness",s.screenBrightness);

    writeUsb("setting,%s,%ld\r\n","stealthMode",s.stealthMode);
    writeUsb("setting,%s,%ld\r\n","vsetLock",s.vsetLock);
    writeUsb("setting,%s,%i\r\n","dumpPids",s.dumpPids);
    writeUsb("setting,%s,%i\r\n","tunePids",s.tunePids);
}

void updateAtomizer(char *buffer, char *response) {
    char *setting;
    char *value;
    const char delim = ',';

    strtok(buffer, &delim); // eat the 'S'
    setting = strtok(NULL, &delim);
    value = strtok(NULL, &delim);
    if (!setting || !value) {
        response[0] = '~';
        return;
    }

    if (isSetting(setting,"baseResistance")) {
        if (parseUInt16(value, "baseResistance", response, 3500, 0, &g.atomInfo.baseResistance))
            return;
    } else if (isSetting(setting,"baseTemperature")) {
        if (parseUInt8(value, "baseTemperature", response, 100, 0, &g.atomInfo.baseTemperature))
            return;
    }

    if (response[0] == '$') {
        writeUsb("INFO,setting atomInfo.%s to %s\r\n", setting, value);
    }
}

void dumpAtomizer(char *buffer, char *response) {
    Atomizer_Info_t *a = &g.atomInfo;
    writeUsb("INFO,dumpSettings\r\n");
    writeUsb("atomInfo,%s,%i\r\n","voltage",a->voltage);
    writeUsb("atomInfo,%s,%i\r\n","resistance",a->resistance);
    writeUsb("atomInfo,%s,%i\r\n","current",a->current);
    writeUsb("atomInfo,%s,%i\r\n","baseResistance",a->baseResistance);
    writeUsb("atomInfo,%s,%d\r\n","baseTemperature",a->baseTemperature);
}
