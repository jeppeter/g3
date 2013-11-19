
#ifndef __IO_CAP_CTRL_H__
#define __IO_CAP_CTRL_H__

#include <iocapcommon.h>


typedef struct
{
	HANDLE hEvent;
	DWORD Idx;
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
	int __ChangeInputToFreeThread(DWORD idx);
	DWORD __ThreadImpl();
	static DWORD WINAPI ThreadProc(LPVOID pParam);
	void __ReleaseAllEvents();
	int __AllocateAllEvents();
	void __ReleaseCapEvents();
	int __AllocateCapEvents();
	int __CallStopIoCapControl();
	int __CallStartIoCapControl();
	int __CallAddDeviceIoCapControl();
	int __CallRemoveDeviceIoCapControl();
	int __CallInnerControl(PIO_CAP_CONTROL_t pControl,int timeout);

private:
	HANDLE m_hProc;
	thread_control_t m_BackGroundThread;
	CRITICAL_SECTION m_EvtCS;	
	int m_Started;
	uint8_t m_MemShareName[IO_NAME_MAX_SIZE];
	int m_BufferNum;
	int m_BufferSectSize;
	int m_BufferTotalSize;
	void *m_pMemShareBase;
	uint8_t m_FreeEvtBaseName[IO_NAME_MAX_SIZE];
	HANDLE *m_pFreeTotalEvts;
	uint8_t m_InputEvtBaseName[IO_NAME_MAX_SIZE];
	HANDLE *m_pInputTotalEvts;
	PIO_CAP_EVENTS_t m_pIoCapEvents;
	std::vector<PIO_CAP_EVENTS_t> m_InputEvts;
	std::vector<PIO_CAP_EVENTS_t> m_FreeEvts;
};



#endif /*__IO_CAP_CTRL_H__*/

