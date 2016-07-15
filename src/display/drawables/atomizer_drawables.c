#include <Display.h>
#include <Battery.h>
#include <Atomizer.h>

#include "drawables.h"
#include "display.h"
#include "debug.h"

#include "images/short.h"
#include "images/ohm.h"

uint8_t atomizerIconGetDimension(void *priv, uint8_t *args, struct layoutProperties *object) {
    switch (Atomizer_GetError()) {
    case SHORT:
    case OPEN:
        object->W = shortBIT_width;
        object->H = shortBIT_height;
        break;
    default:
        object->W = ohm_width;
        object->H = ohm_height;
        break;
    }
    return 0;
}

uint8_t atomizerIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    switch (Atomizer_GetError()) {
    case SHORT:
    case OPEN:
        Display_PutPixels(object->X, object->Y, shortBIT, shortBIT_width, shortBIT_height);
        break;
    default:
        Display_PutPixels(object->X, object->Y, ohm, ohm_width, ohm_height);
        break;
    }
    return 0;
}

const struct displayItem __atomizerIcon = {
    .label = "atmzIcon",
    .getDimensions = &atomizerIconGetDimension,
    .drawAt = &atomizerIconDraw
};


void atomizerGetText(char *text, uint8_t len) {
    uint16_t displayRes;
    uint8_t atomizerOn = Atomizer_IsOn();
    switch (Atomizer_GetError()) {
    case SHORT:
    case OPEN:
        getString(text, len, "SHRT");
        break;
    default:
        if (atomizerOn) {
            displayRes = g.atomInfo.resistance;
        } else {
            displayRes = g.baseRes;
        }
        getFloating(text, len, displayRes);
        break;
    }
}

struct textPriv atyTextLargePriv = {
    .font = FONT_LARGE,
    .len = 6,
    .getText = &atomizerGetText
};

struct textPriv atyTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 6,
    .getText = &atomizerGetText
};

struct textPriv atyTextSmPriv = {
    .font = FONT_SMALL,
    .len = 6,
    .getText = &atomizerGetText
};

const struct displayItem __atyTextL = {
    .label = "atyTextL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &atyTextLargePriv
};

const struct displayItem __atyTextM = {
    .label = "atyTextM",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &atyTextMedPriv
};

const struct displayItem __atyTextS = {
    .label = "atyTextS",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &atyTextSmPriv
};

static void __attribute__((constructor)) registerChargeDrawables(void) {
    drawables[ATOMIZERICON] = &__atomizerIcon;
    drawables[ATOMIZERRESL] = &__atyTextL;
    drawables[ATOMIZERRESM] = &__atyTextM;
    drawables[ATOMIZERRESS] = &__atyTextS;
}