
#include "output_monitor.h"
#include <assert.h>
#include <memshare.h>
#include <evt.h>

#define  ERROROUT(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)
#define  INFOOUT(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)


OutputMonitor::OutputMonitor()
{
    ZeroMemory(&m_ThreadControl,sizeof(m_ThreadControl));
    m_ThreadControl.exited = 1;
    InitializeCriticalSection(&(m_CS));
    m_hNotifyEvt = NULL;
    m_hDBWinMutex = NULL;
    m_hDBWinBufferReady = NULL;
    m_hDBWinDataReady = NULL;
    m_hDBWinMapBuffer = NULL;
    m_pDBWinBuffer = NULL;
	m_pAvailBuffers = NULL;
	m_pFreeBuffers = NULL;
	m_pPids = NULL;
    assert(m_pAvailBuffers == NULL);
    assert(m_pFreeBuffers  == NULL);
    assert(m_pPids == NULL);
    m_Started = 0;
}


OutputMonitor::~OutputMonitor()
{
    this->Stop();
    if(this->m_pPids)
    {
        this->m_pPids->clear();
        delete this->m_pPids;
    }
    this->m_pPids = NULL;
    DeleteCriticalSection(&m_CS);
}

int OutputMonitor::__GetStarted()
{
    int started = 0;
    EnterCriticalSection(&(this->m_CS));
    started = this->m_Started;
    LeaveCriticalSection(&(this->m_CS));
    return started;
}

int OutputMonitor::__SetStarted(int started)
{
    int oldstate=0;

    EnterCriticalSection(&(this->m_CS));
    oldstate = this->m_Started;
    this->m_Started = started;
    LeaveCriticalSection(&(this->m_CS));
    return oldstate;
}

void OutputMonitor::__AssertStop()
{
    assert(this->m_ThreadControl.exited == 1);
    assert(this->m_ThreadControl.exitevt == NULL);
    assert(this->m_ThreadControl.running == 0);
    assert(this->m_ThreadControl.thread == NULL);
    assert(this->m_ThreadControl.threadid == 0);
    assert(this->m_hNotifyEvt == NULL);
    assert(this->m_hDBWinMutex == NULL);
    assert(this->m_hDBWinBufferReady == NULL);
    assert(this->m_hDBWinDataReady == NULL);
    assert(this->m_hDBWinMapBuffer == NULL);
    assert(this->m_pDBWinBuffer == NULL);
    assert(this->m_pAvailBuffers == NULL);
    assert(this->m_pFreeBuffers == NULL);
    assert(this->m_Started == 0);
    return ;
}

void OutputMonitor::__ClearBuffers()
{
    void* pBuffer = NULL;
    if(this->m_pAvailBuffers)
    {
        while(this->m_pAvailBuffers->size() > 0)
        {
            pBuffer = this->m_pAvailBuffers->at(0);
            this->m_pAvailBuffers->erase(this->m_pAvailBuffers->begin());
            free(pBuffer);
            pBuffer = NULL;
        }
        delete this->m_pAvailBuffers;
    }
    this->m_pAvailBuffers = NULL;

    if(this->m_pFreeBuffers)
    {
        while(this->m_pFreeBuffers->size() > 0)
        {
            pBuffer = this->m_pFreeBuffers->at(0);
            this->m_pFreeBuffers->erase(this->m_pFreeBuffers->begin());
            free(pBuffer);
            pBuffer = NULL;
        }
        delete this->m_pFreeBuffers;
    }
    this->m_pFreeBuffers = NULL;

    return ;
}

void OutputMonitor::__UnMapBuffer()
{
    UnMapFileBuffer((unsigned char**)&(this->m_pDBWinBuffer));
    CloseMapFileHandle(&(this->m_hDBWinMapBuffer));
    return ;
}

void OutputMonitor::__CloseMutexEvent()
{
    if(this->m_hDBWinDataReady)
    {
        CloseHandle(this->m_hDBWinDataReady);
    }
    this->m_hDBWinDataReady = NULL;

    if(this->m_hDBWinBufferReady)
    {
        CloseHandle(this->m_hDBWinBufferReady);
    }
    this->m_hDBWinBufferReady = NULL;

    if(this->m_hDBWinMutex)
    {
        CloseHandle(this->m_hDBWinMutex);
    }
    this->m_hDBWinMutex = NULL;

    if(this->m_hNotifyEvt)
    {
        CloseHandle(this->m_hNotifyEvt);
    }
    this->m_hNotifyEvt = NULL;
    return ;
}

void OutputMonitor::__Stop()
{
    /*first to stop thread control*/
    StopThreadControl(&(this->m_ThreadControl));

    /*now we should */
    /**/
    this->__ClearBuffers();
    this->__CloseMutexEvent();
    this->__UnMapBuffer();
    return ;
}


void OutputMonitor::Stop()
{
    int started = 0;

    started = this->__SetStarted(0);
    if(started == 0)
    {
        /*have already stopped ,so not running*/
        this->__AssertStop();
        return ;
    }

    /*now we should stop this function*/
    this->__Stop();
    this->__AssertStop();
    return ;
}

PDBWIN_BUFFER_t OutputMonitor::__GetDbWinBuffer()
{
    PDBWIN_BUFFER_t pRetBuffer=NULL;
    int ret = 0;

    EnterCriticalSection(&(this->m_CS));
    if(this->m_pFreeBuffers && this->m_pFreeBuffers->size() > 0)
    {
        pRetBuffer = this->m_pFreeBuffers->at(0);
        this->m_pFreeBuffers->erase(this->m_pFreeBuffers->begin());
    }
    LeaveCriticalSection(&(this->m_CS));

    if(pRetBuffer == NULL)
    {
        pRetBuffer = (PDBWIN_BUFFER_t)malloc(sizeof(*pRetBuffer));
        if(pRetBuffer == NULL)
        {
            ret = GETERRNO();
        }
    }

    SETERRNO(ret);
    return pRetBuffer;
}

int OutputMonitor::GetBuffer(std::vector < PDBWIN_BUFFER_t > & pBuffers)
{
    int ret=0;
    PDBWIN_BUFFER_t pBuffer=NULL;

    if(pBuffers.size() != 0)
    {
        ret= ERROR_INVALID_PARAMETER;
        SETERRNO(ret);
        return -ret;
    }

    EnterCriticalSection(&(this->m_CS));
    if(this->m_Started== 0)
    {
        ret = -ERROR_BAD_ENVIRONMENT;
    }
    else
    {
        if(this->m_pAvailBuffers)
        {
            while(this->m_pAvailBuffers->size() > 0)
            {
                ret ++;
                assert(pBuffer == NULL);
                pBuffer = this->m_pAvailBuffers->at(0);
                this->m_pAvailBuffers->erase(this->m_pAvailBuffers->begin());
                pBuffers.push_back(pBuffer);
                pBuffer = NULL;
            }
        }
        else
        {
            ret = -ERROR_BAD_ENVIRONMENT;
        }
    }
    LeaveCriticalSection(&(this->m_CS));

    if(ret < 0)
    {
        SETERRNO(-ret);
    }
    else
    {
        SETERRNO(0);
    }
    return ret;
}

void OutputMonitor::ReleaseBuffer(std::vector<PDBWIN_BUFFER_t>& pBuffers)
{
    int left= pBuffers.size();
    PDBWIN_BUFFER_t pBuffer=NULL;

    EnterCriticalSection(&(this->m_CS));
    if(this->m_Started && this->m_pFreeBuffers)
    {
        while(this->m_pFreeBuffers->size() < 50 && left > 0)
        {
            assert(pBuffer== NULL);
            pBuffer = pBuffers[0];
            pBuffers.erase(pBuffers.begin());
            this->m_pFreeBuffers->push_back(pBuffer);
            pBuffer = NULL;
            left --;
        }
    }
    LeaveCriticalSection(&(this->m_CS));

    while(pBuffers.size() > 0)
    {
        assert(pBuffer == NULL);
        pBuffer = pBuffers[0];
        pBuffers.erase(pBuffers.begin());
        free(pBuffer);
        pBuffer = NULL;
        left --;
    }

    assert(left == 0);

    return ;
}


int OutputMonitor::__CreateBuffers()
{
    int ret=0;

    assert(this->m_pFreeBuffers == NULL);
    assert(this->m_pAvailBuffers == NULL);

    this->m_pAvailBuffers = new std::vector<PDBWIN_BUFFER_t>();
    if(this->m_pAvailBuffers == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }
    this->m_pFreeBuffers = new std::vector<PDBWIN_BUFFER_t>();
    if(this->m_pFreeBuffers == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    return 0;
fail:
    this->__ClearBuffers();
    SETERRNO(ret);
    return -ret;
}

int OutputMonitor::__CreateMutexEvent()
{
    int ret;

    assert(this->m_hNotifyEvt == NULL);
    assert(this->m_hDBWinMutex == NULL);
    assert(this->m_hDBWinBufferReady == NULL);
    assert(this->m_hDBWinDataReady == NULL);

    this->m_hNotifyEvt = GetEvent(NULL,1);
    if(this->m_hNotifyEvt == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    /*now first to make sure that the*/
    this->m_hDBWinMutex = GetMutex("DBWinMutex",0);
    if(this->m_hDBWinMutex == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    this->m_hDBWinBufferReady = GetEvent("DBWIN_BUFFER_READY",0);
    if(this->m_hDBWinBufferReady == NULL)
    {
        this->m_hDBWinBufferReady = GetEvent("DBWIN_BUFFER_READY",1);
        if(this->m_hDBWinBufferReady == NULL)
        {
            ret = GETERRNO();
            goto fail;
        }
    }

    this->m_hDBWinDataReady = GetEvent("DBWIN_DATA_READY",0);
    if(this->m_hDBWinDataReady == NULL)
    {
        this->m_hDBWinDataReady = GetEvent("DBWIN_DATA_READY",1);
        if(this->m_hDBWinDataReady == NULL)
        {
            ret = GETERRNO();
            goto fail;
        }
    }

    SETERRNO(0);
    return 0;
fail:
    this->__CloseMutexEvent();
    SETERRNO(ret);
    return -ret;
}

int OutputMonitor::__MapBuffer()
{
    int ret;

    assert(this->m_hDBWinMapBuffer == NULL);
    assert(this->m_pDBWinBuffer == NULL);

    this->m_hDBWinMapBuffer = CreateMapFile("DBWIN_BUFFER",sizeof(DBWIN_BUFFER_t),0);
    if(this->m_hDBWinMapBuffer == NULL)
    {
        this->m_hDBWinMapBuffer = CreateMapFile("DBWIN_BUFFER",sizeof(DBWIN_BUFFER_t),1);
        if(this->m_hDBWinMapBuffer == NULL)
        {
            ret = GETERRNO();
            goto fail;
        }
    }

    this->m_pDBWinBuffer = MapFileBuffer(this->m_hDBWinMapBuffer,sizeof(DBWIN_BUFFER_t));
    if(this->m_pDBWinBuffer == NULL)
    {
        ret = GETERRNO();
        goto fail;
    }

    SETERRNO(0);
    return 0;
fail:
    this->__UnMapBuffer();
    SETERRNO(ret);
    return -ret;
}

DWORD WINAPI OutputMonitor::__ProcessMonitor(void * pParam)
{
    OutputMonitor* pThis = (OutputMonitor*)pParam;
    return pThis->__ProcessImpl();
}

int OutputMonitor::__IsInProcessPids()
{
    int ret = 1;
    UINT i;
    PDBWIN_BUFFER_t pWaitbuffer=NULL;

    pWaitbuffer = (PDBWIN_BUFFER_t)this->m_pDBWinBuffer;
    EnterCriticalSection(&(this->m_CS));
    if(this->m_pPids && this->m_pPids->size() > 0)
    {
        ret = 0;
        for(i=0; i<this->m_pPids->size(); i++)
        {
            if(this->m_pPids->at(i) == pWaitbuffer->dwProcessId)
            {
                ret = 1;
                break;
            }
        }
    }
    LeaveCriticalSection(&(this->m_CS));

    return ret;
}

int OutputMonitor::__InsertDbWinBuffer(PDBWIN_BUFFER_t pBuffer)
{
    int ret = -ERROR_BAD_ENVIRONMENT;
	INFOOUT("insert[%d]%s\n",pBuffer->dwProcessId,pBuffer->data);
    EnterCriticalSection(&(this->m_CS));
    if(this->m_Started && this->m_pAvailBuffers)
    {
        this->m_pAvailBuffers->push_back(pBuffer);
        ret = 0;
    }
    LeaveCriticalSection(&(this->m_CS));

    if(ret >= 0)
    {
        SetEvent(this->m_hNotifyEvt);
    }
    SETERRNO(-ret);
    return ret;
}

HANDLE OutputMonitor::GetNotifyHandle()
{
    return this->m_hNotifyEvt;
}

int OutputMonitor::__HandleBufferIn()
{
    PDBWIN_BUFFER_t pBuffer=NULL;
    int ret;

    ret = this->__IsInProcessPids();
    if(ret == 0)
    {
        SetEvent(this->m_hDBWinBufferReady);
        return 0;
    }

    /*now we should */
    pBuffer = this->__GetDbWinBuffer();
    if(pBuffer == NULL)
    {
        ret = GETERRNO();
        SetEvent(this->m_hDBWinBufferReady);
        return -ret;
    }

    CopyMemory(pBuffer,this->m_pDBWinBuffer,sizeof(*pBuffer));
    ret = this->__InsertDbWinBuffer(pBuffer);
    if(ret < 0)
    {
        ret = GETERRNO();
        free(pBuffer);
        pBuffer = NULL;
        SetEvent(this->m_hDBWinBufferReady);
        return -ret;
    }
    pBuffer = NULL;
    SetEvent(this->m_hDBWinBufferReady);
    return 1;
}

DWORD OutputMonitor::__ProcessImpl()
{
    DWORD dret;
    int ret;
    HANDLE hWaitHandle[2];

    hWaitHandle[0] = this->m_hDBWinDataReady;
    hWaitHandle[1] = this->m_ThreadControl.exitevt;

	/*make sure buffer ready for client handle*/
	SetEvent(this->m_hDBWinBufferReady);

    while(this->m_ThreadControl.running)
    {
        dret = WaitForMultipleObjectsEx(2,hWaitHandle,FALSE,INFINITE,TRUE);
		INFOOUT("dret %d\n",dret);
        if(dret == WAIT_OBJECT_0)
        {
            ret = this->__HandleBufferIn();
            if(ret < 0)
            {
                ret = GETERRNO();
                dret = -ret;
                goto out;
            }
        }
        else if(dret == WAIT_FAILED || dret == WAIT_ABANDONED)
        {
            ret = GETERRNO();
            dret = -ret;
            goto out;
        }
    }

    ret = 0;

out:
    this->m_ThreadControl.exited = 1;
    SETERRNO(ret);
    return dret;
}

int OutputMonitor::Start()
{
    int started = 0;
    int ret;

    started = this->__SetStarted(1);
    if(started > 0)
    {
        return 0;
    }

    ret = this->__CreateBuffers();
    if(ret < 0)
    {
        ret = GETERRNO();
        this->Stop();
        SETERRNO(ret);
        return -ret;
    }

    /*now we should give it started*/
    ret = this->__CreateMutexEvent();
    if(ret < 0)
    {
        ret = GETERRNO();
        this->Stop();
        SETERRNO(ret);
        return -ret;
    }

    ret = this->__MapBuffer();
    if(ret < 0)
    {
        ret = GETERRNO();
        this->Stop();
        SETERRNO(ret);
        return -ret;
    }

    ret = StartThreadControl(&(this->m_ThreadControl),OutputMonitor::__ProcessMonitor,this,1);
    if(ret < 0)
    {
        ret = GETERRNO();
        this->Stop();
        SETERRNO(ret);
        return -ret;
    }

    return 0;
}


int OutputMonitor::SetFilterPid(int pid)
{
    int ret = 1;
    int findidx=-1;
    UINT i;

    EnterCriticalSection(&(this->m_CS));
    if(this->m_pPids == NULL)
    {
        this->m_pPids = new std::vector<int>();
    }
    if(this->m_pPids == NULL)
    {
        ret = -GETERRNO();
        goto unlock;
    }
    for(i=0; i<this->m_pPids->size(); i++)
    {
        if(this->m_pPids->at(i) == pid)
        {
            findidx = i;
			ret = 0;
            break;
        }
    }

    if(findidx < 0)
    {
        this->m_pPids->push_back(pid);
    }
unlock:
    LeaveCriticalSection(&(this->m_CS));
    return ret;
}
