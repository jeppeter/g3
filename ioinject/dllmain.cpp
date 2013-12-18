// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "ioinject.h"
#include <output_debug.h>


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DEBUG_INFO("IoInject hModule 0x%x Pid %d\n",hModule,GetCurrentProcessId());
		InitIoInject(hModule);
		DEBUG_INFO("IoInject Init Clean\n");
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DEBUG_INFO("IoInject hModule 0x%x Pid %d\n",hModule,GetCurrentProcessId());
		FiniIoInject(hModule);
		DEBUG_INFO("IoInject Fini Clean\n");
		break;
	}
	return TRUE;
}



