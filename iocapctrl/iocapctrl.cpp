
#include "iocapctrl.h"
#include <assert.h>
#include <injectctrl.h>
#include <output_debug.h>
#include <sched.h>
#include <evt.h>
#include <dllinsert.h>
#include <memshare.h>

#define  IO_FREE_EVT_BASENAME               "GlobalIoInjectFreeEvt"
#define  IO_INPUT_EVT_BASENAME              "GlobalIoInjectInputEvt"
#define  IO_MAP_MEM_BASENAME                "GlobalIoInjectMapMem"

#define  IO_MAP_CURSOR_BITMAP_INFO_BASENAME  "GlobalIoInjectCursorBitmapInfo"
#define  IO_MAP_CURSOR_BITMAP_DATA_BASENAME  "GlobalIoInjectCursorBitmapData"

#ifdef _DEBUG
#define IOINJECT_DLL  "ioinjectd.dll"
#else
#define IOINJECT_DLL  "ioinject.dll"
#endif

#define IOINJECT_CONTROL_FUNC  "IoInjectControl"

CIOController::CIOController()
{
    m_hProc = NULL;
    m_Pid = 0;
    ZeroMemory(m_TypeIds,sizeof(m_TypeIds));
    ZeroMemory(&m_BackGroundThread,sizeof(m_BackGroundThread));
    m_BackGroundThread.exited = 1;
    InitializeCriticalSection(&(m_EvtCS));
    m_Started = 0;
    ZeroMemory(&m_MemShareName,sizeof(m_MemShareName));
    m_BufferNum = 0;
    m_BufferSectSize = 0;
    m_BufferTotalSize = 0;
    m_pMemShareBase = NULL;
    m_hMapFile = NULL;
    ZeroMemory(m_FreeEvtBaseName,sizeof(m_FreeEvtBaseName));
    m_pFreeTotalEvts = NULL;
    ZeroMemory(m_InputEvtBaseName,sizeof(m_InputEvtBaseName));
    m_pInputTotalEvts = NULL;
    m_pIoCapEvents = NULL;
    assert(m_InputEvts.size() == 0);
    assert(m_FreeEvts.size() == 0);
    assert(m_WaitEvts.size() == 0);
    m_InsertEvts = 0;
    m_UnPressedKey = -1;
    m_SeqId = 0;
    m_CurPointSeqId = 0;
    m_CurPoint.x = 0;
    m_CurPoint.y = 0;
}


CIOController::~CIOController()
{
    this->Stop();
    /*now we should free the Critical Section and HANDLE*/
    DeleteCriticalSection(&(m_EvtCS));
}


void CIOController::__StopBackGroundThread()
{
    /*now first to make sure it is exited*/
    StopThreadControl(&(this->m_BackGroundThread));
}

int CIOController::__StartBackGroundThread()
{
    return StartThreadControl(&(this->m_BackGroundThread),CIOController::ThreadProc,this,1);
}


PIO_CAP_EVENTS_t CIOController::__GetInputEvent(DWORD idx)
{
    PIO_CAP_EVENTS_t pIoCapEvt=NULL;
    int findidx = -1;
    UINT i;
    int ret=ERROR_NO_DATA;
    if(this->m_Started == 0)
    {
        SetLastError(ret);
        return NULL;
    }

    EnterCriticalSection(&(this->m_EvtCS));
    for(i=0; i<this->m_InputEvts.size() ; i++)
    {
        if(this->m_InputEvts[i]->Idx == idx)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        pIoCapEvt = this->m_InputEvts[findidx];
        this->m_InputEvts.erase(this->m_InputEvts.begin() + findidx);
        ret = 0;
    }
    LeaveCriticalSection(&(this->m_EvtCS));

    SetLastError(ret);
    return pIoCapEvt;
}

PIO_CAP_EVENTS_t CIOController::__GetWaitEvent()
{
    PIO_CAP_EVENTS_t pIoCapEvt=NULL;
    int ret=ERROR_NO_DATA;
    assert(this->m_InsertEvts > 0);
    EnterCriticalSection(&(this->m_EvtCS));
    if(this->m_WaitEvts.size() > 0)
    {
        ret = 0;
        pIoCapEvt = this->m_WaitEvts[0];
        this->m_WaitEvts.erase(this->m_WaitEvts.begin());
    }
    LeaveCriticalSection(&(this->m_EvtCS));
    SetLastError(ret);
    return pIoCapEvt;
}

BOOL CIOController::__InsertWaitEvent(PIO_CAP_EVENTS_t pIoCapEvt)
{
    LPSEQ_CLIENTMOUSEPOINT pMousePoint=NULL;
    int findidx = -1;
    UINT i;

    assert(this->m_InsertEvts > 0);
    pMousePoint = (LPSEQ_CLIENTMOUSEPOINT)pIoCapEvt->pEvent;
    EnterCriticalSection(&(this->m_EvtCS));
    for(i=0; i<this->m_WaitEvts.size() ; i++)
    {
        if(this->m_WaitEvts[i]->seqid > pMousePoint->seqid)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        this->m_WaitEvts.insert(this->m_WaitEvts.begin() + findidx,pIoCapEvt);
    }
    else
    {
        this->m_WaitEvts.push_back(pIoCapEvt);
    }
    LeaveCriticalSection(&(this->m_EvtCS));

    return TRUE;
}

BOOL CIOController::__InsertFreeEvent(PIO_CAP_EVENTS_t pIoCapEvt)
{
    int findidx=-1;
    UINT i;
    BOOL bret=FALSE;

    EnterCriticalSection(&(this->m_EvtCS));
    for(i=0; i<this->m_FreeEvts.size(); i++)
    {
        if(this->m_FreeEvts[i] == pIoCapEvt)
        {
            findidx = i;
            ERROR_INFO("<0x%p> In FreeEvts[%d]\n",pIoCapEvt,i);
            break;
        }
    }

    if(findidx < 0)
    {
        for(i=0; i<this->m_InputEvts.size(); i++)
        {
            if(this->m_InputEvts[i] == pIoCapEvt)
            {
                findidx = i;
                ERROR_INFO("<0x%p> In InputEvts[%d]\n",pIoCapEvt,i);
                break;
            }
        }
    }

    if(findidx < 0)
    {
        this->m_FreeEvts.push_back(pIoCapEvt);
        bret = TRUE;
    }
    LeaveCriticalSection(&(this->m_EvtCS));
    return bret;
}

BOOL CIOController::__HandleFreeEvent(DWORD idx)
{
    BOOL bret;
    int ret,res;
    int tries;
    LPSEQ_CLIENTMOUSEPOINT pMousePoint;
    PIO_CAP_EVENTS_t pIoCapEvt=NULL;
    tries = 0;

    while(1)
    {
        assert(pIoCapEvt == NULL);
        pIoCapEvt = this->__GetInputEvent(idx);
        if(pIoCapEvt)
        {
            break;
        }
        tries ++;
        if(tries > 5)
        {
            ERROR_INFO("Wait idx (%d) Timeout\n",idx);
            goto fail;
        }
        SchedOut();
    }

    pMousePoint = (LPSEQ_CLIENTMOUSEPOINT) pIoCapEvt->pEvent;
    if((this->m_CurPointSeqId + 1) == pMousePoint->seqid)
    {
        bret = this->__SetCurPoint(pMousePoint->x,pMousePoint->y);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could not set point(%d:%d) Error(%d)\n",pMousePoint->x,pMousePoint->y,ret);
            goto fail;
        }

        bret = this->__InsertFreeEvent(pIoCapEvt);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Insert FreeEvent Error(%d)\n",ret);
            goto fail;
        }
        pIoCapEvt = NULL;
        this->m_CurPointSeqId ++;

        /*if we have insert it into the wait ,so we should check for the seqid*/
        while(1)
        {
            assert(pIoCapEvt == NULL);
            pIoCapEvt = this->__GetWaitEvent();
            if(pIoCapEvt == NULL)
            {
                break;
            }

            pMousePoint = (LPSEQ_CLIENTMOUSEPOINT) pIoCapEvt->pEvent;
            if((this->m_CurPointSeqId + 1) == pMousePoint->seqid)
            {
                bret = this->__SetCurPoint(pMousePoint->x,pMousePoint->y);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("could not set point(%d:%d) Error(%d)\n",pMousePoint->x,pMousePoint->y,ret);
                    goto fail;
                }

                bret = this->__InsertFreeEvent(pIoCapEvt);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("Insert FreeEvent Error(%d)\n",ret);
                    goto fail;
                }
                pIoCapEvt = NULL;
                this->m_CurPointSeqId ++;
            }
            else
            {
                bret = this->__InsertWaitEvent(pIoCapEvt);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("Insert WaitEvent Error(%d)\n",ret);
                    goto fail;
                }
                pIoCapEvt = NULL;
				/*we break out ,for it will for next wait*/
				break;
            }
        }

    }
    else if((this->m_CurPointSeqId) < pMousePoint->seqid)
    {
        bret = this->__InsertWaitEvent(pIoCapEvt);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Insert WaitEvent Error(%d)\n",ret);
            goto fail;
        }
        pIoCapEvt = NULL;
    }
    else
    {
        ERROR_INFO("[%d]seqid (%lld) > mouseid (%lld)\n",idx,this->m_CurPointSeqId,pMousePoint->seqid);
        bret = this->__InsertFreeEvent(pIoCapEvt);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Insert FreeEvent Error(%d)\n",ret);
            goto fail;
        }
        pIoCapEvt = NULL;
    }

    assert(pIoCapEvt == NULL);
    SetLastError(0);
    return TRUE;
fail:
    if(pIoCapEvt)
    {
        bret = this->__InsertWaitEvent(pIoCapEvt);
        if(!bret)
        {
            res = LAST_ERROR_CODE();
            ERROR_INFO("<0x%p> Reinsert Error(%d)\n",pIoCapEvt,res);
        }
    }

    pIoCapEvt = NULL;
    SetLastError(ret);
    return FALSE;
}


DWORD CIOController::__ThreadImpl()
{
    HANDLE* pWaitHandles=NULL;
    DWORD dret,idx;
    int waitnum = 0;
    int tries=0,ret;
    BOOL bret;
    int cont =0;

    /*including the exit notify event*/
    assert(this->m_BufferNum > 0);
    waitnum = this->m_BufferNum + 1;
    pWaitHandles = (HANDLE*)calloc(waitnum,sizeof(*pWaitHandles));
    if(pWaitHandles == NULL)
    {
        dret = LAST_ERROR_CODE();
        goto out;
    }

    /*now copy for the wait handler*/
    CopyMemory(pWaitHandles,this->m_pFreeTotalEvts,sizeof(*pWaitHandles)*this->m_BufferNum);

    /*now to set the last one*/
    assert(this->m_BackGroundThread.exitevt);
    pWaitHandles[waitnum - 1] = this->m_BackGroundThread.exitevt;


    while(this->m_BackGroundThread.running)
    {
        dret = WaitForMultipleObjectsEx(waitnum,pWaitHandles,FALSE,INFINITE,TRUE);
        if(dret >= WAIT_OBJECT_0 && dret <= (WAIT_OBJECT_0 + waitnum -2))
        {
            idx = dret - WAIT_OBJECT_0;
            /*now first to make sure we get the event*/
            bret = this->__HandleFreeEvent(idx);
            if(!bret)
            {
            	ret = LAST_ERROR_CODE();
				ERROR_INFO("[%d] InputFreeEvent Error(%d)\n",idx,ret);
                goto out;
            }

        }
        else if(dret == (WAIT_OBJECT_0+waitnum - 1))
        {
            DEBUG_INFO("<0x%p> thread exit notify\n",this);
        }
        else if(dret = WAIT_FAILED)
        {
            dret = LAST_ERROR_CODE();
            ERROR_INFO("<0x%p> thread wait failed error(%d)\n",this,dret);
            goto out;
        }
    }

    dret = 0;

out:
    if(pWaitHandles)
    {
        free(pWaitHandles);
    }
    pWaitHandles = NULL;
    SetLastError(dret);
    this->m_BackGroundThread.exited = 1;
    return dret;
}

DWORD WINAPI CIOController::ThreadProc(LPVOID pParam)
{
    CIOController *pThis = (CIOController*)pParam;
    return pThis->__ThreadImpl();
}

void CIOController::__ReleaseAllEvents()
{
    PIO_CAP_EVENTS_t pIoCapEvent=NULL;
    unsigned int i;
    /*we make sure in the exited thread mode call this function*/
    assert(this->m_BackGroundThread.exited > 0);


    if(this->m_pFreeTotalEvts)
    {
        for(i=0; i<this->m_BufferNum; i++)
        {
            if(this->m_pFreeTotalEvts[i])
            {
                CloseHandle(this->m_pFreeTotalEvts[i]);
            }
            this->m_pFreeTotalEvts[i] = NULL;
        }

        free(this->m_pFreeTotalEvts);
    }
    this->m_pFreeTotalEvts = NULL;
    ZeroMemory(this->m_FreeEvtBaseName,sizeof(this->m_FreeEvtBaseName));



    if(this->m_pInputTotalEvts)
    {
        for(i=0; i<this->m_BufferNum; i++)
        {
            if(this->m_pInputTotalEvts[i])
            {
                CloseHandle(this->m_pInputTotalEvts[i]);
            }
            this->m_pInputTotalEvts[i] = NULL;
        }
        free(this->m_pInputTotalEvts);
    }
    this->m_pInputTotalEvts = NULL;
    ZeroMemory(this->m_InputEvtBaseName,sizeof(this->m_InputEvtBaseName));

    return ;
}

int CIOController::__AllocateAllEvents()
{
    /*now we should */
    uint8_t fullname[IO_NAME_MAX_SIZE];
    uint8_t curbasename[IO_NAME_MAX_SIZE];
    uint32_t pid,i;
    int ret;

    this->__ReleaseAllEvents();
    if(this->m_hProc == NULL || this->m_BufferNum == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    pid = this->m_Pid;

    _snprintf_s((char*)curbasename,sizeof(curbasename),_TRUNCATE,"%s%d",IO_FREE_EVT_BASENAME,pid);
    this->m_pFreeTotalEvts = (HANDLE*)calloc(this->m_BufferNum,sizeof(this->m_pFreeTotalEvts[0]));
    if(this->m_pFreeTotalEvts == NULL)
    {
        ret = LAST_ERROR_CODE();
        this->__ReleaseAllEvents();
        SetLastError(ret);
        return -ret;
    }

    for(i=0; i<this->m_BufferNum; i++)
    {
        _snprintf_s((char*)fullname,sizeof(fullname),_TRUNCATE,"%s_%d",curbasename,i);
        this->m_pFreeTotalEvts[i]= GetEvent((char*)fullname,1);
        if(this->m_pFreeTotalEvts[i] == NULL)
        {
            ret =LAST_ERROR_CODE();
            ERROR_INFO("<0x%08x> create %s event Error(%d)\n",this->m_hProc,fullname,ret);
            this->__ReleaseAllEvents();
            SetLastError(ret);
            return -ret;
        }
        DEBUG_INFO("[%d] FreeEvts name(%s)\n",i,fullname);
    }

    strncpy_s((char*)this->m_FreeEvtBaseName,sizeof(this->m_FreeEvtBaseName),(const char*)curbasename,_TRUNCATE);

    /*now for the input event*/
    this->m_pInputTotalEvts =(HANDLE*) calloc(this->m_BufferNum,sizeof(this->m_pInputTotalEvts[0]));
    if(this->m_pInputTotalEvts == NULL)
    {
        ret = LAST_ERROR_CODE();
        this->__ReleaseAllEvents();
        SetLastError(ret);
        return -ret;
    }

    _snprintf_s((char*)curbasename,sizeof(curbasename),_TRUNCATE,"%s%d",IO_INPUT_EVT_BASENAME,pid);
    for(i=0; i<this->m_BufferNum; i++)
    {
        _snprintf_s((char*)fullname,sizeof(fullname),_TRUNCATE,"%s_%d",curbasename,i);
        this->m_pInputTotalEvts[i] = GetEvent((char*)fullname,1);
        if(this->m_pInputTotalEvts[i] == NULL)
        {
            ret =LAST_ERROR_CODE();
            ERROR_INFO("<0x%08x> create %s event Error(%d)\n",this->m_hProc,fullname,ret);
            this->__ReleaseAllEvents();
            SetLastError(ret);
            return -ret;
        }
        DEBUG_INFO("[%d] InputEvts (%s)\n",i,fullname);
    }
    strncpy_s((char*)this->m_InputEvtBaseName,sizeof(this->m_InputEvtBaseName),(const char*)curbasename,_TRUNCATE);
    return 0;
}

int CIOController::__CallInnerControl(PIO_CAP_CONTROL_t pControl,int timeout)
{
    uint32_t pid;
    int ret,res;
    void* pFnAddr=NULL;
    PIO_CAP_CONTROL_t pRemoteControl=NULL;
    BOOL bret;
    SIZE_T retsize;
    int retval;

    if(this->m_hProc == NULL)
    {
        /*this is ok to no process specify*/
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    pid = this->m_Pid;

    /*now we should get the address of the */
    ret = GetRemoteProcAddress(pid,IOINJECT_DLL,IOINJECT_CONTROL_FUNC,&pFnAddr);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not find[%d] %s:%s Error(%d)\n",pid,IOINJECT_DLL,IOINJECT_CONTROL_FUNC,ret);
        SetLastError(ret);
        return -ret;
    }

    pRemoteControl =(PIO_CAP_CONTROL_t) VirtualAllocEx(this->m_hProc,NULL,sizeof(*pRemoteControl),MEM_COMMIT,PAGE_READWRITE);
    if(pRemoteControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%08x> size(%d) Error(%d)\n",
                   this->m_hProc,sizeof(*pRemoteControl),ret);
        goto fail;
    }

    bret = WriteProcessMemory(this->m_hProc,pRemoteControl,pControl,sizeof(*pRemoteControl),&retsize);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%08x> write(0x%p:0x%08x) Error(%d)\n",
                   this->m_hProc,pRemoteControl,sizeof(*pRemoteControl),ret);
        goto fail;
    }
    if(retsize != sizeof(*pRemoteControl))
    {
        ret = ERROR_INVALID_BLOCK;
        ERROR_INFO("<0x%08x> write(0x%p:0x%08x) Return(%d)\n",
                   this->m_hProc,pRemoteControl,sizeof(*pRemoteControl),retsize);
        goto fail;
    }


    ret = CallRemoteFuncRemoteParam(pid,pFnAddr,pRemoteControl,timeout,(void**)&retval);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Call ioinject.dll:DetourDirectInputControl Error(%d)\n",ret);
        goto fail;
    }

    if(retval != 0)
    {
        ret = retval > 0  ? retval : -retval;
        ERROR_INFO("Call ioinject.dll:DetourDirectInputControl Return value(%d)\n",retval);
        goto fail;
    }

    /*all is ok*/

    if(pRemoteControl)
    {
        bret = VirtualFreeEx(this->m_hProc,pRemoteControl,0,MEM_RELEASE);
        if(!bret)
        {
            res = LAST_ERROR_CODE();
            ERROR_INFO("FreeEx(0x%08x:0x%p) Error(%d)\n",this->m_hProc,pRemoteControl,ret);
        }
    }
    pRemoteControl = NULL;
    SetLastError(0);
    return 0;
fail:
    if(pRemoteControl)
    {
        bret = VirtualFreeEx(this->m_hProc,pRemoteControl,0,MEM_RELEASE);
        if(!bret)
        {
            res = LAST_ERROR_CODE();
            ERROR_INFO("FreeEx(0x%08x:0x%p) Error(%d)\n",this->m_hProc,pRemoteControl,ret);
        }
    }
    pRemoteControl = NULL;
    SetLastError(ret);
    return -ret;
}


void CIOController::__ReleaseCapEvents()
{
    int fullevents;
    int tries =0;
    assert(this->m_Started == 0);

    /*we must make sure this is the iocap events ok*/
    while(this->m_InsertEvts)
    {
        fullevents = 0;
        EnterCriticalSection(&(this->m_EvtCS));
        if((this->m_InputEvts.size() + this->m_FreeEvts.size() + this->m_WaitEvts.size())==this->m_BufferNum)
        {
            fullevents = 1;
        }
        LeaveCriticalSection(&(this->m_EvtCS));

        if(fullevents > 0)
        {
            break;
        }

        SchedOut();
        tries ++;
        ERROR_INFO("Wait %d FreeEvents tries(%d)\n",this->m_BufferNum,tries);
    }
    this->m_InputEvts.clear();
    this->m_FreeEvts.clear();
    this->m_WaitEvts.clear();
    this->m_InsertEvts = 0;
    if(this->m_pIoCapEvents)
    {
        free(this->m_pIoCapEvents);
    }
    this->m_pIoCapEvents = NULL;
    return ;
}

int CIOController::__AllocateCapEvents()
{
    int ret;
    unsigned int i;
    if(this->m_hProc == NULL || this->m_BufferNum == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    this->__ReleaseCapEvents();

    this->m_pIoCapEvents = (PIO_CAP_EVENTS_t)calloc(this->m_BufferNum,sizeof(this->m_pIoCapEvents[0]));
    if(this->m_pIoCapEvents == NULL)
    {
        ret = LAST_ERROR_CODE();
        this->__ReleaseCapEvents();
        SetLastError(ret);
        return -ret;
    }

    for(i=0; i<this->m_BufferNum; i++)
    {
        this->m_pIoCapEvents[i].hEvent = this->m_pInputTotalEvts[i];
        this->m_pIoCapEvents[i].Idx = i;
        this->m_pIoCapEvents[i].pEvent = (LPSEQ_DEVICEEVENT)(this->m_pMemShareBase + this->m_BufferSectSize * i);
        this->m_FreeEvts.push_back(&(this->m_pIoCapEvents[i]));
    }

    this->m_InsertEvts = 1;

    SetLastError(0);
    return 0;
}

void CIOController::__ReleaseMapMem()
{
    UnMapFileBuffer((unsigned char**)&(this->m_pMemShareBase));
    CloseMapFileHandle(&(this->m_hMapFile));
    ZeroMemory(this->m_MemShareName,sizeof(this->m_MemShareName));
    return;
}

int CIOController::__AllocateMapMem()
{
    int ret;
    if(this->m_hProc == NULL || this->m_BufferNum ==0 ||
            this->m_BufferSectSize == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }
    this->__ReleaseMapMem();

    _snprintf_s((char*)this->m_MemShareName,sizeof(this->m_MemShareName),_TRUNCATE,"%s%d",IO_MAP_MEM_BASENAME,this->m_Pid);
    this->m_hMapFile = CreateMapFile((char*)this->m_MemShareName,this->m_BufferTotalSize,1);
    if(this->m_hMapFile == NULL)
    {
        ret =LAST_ERROR_CODE();
        ERROR_INFO("CreateMemMap (%s) size(0x%08x) Error(%d)\n",this->m_MemShareName,this->m_BufferTotalSize,ret);
        this->__ReleaseMapMem();
        return -ret;
    }

    this->m_pMemShareBase = (ptr_t)MapFileBuffer(this->m_hMapFile,this->m_BufferTotalSize);
    if(this->m_pMemShareBase == NULL)
    {
        ret =LAST_ERROR_CODE();
        ERROR_INFO("MapFileBuffer (%s) size(0x%08x) Error(%d)\n",this->m_MemShareName,this->m_BufferTotalSize,ret);
        this->__ReleaseMapMem();
        return -ret;
    }

    DEBUG_INFO("Memsahre(%s)0x%p bufnum %d bufsectsize %d\n",this->m_MemShareName,this->m_pMemShareBase,this->m_BufferNum,this->m_BufferSectSize);

    SetLastError(0);
    return 0;
}

int CIOController::__CallStopIoCapControl()
{
    PIO_CAP_CONTROL_t pControl=NULL;
    int ret;

    if(this->m_hProc == NULL)
    {
        SetLastError(0);
        return 0;
    }

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    pControl->opcode = IO_INJECT_STOP;

    ret = this->__CallInnerControl(pControl,3);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("call stop Error(%d)\n",ret);
        goto fail;
    }

    free(pControl);
    pControl = NULL;
    SetLastError(0);
    return 0;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return -ret;
}

#define CHECK_STOP_STATE()  \
do\
{\
    int __i;\
    assert(this->m_hProc == NULL);\
    assert(this->m_Pid == 0);\
    for(__i = 0 ; __i < DEVICE_TYPE_MAX ; __i++)\
    {\
        assert(this->m_TypeIds[__i] == 0);\
    }\
    assert(this->m_BackGroundThread.thread == NULL);\
    assert(this->m_BackGroundThread.threadid == 0);\
    assert(this->m_BackGroundThread.exitevt == NULL);\
    assert(this->m_BackGroundThread.running == 0);\
    assert(this->m_BackGroundThread.exited == 1);\
    assert(this->m_Started == 0);\
    for(__i = 0; __i < sizeof(this->m_MemShareName); __i++)\
    {\
        assert(this->m_MemShareName[__i] == '\0');\
    }\
    assert(this->m_BufferNum == 0);\
    assert(this->m_BufferSectSize == 0);\
    assert(this->m_BufferTotalSize == 0);\
    assert(this->m_pMemShareBase == NULL);\
    assert(this->m_hMapFile == NULL);\
    for(__i = 0; __i < sizeof(this->m_FreeEvtBaseName); __i++)\
    {\
        assert(this->m_FreeEvtBaseName[__i] == '\0');\
    }\
    assert(this->m_pFreeTotalEvts == NULL);\
    for(__i = 0; __i < sizeof(this->m_InputEvtBaseName); __i++)\
    {\
        assert(this->m_InputEvtBaseName[__i] == '\0');\
    }\
    assert(this->m_pInputTotalEvts == NULL);\
    assert(this->m_pIoCapEvents == NULL);\
    assert(this->m_InputEvts.size() == 0);\
    assert(this->m_FreeEvts.size() == 0);\
    assert(this->m_WaitEvts.size() == 0);\
    assert(this->m_InsertEvts == 0);\
    assert(this->m_UnPressedKey == -1);\
    assert(this->m_SeqId == 0);\
    assert(this->m_CurPointSeqId == 0);\
    assert(this->m_CurPoint.x == 0);\
    assert(this->m_CurPoint.y == 0);\
}\
while(0)


VOID CIOController::Stop()
{
    /*first we should make the indicator to be stopped ,and this will give it ok*/
    this->m_Started = 0;
    /*now we should stop thread*/
    this->__StopBackGroundThread();

    /*we put the default value for cursor value*/
    this->EnableSetCursorPos(TRUE);
    this->HideCursor(FALSE);

    this->__CallStopIoCapControl();

    this->__ReleaseCapEvents();

    this->__ReleaseAllEvents();

    this->__ReleaseMapMem();
    ZeroMemory(m_TypeIds,sizeof(m_TypeIds));
    this->m_BufferNum = 0;
    this->m_BufferSectSize = 0;
    this->m_BufferTotalSize = 0;

    this->m_hProc = NULL;
    this->m_Pid = 0;
    this->m_UnPressedKey = -1;
    this->m_SeqId = 0;
    this->m_CurPointSeqId = 0;
    this->m_CurPoint = {0,0};
    CHECK_STOP_STATE();
    return ;
}

int CIOController::__CallStartIoCapControl()
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret=  LAST_ERROR_CODE();
        goto fail;
    }
    pControl->opcode = IO_INJECT_START;

    strncpy_s((char*)pControl->memsharename,sizeof(pControl->memsharename),(const char*)this->m_MemShareName,_TRUNCATE);
    pControl->memsharesize = this->m_BufferTotalSize;
    pControl->memsharenum = this->m_BufferNum;
    pControl->memsharesectsize = this->m_BufferSectSize;

    strncpy_s((char*)pControl->freeevtbasename,sizeof(pControl->freeevtbasename),(const char*)this->m_FreeEvtBaseName,_TRUNCATE);
    strncpy_s((char*)pControl->inputevtbasename,sizeof(pControl->inputevtbasename),(const char*)this->m_InputEvtBaseName,_TRUNCATE);

    ret = this->__CallInnerControl(pControl,5);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Start IoCap Error(%d)\n",ret);
        goto fail;
    }

    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(0);
    return 0;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return -ret;
}

BOOL CIOController::Start(HANDLE hProc,uint32_t bufnum,uint32_t bufsize)
{
    int ret;
    if(hProc == NULL || bufnum == 0 || bufsize < sizeof(DEVICEEVENT))
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }
    this->Stop();
    this->m_hProc = hProc;
    this->m_BufferNum = bufnum;
    this->m_BufferSectSize = bufsize;
    this->m_BufferTotalSize = bufnum * bufsize;
    SetLastError(0);
    this->m_Pid = GetProcessId(hProc);
    if(GetLastError() != 0)
    {
        ret=  LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x> ProcessId Error(%d)\n",this->m_hProc,ret);
        this->Stop();
        return FALSE;
    }

    /*now first to allocate memory*/
    ret = this->__AllocateMapMem();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    ret= this->__AllocateAllEvents();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    ret= this->__AllocateCapEvents();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    ret = this->__StartBackGroundThread();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    ret = this->__CallStartIoCapControl();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    this->m_Started = 1;

    SetLastError(0);
    return TRUE;
}

int CIOController::__CallAddDeviceIoCapControl(uint32_t devtype,uint32_t * pDevId)
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;
    if(devtype >= DEVICE_TYPE_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    if(this->m_Started == 0)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        SetLastError(ret);
        return -ret;
    }

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pControl->opcode = IO_INJECT_ADD_DEVICE;
    pControl->devtype = devtype;
    pControl->devid = this->m_TypeIds[devtype];

    ret = this->__CallInnerControl(pControl,2);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Add Device Type(%d:%d) Error(%d)\n",devtype,this->m_TypeIds[devtype],ret);
        goto fail;
    }


    *pDevId = this->m_TypeIds[devtype];
    this->m_TypeIds[devtype] = this->m_TypeIds[devtype]+1;

    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(0);
    return 0;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return -ret;
}

int CIOController::__CallRemoveDeviceIoCapControl(uint32_t devtype,uint32_t devid)
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;
    if(devtype >= DEVICE_TYPE_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pControl->opcode = IO_INJECT_REMOVE_DEVICE;
    pControl->devtype = devtype;
    pControl->devid = devid;

    ret = this->__CallInnerControl(pControl,2);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Remove Device Type(%d:%d) Error(%d)\n",devtype,devid,ret);
        goto fail;
    }


    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(0);
    return 0;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return -ret;
}

BOOL CIOController::AddDevice(uint32_t iType,uint32_t * pIId)
{
    int ret;
    if(this->m_hProc == NULL ||
            this->m_Pid == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    ret = this->__CallAddDeviceIoCapControl(iType,pIId);
    if(ret < 0)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL CIOController::RemoveDevice(uint32_t iType,uint32_t iId)
{
    int ret;
    if(this->m_hProc == NULL ||
            this->m_Pid == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }
    ret = this->__CallRemoveDeviceIoCapControl(iType,iId);
    if(ret < 0)
    {
        return FALSE;
    }
    return TRUE;
}

PIO_CAP_EVENTS_t CIOController::__GetFreeEvent()
{
    PIO_CAP_EVENTS_t pIoCapEvt=NULL;
    int ret= ERROR_NO_DATA;

    if(this->m_Started == 0)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        SetLastError(ret);
        return NULL;
    }

    EnterCriticalSection(&(this->m_EvtCS));
    if(this->m_FreeEvts.size() > 0)
    {
        pIoCapEvt = this->m_FreeEvts[0];
        this->m_FreeEvts.erase(this->m_FreeEvts.begin());
        ret = 0;
    }
    LeaveCriticalSection(&(this->m_EvtCS));
    SetLastError(ret);
    return pIoCapEvt;
}

BOOL CIOController::__InsertInputEvent(PIO_CAP_EVENTS_t pIoCapEvt)
{
    BOOL bret=TRUE;
    int ret = 0;

    EnterCriticalSection(&(this->m_EvtCS));
    this->m_SeqId ++;
    pIoCapEvt->pEvent->seqid = this->m_SeqId;
    this->m_InputEvts.push_back(pIoCapEvt);
    LeaveCriticalSection(&(this->m_EvtCS));

    bret = SetEvent(pIoCapEvt->hEvent);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%08x>[%d] SetEvent Error(%d)\n",this->m_hProc,pIoCapEvt->Idx,ret);
    }

    SetLastError(ret);
    return bret;
}

BOOL CIOController::PushEvent(DEVICEEVENT * pDevEvt)
{
    int ret;
    PIO_CAP_EVENTS_t pIoCapEvt=NULL;
    unsigned long stime,etime,ctime;

    if(this->m_Started == 0)
    {
        ret = ERROR_ACCESS_DENIED;
        ERROR_INFO("Not Started\n");
        SetLastError(ret);
        return FALSE;
    }

    stime = GetTickCount();
    etime = stime + 100;
    ctime = stime;

    while(1)
    {
        /*now check for the device*/
        pIoCapEvt = this->__GetFreeEvent();
        if(pIoCapEvt)
        {
            break;
        }
        ctime = GetTickCount();
        DEBUG_INFO("(0x%08x) Waitfor IoCapEvt\n",ctime);
        if(etime <= ctime || (etime >= 0xffffff00 && ctime <= 0xff))
        {
            ret = ERROR_NO_DATA;
            ERROR_INFO("Could not Get FreeEvent\n");
            SetLastError(ret);
            return FALSE;
        }
        SchedOut();
    }
    if(pDevEvt->devtype == DEVICE_TYPE_KEYBOARD && pDevEvt->devid == 0)
    {
        DEBUG_INFO("Keyboard Input Events[%d]\n",pIoCapEvt->Idx);
        if(pDevEvt->event.keyboard.event == KEYBOARD_EVENT_DOWN)
        {
            this->m_UnPressedKey = -1;
        }
        else if(pDevEvt->event.keyboard.event == KEYBOARD_EVENT_UP)
        {
            if(this->m_UnPressedKey == pDevEvt->event.keyboard.code)
            {
                /*we do not let this ok*/
                DEBUG_INFO("<0x%p>UnPressed key double(0x%08x:%d)\n",pIoCapEvt,this->m_UnPressedKey,this->m_UnPressedKey);
                this->__InsertFreeEvent(pIoCapEvt);
                return TRUE;
            }
            this->m_UnPressedKey = pDevEvt->event.keyboard.code;
        }
    }

    CopyMemory((&(pIoCapEvt->pEvent->devevent)),pDevEvt,sizeof(*pDevEvt));
    if(pDevEvt->devtype == DEVICE_TYPE_KEYBOARD)
    {
        DEBUG_INFO("PushEvent(0x%08x) keyevent(0x%08x:%d) keycode (0x%08x:%d)\n",GetTickCount(),pDevEvt->event.keyboard.event,pDevEvt->event.keyboard.event,pDevEvt->event.keyboard.code,pDevEvt->event.keyboard.code);
    }
    //DEBUG_INFO("BaseAddr 0x%x IoEvent 0x%x type(%d)\n",this->m_pMemShareBase,pIoCapEvt->pEvent,pIoCapEvt->pEvent->devtype);
    return this->__InsertInputEvent(pIoCapEvt);
}


BOOL CIOController::EnableSetCursorPos(BOOL enabled)
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;

    if(this->m_hProc == NULL)
    {
        ret = ERROR_INVALID_HANDLE;
        ERROR_INFO("%s set cursor pos not init hproc\n",enabled ? "enable" : "disable");
        goto fail;
    }

    pControl =(PIO_CAP_CONTROL_t) calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    if(enabled)
    {
        pControl->opcode = IO_INJECT_ENABLE_SET_POS;
    }
    else
    {
        pControl->opcode = IO_INJECT_DISABLE_SET_POS;
    }

    ret=  this->__CallInnerControl(pControl,2);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not %s cursor set position Error(%d)\n",enabled ? "enable": "disable",ret);
        goto fail;
    }

    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(0);
    return TRUE;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CIOController::HideCursor(BOOL hideenable)
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;

    if(this->m_hProc == NULL)
    {
        ret = ERROR_INVALID_HANDLE;
        ERROR_INFO("%s hide cursor not init hproc\n",hideenable ? "enable" : "disable");
        goto fail;
    }

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    if(hideenable)
    {
        pControl->opcode = IO_INJECT_HIDE_CURSOR;
    }
    else
    {
        pControl->opcode = IO_INJECT_NORMAL_CURSOR;
    }

    ret=  this->__CallInnerControl(pControl,2);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not %s hide cursor Error(%d)\n",hideenable ? "enable": "disable",ret);
        goto fail;
    }

    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(0);
    return TRUE;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CIOController::GetCursorBitmap(PVOID *ppCursorBitmapInfo,UINT*pInfoSize,UINT *pInfoLen,
                                    PVOID* ppCursorBitmapData,UINT* pDataSize,UINT *pDataLen,
                                    PVOID *ppCursorMaskInfo,UINT *pMInfoSize,UINT* pMInfoLen,
                                    PVOID* ppCursorMaskData,UINT *pMDataSize,UINT *pMDataLen)
{
    PIO_CAP_CONTROL_t pControl=NULL;
    HANDLE hinfomap=NULL,hdatamap=NULL,hminfomap=NULL,hmdatamap=NULL;
    LPVOID pInfoBuf=NULL,pDataBuf=NULL,pMInfoBuf=NULL,pMDataBuf=NULL;
    PVOID pRetInfo=*ppCursorBitmapInfo;
    PVOID pRetData=*ppCursorBitmapData;
    PVOID pRetMInfo=*ppCursorMaskInfo;
    PVOID pRetMData=*ppCursorMaskData;
    UINT retinfosize=*pInfoSize,infolen;
    UINT retdatasize=*pDataSize,datalen;
    UINT retminfosize=*pMInfoSize,minfolen;
    UINT retmdatasize=*pMDataSize,mdatalen;
    int ret;
    UINT sectorsize=64*1024;
    int cont=0;
    LPSHARE_DATA pShareInfo=NULL,pShareData=NULL,pShareMInfo=NULL,pShareMData=NULL;
    std::auto_ptr<unsigned char> pShareName2(new unsigned char[IO_NAME_MAX_SIZE]);
    unsigned char* pShareName = pShareName2.get();
    BOOL bret;

    if(this->m_hProc == NULL || this->m_Pid == 0)
    {
        ret = ERROR_INVALID_HANDLE;
        ERROR_INFO("GetCursorBitmap not init hproc\n");
        goto fail;
    }

    pControl = (PIO_CAP_CONTROL_t)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pControl->opcode = IO_INJECT_GET_CURSOR_BMP;

    _snprintf_s((char*)pControl->freeevtbasename,sizeof(pControl->freeevtbasename),_TRUNCATE,"%s%d",IO_MAP_CURSOR_BITMAP_INFO_BASENAME,this->m_Pid);
    _snprintf_s((char*)pControl->inputevtbasename,sizeof(pControl->inputevtbasename),_TRUNCATE,"%s%d",IO_MAP_CURSOR_BITMAP_DATA_BASENAME,this->m_Pid);

    /*now we call the map for it */

    do
    {
        cont = 0;
        this->__DeleteMap(&hminfomap,&pMInfoBuf);
        this->__DeleteMap(&hmdatamap,&pMDataBuf);
        this->__DeleteMap(&hinfomap,&pInfoBuf);
        this->__DeleteMap(&hdatamap,&pDataBuf);

        /*now to reset for the sector size*/
        pControl->memsharesectsize = sectorsize;

        _snprintf_s((char*)pShareName,IO_NAME_MAX_SIZE,_TRUNCATE,"%s_1",pControl->freeevtbasename);
        bret = this->__CreateMap((char*)pShareName,sectorsize,&hminfomap,&pMInfoBuf);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("CreateMaskInfo(%s) Error(%d)\n",pShareName,ret);
            goto fail;
        }

        _snprintf_s((char*)pShareName,IO_NAME_MAX_SIZE,_TRUNCATE,"%s_1",pControl->inputevtbasename);
        bret = this->__CreateMap((char*)pShareName,sectorsize,&hmdatamap,&pMDataBuf);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("CreateMaskData(%s) Error(%d)\n",pShareName,ret);
            goto fail;
        }


        _snprintf_s((char*)pShareName,IO_NAME_MAX_SIZE,_TRUNCATE,"%s_2",pControl->freeevtbasename);
        bret = this->__CreateMap((char*)pShareName,sectorsize,&hinfomap,&pInfoBuf);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("CreateColorInfo(%s) Error(%d)\n",pShareName,ret);
            goto fail;
        }

        _snprintf_s((char*)pShareName,IO_NAME_MAX_SIZE,_TRUNCATE,"%s_2",pControl->inputevtbasename);
        bret = this->__CreateMap((char*)pShareName,sectorsize,&hdatamap,&pDataBuf);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("CreateColorData(%s) Error(%d)\n",pShareName,ret);
            goto fail;
        }



        /*now to call for the function*/
        ret = this->__CallInnerControl(pControl,3);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            if(ret != ERROR_INSUFFICIENT_BUFFER)
            {
                goto fail;
            }
            cont = 1;
        }

        if(cont)
        {
            sectorsize <<= 1;
        }
    }
    while(cont);


    /*ok ,this will do good job*/
    pShareData = (LPSHARE_DATA)pMInfoBuf;
    ret = this->__ExtractBuffer(pShareData,sectorsize,&pRetMInfo,&retminfosize,&minfolen,CURSOR_MASK_BITMAPINFO);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("ExtractMaskInfo Error(%d)\n",ret);
        goto fail;
    }

    pShareData = (LPSHARE_DATA)pMDataBuf;
    ret = this->__ExtractBuffer(pShareData,sectorsize,&pRetMData,&retmdatasize,&mdatalen,CURSOR_MASK_BITDATA);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("ExtractMaskData Error(%d)\n",ret);
        goto fail;
    }

    pShareData = (LPSHARE_DATA)pInfoBuf;
    ret = this->__ExtractBuffer(pShareData,sectorsize,&pRetInfo,&retinfosize,&infolen,CURSOR_COLOR_BITMAPINFO);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("ExtractColorInfo Error(%d)\n",ret);
        goto fail;
    }

    pShareData = (LPSHARE_DATA)pDataBuf;
    ret = this->__ExtractBuffer(pShareData,sectorsize,&pRetData,&retdatasize,&datalen,CURSOR_COLOR_BITDATA);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("ExtractColorData Error(%d)\n",ret);
        goto fail;
    }


    this->__DeleteMap(&hdatamap,&pDataBuf);
    this->__DeleteMap(&hinfomap,&pInfoBuf);
    this->__DeleteMap(&hminfomap,&pMInfoBuf);
    this->__DeleteMap(&hmdatamap,&pMDataBuf);


    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;

    if(*ppCursorBitmapInfo && *ppCursorBitmapInfo != pRetInfo)
    {
        free(*ppCursorBitmapInfo);
    }
    *ppCursorBitmapInfo = pRetInfo;
    *pInfoSize = retinfosize;
    *pInfoLen = infolen;

    if(*ppCursorBitmapData && *ppCursorBitmapData != pRetData)
    {
        free(*ppCursorBitmapData);
    }
    *ppCursorBitmapData = pRetData;
    *pDataSize = retdatasize;
    *pDataLen = datalen;

    if(*ppCursorMaskInfo && *ppCursorMaskInfo != pRetMInfo)
    {
        free(*ppCursorMaskInfo);
    }
    *ppCursorMaskInfo = pRetMInfo;
    *pMInfoSize = retminfosize;
    *pMInfoLen = minfolen;

    if(*ppCursorMaskData && *ppCursorMaskData != pRetMData)
    {
        free(*ppCursorMaskData);
    }
    *ppCursorMaskData = pRetMData;
    *pMDataSize = retmdatasize;
    *pMDataLen = mdatalen;



    SetLastError(0);
    return TRUE;
fail:

    this->__DeleteMap(&hdatamap,&pDataBuf);
    this->__DeleteMap(&hinfomap,&pInfoBuf);
    this->__DeleteMap(&hminfomap,&pMInfoBuf);
    this->__DeleteMap(&hmdatamap,&pMDataBuf);

    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;

    if(pRetInfo && pRetInfo != *ppCursorBitmapInfo)
    {
        free(pRetInfo);
    }
    pRetInfo = NULL;

    if(pRetData && pRetData != *ppCursorBitmapData)
    {
        free(pRetData);
    }
    pRetData = NULL;

    if(pRetMInfo && pRetMInfo != *ppCursorMaskInfo)
    {
        free(pRetMInfo);
    }
    pRetMInfo = NULL;

    if(pRetMData && pRetMData != *ppCursorMaskData)
    {
        free(pRetMData);
    }
    pRetMData = NULL;

    SetLastError(ret);
    return FALSE;
}


BOOL CIOController::__ExtractBuffer(LPSHARE_DATA pShareData,int sectsize,PVOID * ppBuffer,UINT * pBufSize,UINT * pBufLen,int type)
{
    int ret;
    PVOID pRetBuf=*ppBuffer;
    UINT retsize=*pBufSize;

    if(pShareData->datalen > (sectsize - sizeof(*pShareData) + sizeof(pShareData->data)))
    {
        ret=  ERROR_INVALID_DATA;
        ERROR_INFO("<0x%p>datalen (%d) not valid for %d\n",pShareData,pShareData->datalen,sectsize);
        goto fail;
    }

    if(pShareData->datatype != type)
    {
        ret=  ERROR_INVALID_DATA;
        ERROR_INFO("<0x%p>datatype (%d) != (%d)\n",pShareData->datatype,type);
        goto fail;
    }

    if(retsize < pShareData->datalen || pRetBuf == NULL)
    {
        if(retsize < pShareData->datalen)
        {
            retsize = pShareData->datalen;
        }
        pRetBuf = malloc(retsize);
    }

    if(pRetBuf == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    CopyMemory(pRetBuf,pShareData->data,pShareData->datalen);

    if(*ppBuffer && pRetBuf != *ppBuffer)
    {
        free(*ppBuffer);
    }
    *ppBuffer = pRetBuf;
    *pBufSize = retsize;
    *pBufLen = pShareData->datalen;
    SetLastError(0);
    return TRUE;
fail:

    if(pRetBuf && pRetBuf != *ppBuffer)
    {
        free(pRetBuf);
    }
    pRetBuf = NULL;

    SetLastError(ret);
    return FALSE;
}


BOOL CIOController::__CreateMap(char * pShareName,int size,HANDLE * pHandle,PVOID * ppMapBuf)
{
    HANDLE handle=NULL;
    PVOID pMapBuf=NULL;
    int ret;

    if(pHandle == NULL || *pHandle || ppMapBuf == NULL || *ppMapBuf || pShareName == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        goto fail;
    }

    handle = CreateMapFile(pShareName,size,1);
    if(handle == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("CreateMapFile(%s) Error(%d)\n",pShareName,ret);
        goto fail;
    }

    pMapBuf = MapFileBuffer(handle,size);
    if(pMapBuf == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("MapFileBuffer(%s) size(0x%08x:%d) Error(%d)\n",pShareName,size,size,ret);
        goto fail;
    }

    *pHandle = handle;
    *ppMapBuf = pMapBuf;
    SetLastError(0);
    return TRUE;
fail:
    this->__DeleteMap(&handle,&pMapBuf);
    SetLastError(ret);
    return FALSE;
}


void CIOController::__DeleteMap(HANDLE * pHandle,PVOID * ppMapBuf)
{
    UnMapFileBuffer((unsigned char**)ppMapBuf);
    CloseMapFileHandle(pHandle);
    return ;
}

BOOL CIOController::GetCursorPoint(LPPOINT pPoint)
{
    int ret;

    if(this->m_Started == 0)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        SetLastError(ret);
        return FALSE;
    }

    if(pPoint == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    EnterCriticalSection(&(this->m_EvtCS));
    pPoint->x = this->m_CurPoint.x;
    pPoint->y = this->m_CurPoint.y;
    LeaveCriticalSection(&(this->m_EvtCS));
    SetLastError(0);
    return TRUE;
}

BOOL CIOController::__SetCurPoint(int x,int y)
{
    int ret;
    if(this->m_Started == 0)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        SetLastError(ret);
        return FALSE;
    }
    EnterCriticalSection(&(this->m_EvtCS));
    this->m_CurPoint.x = x;
    this->m_CurPoint.y = y;
    LeaveCriticalSection(&(this->m_EvtCS));
    SetLastError(0);
    return TRUE;
}

