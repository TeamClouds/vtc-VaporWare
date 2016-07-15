#include <Atomizer.h>
#include <Display.h>
#include "display.h"
#include "drawables.h"
#include "globals.h"
#include "settings.h"
#include "debug.h"

uint8_t mainScrnItems[] = {
    ROWGROUP,
        MODESETTINGL, ATTRGROUP, ATTRALIGN, VCENTER, ATTRDIMENSIONS, 45, 0, ENDATTRGROUP,
        COLGROUP,
            TEMPSCALES,
            MATERIALS,
        ENDCOLGROUP,
    ENDROWGROUP,
    ROWGROUP, ATTRGROUP, ATTRMARGINS, 0, 0, 0, 10, ENDATTRGROUP,
        DRAWLINE, 0, 0, 60, 0,
    ENDROWGROUP,
    COLGROUP, ATTRGROUP, ATTRALIGN, CENTER, ENDATTRGROUP,
        ROWGROUP,
            CHARGEICON, ATTRGROUP, ATTRALIGN, VCENTER, ENDATTRGROUP,
            CHARGEPERCENTTEXTMED, ATTRGROUP, ATTRALIGN, VCENTER, ATTRMARGINS, 2, 0, 0, 0, ENDATTRGROUP,
        ENDROWGROUP,
        ROWGROUP,
            ATOMIZERICON, ATTRGROUP, ATTRALIGN, VCENTER, ENDATTRGROUP,
            ATOMIZERRESM, ATTRGROUP, ATTRALIGN, VCENTER, ATTRMARGINS, 2, 0, 0, 0, ENDATTRGROUP,
        ENDROWGROUP,
        ROWGROUP,
            MODEALTICON, ATTRGROUP, ATTRALIGN, VCENTER, ENDATTRGROUP,
            MODEALTTEXTM, ATTRGROUP, ATTRALIGN, VCENTER, ATTRMARGINS, 2, 0, 0, 0, ENDATTRGROUP,
        ENDROWGROUP,
    ENDCOLGROUP,

    LD
};

//

void updateScreen() {

    if (s.stealthMode)
        return;

    setupScreen();
    fadeInTransition();
    drawScreen(mainScrnItems);

    Display_Update();
}