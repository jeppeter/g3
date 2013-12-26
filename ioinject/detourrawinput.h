
#ifndef __DETOUR_RAWINPUT_H__
#define __DETOUR_RAWINPUT_H__


#include <iocapcommon.h>


BOOL DetourRawInputInit(HMODULE hModule);
void DetourRawInputFini(HMODULE hModule);


int DetourRawInputControl(PIO_CAP_CONTROL_t pControl);
int MapVirtualKeyEmulation(int scancode);
int IsRawInputKeyboardRegistered();
int IsRawInputMouseRegistered();


#endif  /*__DETOUR_RAWINPUT_H__*/


