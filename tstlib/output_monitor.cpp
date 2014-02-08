
#include "output_monitor.h"

OutputMonitor::OutputMonitor()
{
    m_hDBWinMutex = NULL;
    m_hDBWinBufferReady = NULL;
    m_hDBWinDataReady = NULL;
    m_hDBWinMapBuffer = NULL;
    m_pDBWinBuffer = NULL;
    assert(m_pBuffers.size() == 0);
    assert(m_Pids.size() == 0);
    m_Started = 0;
    ZeroMemory(&m_ThreadControl,sizeof(m_ThreadControl));
    m_ThreadControl.exited = 1;
    InitializeCriticalSection(&(m_CS));
}


OutputMonitor::~OutputMonitor()
{
    this->Stop();
    this->m_Pids.clear();
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
    assert(this->m_hDBWinMutex == NULL);
    assert(this->m_hDBWinBufferReady == NULL);
    assert(this->m_hDBWinDataReady == NULL);
    assert(this->m_hDBWinMapBuffer == NULL);
    assert(this->m_pDBWinBuffer == NULL);
    assert(this->m_pBuffers.size() == 0);
    assert(this->m_Started == 0);
    assert(this->m_ThreadControl.exited == 1);
    assert(this->m_ThreadControl.exitevt == NULL);
    assert(this->m_ThreadControl.running == 0);
    assert(this->m_ThreadControl.thread == NULL);
    assert(this->m_ThreadControl.threadid == 0);
    return ;
}

void OutputMonitor::__ClearBuffers()
{
    void* pBuffer = NULL;
    while(this->m_pAvailBuffers.size() > 0)
    {
        pBuffer = this->m_pAvailBuffers[0];
        this->m_pAvailBuffers.erase(this->m_pAvailBuffers.begin());
        free(pBuffer);
        pBuffer = NULL;
    }

    while(this->m_pFreeBuffers.size() > 0)
    {
        pBuffer = this->m_pFreeBuffers[0];
        this->m_pFreeBuffers.erase(this->m_pFreeBuffers.begin());
        free(pBuffer);
        pBuffer = NULL;
    }

    return ;
}

void OutputMonitor::__UnMapBuffer()
{
    UnMapFileBuffer(&(this->m_pDBWinBuffer));
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

    EnterCriticalSection(&(this->m_CS));
    if(this->m_pFreeBuffers.size() > 0)
    {
        pRetBuffer = this->m_pFreeBuffers[0];
        this->m_pFreeBuffers.erase(this->m_pFreeBuffers.begin());
    }
    LeaveCriticalSection(&(this->m_CS));

    if(pRetBuffer == NULL)
    {
        pRetBuffer = (PDBWIN_BUFFER_t)malloc(sizeof(*pRetBuffer));
    }

    return pRetBuffer;
}

void OutputMonitor::ReleaseBuffer(std::vector<PDBWIN_BUFFER_t>& pBuffers)
{
    int left= pBuffers.size();
    PDBWIN_BUFFER_t pBuffer=NULL;

    EnterCriticalSection(&(this->m_CS));
    while(this->m_pFreeBuffers.size() < 50 && pBuffers.size() > 0)
    {
        assert(pBuffer);
        pBuffer = pBuffers[0];
        pBuffers.erase(pBuffers.begin());
        this->m_pFreeBuffers.push_back(pBuffer);
        pBuffer = NULL;
        left --;
    }
    LeaveCriticalSection(&(this->m_CS));

    while(pBuffers.size() > 0)
    {
        pBuffer = pBuffers[0];
        pBuffers.erase(pBuffers.begin());
        free(pBuffer);
        pBuffer = NULL;
    }

    return ;
}

int OutputMonitor::__CreateMutexEvent()
{
    int ret;

    assert(this->m_hDBWinMutex == NULL);
    assert(this->m_hDBWinBufferReady == NULL);
    assert(this->m_hDBWinDataReady == NULL);

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

int OutputMonitor::Start()
{
    int started = 0;
    int ret;

    started = this->__SetStarted(1);
    if(started > 0)
    {
        return 0;
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
