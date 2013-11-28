
#ifndef __DETOUR_DINPUT_H__
#define __DETOUR_DINPUT_H__

#include <Windows.h>
#include <ioinject.h>

BOOL DetourDirectInputInit(void);
void DetourDirectInputFini(void);

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl);

//int DetourDinputPressKeyDown(UINT scancode);
//int DetourDinputClientRectMousePoint(POINT* pPoint);

#endif /*__DETOUR_DINPUT_H__*/

