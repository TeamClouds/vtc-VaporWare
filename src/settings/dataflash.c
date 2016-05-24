#include <Dataflash.h>

#include <stdio.h>

#include "dataflash.h"
#include "debug.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"



void DFSettingsToGlobals(struct baseSettings_2 *b, struct freqSettings_2 *f, uint8_t isDef) {
    s.fromRom = isDef;

    // Must set material before temperature stuff
    materialIndexSet(b->materialIndex);

    if (b->tcr == 0)
        tcrSet(vapeMaterialList[s.materialIndex].tcr);
    else
        tcrSet(b->tcr);

    // Must set scale type before temperature
    // dependant values
    tempScaleTypeIndexSet(b->tempScaleTypeIndex);
    displayTemperatureSet(f->displayTemperature);
    targetTemperatureSet(f->targetTemperature);
    targetWattsSet(f->targetWatts);
    targetVoltsSet(f->targetVolts);

    screenTimeoutSet(b->screenTimeout);
    fadeInTimeSet(b->fadeInTime);
    fadeOutTimeSet(b->fadeOutTime);
    flipOnVapeSet(b->flipOnVape);
    invertDisplaySet(b->invertDisplay);
    screenBrightnessSet(b->screenBrightness);

    if(f->baseFromUser) {
        baseTempSet(f->baseTemp);
        baseResSet(f->baseRes);
        baseFromUserSet(f->baseFromUser);
    }

    pidPSet(b->pidP);
    pidISet(b->pidI);
    pidDSet(b->pidD);
    initWattsSet(b->initWatts);
    pidSwitchSet(b->pidSwitch);

    // Setting mode may depend on the rest of the settings being setup so just set it last
    modeSet(f->mode);
}

void globalsToDFSettings(struct baseSettings_2 *b, struct freqSettings_2 *f) {
    b->pidP = s.pidP;
    b->pidI = s.pidI;
    b->pidD = s.pidD;
    b->initWatts = s.initWatts;
    b->pidSwitch = s.pidSwitch;
    b->screenTimeout = s.screenTimeout / 10;
    b->fadeInTime = s.fadeInTime / 10;
    b->fadeOutTime = s.fadeOutTime / 10;
    b->materialIndex = s.materialIndex;
    b->tempScaleTypeIndex = s.tempScaleTypeIndex;
    b->tcr = s.tcr;
    b->flipOnVape = s.flipOnVape;
    b->invertDisplay = s.invertDisplay;
    b->screenBrightness = s.screenBrightness;

    f->displayTemperature = s.displayTemperature;
    f->targetTemperature = s.targetTemperature;
    f->targetWatts = s.targetWatts;
    f->targetVolts = s.targetVolts;
    f->mode = s.mode;
    f->baseFromUser = s.baseFromUser;
    f->baseTemp = s.baseTemp;
    f->baseRes = s.baseRes;
}

void default_base_2(struct baseSettings_2 *b) {
    b->pidP = DEFPIDP;
    b->pidI = DEFPIDI;
    b->pidD = DEFPIDD;
    b->initWatts = DEFWATTS;
    b->pidSwitch = STEMPDEF;
    b->screenTimeout = SCREENDEFAULTTIMEOUT / 10;
    b->fadeInTime = FADEINTIME / 10;
    b->fadeOutTime = FADEOUTTIME / 10;
    b->materialIndex = DEFAULTMATERIAL;
    b->tempScaleTypeIndex = DEFAULTTEMPSCALE;
    b->tcr = TCRDEF;
    b->flipOnVape = FLIPDEF;
    b->invertDisplay = INVERTDEF;
    b->screenBrightness = DEFBRIGHTNESS;
}

void upgrade_base_1_base_2(struct baseSettings_2 *b2, struct baseSettings_1 *b1) {
    b2->pidP = b1->pidP;
    b2->pidI = b1->pidI;
    b2->pidD = b1->pidD;
    b2->initWatts = b1->initWatts;
    b2->pidSwitch = b1->pidSwitch;
    b2->screenTimeout = b1->screenTimeout;
    b2->materialIndex = b1->materialIndex;
    b2->tempScaleTypeIndex = b1->tempScaleTypeIndex;
}

void default_base_1(struct baseSettings_1 *b) {
    b->pidP = DEFPIDP;
    b->pidI = DEFPIDI;
    b->pidD = DEFPIDD;
    b->initWatts = DEFWATTS;
    b->pidSwitch = STEMPDEF;
    b->screenTimeout = SCREENDEFAULTTIMEOUT;
    b->materialIndex = DEFAULTMATERIAL;
    b->tempScaleTypeIndex = DEFAULTTEMPSCALE;
}

void default_freq_2(struct freqSettings_2 *f) {
    f->displayTemperature = tempScaleType[DEFAULTTEMPSCALE].def;
    f->targetTemperature = displayToC(tempScaleType[DEFAULTTEMPSCALE].def);
    f->targetWatts = DEFWATTS;
    f->targetVolts = DEFVOLTS;
    f->mode = DEFAULTMODE;

    // These make no sense to 'default'
    f->baseTemp = 0;
    f->baseRes = 0;
}

void upgrade_freq_1_freq_2 (struct freqSettings_2 *f2, struct freqSettings_1 *f1) {
    f2->mode = f1->mode;
    f2->displayTemperature = f1->displayTemperature;
    f2->targetTemperature = f1->targetTemperature;
}

void default_freq_1(struct freqSettings_1 *f) {
    f->mode = DEFAULTMODE;
    f->displayTemperature = tempScaleType[DEFAULTTEMPSCALE].def;
    f->targetTemperature = displayToC(tempScaleType[DEFAULTTEMPSCALE].def);
}


int defaultSettings() {
    struct baseSettings_2 b = {};
    struct freqSettings_2 f = {};
    default_base_2(&b);
    default_freq_2(&f);
    DFSettingsToGlobals(&b, &f, 0);
    return 0;
}

Dataflash_StructInfo_t base_structInfo_v2 = {
    .magic = SETTINGS_V2,
    .size = sizeof(struct baseSettings_2),
};

Dataflash_StructInfo_t freq_structInfo_v2 = {
    .magic = FREQ_SETTINGS_V2,
    .size = sizeof(struct freqSettings_2),
};

Dataflash_StructInfo_t base_structInfo_v1 = {
    .magic = SETTINGS_V1,
    .size = sizeof(struct baseSettings_1),
};

Dataflash_StructInfo_t freq_structInfo_v1 = {
    .magic = FREQ_SETTINGS_V1,
    .size = sizeof(struct freqSettings_1),
};

#define CURBASETYPE baseSettings_2
#define CURFREQTYPE freqSettings_2

#define CURBASESTRINFO base_structInfo_v2
#define CURFREQSTRINFO freq_structInfo_v2

int readSettings() {
    struct baseSettings_2 base_v2 = {};
    default_base_2(&base_v2);

    struct freqSettings_2 freq_v2 = {};
    default_freq_2(&freq_v2);

    struct baseSettings_1 base_v1 = {};
    default_base_1(&base_v1);

    struct freqSettings_1 freq_v1 = {};
    default_freq_1(&freq_v1);

#define CURBASESTR base_v2
#define CURFREQSTR freq_v2

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
                    upgradableBaseVer = upgradableBaseVer < 1 ? 1 : upgradableBaseVer;
                    structList[upgradeCnt++] = &base_structInfo_v1;
                }
                break;
            case FREQ_SETTINGS_V1:
                if (Dataflash_ReadStruct(&freq_structInfo_v1, &freq_v1)) {
                    upgradableFreqVer = upgradableFreqVer < 1 ? 1 : upgradableFreqVer;
                    structList[upgradeCnt++] = &freq_structInfo_v1;
                }
                break;
            case SETTINGS_V2:
                if (Dataflash_ReadStruct(&base_structInfo_v2, &base_v2)) {
                    gotTargetBase = 1;
                    structList[upgradeCnt++] = &base_structInfo_v2;
                }
                break;
            case FREQ_SETTINGS_V2:
                if (Dataflash_ReadStruct(&freq_structInfo_v2, &freq_v2)) {
                    gotTargetFreq = 1;
                    structList[upgradeCnt++] = &freq_structInfo_v2;
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
                    upgrade_base_1_base_2(&base_v2, &base_v1);
                case 2:
                    upgraded = 1;
                    break;
            }
        } else {
            default_base_2(&CURBASESTR);
            structList[upgradeCnt++] = &CURBASESTRINFO;
        }
    }

    if (!gotTargetFreq) {
        if (upgradableFreqVer) {
            switch (upgradableFreqVer) {
                case 1:
                    upgrade_freq_1_freq_2(&freq_v2, &freq_v1);
                case 2:
                    upgraded = 1;
                    break;
            }
        } else {
            default_freq_2(&CURFREQSTR);
            structList[upgradeCnt++] = &CURFREQSTRINFO;
        }
    }


    if (!Dataflash_SelectStructSet(structList, upgradeCnt)) {
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
