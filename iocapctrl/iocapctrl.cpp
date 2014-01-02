
#include "iocapctrl.h"
#include <assert.h>
#include <injectctrl.h>
#include <output_debug.h>
#include <sched.h>
#include <evt.h>
#include <dllinsert.h>
#include <memshare.h>

#define  IO_FREE_EVT_BASENAME  "GlobalIoInjectFreeEvt"
#define  IO_INPUT_EVT_BASENAME  "GlobalIoInjectInputEvt"
#define  IO_MAP_MEM_BASENAME    "GlobalIoInjectMapMem"

#ifdef _DEBUG
#define IOINJECT_DLL  "ioinjectd.dll"
#else
#define IOINJECT_DLL  "ioinject.dll"
#endif

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
    m_InsertEvts = 0;
    m_UnPressedKey = -1;
    m_SeqId = 0;
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


int CIOController::__ChangeInputToFreeThread(DWORD idx)
{
    int ret = 0;
    unsigned int i;
    int findidx = -1;
    PIO_CAP_EVENTS_t pIoCapEvent=NULL;
    EnterCriticalSection(&(this->m_EvtCS));
    for(i=0; i<this->m_InputEvts.size() ; i++)
    {
        if(this->m_InputEvts[i]->Idx == idx)
        {
            findidx = i;
            ret = 1;
            break;
        }
    }


    if(findidx >= 0)
    {
        pIoCapEvent = this->m_InputEvts[i];
        this->m_InputEvts.erase(this->m_InputEvts.begin() + findidx);
        this->m_FreeEvts.push_back(pIoCapEvent);
    }
    LeaveCriticalSection(&(this->m_EvtCS));
    return ret;
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

DWORD CIOController::__ThreadImpl()
{
    HANDLE* pWaitHandles=NULL;
    DWORD dret,idx;
    int waitnum = 0;
    int tries=0,ret;

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
            while(1)
            {
                ret = this->__ChangeInputToFreeThread(idx);
                if(ret > 0)
                {
                    break;
                }

                tries ++;
                if(tries > 5)
                {
                    ERROR_INFO("Change InputTo Free Not Set tries > 5\n");
                    dret = ERROR_TOO_MANY_MUXWAITERS;
                    goto out;
                }
                ERROR_INFO("Change <%d>[%d] not commit\n",idx,tries);
                SchedOut();
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
    ret = GetRemoteProcAddress(pid,IOINJECT_DLL,"IoInjectControl",&pFnAddr);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not find[%d] %s:%s Error(%d)\n",pid,IOINJECT_DLL,"IoInjectControl",ret);
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
        if((this->m_InputEvts.size() + this->m_FreeEvts.size())==this->m_BufferNum)
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


VOID CIOController::Stop()
{
    /*first we should make the indicator to be stopped ,and this will give it ok*/
    this->m_Started = 0;

    this->__CallStopIoCapControl();
    /*now we should stop thread*/
    this->__StopBackGroundThread();

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

    EnterCriticalSection(&(this->m_EvtCS));
    if(this->m_FreeEvts.size() > 0)
    {
        pIoCapEvt = this->m_FreeEvts[0];
        this->m_FreeEvts.erase(this->m_FreeEvts.begin());
    }
    LeaveCriticalSection(&(this->m_EvtCS));
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
