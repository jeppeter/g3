
#include "iocapctrl.h"

#define  IO_FREE_EVT_BASENAME  "GlobalIoInjectFreeEvt"
#define  IO_INPUT_EVT_BASENAME  "GlobalIoInjectInputEvt"

CIOController::CIOController()
{
    m_hProc = NULL;
    ZeroMemory(&m_BackGroundThread,sizeof(m_BackGroundThread));
    m_BackGroundThread.exited = 1;
    InitializeCriticalSection(&(m_EvtCS));
    m_Started = 0;
    ZeroMemory(&m_MemShareName,sizeof(m_MemShareName));
    m_BufferNum = 0;
    m_BufferSectSize = 0;
    m_BufferTotalSize = 0;
    m_pMemShareBase = NULL;
    ZeroMemory(&m_EvtShareBaseName,sizeof(m_EvtShareBaseName));
    m_pTotalEvts = NULL;
    m_TotalEventNum = 0;
    m_pIoEvents = NULL;
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


VOID CIOController::Stop()
{
    /*first we should make the indicator to be stopped ,and this will give it ok*/
    this->m_Started = 0;

    /*now we should stop thread*/
    this->__StopBackGroundThread();
    /*now we should call stop the injected thread and it will ok*/
}
