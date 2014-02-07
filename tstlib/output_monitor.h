
#ifndef __OUTPUT_MONITOR_H__
#define __OUTPUT_MONITOR_H__


#include <injectctrl.h>

class OutputMonitor
{
public:
	OutputMonitor();
	~OutputMonitor();
	int SetFilterPid(int pid);
	int Start();
	void Stop();
	int GetBuffer(std::vector<void*> pBuffers);

private:
	int __Start();
	void __Stop();
	int __GetStarted();
	int __SetStarted(int started,int force);
	static DWORD __ProcessMonitor(void* pParam);
	DWORD __ProcessImpl();

private:
	HANDLE m_hDBWinMutex;
	HANDLE m_hDBWinBufferReady;
	HANDLE m_hDBWinDataReady;
	HANDLE m_hDBWinMapBuffer;
	void *m_pDBWinBuffer;
	std::vector<void*> m_pBuffers;
	std::vector<int> m_Pids;
	int m_Started;
	thread_control_t m_ThreadControl;
	CRITICAL_SECTIONS m_CS;	
};

#endif /*__OUTPUT_MONITOR_H__*/

