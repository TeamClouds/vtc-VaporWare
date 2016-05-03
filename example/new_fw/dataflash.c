#include <Dataflash.h>

#include <stdio.h>

#include "dataflash.h"
#include "debug.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"



void DFSettingsToGlobals(struct baseSettings_1 *b, struct freqSettings_1 *f, uint8_t isDef) {
    s.fromRom = isDef;

    // Must set material before temperature stuff
    materialIndexSet(b->materialIndex);
    // Must set scale type before temperature
    // dependant values
    tempScaleTypeIndexSet(b->tempScaleTypeIndex);
    displayTemperatureSet(f->displayTemperature);
    targetTemperatureSet(f->targetTemperature);

    screenTimeoutSet(b->screenTimeout);
    pidPSet(b->pidP);
    pidISet(b->pidI);
    pidDSet(b->pidD);
    initWattsSet(b->initWatts);
    pidSwitchSet(b->pidSwitch);

    // Setting mode may depend on the rest of the settings being setup so just set it last
    modeSet(f->mode);
}

void globalsToDFSettings(struct baseSettings_1 *b, struct freqSettings_1 *f) {
    f->mode = s.mode;
    b->screenTimeout = s.screenTimeout;
    f->displayTemperature = s.displayTemperature;
    f->targetTemperature = s.targetTemperature;
    b->materialIndex = s.materialIndex;
    b->tempScaleTypeIndex = s.tempScaleTypeIndex;
    b->pidP = s.pidP;
    b->pidI = s.pidI;
    b->pidD = s.pidD;
    b->initWatts = s.initWatts;
    b->pidSwitch = s.pidSwitch;
}

void default_base(struct baseSettings_1 *b) {
    b->pidP = DEFPIDP;
    b->pidI = DEFPIDI;
    b->pidD = DEFPIDD;
    b->initWatts = DEFWATTS;
    b->pidSwitch = STEMPDEF;
    b->screenTimeout = SCREENDEFAUTLTIMEOUT;
    b->materialIndex = DEFAULTMATERIAL;
    b->tempScaleTypeIndex = DEFAULTTEMPSCALE;
}

void default_freq(struct freqSettings_1 *f) {
    f->mode = DEFAULTMODE;
    f->displayTemperature = tempScaleType[DEFAULTTEMPSCALE].def;
    f->targetTemperature = displayToC(tempScaleType[DEFAULTTEMPSCALE].def);
}

int defaultSettings() {
    struct baseSettings_1 b = {};
    struct freqSettings_1 f = {};
    default_base(&b);
    default_freq(&f);
    DFSettingsToGlobals(&b, &f, 0);
    return 0;
}

Dataflash_StructInfo_t base_structInfo_v1 = {
    .magic = SETTINGS_V1,
    .size = sizeof(struct baseSettings_1),
};

Dataflash_StructInfo_t freq_structInfo_v1 = {
    .magic = FREQ_SETTINGS_V1,
    .size = sizeof(struct freqSettings_1),
};

#define CURBASETYPE baseSettings_1
#define CURFREQTYPE freqSettings_1

#define CURBASESTRINFO base_structInfo_v1
#define CURFREQSTRINFO freq_structInfo_v1

int readSettings() {
    struct baseSettings_1 base_v1 = {};
    default_base(&base_v1);

    struct freqSettings_1 freq_v1 = {};
    default_freq(&freq_v1);

#define CURBASESTR base_v1
#define CURFREQSTR freq_v1

    Dataflash_StructInfo_t *structList[SETTINGS_VCNT + FREQ_SETTINGS_VCNT];

    uint32_t magicList[DATAFLASH_STRUCT_MAX_COUNT];
    uint8_t magicCount, i, upgradeCnt = 0, upgraded = 0;
    
    uint8_t gotTargetBase = 0, upgradableBaseVer = 0;
    uint8_t gotTargetFreq = 0, upgradableFreqVer = 0;

    magicCount = Dataflash_GetMagicList(magicList);

    for(i = 0; i < magicCount; i++) {
        switch (magicList[i]) {
            case SETTINGS_V1:
                if (Dataflash_ReadStruct(&base_structInfo_v1, &base_v1)) {
                    gotTargetBase = 1;
                    upgradableBaseVer = upgradableBaseVer < 1 ? 1 : upgradableBaseVer;
                    structList[upgradeCnt++] = &base_structInfo_v1;
                }
                    
                break;
            case FREQ_SETTINGS_V1:
                if (Dataflash_ReadStruct(&freq_structInfo_v1, &freq_v1)) {
                    gotTargetFreq = 1;
                    upgradableFreqVer = upgradableFreqVer < 1 ? 1 : upgradableFreqVer;
                    structList[upgradeCnt++] = &freq_structInfo_v1;
                }
                break;

        }
        if (gotTargetBase && gotTargetFreq)
            break;
    }

    if (!gotTargetBase) {
        if (upgradableBaseVer) {
            switch (upgradableBaseVer) {
                case 1:
                // Add code to upgrade to v2 here, leave fallthrough
                case 2:
                    upgraded = 1;
                    break;
            }
        } else {
            default_base(&CURBASESTR);
            structList[upgradeCnt++] = &CURBASESTRINFO;
        }
    }

    if (!gotTargetFreq) {
        if (upgradableFreqVer) {
            switch (upgradableFreqVer) {
                case 1:
                // Add code to upgrade to v2 here, leave fallthrough
                case 2:
                    upgraded = 1;
                    break;
            }
        } else {
            default_freq(&CURFREQSTR);
            structList[upgradeCnt++] = &CURFREQSTRINFO;
        }
    }


    if (!Dataflash_SelectStructSet(structList, 2)) {
        return defaultSettings();
    }

    DFSettingsToGlobals(&CURBASESTR, &CURFREQSTR, !upgraded && gotTargetBase && gotTargetFreq);
   
    if (!gotTargetBase)
        Dataflash_UpdateStruct(&CURBASESTRINFO, &CURBASESTR);

    if (!gotTargetFreq)
        Dataflash_UpdateStruct(&CURFREQSTRINFO, &CURFREQSTR);

    return 1;
}

int writeSettings() {
    struct CURBASETYPE b;
    struct CURFREQTYPE f;

    globalsToDFSettings(&b,&f);
    if (g.baseSettingsChanged)
        Dataflash_UpdateStruct(&CURBASESTRINFO, &b);
    if (g.freqSettingsChanged)
        Dataflash_UpdateStruct(&CURFREQSTRINFO, &f);
    g.freqSettingsChanged = 0;
    g.baseSettingsChanged = 0;
    g.settingsChanged = 0;
    return 1;
}

#ifdef WITHFLASHDAMAGESUPPORT
void makeDFInvalid() {
    Dataflash_InvalidateStruct(&CURBASESTRINFO);
    Dataflash_InvalidateStruct(&CURFREQSTRINFO);
}

void eraseDF() {
    Dataflash_Erase();
}
#endif
