
#ifndef __INJECTBASE_WINDOW_H__
#define __INJECTBASE_WINDOW_H__
#include <Windows.h>

BOOL InsertHwnd(HWND hwnd,HINSTANCE hInst);
BOOL RemoveHwnd(HWND hwnd);
int DetourShowCursorFunction(void);


#endif  /*__INJECTBASE_WINDOW_H__*/

