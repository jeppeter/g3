
#ifndef __DETOUR_DINPUT_H__
#define __DETOUR_DINPUT_H__

#include <Windows.h>
#include <ioinject.h>

#define  MOUSE_LEFT_BTN          1
#define  MOUSE_RIGHT_BTN       2
#define  MOUSE_MIDDLE_BTN    3
#define  MOUSE_MIN_BTN          1
#define  MOUSE_MAX_BTN         3

#ifdef __cplusplus
extern "C" {
#endif
BOOL DetourDirectInputInit(void);
void DetourDirectInputFini(void);

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl);

int DetourDinputPressKeyDownTimes(UINT scancode);
int DetourDinputMouseBtnDown(UINT btn);
int DetourDinputScreenMousePoint(HWND hwnd,POINT* pPoint);
int DetourDinputSetWindowsRect(HWND hWnd,RECT *pRect);
int DetourDinput8GetMousePointAbsolution(POINT *pPoint);



#ifdef __cplusplus
};
#endif



#endif /*__DETOUR_DINPUT_H__*/

