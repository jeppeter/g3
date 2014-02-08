
#ifndef __OUTPUT_MONITOR_H__
#define __OUTPUT_MONITOR_H__


#include <injectctrl.h>

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

private:
	void __ClearBuffers();
	void __UnMapBuffer();
	void __CloseMutexEvent();
	int __MapBuffer();
	int __CreateMutexEvent();
	
	void __AssertStop();
	int __Start();
	void __Stop();
	int __GetStarted();
	int __SetStarted(int started);
	static DWORD __ProcessMonitor(void* pParam);
	DWORD __ProcessImpl();
	int __HandleBufferIn();
	int __IsInProcessPids();
	PDBWIN_BUFFER_t __GetDbWinBuffer();
	int __InsertDbWinBuffer(PDBWIN_BUFFER_t pBuffer);

private:
	HANDLE m_hDBWinMutex;
	HANDLE m_hDBWinBufferReady;
	HANDLE m_hDBWinDataReady;
	HANDLE m_hDBWinMapBuffer;
	void *m_pDBWinBuffer;
	std::vector<void*> m_pAvailBuffers;
	std::vector<void*> m_pFreeBuffers;
	std::vector<int> m_Pids;
	int m_Started;
	thread_control_t m_ThreadControl;
	CRITICAL_SECTIONS m_CS;	
};

#endif /*__OUTPUT_MONITOR_H__*/

