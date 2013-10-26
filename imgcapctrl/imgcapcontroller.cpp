
#include "imgcapcontroller.h"
#include <imgcapctrl.h>


CImgCapController::CImgCapController()
{
    m_hProc = NULL;
    memset(m_strDllName,0,sizeof(m_strDllName));
    m_iState = IMGCAPCTRL_STATE_CLOSE;
    m_iOperation = IMGCAPCTRL_OPERATION_NONE;
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
        this->m_hProc = NULL;
    }
    memset(m_strDllName,0,sizeof(m_strDllName));
    this->m_iState = IMGCAPCTRL_STATE_CLOSE;
    this->m_iOperation = IMGCAPCTRL_OPERATION_NONE;

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

int CImgCapController::GetState()
{
    return this->m_iState;
}

int CImgCapController::GetOperation()
{
    return this->m_iOperation;
}


BOOL CImgCapController::Start(HANDLE hProc,const char * dllname,int iMaxDelay)
{
    BOOL bret;

    this->Stop();

    this->m_iMaxDelay;
    _strncpy_s(this->m_strDllName,sizeof(this->m_strDllName),dllname,_TRUNCATE);
    this->m_hProc = hProc;
    return TRUE;
}


BOOL CImgCapController::CapImage(uint8_t * pData,int iLen,int * iFormat,int * iWidth,int * iHeight,int * iTimeStamp,int * iLastTimeStamp)
{
    BOOL bret;
    int ret;
	unsigned int curticks;

    if(this->m_hProc == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    ret = D3DHook_CaptureImageBuffer(this->m_hProc,this->m_strDllName,pData,iLen,iFormat,iWidth,iHeight);
    if(ret < 0)
    {
        ret = -ret;
		ERROR_INFO("could not capimage %s data 0x%p size %d error(%d)\n",this->m_strDllName,pData,iLen,ret);
        goto fail;
    }

	/*now to give the stamp*/
	GetCurrentTicks(&curticks);
	*iTimeStamp = curticks;
	*iLastTimeStamp = this->m_iLastTimeStamp;
	this->m_iLastTimeStamp = curticks;
	
	SetLastError(0);
	return TRUE;

fail:
    assert(ret > 0);
    SetLastError(ret);
    return FALSE;
}