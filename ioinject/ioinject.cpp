// ioinject.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "detourdinput.h"
#include "ioinject.h"
#include <injectbase.h>

static int st_IoInjectInited=0;

void FiniIoInject(HMODULE hModule)
{
    if(st_IoInjectInited)
    {
        DetourDirectInputFini();
    }
    st_IoInjectInited = 0;
    return ;
}

BOOL InitIoInject(HMODULE hModule)
{
    BOOL bret;
    int ret;

    bret = DetourDirectInputInit();
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


BOOL IoInjectDummyExport(void)
{
	return TRUE;
}

