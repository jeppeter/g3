
#include "iocapctrl.h"


CIOController::CIOController()
{
	m_hProc = NULL;
	ZeroMemory(&m_BackGroundThread,sizeof(m_BackGroundThread));
	InitializeCriticalSection(&(m_EvtCS));
	m_Started = 0;
	m_BufferNum = 0;
	m_BufferSectSize = 0;
	m_BufferTotalSize = 0;
	m_pMemShareBase = NULL;
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
	this->m_BackGroundThread.running = 0;
}


VOID CIOController::Stop()
{
	/*first we should make the indicator to be stopped ,and this will give it ok*/
	this->m_Started = 0;

	/*now we should stop thread*/
}