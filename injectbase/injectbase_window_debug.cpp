
#include <iocapcommon.h>

#ifdef IOCAP_DEBUG
#include <injectbase.h>


int BaseSetKeyMouseState(LPVOID pParam,LPVOID pInput)
{
	return 0;
}


int SetShowCursorNormal()
{
	return 0;
}


int SetShowCursorHide()
{
	return 0;
}


BOOL InsertHwnd(HWND hwnd)
{
	return TRUE;
}


BOOL RemoveHwnd(HWND hwnd)
{
	return TRUE;
}


int DetourShowCursorFunction(void)
{
	return 0;
}


#endif /*IOCAP_DEBUG*/
