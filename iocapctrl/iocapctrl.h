
#ifndef __IO_CAP_CTRL_H__
#define __IO_CAP_CTRL_H__

#include <iocapcommon.h>


typedef struct
{
	HANDLE hEvent;
	LPDEVICEEVENT pEvent;
} IO_CAP_EVENTS_t,*PIO_CAP_EVENTS_t;

class CIOController
{
public:
	CIOController();
	~CIOController();

	BOOL Start(HANDLE hProc);		
	VOID Stop();
	BOOL AddDevice(int iType, int iId);
	int GetDeviceNum(int iType);
	BOOL GetDeviceIds(int iType, ptr_t * pIds, int iSize);

	BOOL PushEvent(DEVICEEVENT * pDevEvt);

private:
	void __StopBackGroundThread();
	int __StartBackGroundThread();

private:
	HANDLE m_hProc;
	thread_control_t m_BackGroundThread;
	CRITICAL_SECTION m_EvtCS;
	int m_Started;
	int m_BufferNum;
	int m_BufferSectSize;
	int m_BufferTotalSize;
	void *m_pMemShareBase;
	HANDLE *m_pTotalEvts;
	int m_TotalEventNum;
	PIO_CAP_EVENTS_t m_pIoEvents;
	std::vector<PIO_CAP_EVENTS_t> m_InputEvts;
	std::vector<PIO_CAP_EVENTS_t> m_FreeEvts;
};



#endif /*__IO_CAP_CTRL_H__*/

