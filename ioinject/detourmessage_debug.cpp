
GetMessageFunc_t GetMessageANext= GetMessageA;
PeekMessageFunc_t PeekMessageANext=PeekMessageA;
GetMessageFunc_t GetMessageWNext= GetMessageW;
PeekMessageFunc_t PeekMessageWNext=PeekMessageW;


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
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
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
    bret = PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
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
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
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
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW Keyboard hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW Mouse hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
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

