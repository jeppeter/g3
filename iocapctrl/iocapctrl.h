
#ifndef __IO_CAP_CTRL_H__
#define __IO_CAP_CTRL_H__

#include <iocapcommon.h>

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
	HANDLE m_hProc;
	thread_control_t m_BackGroundThread;
	CRITICAL_SECTION m_EvtCS;
	HANDLE *m_pTotalEvts;
	int m_TotalEventNum;
	std::vector<HANDLE*> m_InputEvts;
	std::vector<HANDLE*> m_FreeEvts;
};



#endif /*__IO_CAP_CTRL_H__*/

