
#include "imgcapcontroller.h"


CImgCapController::CImgCapController()
{
    m_hProc = NULL;
    m_iMaxDelay = 0;
    m_iLastTimeStamp = 0;

    m_iTotalCapTime = 0;
    m_iTotalCapImgNum = 0;
    m_iMaxCapTime = 0;
    m_iMinCapTime = 0;
}


VOID CImgCapController::Stop()
{
    if(this->m_hProc)
    {
        this->__SetOperationNone();
        this->m_hProc = NULL;
    }

    m_iMaxDelay = 0;
    m_iLastTimeStamp = 0;

    m_iTotalCapTime = 0;
    m_iTotalCapImgNum = 0;
    m_iMaxCapTime = 0;
    m_iMinCapTime = 0;
	return ;
}

CImgCapController::~CImgCapController()
{
	this->Stop();
}


