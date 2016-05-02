#ifndef ____DEBUG
#define ____DEBUG
#include <USB_VirtualCOM.h>


#define D() ;{char __buff[63];siprintf(__buff, "File: %s Line: %i\r\n", __FILE__, __LINE__);USB_VirtualCOM_SendString(__buff);};
#define W() ;{while(USB_VirtualCOM_GetAvailableSize() == 0){;}};
#endif
