
#ifndef  __PCMCAP_DEMO_CALLBACK_H__
#define  __PCMCAP_DEMO_CALLBACK_H__

#include "pcmcapctrl.h"
#include <vector>
#include "waveplay.h"


class CPcmCapDemoCallBack : public IPcmCapControllerCallback
{
public:
	CPcmCapDemoCallBack();
	~CPcmCapDemoCallBack();
	virtual VOID WaveOpenCb(LPVOID lpParam);
	virtual VOID WaveInCb(pcmcap_buffer_t* pPcmItem, LPVOID lpParam);
	virtual VOID WaveCloseCb(LPVOID lpParam);
	int OpenFile(const char* fname);
	void CloseFile();

private:
	int __StartPcmPlay(pcmcap_format_t* pFormat);
	int __PcmPlay(pcmcap_format_t*pFormat,unsigned char* pBuffer,int bytes);
	void __StopPcmPlay();
	void __InnerPcmPlay(pcmcap_buffer_t *pPcmItem,LPVOID lpParam);
	void __StopPlay();
	
	int __WriteFile(pcmcap_buffer_t * pPcmItem,LPVOID lpParam);
	

private:
	std::vector<FILE*> m_FpVecs;
	std::vector<void*> pointerVecs;
	std::vector<int> m_WriteBlockSizeVecs;
	pcmcap_format_t format;
	unsigned char m_FileNameBase[128];
};


#endif /*__PCMCAP_DEMO_CALLBACK_H__*/


