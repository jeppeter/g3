// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "ioinject.h"
#include <output_debug.h>

#ifdef _DEBUG
#pragma comment(lib,"injectbased.lib")
#else
#pragma comment(lib,"injectbase.lib")
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DEBUG_INFO("hModule %d Pid %d\n",hModule,GetCurrentProcessId());
		InitIoInject(hModule);
		break;
	case DLL_THREAD_ATTACH:
		DEBUG_INFO("hModule %d Pid %d\n",hModule,GetCurrentProcessId());
		break;
	case DLL_THREAD_DETACH:
		DEBUG_INFO("hModule %d Pid %d\n",hModule,GetCurrentProcessId());
		break;
	case DLL_PROCESS_DETACH:
		DEBUG_INFO("hModule %d Pid %d\n",hModule,GetCurrentProcessId());
		FiniIoInject(hModule);
		break;
	}
	return TRUE;
}



