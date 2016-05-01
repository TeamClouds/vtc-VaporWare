#include <Dataflash.h>

#include "dataflash.h"
#include "helper.h"
#include "settings.h"

#define CR(A,B,C,D) if(B >= C && B <= D) A = B
void DFSettingsToGlobals(struct baseSettings_1 *b, struct freqSettings_1 *f, uint8_t isDef) {
    s.fromRom = isDef;
    CR(s.mode, f->mode, 0, 2);
    CR(s.screenTimeout, b->screenTimeout, 1, 1000);
    CR(s.displayTemperature, f->displayTemperature, 0, 600);
    CR(s.targetTemperature, f->targetTemperature,0,600);
    CR(s.materialIndex, b->materialIndex, 0,4);
    CR(s.tempScaleTypeIndex, b->tempScaleTypeIndex,0,2);
    CR(s.pidP, b->pidP, 0, 0xFFFF);
    CR(s.pidI, b->pidI, 0, 0xFFFF);
    CR(s.pidD, b->pidD, 0, 0xFFFF);
    CR(s.initWatts, b->initWatts,0, 60000);
    CR(s.pidSwitch, b->pidSwitch,0, 600);
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
    b->pidP = 17000;
    b->pidI = 5500;
    b->pidD = 0;
    b->initWatts = 15000;
    b->pidSwitch = 600;
    b->screenTimeout = 30;   // 100s of s
    b->materialIndex = 1;
    b->tempScaleTypeIndex = 1;
}

void default_freq(struct freqSettings_1 *f) {
    f->mode = 2;
    f->displayTemperature = tempScaleType[1].def;
    f->targetTemperature = displayToC(tempScaleType[1].def);
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
    Dataflash_UpdateStruct(&CURBASESTRINFO, &b);
    Dataflash_UpdateStruct(&CURFREQSTRINFO, &f);
    return 1;
}

