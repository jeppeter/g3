
#ifndef __OUTPUT_MONITOR_H__
#define __OUTPUT_MONITOR_H__




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
	CRITICAL_SECTIONS m_CS;	
};

#endif /*__OUTPUT_MONITOR_H__*/

