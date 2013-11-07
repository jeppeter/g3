#ifndef __IMG_CAP_CONTROLLER_H__
#define __IMG_CAP_CONTROLLER_H__

#include <imgcapcommon.h>
#include <Windows.h>
#include <stdint.h>

enum IMGCAPPER_STATE
{
	IMGCAPCTRL_STATE_OPEN = 0,			 // 注入成功
	IMGCAPCTRL_STATE_CLOSE, 			 // 未注入或未注入完毕
};

enum IMGCAPPER_OPERATION
{
	IMGCAPCTRL_OPERATION_NONE = 0,		 // 无可截取内容或不知道可截取内容是以什么方式呈现的
	IMGCAPCTRL_OPERATION_GDI,
	IMGCAPCTRL_OPERATION_DIRECTX,
	IMGCAPCTRL_OPERATION_OPENGL,
};


class CImgCapController
{
public:
    CImgCapController();
    ~CImgCapController();


    BOOL Start(HANDLE hProc,const char* dllname, int iMaxDelay);
    VOID Stop();

    BOOL CapImage(uint8_t *pData, int iLen, int * iFormat, int * iWidth, int * iHeight, int * iTimeStamp, int * pRetLen);
    int GetState();
    int GetOperation();

private:
	BOOL __CapImageWindowed(uint8_t *pData, int iLen, int * iFormat, int * iWidth, int * iHeight, int * iTimeStamp, int * pRetLen);


private:
    HANDLE m_hProc;         // 被截图进程句柄
    unsigned char m_strDllName[256];
    int m_iState;
	int m_iOperation;
    int m_iMaxDelay;        // 尝试截图的最大时间
    int m_iLastTimeStamp;   // 最后一次截图时间戳

    uint64_t m_iTotalCapTime;
    uint64_t m_iTotalCapImgNum;
    uint64_t m_iMaxCapTime;
    uint64_t m_iMinCapTime;
};

#endif /*__IMG_CAP_CONTROLLER_H__*/

