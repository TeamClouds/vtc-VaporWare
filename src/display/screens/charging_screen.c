#include <Display.h>
#include "display.h"
#include "drawables.h"


uint8_t chrgScrnItems[] = {
    ROWGROUP, ATTRGROUP, ATTRALIGN, VCENTER, ENDATTRGROUP,
        COLGROUP, ATTRGROUP, ATTRALIGN, CENTER, ENDATTRGROUP,
            CHARGEPERCENTTEXT, ATTRGROUP, ATTRALIGN, CENTER, ENDATTRGROUP,
            CHARGEICON,ATTRGROUP, ATTRALIGN, CENTER, ENDATTRGROUP,
        ENDCOLGROUP,
    ENDROWGROUP,
    LD};

void displayCharging() {
    setupScreen();

    drawScreen(chrgScrnItems);

    Display_Update();
    if (g.screenFadeInTime != 0) {
        g.screenFadeInTime = 0;
    }
}