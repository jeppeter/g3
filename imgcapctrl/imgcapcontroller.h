#ifndef __IMG_CAP_CONTROLLER_H__
#define __IMG_CAP_CONTROLLER_H__

#include <imgcap_common.h>
#include <Windows.h>


class CImgCapController
{
public:
    CImgCapController();
    ~CImgCapController();


    BOOL Start(HANDLE hProc, int iMaxDelay);
    VOID Stop();

    BOOL CapImage(uint8_t * pData, int iLen, int * iFormat, int * iWidth, int * iHeight, int * iTimeStamp, int * iLastTimeStamp);
    int GetState() const;
    int GetOperation() const;
    int GetAverageCapTime() const;

private:
	void __SetOperationNone();

private:
    HANDLE m_hProc;         // 被截图进程句柄
    int m_iMaxDelay;        // 尝试截图的最大时间
    int m_iLastTimeStamp;   // 最后一次截图时间戳

    uint64_t m_iTotalCapTime;
    uint64_t m_iTotalCapImgNum;
    uint64_t m_iMaxCapTime;
    uint64_t m_iMinCapTime;
};

#endif /*__IMG_CAP_CONTROLLER_H__*/

