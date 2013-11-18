
#ifndef __DETOUR_DINPUT_H__
#define __DETOUR_DINPUT_H__

#include <Windows.h>
#include <ioinject.h>

BOOL DetourDirectInputInit(void);
void DetourDirectInputFini(void);

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl);

#endif /*__DETOUR_DINPUT_H__*/

