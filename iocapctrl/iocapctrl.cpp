
#include "iocapctrl.h"

#define  IO_FREE_EVT_BASENAME  "GlobalIoInjectFreeEvt"
#define  IO_INPUT_EVT_BASENAME  "GlobalIoInjectInputEvt"
#define  IO_MAP_MEM_BASENAME    "GlobalIoInjectMapMem"

CIOController::CIOController()
{
    m_hProc = NULL;
    ZeroMemory(&m_BackGroundThread,sizeof(m_BackGroundThread));
    ZeroMemory(m_TypeIds,sizeof(m_TypeIds));
    m_BackGroundThread.exited = 1;
    InitializeCriticalSection(&(m_EvtCS));
    m_Started = 0;
    ZeroMemory(&m_MemShareName,sizeof(m_MemShareName));
    m_BufferNum = 0;
    m_BufferSectSize = 0;
    m_BufferTotalSize = 0;
    m_pMemShareBase = NULL;
    m_hMapFile = NULL;
    m_pFreeTotalEvts = NULL;
    ZeroMemory(m_FreeEvtBaseName,sizeof(m_FreeEvtBaseName));
    m_pInputTotalEvts = NULL;
    ZeroMemory(m_InputEvtBaseName,sizeof(m_InputEvtBaseName));
    m_pIoCapEvents = NULL;
    assert(m_InputEvts.size() == 0);
    assert(m_FreeEvts.size() == 0);
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

DWORD CIOController::__ThreadImpl()
{
    HANDLE* pWaitHandle=NULL;
    DWORD dret,idx;
    int waitnum = 0;
    int tries=0,ret;

    /*including the exit notify event*/
    waitnum = this->m_TotalEventNum + 1;
    pWaitHandle = calloc(sizeof(*pWaitHandle),waitnum);
    if(pWaitHandle == NULL)
    {
        dret = LAST_ERROR_CODE();
        goto out;
    }

    /*now copy for the wait handler*/
    if(this->m_TotalEventNum > 0)
    {
        CopyMemory(pWaitHandle,this->m_pFreeTotalEvts,sizeof(*pWaitHandle)*this->m_BufferNum);
    }

    /*now to set the last one*/
    assert(this->m_BackGroundThread.exitevt);
    pWaitHandle[waitnum - 1] = this->m_BackGroundThread.exitevt;


    while(this->m_BackGroundThread.running)
    {
        dret = WaitForMultipleObjectsEx(waitnum,pWaitHandle,FALSE,INFINITE,TRUE);
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
                ERROR_INFO("Change <%d> not commit\n",idx);
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

out:
    if(pWaitHandle)
    {
        free(pWaitHandle);
    }
    pWaitHandle = NULL;
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

    /*now free input evts*/
    this->m_InputEvts.clear();
    this->m_FreeEvts.clear();

    if(this->m_pIoCapEvents)
    {
        free(this->m_pIoCapEvents);
    }
    this->m_pIoCapEvents = NULL;

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
    SetLastError(0);
    pid = GetProcessId(this->m_hProc);
    if(GetLastError() != 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not get <0x%08x> pid Error(%d)\n",this->m_hProc,ret);
        this->__ReleaseAllEvents();
        SetLastError(ret);
        return -ret;
    }

    _snprintf_s(curbasename,sizeof(curbasename),_TRUNCATE,"%s%d",IO_FREE_EVT_BASENAME,pid);
    this->m_pFreeTotalEvts = calloc(sizeof(this->m_pFreeTotalEvts[0]),this->m_BufferNum);
    if(this->m_pFreeTotalEvts == NULL)
    {
        ret = LAST_ERROR_CODE();
        this->__ReleaseAllEvents();
        SetLastError(ret);
        return -ret;
    }

    for(i=0; i<this->m_BufferNum; i++)
    {
        _snprintf_s(fullname,sizeof(fullname),_TRUNCATE,"%s_%d",curbasename,i);
        this->m_pFreeTotalEvts[i]= GetEvent(fullname,1);
        if(this->m_pFreeTotalEvts[i] == NULL)
        {
            ret =LAST_ERROR_CODE();
            ERROR_INFO("<0x%08x> create %s event Error(%d)\n",this->m_hProc,fullname,ret);
            this->__ReleaseAllEvents();
            SetLastError(ret);
            return -ret;
        }
    }

    strncpy_s(this->m_FreeEvtBaseName,sizeof(this->m_FreeEvtBaseName),curbasename,_TRUNCATE);

    /*now for the input event*/
    this->m_pInputTotalEvts = calloc(sizeof(this->m_pInputTotalEvts[0]),this->m_BufferNum);
    if(this->m_pInputTotalEvts == NULL)
    {
        ret = LAST_ERROR_CODE();
        this->__ReleaseAllEvents();
        SetLastError(ret);
        return -ret;
    }

    _snprintf_s(curbasename,sizeof(curbasename),_TRUNCATE,"%s%d",IO_INPUT_EVT_BASENAME,pid);
    for(i=0; i<this->m_BufferNum; i++)
    {
        _snprintf_s(fullname,sizeof(fullname),_TRUNCATE,"%s_%d",curbasename,i);
        this->m_pInputTotalEvts[i] = GetEvent(fullname,1);
        if(this->m_pInputTotalEvts[i] == NULL)
        {
            ret =LAST_ERROR_CODE();
            ERROR_INFO("<0x%08x> create %s event Error(%d)\n",this->m_hProc,fullname,ret);
            this->__ReleaseAllEvents();
            SetLastError(ret);
            return -ret;
        }
    }
    strncpy_s(this->m_InputEvtBaseName,sizeof(this->m_InputEvtBaseName),curbasename,_TRUNCATE);
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
    ret = GetRemoteProcAddress(pid,"ioinject.dll","DetourDirectInputControl",&pFnAddr);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not find[%d] %s:%s Error(%d)\n",pid,"ioinject.dll","DetourDirectInputControl",ret);
        SetLastError(ret);
        return -ret;
    }

    pRemoteControl = VirtualAllocEx(this->m_hProc,NULL,sizeof(*pRemoteControl),MEM_COMMIT,PAGE_READWRITE);
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


    ret = CallRemoteFuncRemoteParam(pid,pFnAddr,pRemoteControl,timeout,&retval);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Call ioinject.dll:DetourDirectInputControl Error(%d)\n",ret);
        goto fail;
    }

    if(retval < 0)
    {
        ret = -retval;
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
    this->m_InputEvts.clear();
    this->m_FreeEvts.clear();
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

    this->m_pIoCapEvents = calloc(sizeof(this->m_pIoCapEvents[0]),this->m_BufferNum);
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
        this->m_pIoCapEvents[i].pEvent = (LPDEVICEEVENT)(this->m_pMemShareBase + this->m_BufferSectSize * i);
        this->m_pFreeTotalEvts.push_back(&(this->m_pIoCapEvents[i]));
    }

    SetLastError(0);
    return 0;
}

void CIOController::__ReleaseMapMem()
{
    UnMapFileBuffer(&(this->m_pMemShareBase));
    CloseMapFileHandle(&(this->m_hMapFile));
    ZeroMemory(this->m_MemShareName,sizeof(this->m_MemShareName));
    return;
}

int CIOController::__AllocateMapMem()
{
    int ret;
    uint32_t pid;


    if(this->m_hProc == NULL || this->m_BufferNum ==0 ||
            this->m_BufferSectSize == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }
    this->__ReleaseMapMem();

    _snprintf_s(this->m_MemShareName,sizeof(this->m_MemShareName),_TRUNCATE,"%s%d",IO_MAP_MEM_BASENAME,this->m_Pid);
    this->m_hMapFile = CreateMapFile(this->m_MemShareName,this->m_BufferTotalSize,1);
    if(this->m_hMapFile == NULL)
    {
        ret =LAST_ERROR_CODE();
        ERROR_INFO("CreateMemMap (%s) size(0x%08x) Error(%d)\n",this->m_MemShareName,this->m_BufferTotalSize,ret);
        this->__ReleaseMapMem();
        return -ret;
    }

    this->m_pMemShareBase = MapFileBuffer(this->m_hMapFile,this->m_BufferTotalSize);
    if(this->m_pMemShareBase == NULL)
    {
        ret =LAST_ERROR_CODE();
        ERROR_INFO("MapFileBuffer (%s) size(0x%08x) Error(%d)\n",this->m_MemShareName,this->m_BufferTotalSize,ret);
        this->__ReleaseMapMem();
        return -ret;
    }

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

    pControl = calloc(sizeof(*pControl),1);
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

    return ;
}

int CIOController::__CallStartIoCapControl()
{
    int ret;
    PIO_CAP_CONTROL_t pControl=NULL;

    pControl = calloc(sizeof(*pControl),1);
    if(pControl == NULL)
    {
        ret=  LAST_ERROR_CODE();
        goto fail;
    }
    pControl->opcode = IO_INJECT_START;

    strncpy_s(pControl->memsharename,sizeof(pControl->memsharename),this->m_MemShareName,_TRUNCATE);
    pControl->memsharesize = this->m_BufferTotalSize;
    pControl->memsharenum = this->m_BufferNum;
    pControl->memsharesectsize = this->m_BufferSectSize;

    strncpy_s(pControl->freeevtbasename,sizeof(pControl->freeevtbasename),this->m_FreeEvtBaseName,_TRUNCATE);
    strncpy_s(pControl->inputevtbasename,sizeof(pControl->inputevtbasename),this->m_InputEvtBaseName,_TRUNCATE);

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

int CIOController::Start(HANDLE hProc,uint32_t bufnum,uint32_t bufsize)
{
    int ret;
    if(hProc == NULL || bufnum == 0 || bufsize == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
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
        return -ret;
    }

    /*now first to allocate memory*/
    ret = this->__AllocateMapMem();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return -ret;
    }

    ret= this->__AllocateAllEvents();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return -ret;
    }

    ret= this->__AllocateCapEvents();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return -ret;
    }

    ret = this->__StartBackGroundThread();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return -ret;
    }

    ret = this->__CallStartIoCapControl();
    if(ret < 0)
    {
        ret= LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return -ret;
    }

    SetLastError(0);
    return 0;
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

    pControl = calloc(sizeof(*pControl),1);
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

    pControl = calloc(sizeof(*pControl),1);
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
    ret = this->__CallRemoveDeviceIoCapControl(iType,iId);
    if(ret < 0)
    {
        return FALSE;
    }
    return TRUE;
}
