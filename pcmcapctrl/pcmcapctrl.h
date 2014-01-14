
#ifndef __PCMCAPCTRL_H__
#define __PCMCAPCTRL_H__

#include "pcmcapcommon.h"
#include <vector>

#define PCMITEM_MAX_SIZE 10240

class CPcmCapController;

class IPcmCapControllerCallback
{
public:
    friend class CPcmCapController;
protected:
    virtual VOID WaveOpenCb(LPVOID lpParam) = 0;				        // 声音被打开时调用，首次对某进程Inject不调用
    virtual VOID WaveInCb(pcmcap_buffer_t* pPcmItem, LPVOID lpParam) = 0;	    // 有声音数据回传时调用
    virtual VOID WaveCloseCb(LPVOID lpParam) = 0;				        // 被Inject进程关闭或被Inject进程被关闭声音时调用
};

class CPcmCapController
{
public:
    CPcmCapController();
    ~CPcmCapController();

    BOOL Start(HANDLE hProc, int iOperation, int iBufNum,int iBlockSize, IPcmCapControllerCallback * pPcc, LPVOID lpParam);
    BOOL Stop();

    BOOL SetAudioOperation(int iOperation);
    int GetAudioOperation() const;

protected:

private:
	BOOL __SetOperationNone();
	BOOL __SetOperationCapture();
	BOOL __SetOperationRender();
	BOOL __SetOperationBoth();
	BOOL __SetOperationInner(pcmcap_control_t* pControl,DWORD* pRetCode);
	BOOL __StopOperation(int iOperation);
	BOOL __StartOperation(int iOperation);
	int __CreateMap();
	int __FreeAllEvent();
	void __DestroyMap();
	int __CreateEvent();
	void __DestroyEvent();
	void __DestroyPcmIds();

	int __StartThread();
	void __StopThread();
	static DWORD WINAPI ThreadFunc(void* arg);
	DWORD __ThreadImpl();
	void __AudioRenderBuffer(int idx);
	void __AudioStartCall();
	void __AudioStopCall();

private:
    HANDLE m_hProc;     // 进程句柄
    DWORD m_ProcessId;  // processid of the m_hProc
    int m_iOperation;	// 对进程进行的操作    
    IPcmCapControllerCallback * m_pPcmCapperCb;
    LPVOID m_lpParam;
	unsigned int m_iBufNum;
	unsigned int m_iBufBlockSize;
	
	thread_control_t m_tThreadControl;
	CRITICAL_SECTION m_PcmCS;
	std::vector<int> m_PcmIdx;
	std::vector<uint64_t> m_PcmIds;
	uint64_t m_CurPcmIds;

	HANDLE m_hMapFile;
	unsigned char * m_pMapBuffer;
	unsigned char m_strMapBaseName[128];
	
	unsigned char m_strFreeEvtBaseName[128];
	unsigned char m_strFillEvtBaseName[128];
	unsigned char m_strStartEvtBaseName[128];
	unsigned char m_strStopEvtBaseName[128];
	HANDLE m_hStartEvt;
	HANDLE m_hStopEvt;
	HANDLE * m_pFreeEvt;
	HANDLE * m_pFillEvt;
};


#endif /*__PCMCAP_CAPPER_H__*/

