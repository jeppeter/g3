
#ifndef __INJECTBASE_WINDOW_H__
#define __INJECTBASE_WINDOW_H__
#include <Windows.h>

BOOL InsertHwnd(HWND hwnd,HINSTANCE hInst);
BOOL RemoveHwnd(HWND hwnd);
int DetourShowCursorFunction(void);
int DetourForegroundWindow();

typedef BOOL (WINAPI *SetForegroundWindowFunc_t)(HWND hWnd);
typedef HWND (WINAPI *GetForegroundWindowFunc_t)();



#endif  /*__INJECTBASE_WINDOW_H__*/

