#include <Display.h>
#include <Battery.h>
#include <Atomizer.h>

#include "drawables.h"
#include "display.h"
#include "debug.h"

#include "globals.h"
#include "settings.h"

#include "images/watts.h"
#include "images/temperature.h"

void modeGetText(char *text, uint8_t len) {
    if (g.vapeModes[s.mode]->getDisplayText)
        g.vapeModes[s.mode]->getDisplayText(text, len);
    else
        sniprintf(text,5,"none");
}

struct textPriv modeTextLargePriv = {
    .font = FONT_LARGE,
    .len = 5,
    .getText = &modeGetText
};

struct textPriv modeTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 5,
    .getText = &modeGetText
};

struct textPriv modeTextSmPriv = {
    .font = FONT_SMALL,
    .len = 5,
    .getText = &modeGetText
};

const struct displayItem __modeTextL = {
    .label = "modeTxtL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeTextLargePriv
};

const struct displayItem __modeTextM = {
    .label = "modeTxtM",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeTextMedPriv
};

const struct displayItem __modeTextS = {
    .label = "modeTxtS",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeTextSmPriv
};

void modeGetAltText(char *text, uint8_t len) {
    if (g.vapeModes[s.mode]->getAltDisplayText)
        g.vapeModes[s.mode]->getAltDisplayText(text, len);
    else
        sniprintf(text,5,"none");
}


struct textPriv modeAltTextLargePriv = {
    .font = FONT_LARGE,
    .len = 5,
    .getText = &modeGetAltText
};

struct textPriv modeAltTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 5,
    .getText = &modeGetAltText
};

struct textPriv modeAltTextSmPriv = {
    .font = FONT_SMALL,
    .len = 5,
    .getText = &modeGetAltText
};

const struct displayItem __modeAltTextL = {
    .label = "modeAltL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeAltTextLargePriv
};

const struct displayItem __modeAltTextM = {
    .label = "modeAltM",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeAltTextMedPriv
};

const struct displayItem __modeAltTextS = {
    .label = "modeAltS",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &modeAltTextSmPriv
};


void tempScaleGetText(char *text, uint8_t len) {
    getString(text, len, (char *) tempScaleType[s.tempScaleTypeIndex].display);
}

struct textPriv scaleTextLargePriv = {
    .font = FONT_LARGE,
    .len = 2,
    .getText = &tempScaleGetText
};

struct textPriv scaleTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 2,
    .getText = &tempScaleGetText
};

struct textPriv scaleTextSmPriv = {
    .font = FONT_SMALL,
    .len = 2,
    .getText = &tempScaleGetText
};

const struct displayItem scaleTextL = {
    .label = "sclTxtL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &scaleTextLargePriv
};

const struct displayItem scaleTextM = {
    .label = "sclTxtM",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &scaleTextMedPriv
};

const struct displayItem scaleTextS = {
    .label = "sclTxtS",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &scaleTextSmPriv
};


void materialGetText(char *text, uint8_t len) {
    getString(text, len, vapeMaterialList[s.materialIndex].name);
}

struct textPriv materialTextLargePriv = {
    .font = FONT_LARGE,
    .len = 3,
    .getText = &materialGetText
};

struct textPriv materialTextMedPriv = {
    .font = FONT_MEDIUM,
    .len = 3,
    .getText = &materialGetText
};

struct textPriv materialTextSmPriv = {
    .font = FONT_SMALL,
    .len = 3,
    .getText = &materialGetText
};

const struct displayItem __materialTextL = {
    .label = "matTxtL",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &materialTextLargePriv
};

const struct displayItem __materialTextM = {
    .label = "matTxtM",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &materialTextMedPriv
};

const struct displayItem __materialTextS = {
    .label = "matTxtS",
    .getDimensions = &textGetDimensions,
    .drawAt = &textDraw,
    .priv = &materialTextSmPriv
};


uint8_t modeAltIconGetDimension(void *priv, uint8_t *args, struct layoutProperties *object) {
    if (g.vapeModes[s.mode]->altIconDrawable) {
        return drawables[g.vapeModes[s.mode]->altIconDrawable]->getDimensions(priv, args, object);
    } else {
        object->W = 0;
        object->H = 0;
    }
    return 0;
}

uint8_t modeAltIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    if (g.vapeModes[s.mode]->altIconDrawable) {
        return drawables[g.vapeModes[s.mode]->altIconDrawable]->drawAt(priv, args, object);
    }
    return 0;
}

const struct displayItem modeAltIcon = {
    .label = "mdAltIcn",
    .getDimensions = &modeAltIconGetDimension,
    .drawAt = &modeAltIconDraw
};

uint8_t wattIconGetDimension(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = watts_width;
    object->H = watts_height;
    return 0;
}

uint8_t wattIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    Display_PutPixels(object->X, object->Y, watts, watts_width, watts_height);
    return 0;
}

const struct displayItem __wattIcon = {
    .label = "wattIcon",
    .getDimensions = &wattIconGetDimension,
    .drawAt = &wattIconDraw
};

uint8_t tempIconGetDimension(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = tempImage_width;
    object->H = tempImage_height;
    return 0;
}

uint8_t tempIconDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    Display_PutPixels(object->X, object->Y, tempImage, tempImage_width, tempImage_height);
    return 0;
}

const struct displayItem __tempIcon = {
    .label = "tempIcon",
    .getDimensions = &tempIconGetDimension,
    .drawAt = &tempIconDraw
};

static void __attribute__((constructor)) registerChargeDrawables(void) {
    drawables[MODESETTINGL] = &__modeTextL;
    drawables[MODESETTINGM] = &__modeTextM;
    drawables[MODESETTINGS] = &__modeTextS;

    drawables[MODEALTTEXTL] = &__modeAltTextL;
    drawables[MODEALTTEXTM] = &__modeAltTextM;
    drawables[MODEALTTEXTS] = &__modeAltTextS;

    drawables[MATERIALL] = &__materialTextL;
    drawables[MATERIALM] = &__materialTextM;
    drawables[MATERIALS] = &__materialTextS;

    drawables[TEMPSCALEL] = &scaleTextL;
    drawables[TEMPSCALEM] = &scaleTextM;
    drawables[TEMPSCALES] = &scaleTextS;

    drawables[MODEALTICON] = &modeAltIcon;
    drawables[WATTICON] = &__wattIcon;
    drawables[TEMPICON] = &__tempIcon;
}