
#include <iocapcommon.h>

#ifdef IOCAP_DEBUG
#include <injectbase.h>
#include <injectbase_window.h>


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


BOOL InsertHwnd(HWND hwnd,HINSTANCE hInst)
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

SetForegroundWindowFunc_t SetForegroundWindowNext=SetForegroundWindow;
GetForegroundWindowFunc_t GetForegroundWindowNext=GetForegroundWindow;

BOOL WINAPI SetForegroundWindowCallBack(HWND hwnd)
{
    BOOL bret;

    bret = SetForegroundWindowNext(hwnd);
    if(bret)
    {
        DEBUG_INFO("SetForegroundWindow(0x%08x)\n",hwnd);
    }
    return bret;
}

HWND WINAPI GetForegroundWindowCallBack()
{
    HWND hwnd=NULL;

    hwnd = GetForegroundWindowNext();

    DEBUG_INFO("GetForegroundWindow(0x%08x)\n",hwnd);
    return hwnd;
}

int DetourForegroundWindow()
{
    DEBUG_BUFFER_FMT(SetForegroundWindowNext,10,"Before SetForegroundWindowNext (0x%p)",SetForegroundWindowNext);
    DEBUG_BUFFER_FMT(GetForegroundWindowNext,10,"Before GetForegroundWindowNext (0x%p)",GetForegroundWindowNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&SetForegroundWindowNext,SetForegroundWindowCallBack);
    DetourAttach((PVOID*)&GetForegroundWindowNext,GetForegroundWindowCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(SetForegroundWindowNext,10,"After SetForegroundWindowNext (0x%p)",SetForegroundWindowNext);
    DEBUG_BUFFER_FMT(GetForegroundWindowNext,10,"After GetForegroundWindowNext (0x%p)",GetForegroundWindowNext);
    return 0;
}

#endif /*IOCAP_DEBUG*/
