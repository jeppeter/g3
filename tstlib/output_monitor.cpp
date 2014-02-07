
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
    while(this->m_pBuffers.size() > 0)
    {
        pBuffer = this->m_pBuffers[0];
        this->m_pBuffers.erase(this->m_pBuffers.begin());
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