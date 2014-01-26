
#include <iocapcommon.h>

#ifdef IOCAP_DEBUG
#include <injectbase.h>


int EnableSetCursorPos(void)
{
	int ret;
	ret = ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return -ret;
}

int DisableSetCursorPos(void)
{
	int ret;
	ret = ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return -ret;
}


int BaseSetKeyMouseState(LPVOID pParam,LPVOID pInput)
{
	int ret;
	ret = ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return -ret;
}


int SetShowCursorNormal()
{
	int ret=ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return -ret;
}


int SetShowCursorHide()
{
	int ret=ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return -ret;
}


BOOL InsertHwnd(HWND hwnd)
{
	int ret=ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return FALSE;
}


BOOL RemoveHwnd(HWND hwnd)
{
	int ret=ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return FALSE;
}


int DetourShowCursorFunction(void)
{
	return 0;
}

int InitBaseKeyState(void)
{
	return 0;
}

int InitBaseMouseState(void)
{
	return 0;
}

HWND GetCurrentProcessActiveWindow()
{
	return NULL;
}

int BaseScreenMousePoint(HWND hwnd,POINT * pPoint)
{
	int ret;
	ret = ERROR_NOT_SUPPORTED;
	return -ret;
}


#endif /*IOCAP_DEBUG*/
