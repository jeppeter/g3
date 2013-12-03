// ioinject.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "detourdinput.h"
#include "detourrawinput.h"
#include "detourmessage.h"
#include "ioinject.h"
#include <injectbase.h>

static int st_IoInjectInited=0;

void FiniIoInject(HMODULE hModule)
{
    if(st_IoInjectInited)
    {
        DetourDirectInputFini();
        DetourRawInputFini();
        DetourMessageInputFini();
		IoInjectThreadFini(hModule);
    }
    st_IoInjectInited = 0;
    return ;
}

BOOL InitIoInject(HMODULE hModule)
{
    BOOL bret;
    int ret;

    DEBUG_INFO("st_IoInjectInited = %d\n",st_IoInjectInited);

    ret = IoInjectThreadInit(hModule);
    if(ret < 0)
    {
        return TRUE;
    }

    bret = DetourDirectInputInit();
    if(!bret)
    {
        return TRUE;
    }

    bret = DetourRawInputInit();
    if(!bret)
    {
        return TRUE;
    }

    bret = DetourMessageInputInit();
    if(!bret)
    {
        return TRUE;
    }

    ret = InsertModuleFileName(hModule);
    if(ret < 0)
    {
        return TRUE;
    }

    st_IoInjectInited = 1;
    return TRUE;

}


int IoInjectControl(PIO_CAP_CONTROL_t pControl)
{
    int ret;
    int lasterr=0;
    int totalret=0;
    ret = DetourDirectInputControl(pControl);
    if(ret < 0)
    {
        totalret = -ret;
        lasterr = LAST_ERROR_CODE();
    }

    ret = DetourRawInputControl(pControl);
    if(ret < 0)
    {
        totalret = -ret;
        lasterr = LAST_ERROR_CODE();
    }

    SetLastError(lasterr);
    return -totalret;
}

