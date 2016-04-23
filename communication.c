#include "communication.h"
#include "main.h"

void Communication_Init() {
    // We may want to only init this if the user tells us it's cool.

    // The virtual COM port is not initialized by default.
    // To initialize it, follow those steps:
    // 1) Unlock system control registers.
    SYS_UnlockReg();
    // 2) Initialize the virtual COM port.
    USB_VirtualCOM_Init();
    // 3) Lock system control registers.
    SYS_LockReg();
    USB_VirtualCOM_SetAsyncMode(1);
}

void Communication_Command(char *buffer) {
    char response[4];
    response[0] = '-';
    response[1] = buffer[0];
    response[2] = '\r';
    response[3] = '\n';
    switch(buffer[0]) {
    case '@':
        response[0] = '$';
        USB_VirtualCOM_SendString("AT HOME YOU ARE\r\n");
        break;
    case 'A':
        updateAtomizer(buffer, response);
        break;
    case 'a':
        dumpAtomizer(buffer, response);
        break;
    case 'S':
        updateSettings(buffer, response);
        break;
    case 's':
        dumpSettings(buffer, response);
        break;
    case 'U':
        SYS_UnlockReg();
        SYS_CLEAR_RST_SOURCE(SYS_RSTSTS_PORF_Msk | SYS_RSTSTS_PINRF_Msk);
        FMC_SELECT_NEXT_BOOT(1);
        NVIC_SystemReset();
        break;
    default:
        response[0] = '~';
        break;
    }
    USB_VirtualCOM_SendString(response);
}
