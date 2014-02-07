
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

int OutputMonitor::__SetStarted(int started,int force)
{
    int oldstate=0;

    EnterCriticalSection(&(this->m_CS));
    oldstate = this->m_Started;

    if((oldstate == 1 && started == 0) ||
            (oldstate == 2 && started == 1))
    {
        this->m_Started = started;
    }
    else if(force)
    {
        this->m_Started = started;
    }
    LeaveCriticalSection(&(this->m_CS));
    return oldstate;
}

void OutputMonitor::Stop()
{
    int started = 0;

    started = this->__SetStarted(-1,1);
    if(started == 0)
    {
        /*have already stopped ,so not running*/
        return ;
    }
}