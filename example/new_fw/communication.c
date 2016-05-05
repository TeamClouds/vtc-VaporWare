#include "communication.h"
#include "mode.h"
#include "globals.h"
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
    char buff[63];
    errno = 0;
    uint32_t val32 = strtoul(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        siprintf(buff, "INFO,%s not valid %s\r\n", V, C);
        USB_VirtualCOM_SendString(buff);
        return 1;
    }
    
    *o = val32;
    R[0] = '$';
    return 0;
}

int8_t parseInt32(char *V, const char *C, char *R, int32_t M, int32_t m, int32_t *o) {
    char *endptr;
    char buff[63];
    errno = 0;
    int32_t val32 = strtol(V, &endptr, 10);
    if (V == endptr) {
        R[0] = '~';
        return 1;
    }

    if (errno || val32 < m || val32 > M) {
        R[0] = '~';
        siprintf(buff, "INFO,%s not valid %s\r\n", V, C);
        USB_VirtualCOM_SendString(buff);
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

void updateSettings(char *buffer, char *response) {
    char buff[63];
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

    if (strncmp(setting,"mode",4) == 0) {
        if (parseUInt8(value, "mode", response, MAX_CONTROL, 0, &s.mode))
            return;
    } else if (strncmp(setting, "screenTimeout", 13) == 0) {
        if (parseUInt16(value, "screenTimeout", response, 600, 0, &s.screenTimeout))
            return;
    } else if (strncmp(setting, "targetTemperature", 17) == 0) {
        struct tempScale *t = &tempScaleType[s.tempScaleTypeIndex];
        uint32_t ttemp;
        if (parseUInt32(value, "targetTemperature", response, t->max, t->min, &ttemp))
            return;
        s.displayTemperature = ttemp;
        s.targetTemperature = displayToC(s.displayTemperature);
    } else if (strncmp(setting, "materialIndex", 13) == 0) {
        uint8_t tindex = 0;
        if (parseUInt8(value, "materialIndex", response, vapeMaterialsCount, 0, &tindex))
            return;
        materialIndexSet(tindex);
    } else if (strncmp(setting, "tempScaleTypeIndex", 13) == 0) {
        uint8_t tindex;
        if (parseUInt8(value, "tempScaleTypeIndex", response, tempScaleCount, 0, &tindex))
            return;
        s.tempScaleTypeIndex = tindex;
        s.targetTemperature = tempScaleType[s.tempScaleTypeIndex].def;
    } else if (strncmp(setting, "pidP", 4) == 0) {
        if (parseUInt32(value, "pidP", response, 0xFFFFFFFF, 0, &s.pidP))
            return;
    } else if (strncmp(setting, "pidI", 4) == 0) {
        if (parseUInt32(value, "pidI", response, 0xFFFFFFFF, 0, &s.pidI))
            return;
    } else if (strncmp(setting, "pidD", 4) == 0) {
        if (parseUInt32(value, "pidD", response, 0xFFFFFFFF, 0, &s.pidD))
            return;
    } else if (strncmp(setting, "initWatts", 9) == 0) {
        if (parseInt32(value, "initWatts", response, 60000, 0, &s.initWatts))
            return;
    } else if (strncmp(setting, "pidSwitch", 8) == 0) {
        if (parseInt32(value, "pidSwitch", response, 600, -600, &s.pidSwitch))
            return;
    } else if (strncmp(setting, "dumpPids", 8) == 0) {
        if (parseUInt8(value, "dumpPids", response, 1, 0, &s.dumpPids))
            return;
    }

    if (response[0] == '$') {
        siprintf(buff, "INFO,setting %s to %s\r\n", setting, value);
        USB_VirtualCOM_SendString(buff);
    }
}


void dumpSettings(char *buffer, char *response) {
    char buff[63];
    USB_VirtualCOM_SendString("INFO,dumpSettings\r\n");
    siprintf(buff, "setting,%s,%i\r\n","fromRom",s.fromRom);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","mode",s.mode);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","screenTimeout",s.screenTimeout);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","targetTemperature",s.targetTemperature);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","materialIndex",s.materialIndex);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","tempScaleTypeIndex",s.tempScaleTypeIndex);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidP",s.pidP);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidI",s.pidI);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","pidD",s.pidD);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%ld\r\n","initWatts",s.initWatts);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "setting,%s,%i\r\n","dumpPids",s.dumpPids);
    USB_VirtualCOM_SendString(buff);
}

void updateAtomizer(char *buffer, char *response) {
    char buff[63];
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

    if (strncmp(setting,"base_resistance",15) == 0) {
        if (parseUInt16(value, "base_resistance", response, 3500, 0, &g.baseRes))
            return;
    } else if (strncmp(setting,"base_temperature", 16) == 0) {
        if (parseInt16(value, "base_temperature", response, 200, 0, &g.baseTemp))
            return;
    } else if (strncmp(setting,"tcr", 3) == 0) {
        if (parseUInt16(value, "tcr", response, 1000, 10, &g.tcr))
            return;
    }
    if (response[0] == '$') {
        siprintf(buff, "INFO,setting atomInfo.%s to %s\r\n", setting, value);
        USB_VirtualCOM_SendString(buff);
    }
}

void dumpAtomizer(char *buffer, char *response) {
    char buff[63];
    Atomizer_Info_t *a = &g.atomInfo;
    USB_VirtualCOM_SendString("INFO,dumpSettings\r\n");
    siprintf(buff, "atomInfo,%s,%i\r\n","voltage",a->voltage);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","resistance",a->resistance);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","base_resistance",g.baseRes);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%d\r\n","base_temperature",g.baseTemp);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","current",a->current);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%d\r\n","temperature",g.curTemp);
    USB_VirtualCOM_SendString(buff);
    siprintf(buff, "atomInfo,%s,%i\r\n","tcr",g.tcr);
    USB_VirtualCOM_SendString(buff);
 
}
