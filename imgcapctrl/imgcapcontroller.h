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
    HANDLE m_hProc;         // ����ͼ���̾��
    int m_iMaxDelay;        // ���Խ�ͼ�����ʱ��
    int m_iLastTimeStamp;   // ���һ�ν�ͼʱ���

    uint64_t m_iTotalCapTime;
    uint64_t m_iTotalCapImgNum;
    uint64_t m_iMaxCapTime;
    uint64_t m_iMinCapTime;
};

#endif /*__IMG_CAP_CONTROLLER_H__*/
