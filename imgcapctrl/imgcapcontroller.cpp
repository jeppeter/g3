
#include "imgcapcontroller.h"
#include <string.h>
#include <output_debug.h>
#include <timeticks.h>
#include <assert.h>
#include <winproc.h>


#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))

extern "C" int D3DHook_CaptureImageBuffer(HANDLE hProc,char* strDllName,char * data, int len, int * format, int * width, int * height);

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
    this->Stop();

    this->m_iMaxDelay;
    strncpy_s((char*)this->m_strDllName,sizeof(this->m_strDllName),dllname,_TRUNCATE);
    this->m_hProc = hProc;
    return TRUE;
}


BOOL CImgCapController::CapImage(uint8_t * pData,int iLen,int * iFormat,int * iWidth,int * iHeight,int * iTimeStamp,int * pRetLen)
{
    int ret;
    unsigned int curticks;

    if(this->m_hProc == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    ret = D3DHook_CaptureImageBuffer(this->m_hProc,(char*)this->m_strDllName,(char*)pData,iLen,iFormat,iWidth,iHeight);
    if(ret >=0)
    {

        if(pRetLen)
        {
            *pRetLen = ret;
        }

        /*now to give the stamp*/
        GetCurrentTicks(&curticks);
        *iTimeStamp = curticks;
        this->m_iLastTimeStamp = curticks;

        SetLastError(0);
        return TRUE;
    }

    /*now we should give the error*/
    return this->__CapImageWindowed(pData,iLen,iFormat,iWidth,iHeight,iTimeStamp,pRetLen);
}


BOOL CImgCapController::__CapImageWindowed(uint8_t * pData,int iLen,int * iFormat,int * iWidth,int * iHeight,int * iTimeStamp,int * pRetLen)
{
    HWND *pAllWnds=NULL,*pTopWnd=NULL;
    int allwndsize=0,topwndsize=0;
    int allwndnum=0,topwndnum=0;
    int ret;

    ret = GetProcWindHandles(this->m_hProc,&pAllWnds,&allwndsize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not get 0x%08x windows error(%d)\n",this->m_hProc,ret);
        goto fail;
    }
    else if(ret == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("0x%08x proc widnows 0\n");
        goto fail;
    }
    allwndnum = ret;


    ret = GetTopWinds(pAllWnds,allwndnum,&pTopWnd,&topwndsize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not get 0x%08x top win error(%d)\n",this->m_hProc,ret);
        goto fail;
    }
    else if(ret == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("0x%08x proc top widnows 0\n");
        goto fail;
    }

    topwndnum = ret;

    ret = GetWindowBmpBuffer(pTopWnd[0],pData,iLen,iFormat,iWidth,iHeight);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("GetWindowBmpBuffer error(%d)\n",ret);
        goto fail;
    }


    GetTopWinds(NULL,0,&pTopWnd,&topwndsize);
    topwndnum = 0;
    GetProcWindHandles(NULL,&pAllWnds,&allwndsize);
    allwndnum = 0;
    SetLastError(0);
    return TRUE;
fail:
    GetTopWinds(NULL,0,&pTopWnd,&topwndsize);
    topwndnum = 0;
    GetProcWindHandles(NULL,&pAllWnds,&allwndsize);
    allwndnum = 0;
    SetLastError(ret);
    return FALSE;
}

