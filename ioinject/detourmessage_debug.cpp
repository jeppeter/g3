
#include <TlHelp32.h>
#define MSG_DEBUG_BUFFER_FMT(...)

GetMessageFunc_t GetMessageANext= GetMessageA;
PeekMessageFunc_t PeekMessageANext=PeekMessageA;
GetMessageFunc_t GetMessageWNext= GetMessageW;
PeekMessageFunc_t PeekMessageWNext=PeekMessageW;


HWND FindCurrentActiveWindow()
{
    static HWND st_LastCurWnd=NULL;
    HWND curwnd=NULL;
    DWORD pid;
    HANDLE hSnap=INVALID_HANDLE_VALUE;
    int ret;
    THREADENTRY32 t32;
    BOOL bret;
    std::vector<DWORD> thids;
    UINT i;
    GUITHREADINFO guiinfo;

    pid = GetCurrentProcessId();

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,pid);
    if(hSnap == INVALID_HANDLE_VALUE)
    {
        ret = GETERRNO();
        goto fail;
    }

    ZeroMemory(&t32,sizeof(t32));
    t32.dwSize = sizeof(t32);

    SETERRNO(0);
    for(bret = Thread32First(hSnap,&t32); bret; bret = Thread32Next(hSnap,&t32))
    {
        if(t32.th32OwnerProcessID == pid)
        {
            thids.push_back(t32.th32ThreadID);
        }
        ZeroMemory(&t32,sizeof(t32));
        t32.dwSize = sizeof(t32);
    }

    ret = GETERRNO();
    if(ret != ERROR_NO_MORE_FILES)
    {
        ERROR_INFO("(0x%08x)process thread error(%d)\n",pid,ret);
        goto fail;
    }

    DEBUG_INFO("threadsize(%d)\n",thids.size());
    for(i=0; i<thids.size(); i++)
    {
        guiinfo.cbSize = sizeof(guiinfo);
        bret = GetGUIThreadInfo(thids[i],&guiinfo);
        if(bret)
        {
            DEBUG_INFO("[%d](0x%08x) flags(0x%08x) activewindow (0x%08x) focuswindow (0x%08x) capturewindow (0x%08x) menuowner(0x%08x) movesize(0x%08x) caret(0x%08x)\n",
                       i,
                       thids[i],
                       guiinfo.flags,
                       guiinfo.hwndActive,
                       guiinfo.hwndFocus,
                       guiinfo.hwndCapture,
                       guiinfo.hwndMenuOwner,
                       guiinfo.hwndMoveSize,
                       guiinfo.hwndCaret);
            curwnd = guiinfo.hwndCapture;
        }
        else
        {
            ret = GETERRNO();
            ERROR_INFO("[%d](0x%08x) Get GuiInfo Error(%d)\n",i,thids[i],ret);
        }
    }

    if(st_LastCurWnd && st_LastCurWnd != curwnd)
    {
        DEBUG_INFO("last curwnd(0x%08x) curwnd(0x%08x)\n",st_LastCurWnd,curwnd);
    }
    st_LastCurWnd = curwnd;
    if(hSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSnap);
    }
    hSnap= INVALID_HANDLE_VALUE;
    SETERRNO(0);
    return curwnd;
fail:

    if(hSnap != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSnap);
    }
    hSnap= INVALID_HANDLE_VALUE;
    SETERRNO(ret);
    return NULL;
}

BOOL WINAPI GetMessageACallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
)
{
    BOOL bret;

    bret = GetMessageANext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        FindCurrentActiveWindow();
        DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                         lpMsg->hwnd,lpMsg->message,lpMsg->message,
                         lpMsg->wParam,lpMsg->wParam,
                         lpMsg->lParam,lpMsg->lParam,
                         lpMsg->time,lpMsg->time,
                         lpMsg->pt.x,lpMsg->pt.x,
                         lpMsg->pt.y,lpMsg->pt.y);
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_SIZE)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA WM_SIZE hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_ACTIVATE)
        {
            UINT active;
            UINT minimize;
            active = LOWORD(lpMsg->wParam);
            minimize = HIWORD(lpMsg->wParam);
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"(0x%08x)%s with%s minimize other window thread(0x%08x)\n",
                             active,
                             (active == WA_ACTIVE || active == WA_CLICKACTIVE) ? "activate":"deactivate" ,minimize ? "" : "out",
                             lpMsg->lParam);
        }
        else if(lpMsg->message == WM_MOUSEACTIVATE)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"MouseActivate (0x%08x) lParam 0x%08x\n",lpMsg->wParam,lpMsg->lParam);
        }
    }
    return bret;
}


BOOL WINAPI PeekMessageACallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg
)
{
    BOOL bret;
    SetUnHandlerExceptionDetour();
    bret = PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        FindCurrentActiveWindow();
        DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                         lpMsg->hwnd,lpMsg->message,lpMsg->message,
                         lpMsg->wParam,lpMsg->wParam,
                         lpMsg->lParam,lpMsg->lParam,
                         lpMsg->time,lpMsg->time,
                         lpMsg->pt.x,lpMsg->pt.x,
                         lpMsg->pt.y,lpMsg->pt.y);
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);

        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_ACTIVATE)
        {
            UINT active;
            UINT minimize;
            active = LOWORD(lpMsg->wParam);
            minimize = HIWORD(lpMsg->wParam);
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"(0x%08x)%s with%s minimize other window thread(0x%08x)\n",
                             active,
                             (active == WA_ACTIVE || active == WA_CLICKACTIVE) ? "activate":"deactivate" ,minimize ? "" : "out",
                             lpMsg->lParam);
        }
        else if(lpMsg->message == WM_MOUSEACTIVATE)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"MouseActivate (0x%08x) lParam 0x%08x\n",lpMsg->wParam,lpMsg->lParam);
        }

        if(lpMsg->message == WM_LBUTTONDBLCLK)
        {
            DEBUG_INFO("Left Button Double Click\n");
        }
    }
    return bret;
}


BOOL WINAPI GetMessageWCallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
)
{
    BOOL bret;

    bret = GetMessageWNext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        FindCurrentActiveWindow();
        DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                         lpMsg->hwnd,lpMsg->message,lpMsg->message,
                         lpMsg->wParam,lpMsg->wParam,
                         lpMsg->lParam,lpMsg->lParam,
                         lpMsg->time,lpMsg->time,
                         lpMsg->pt.x,lpMsg->pt.x,
                         lpMsg->pt.y,lpMsg->pt.y);
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            lpMsg->pt.x = 0;
            lpMsg->pt.y = 0;
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_ACTIVATE)
        {
            UINT active;
            UINT minimize;
            active = LOWORD(lpMsg->wParam);
            minimize = HIWORD(lpMsg->wParam);
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"(0x%08x)%s with%s minimize other window thread(0x%08x)\n",
                             active,
                             (active == WA_ACTIVE || active == WA_CLICKACTIVE) ? "activate":"deactivate" ,minimize ? "" : "out",
                             lpMsg->lParam);
        }
        else if(lpMsg->message == WM_MOUSEACTIVATE)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"MouseActivate (0x%08x) lParam 0x%08x\n",lpMsg->wParam,lpMsg->lParam);
        }
    }
    return bret;
}


BOOL WINAPI PeekMessageWCallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg
)
{
    BOOL bret;
    bret = PeekMessageWNext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        FindCurrentActiveWindow();
        DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                         lpMsg->hwnd,lpMsg->message,lpMsg->message,
                         lpMsg->wParam,lpMsg->wParam,
                         lpMsg->lParam,lpMsg->lParam,
                         lpMsg->time,lpMsg->time,
                         lpMsg->pt.x,lpMsg->pt.x,
                         lpMsg->pt.y,lpMsg->pt.y);
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_SIZE)
        {
            MSG_DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA WM_SIZE hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
                                 lpMsg->wParam,lpMsg->wParam,
                                 lpMsg->lParam,lpMsg->lParam,
                                 lpMsg->time,lpMsg->time,
                                 lpMsg->pt.x,lpMsg->pt.x,
                                 lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message == WM_ACTIVATE)
        {
            UINT active;
            UINT minimize;
            active = LOWORD(lpMsg->wParam);
            minimize = HIWORD(lpMsg->wParam);
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"(0x%08x)%s with%s minimize other window thread(0x%08x)\n",
                             active,
                             (active == WA_ACTIVE || active == WA_CLICKACTIVE) ? "activate":"deactivate" ,minimize ? "" : "out",
                             lpMsg->lParam);
        }
        else if(lpMsg->message == WM_MOUSEACTIVATE)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"MouseActivate (0x%08x) lParam 0x%08x\n",lpMsg->wParam,lpMsg->lParam);
        }
    }
    return bret;
}


int __MessageDetour(void)
{
    DEBUG_BUFFER_FMT(GetMessageANext,10,"Before GetMessageANext(0x%p)",GetMessageANext);
    DEBUG_BUFFER_FMT(PeekMessageANext,10,"Before PeekMessageANext(0x%p)",PeekMessageANext);
    DEBUG_BUFFER_FMT(GetMessageWNext,10,"Before GetMessageWNext(0x%p)",GetMessageWNext);
    DEBUG_BUFFER_FMT(PeekMessageWNext,10,"Before PeekMessageWNext(0x%p)",PeekMessageWNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&PeekMessageANext,PeekMessageACallBack);
    DetourAttach((PVOID*)&GetMessageANext,GetMessageACallBack);
    DetourAttach((PVOID*)&PeekMessageWNext,PeekMessageWCallBack);
    DetourAttach((PVOID*)&GetMessageWNext,GetMessageWCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(GetMessageANext,10,"After GetMessageANext(0x%p)",GetMessageANext);
    DEBUG_BUFFER_FMT(PeekMessageANext,10,"After PeekMessageANext(0x%p)",PeekMessageANext);
    DEBUG_BUFFER_FMT(GetMessageWNext,10,"After GetMessageWNext(0x%p)",GetMessageWNext);
    DEBUG_BUFFER_FMT(PeekMessageWNext,10,"After PeekMessageWNext(0x%p)",PeekMessageWNext);
    return 0;
}


int InsertEmulationMessageQueue(LPMSG lpMsg,int back)
{
    return 0;
}
