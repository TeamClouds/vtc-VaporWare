#include <Display.h>
#include <Battery.h>

#include "drawables.h"
#include "display.h"

#include "globals.h"
#include "settings.h"

#include "images/battery.h"
#include "images/hot.h"
#include "images/short.h"
#include "images/ohm.h"


uint8_t* getBatteryIcon() {
    switch (Atomizer_GetError()) {
    case WEAK_BATT:
        return batteryalert;
    default:
        if (Battery_IsPresent()) {
            if (Battery_IsCharging()) {
                return batterycharging;
            } else {
                return battery;
            }
        } else {
            return batteryalert;
        }
    }
}


uint8_t chargeIconGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->H = battery_height;
    object->W = battery_width;
    return 0;
}

uint8_t chargeIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    Display_PutPixels(object->X, object->Y, getBatteryIcon(), battery_width, battery_height);
    return 0;
}

const struct displayItem __chrgIcon = {
    .label = "chrgIcon",
    .getDimensions = &chargeIconGetDimensions,
    .drawAt = &chargeIconDraw
};


uint8_t shortIconGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->H = shortBIT_height;
    object->W = shortBIT_width;
    return 0;
}

uint8_t shortIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    Display_PutPixels(object->X, object->Y, shortBIT, shortBIT_width, shortBIT_height);
    return 0;
}

const struct displayItem __shrtIcon = {
    .label = "shrtIcon",
    .getDimensions = &shortIconGetDimensions,
    .drawAt = &shortIconDraw
};


void chargeBatteryGetText(char *text, uint8_t len) {
    getPercent(text, len, g.batteryPercent);
}

struct textPriv chrgTextPriv = {
    .font = FONT_LARGE,
    .len = 5,
    .getText = &chargeBatteryGetText
};

struct textPriv chrgTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 5,
    .getText = &chargeBatteryGetText
};

struct textPriv chrgTextSmPriv = {
    .font = FONT_SMALL,
    .len = 5,
    .getText = &chargeBatteryGetText
};

const struct displayItem __chrgText = {
    .label = "chrgText",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &chrgTextPriv
};

const struct displayItem __chrgTextMed = {
    .label = "chrgText",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &chrgTextMedPriv
};

const struct displayItem __chrgTextSm = {
    .label = "chrgText",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &chrgTextSmPriv
};

static void __attribute__((constructor)) registerChargeDrawables(void) {
    drawables[CHARGEICON] = &__chrgIcon;
    drawables[SHORTICON] = &__shrtIcon;

    drawables[CHARGEPERCENTTEXT] = &__chrgText;
    drawables[CHARGEPERCENTTEXTMED] = &__chrgTextMed;
    drawables[CHARGEPERCENTTEXTSM] = &__chrgTextSm;
}