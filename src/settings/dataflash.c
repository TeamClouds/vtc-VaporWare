#include <Dataflash.h>

#include <stdio.h>

#include "dataflash.h"

#define XSTR(x) STR(x)
#define STR(x) #x
#pragma message "Base Dataflash version " XSTR(BASE_VER)
#pragma message "Freq Dataflash version " XSTR(FREQ_VER)

#include "debug.h"
#include "globals.h"
#include "helper.h"
#include "settings.h"



void DFSettingsToGlobals(struct baseSettings_3 *b, struct freqSettings_3 *f, uint8_t isDef) {
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

void globalsToDFSettings(struct baseSettings_3 *b, struct freqSettings_3 *f) {
    b->pidP = s.pidP;
    b->pidI = s.pidI;
    b->pidD = s.pidD;
    b->initWatts = s.initWatts;
    b->pidSwitch = s.pidSwitch;
    b->screenTimeout = s.screenTimeout;
    b->fadeInTime = s.fadeInTime;
    b->fadeOutTime = s.fadeOutTime;
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

void default_base_3(struct baseSettings_3 *b) {
    b->pidP = DEFPIDP;
    b->pidI = DEFPIDI;
    b->pidD = DEFPIDD;
    b->initWatts = DEFWATTS;
    b->pidSwitch = STEMPDEF;
    b->screenTimeout = SCREENDEFAULTTIMEOUT;
    b->fadeInTime = FADEINTIME;
    b->fadeOutTime = FADEOUTTIME;
    b->materialIndex = DEFAULTMATERIAL;
    b->tempScaleTypeIndex = DEFAULTTEMPSCALE;
    b->tcr = TCRDEF;
    b->flipOnVape = FLIPDEF;
    b->invertDisplay = INVERTDEF;
    b->screenBrightness = DEFBRIGHTNESS;
}

#if 0
This is left in only to show how upgrade paths SHOULD work

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
#endif

void default_freq_3(struct freqSettings_3 *f) {
    f->displayTemperature = tempScaleType[DEFAULTTEMPSCALE].def;
    f->targetTemperature = displayToC(tempScaleType[DEFAULTTEMPSCALE].def);
    f->targetWatts = DEFWATTS;
    f->targetVolts = DEFVOLTS;
    f->mode = DEFAULTMODE;

    // These make no sense to 'default'
    f->baseTemp = 0;
    f->baseRes = 0;
}

#if 0
This is left in only to show how upgrade paths SHOULD work

void upgrade_freq_1_freq_2 (struct freqSettings_2 *f2, struct freqSettings_1 *f1) {
    f2->mode = f1->mode;
    f2->displayTemperature = f1->displayTemperature;
    f2->targetTemperature = f1->targetTemperature;
}
#endif

int defaultSettings() {
    struct baseSettings_3 b = {};
    struct freqSettings_3 f = {};
    default_base_3(&b);
    default_freq_3(&f);
    DFSettingsToGlobals(&b, &f, 0);
    return 0;
}

Dataflash_StructInfo_t base_structInfo_v3 = {
    .magic = SETTINGS_V3,
    .size = sizeof(struct baseSettings_3),
};

Dataflash_StructInfo_t freq_structInfo_v3 = {
    .magic = FREQ_SETTINGS_V3,
    .size = sizeof(struct freqSettings_3),
};

#define CURBASETYPE baseSettings_3
#define CURFREQTYPE freqSettings_3

#define CURBASESTRINFO base_structInfo_v3
#define CURFREQSTRINFO freq_structInfo_v3

int readSettings() {
    struct baseSettings_3 base_v3 = {};
    default_base_3(&base_v3);

    struct freqSettings_3 freq_v3 = {};
    default_freq_3(&freq_v3);

#define CURBASESTR base_v3
#define CURFREQSTR freq_v3

    Dataflash_StructInfo_t *structList[SETTINGS_VCNT + FREQ_SETTINGS_VCNT];

    uint32_t magicList[DATAFLASH_STRUCT_MAX_COUNT];
    uint8_t magicCount, i, upgradeCnt = 0, upgraded = 0;

    uint8_t gotTargetBase = 0, upgradableBaseVer = 0;
    uint8_t gotTargetFreq = 0, upgradableFreqVer = 0;

    magicCount = Dataflash_GetMagicList(magicList);

    for(i = 0; i < magicCount; i++) {
        switch (magicList[i]) {
            /*
             * Removed due to no way to upgrade:
             * SETTINGS_V1/V2
             * FREQ_SETTINGS_V1/V2
             */
            case SETTINGS_V3:
                if (Dataflash_ReadStruct(&base_structInfo_v3, &base_v3)) {
                    gotTargetBase = 1;
                    structList[upgradeCnt++] = &base_structInfo_v3;
                }
                break;
            case FREQ_SETTINGS_V3:
                if (Dataflash_ReadStruct(&freq_structInfo_v3, &freq_v3)) {
                    gotTargetFreq = 1;
                    structList[upgradeCnt++] = &freq_structInfo_v3;
                }
                break;
        }
        if (gotTargetBase && gotTargetFreq)
            break;
    }

    if (!gotTargetBase) {
        if (upgradableBaseVer) {
            /* No upgrades supported prior to V3 due to dataflash format
               bug
            switch (upgradableBaseVer) {
                case 1:
                    upgrade_base_1_base_2(&base_v2, &base_v1);
                case 2:
                    upgraded = 1;
                    break;
            }
            */
        } else {
            default_base_3(&CURBASESTR);
            structList[upgradeCnt++] = &CURBASESTRINFO;
        }
    }

    if (!gotTargetFreq) {
        if (upgradableFreqVer) {
            /* No upgrades supported prior to V3 due to dataflash format
               bug
            switch (upgradableFreqVer) {
                case 1:
                    upgrade_freq_1_freq_2(&freq_v2, &freq_v1);
                case 2:
                    upgraded = 1;
                    break;
            }
            */
        } else {
            default_freq_3(&CURFREQSTR);
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
