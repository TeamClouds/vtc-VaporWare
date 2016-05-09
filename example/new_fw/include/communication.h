#include <USB_VirtualCOM.h>

void Communication_Init();
void Communication_Command(char *buffer);

void updateSettings(char *buffer, char *response);
void dumpSettings(char *buffer, char *response);

void updateAtomizer(char *buffer, char *response);
void dumpAtomizer(char *buffer, char *response);

void dumpDisplay(char *buffer, char *response);