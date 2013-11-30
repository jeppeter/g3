
#ifndef __DETOUR_DINPUT_H__
#define __DETOUR_DINPUT_H__

#include <Windows.h>
#include <ioinject.h>

#define  MOUSE_LEFT_BTN          1
#define  MOUSE_RIGHT_BTN       2
#define  MOUSE_MIDDLE_BTN    3
#define  MOUSE_MIN_BTN          1
#define  MOUSE_MAX_BTN         3

BOOL DetourDirectInputInit(void);
void DetourDirectInputFini(void);

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl);

int RawInputPressKeyDownTimes(UINT scancode);
int RawInputClientMousePoint(POINT* pPoint);
int RawInputMouseBtnDown(UINT btn);
int RawInputSetWindowsRect(HWND hWnd,RECT *pRect);

#endif /*__DETOUR_DINPUT_H__*/

