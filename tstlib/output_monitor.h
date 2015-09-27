
#ifndef __OUTPUT_MONITOR_H__
#define __OUTPUT_MONITOR_H__


#include <injectctrl.h>
#include <vector>


typedef struct dbwin_buffer
{
	DWORD	dwProcessId;
	char	data[4096-sizeof(DWORD)];
} DBWIN_BUFFER_t,*PDBWIN_BUFFER_t;


class OutputMonitor
{
public:
	OutputMonitor();
	~OutputMonitor();
	int SetFilterPid(int pid);
	int Start();
	void Stop();
	int GetBuffer(std::vector<PDBWIN_BUFFER_t>& pBuffers);
	void ReleaseBuffer(std::vector<PDBWIN_BUFFER_t>& pBuffers);
	HANDLE GetNotifyHandle();
	int SetGlobal();
private:
	void __ClearBuffers();
	void __UnMapBuffer();
	void __CloseMutexEvent();
	int __CreateBuffers();
	int __MapBuffer();
	int __CreateMutexEvent();
	
	void __AssertStop();
	int __Start();
	void __Stop();
	int __GetStarted();
	int __SetStarted(int started);
	static DWORD WINAPI __ProcessMonitor(void* pParam);
	DWORD __ProcessImpl();
	int __HandleBufferIn();
	int __IsInProcessPids();
	PDBWIN_BUFFER_t __GetDbWinBuffer();
	int __InsertDbWinBuffer(PDBWIN_BUFFER_t pBuffer);

private:
	thread_control_t m_ThreadControl;
	CRITICAL_SECTION m_CS;
	HANDLE m_hNotifyEvt;
	HANDLE m_hDBWinMutex;
	HANDLE m_hDBWinBufferReady;
	HANDLE m_hDBWinDataReady;
	HANDLE m_hDBWinMapBuffer;
	void *m_pDBWinBuffer;
	std::vector<PDBWIN_BUFFER_t>* m_pAvailBuffers;
	std::vector<PDBWIN_BUFFER_t>* m_pFreeBuffers;
	std::vector<int>* m_pPids;
	int m_Started;
	int m_GlobalWin32;
};

#endif /*__OUTPUT_MONITOR_H__*/

