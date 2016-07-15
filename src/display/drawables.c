#include <Display.h>
#include "drawables.h"
#include "display.h"
#include "debug.h"

/* I AM NOT PROUD OF THIS HACK */

uint8_t __customGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    if (customGetDimensions != 0)
        return customGetDimensions(object);
    return 0;
}

uint8_t __customDrawAt(void *priv, uint8_t *args, struct layoutProperties *object) {
    if (customDrawAt != 0)
        return customDrawAt(object);
    return 0;
}

const struct displayItem __custom = {
    .label = "custom",
    .getDimensions = &__customGetDimensions,
    .drawAt = &__customDrawAt
};

uint8_t lineGetDimensions(void *priv, uint8_t *args, struct layoutProperties *object) {
    object->W = abs(args[2] - args[0]) + 1;
    object->H = abs(args[3] - args[1]) + 1;
    return 4;
}

uint8_t lineDraw(void *priv, uint8_t *args, struct layoutProperties *object) {
    Display_PutLine(object->X + args[0], object->Y + args[1],
                    object->X + args[2], object->Y + args[3]);
    return 4;
}

const struct displayItem __line = {
    .label = "line",
    .getDimensions = &lineGetDimensions,
    .drawAt = &lineDraw,
    .args = 4
};

static void __attribute__((constructor)) registerCustomDrawables(void) {
    drawables[CUSTOM] = &__custom;
    drawables[DRAWLINE] = &__line;
}

char *index_to_name[] = {
    "ChargeIcon",
    "ShortIcon",
    "ChargePercentText",
    "ChargePercentTextM",
    "ChargePercentTextS",
    "AtomizerIcon",
    "AtomizerResL",
    "AtomizerResM",
    "AtomizerResS",
    "Custom",
    "DrawLine",
    "LastDrawable",
    "MaxDrawables", // IF this goes up, all of the index/offsets will need to be uint16_t
    "ColGroup",
    "EndColGroup",
    "RowGroup",
    "EndRowGroup",
    "AttrGroup",
    "EndAttrGroup"
};

void dumpLayout(char *name, struct layoutProperties *object) {
    writeUsb("%s at %d %d -> %d %d\n", name,
             object->X, object->Y, object->W, object->H );

    writeUsb("\t%d %d %d %d | %d %d %d %d\n",
             object->mXl, object->mXr, object->mYt, object->mYb,
             object->pXl, object->pXr, object->pYt, object->pYb
        );
}

void dumpDrawable(uint8_t drawable, struct layoutProperties *object) {
    uint8_t index = drawable;
    index = drawable > MAXDRAWABLES ? index - MAXDRAWABLES + LASTDRAWABLE + 1 : index;
    dumpLayout(index_to_name[index], object);
}