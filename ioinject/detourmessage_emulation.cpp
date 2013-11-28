
#include <vector>


static GetMessageFunc_t GetMessageANext= GetMessageA;
static PeekMessageFunc_t PeekMessageANext=PeekMessageA;
static GetMessageFunc_t GetMessageWNext= GetMessageW;
static PeekMessageFunc_t PeekMessageWNext=PeekMessageW;

static CRITICAL_SECTION st_MessageEmulationCS;
static std::vector<MSG*> st_MessageEmulationQueue;
static int st_MaxMessageEmulationQueue=20;
static uint32_t st_LastRightBtnUpTimeTick=0;
static uint32_t st_LastLeftBtnUpTimeTick=0;
static uint32_t st_LastMiddleBtnUpTimeTick=0;



static int st_MessageEmualtionInited=0;

#define  EMULATIONMESSAGE_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  EMULATIONMESSAGE_DEBUG_INFO      DEBUG_INFO

#define MAX_UINT32_VALUE 0xffffffffUL

int IsNotExpireTime(uint32_t prevtick,uint32_t curtick,uint32_t expire)
{
    if(curtick > prevtick)
    {
        if((curtick - prevtick) <= expire)
        {
            return 1;
        }
    }
    else
    {
        if((curtick + (MAX_UINT32_VALUE - prevtick)) <= expire)
        {
            return 1;
        }
    }

    return 0;
}

/*return 1 for double click will insert ,
0 for not
negative for error ,not insert into it*/
int __InsertMessageQueue(LPMSG lpMsg,int back)
{
    int ret;
    LPMSG lpRemove=NULL;
    if(st_MessageEmualtionInited)
    {
        uint32_t curtick = GetTickCount();
        ret = 0;
        EnterCriticalSection(&st_MessageEmulationCS);
        if(lpMsg->message == WM_LBUTTONUP)
        {
            ret = IsNotExpireTime(st_LastLeftBtnUpTimeTick,curtick,500);
            st_LastLeftBtnUpTimeTick = curtick;
        }
        else if(lpMsg->message == WM_RBUTTONUP)
        {
            ret = IsNotExpireTime(st_LastRightBtnUpTimeTick,curtick,500);
            st_LastRightBtnUpTimeTick = curtick;
        }
        else if(lpMsg->message == WM_MBUTTONUP)
        {
            ret = IsNotExpireTime(st_LastMiddleBtnUpTimeTick,curtick,500);
            st_LastMiddleBtnUpTimeTick = curtick;
        }

        if(back)
        {
            if(st_MessageEmulationQueue.size() > st_MaxMessageEmulationQueue)
            {
                lpRemove = st_MessageEmulationQueue[0];
                st_MessageEmulationQueue.erase(st_MessageEmulationQueue.begin());
            }
            st_MessageEmulationQueue.push_back(lpMsg);
        }
        else
        {
            st_MessageEmulationQueue.insert(st_MessageEmulationQueue.begin(),lpMsg);
        }
        LeaveCriticalSection(&st_MessageEmulationCS);

        if(lpRemove)
        {
            EMULATIONMESSAGE_DEBUG_INFO("remove 0x%p Message Code(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d)\n",
                                        lpRemove,lpRemove->message,lpRemove->message,
                                        lpRemove->wParam,lpRemove->wParam,
                                        lpRemove->lParam,lpRemove->lParam);
            free(lpRemove);
        }
        lpRemove = NULL;
        return ret;
    }
    ret = ERROR_NOT_INITED;
    SetLastError(ret);
    return -1;
}

int InsertEmulationMessageQueue(LPMSG lpMsg,int back)
{
    int ret=-ERROR_NOT_SUPPORTED;
    LPMSG lcpMsg=NULL;
    if(st_MessageEmualtionInited)
    {
        lcpMsg = calloc(sizeof(*lcpMsg),1);
        if(lcpMsg == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return -ret;
        }
        CopyMemory(lcpMsg,lpMsg,sizeof(*lpMsg));
        ret = __InsertMessageQueue(lcpMsg,back);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            free(lcpMsg);
            return -ret;
        }
        lcpMsg = NULL;

        if(ret > 0)
        {
            /*it will insert double click message*/
            lcpMsg = calloc(sizeof(*lcpMsg),1);
            if(lcpMsg == NULL)
            {
                ret = LAST_ERROR_CODE();
                SetLastError(ret);
                return -ret;
            }

            CopyMemory(lcpMsg,lpMsg,sizeof(*lcpMsg));
            if(lpMsg->message == WM_LBUTTONUP)
            {
                lcpMsg->message = WM_LBUTTONDBLCLK;
            }
            else if(lpMsg->message == WM_RBUTTONUP)
            {
                lcpMsg->message = WM_RBUTTONDBLCLK;
            }
            else if(lpMsg->message == WM_MBUTTONUP)
            {
                lcpMsg->message = WM_MBUTTONDBLCLK;
            }
            else
            {
                assert(0!=0);
            }
            ret = __InsertMessageQueue(lcpMsg,1);
            assert(ret == 0);
        }

        SetLastError(0);
        return 0;
    }
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return -ret;
}


int InsertMessageDevEvent(LPDEVICEEVENT pDevEvent)
{
	
}

LPMSG __GetEmulationMessageQueue()
{
    LPMSG lGetMsg=NULL;
    if(st_MessageEmualtionInited)
    {
        EnterCriticalSection(&st_MessageEmulationCS);
        if(st_MessageEmulationQueue.size() > 0)
        {
            lGetMsg = st_MessageEmulationQueue[0];
            st_MessageEmulationQueue.erase(st_MessageEmulationQueue.begin());
        }
        LeaveCriticalSection(&st_MessageEmulationCS);
    }
    else
    {
        return NULL;
    }

    return lGetMsg;

}



int __GetKeyMouseMessage(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax,UINT remove)
{
    LPMSG lGetMsg=NULL;
    int ret = 0,res;

    lGetMsg = __GetEmulationMessageQueue();
    if(lGetMsg == NULL)
    {
        ret = 0;
        goto out;
    }

    /*now to compare whether it is the ok*/
    if(wMsgFilterMin == 0 && wMsgFilterMax == 0)
    {
        CopyMemory(lpMsg,lGetMsg,sizeof(*lGetMsg));
        if(!(remove & PM_REMOVE))
        {
            /*not remove ,so we put back*/
            res = InsertEmulationMessageQueue(lGetMsg,0);
            assert(res >= 0);
        }
        ret = 1;
    }
    else if(lGetMsg->message >= wMsgFilterMin && lGetMsg->message <= wMsgFilterMax)
    {
        CopyMemory(lpMsg,lGetMsg,sizeof(*lGetMsg));
        if(!(remove & PM_REMOVE))
        {
            /*not remove ,so we put back*/
            res = InsertEmulationMessageQueue(lGetMsg,0);
            assert(res >= 0);
        }
        ret = 1;
    }
    else
    {
        /*now in it ,so we do not use this*/
        res = InsertEmulationMessageQueue(lGetMsg,0);
        assert(res >= 0);
        ret = 0;
    }

out:
    if(lGetMsg)
    {
        free(lGetMsg);
    }
    lGetMsg = NULL;
    SetLastError(0);
    return ret;

fail:
    assert(ret > 0);
    if(lGetMsg)
    {
        res = InsertEmulationMessageQueue(lGetMsg,0);
        assert(res >= 0);
        free(lGetMsg);
    }
    lGetMsg = NULL;
    SetLastError(ret);
    return -ret;
}

BOOL WINAPI GetMessageACallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
)
{
    BOOL bret;
    int ret;

try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,PM_REMOVE);
    if(ret > 0)
    {
        return TRUE;
    }


    bret = GetMessageANext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST))
        {
            /*we discard this message*/
            goto try_again;
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
    int ret;

try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
    if(ret > 0)
    {
        return TRUE;
    }

    bret = PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST))
        {
            if(!(wRemoveMsg & PM_REMOVE))
            {
                /*this means not remove ,so we should do this removed*/
                PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                                 wMsgFilterMax,wRemoveMsg|PM_REMOVE);

            }
            goto try_again;
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
    int ret;

try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,PM_REMOVE);
    if(ret > 0)
    {
        return TRUE;
    }

    bret = GetMessageWNext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST))
        {
            /*we discard this message ,so get the next one*/
            goto try_again;
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
    int ret;

try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
    if(ret > 0)
    {
        return TRUE;
    }

    bret = PeekMessageWNext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST))
        {
            if(!(wRemoveMsg & PM_REMOVE))
            {
                /*this means not remove ,so we should do this removed*/
                PeekMessageWNext(lpMsg,hWnd,wMsgFilterMin,
                                 wMsgFilterMax,wRemoveMsg|PM_REMOVE);

            }
            goto try_again;
        }
    }
    return bret;
}



int __MessageDetour(void)
{
    InitializeCriticalSection(&st_MessageEmulationCS);
    InitializeCriticalSection(&st_KeyStateEmulationCS);
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
    st_MessageEmualtionInited = 1;
    return 0;
}





