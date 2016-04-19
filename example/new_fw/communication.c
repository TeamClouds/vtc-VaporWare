#include "communication.h"

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
