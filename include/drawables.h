#ifndef __DRAWABLES_H
#define __DRAWABLES_H

#include "display.h"

enum {
    CHARGEICON = 0,
    SHORTICON,
    WATTICON,
    TEMPICON,
    CHARGEPERCENTTEXT,
    CHARGEPERCENTTEXTMED,
    CHARGEPERCENTTEXTSM,
    ATOMIZERICON,
    ATOMIZERRESL,
    ATOMIZERRESM,
    ATOMIZERRESS,
    MODESETTINGL,
    MODESETTINGM,
    MODESETTINGS,
    MODEALTICON,
    MODEALTTEXTL,
    MODEALTTEXTM,
    MODEALTTEXTS,
    TEMPSCALEL,
    TEMPSCALEM,
    TEMPSCALES,
    MATERIALL,
    MATERIALM,
    MATERIALS,
    CUSTOM,
    DRAWLINE,
  /*  VAPTRISSQUARE,
    VAPTRISL,
    VAPTRISJ,
    VAPTRISI,
    VAPTRIST,
    VAPTRISS,
    VAPTRISZ,*/
    SPECIAL,
    VAPTRISFIELD,
    VAPTRISPIECE,
    VAPTRISNEXT,
    VAPTRISSCORE,
    VAPTRISLEVEL,
    // Magic guards
    LASTDRAWABLE,
    LD = LASTDRAWABLE,
    MAXDRAWABLES = 0xF8, // IF this goes up, all of the index/offsets will need to be uint16_t
    COLGROUP,
    ENDCOLGROUP,
    ROWGROUP,
    ENDROWGROUP,
    ATTRGROUP,
    ENDATTRGROUP
};

enum {
    ATTRDIMENSIONS, // W, H
    ATTRMARGINS, // L, R, T, B
    ATTRPADDING, // L, R, T, B
    ATTRLOCATION, // X, Y
    ATTRALIGN, // Bitfield
    RESERVED1 = ENDATTRGROUP
};

extern uint8_t attrQty[];

extern const struct displayItem *drawables[];

void dumpLayout(char *name, struct layoutProperties *object);
void dumpDrawable(uint8_t drawable, struct layoutProperties *object);

#endif