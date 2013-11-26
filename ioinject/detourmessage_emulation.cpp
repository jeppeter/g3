
#include <vector>

#define  KEY_STATE_SIZE   256

static GetMessageFunc_t GetMessageANext= GetMessageA;
static PeekMessageFunc_t PeekMessageANext=PeekMessageA;
static GetMessageFunc_t GetMessageWNext= GetMessageW;
static PeekMessageFunc_t PeekMessageWNext=PeekMessageW;

static CRITICAL_SECTION st_MessageEmulationCS;
static std::vector<MSG*> st_MessageEmulationQueue;
static int st_MaxMessageEmulationQueue=20;

static CRITICAL_SECTION  st_KeyStateEmulationCS;
static uint16_t st_KeyStateArray[KEY_STATE_SIZE]={0};
static uint16_t st_AsyncKeyStateArray[KEY_STATE_SIZE]={0};


static int st_MessageEmualtionInited=0;

#define  EMULATIONMESSAGE_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  EMULATIONMESSAGE_DEBUG_INFO      DEBUG_INFO


int InsertEmulationMessageQueue(LPMSG lpMsg,int back)
{
    int ret=-ERROR_NOT_SUPPORTED;
    LPMSG lcpMsg=NULL,lpRemove=NULL;
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
        ret = 1;
        EnterCriticalSection(&st_MessageEmulationCS);
        if(st_MessageEmulationQueue.size() >= st_MaxMessageEmulationQueue)
        {
            /*now we should remove the message*/
            ret = 0;
            lpRemove = st_MessageEmulationQueue[0];
            st_MessageEmulationQueue.erase(st_MessageEmulationQueue.begin());
        }

        if(back)
        {
            st_MessageEmulationQueue.push_back(lcpMsg);
        }
        else
        {
            st_MessageEmulationQueue.insert(st_MessageEmulationQueue.begin(),lcpMsg);
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
    }
    else
    {
        SetLastError(ret);
        return -ret;
    }
    return ret;
}


#define  ASYNC_KEY_PRESSED_STATE   0x8000
#define  ASYNC_KEY_TOGGLED_STATE   0x1

#define  KEY_PRESSED_STATE         0x8000
#define  KEY_TOGGLE_STATE          0x1

int SetKeyState(UINT vsk,int keydown)
{
    int ret;
    UINT vk;
    vk = MapVirtualKey(vsk,MAPVK_VSC_TO_VK_EX);
    if(vk == 0)
    {
        /*if not return ,just not set*/
        return 0;
    }
    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(keydown)
    {
        st_KeyStateArray[vk] |= KEY_PRESSED_STATE;

        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_PRESSED_STATE;
        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_PRESSED_STATE;
    }
    else
    {
        st_KeyStateArray[vk] &=~(KEY_PRESSED_STATE) ;
        if(st_KeyStateArray[vk] & KEY_TOGGLE_STATE)
        {
            st_KeyStateArray[vk] &= ~(KEY_TOGGLE_STATE);
        }
        else
        {
            st_KeyStateArray[vk] |= KEY_TOGGLE_STATE;
        }

        st_AsyncKeyStateArray[vk] &= ~(ASYNC_KEY_PRESSED_STATE);
        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_TOGGLED_STATE;

    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return 0;
}

USHORT __InnerGetKeyState(UINT vk)
{
    USHORT uret;
    UINT uvk[2];
    UINT exvk[2];
    int expanded=0;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }

    if(vk == VK_CONTROL)
    {
        expanded = 1;
        exvk[0] = VK_LCONTROL;
        exvk[1] = VK_RCONTROL;
    }
    else if(vk == VK_MENU)
    {
        expanded = 1;
        exvk[0] = VK_LMENU;
        exvk[1] = VK_RMENU;
    }
    else if(vk == VK_SHIFT)
    {
        expanded = 1;
        exvk[0] = VK_LSHIFT;
        exvk[1] = VK_RSHIFT;
    }

    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(expanded)
    {
        uvk[0] = st_KeyStateArray[exvk[0]];
        uvk[1] = st_KeyStateArray[exvk[1]];

        /*
        	we call the last more key value ,so it will return for more
        */
        if(uvk[0] > uvk[1])
        {
            uret = uvk[0];
        }
        else
        {
            uret = uvk[1];
        }

    }
    else
    {
        uret = st_KeyStateArray[vk];
    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return uret;
}

USHORT __InnerGetAsynState(UINT vk)
{
    USHORT uret;
    UINT uvk[2];
    UINT exvk[2];
    int expanded=0;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }
    if(vk == VK_CONTROL)
    {
        expanded = 1;
        exvk[0] = VK_LCONTROL;
        exvk[1] = VK_RCONTROL;
    }
    else if(vk == VK_MENU)
    {
        expanded = 1;
        exvk[0] = VK_LMENU;
        exvk[1] = VK_RMENU;
    }
    else if(vk == VK_SHIFT)
    {
        expanded = 1;
        exvk[0] = VK_LSHIFT;
        exvk[1] = VK_RSHIFT;
    }
    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(expanded)
    {
        uvk[0] = st_AsyncKeyStateArray[exvk[0]];
        uvk[1] = st_AsyncKeyStateArray[exvk[1]];
        st_AsyncKeyStateArray[exvk[0]] &= ~(ASYNC_KEY_TOGGLED_STATE);
        st_AsyncKeyStateArray[exvk[1]] &= ~(ASYNC_KEY_TOGGLED_STATE);

        if(uvk[0] > uvk[1])
        {
            uret = uvk[0];
        }
        else
        {
            uret = uvk[1];
        }
    }
    else
    {
        uret = st_AsyncKeyStateArray[vk];
        st_AsyncKeyStateArray[vk] &= ~(ASYNC_KEY_TOGGLED_STATE);
    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return uret;
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





