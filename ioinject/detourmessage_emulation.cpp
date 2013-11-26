
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
static uint8_t st_KeyStateArray[KEY_STATE_SIZE];
static uint8_t st_AsyncKeyStateArray[KEY_STATE_SIZE];


static int st_MessageEmualtionInited=0;

#define  EMULATIONMESSAGE_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  EMULATIONMESSAGE_DEBUG_INFO      DEBUG_INFO


int InsertEmulationMessageQueue(LPMSG lpMsg)
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

        st_MessageEmulationQueue.push_back(lcpMsg);
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


int SetKeyState(int scancode,int keydown)
{
	int ret;
	EnterCriticalSection(&st_KeyStateEmulationCS);
	LeaveCriticalSection(&st_KeyStateEmulationCS);
	return 0;
}

LPMSG GetEmulationMessageQueue()
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





