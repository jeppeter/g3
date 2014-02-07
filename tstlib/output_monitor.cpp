
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
	InitializeCriticalSection(&(m_CS));
}

