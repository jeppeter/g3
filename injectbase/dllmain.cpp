// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <injectbase.h>



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DEBUG_INFO("InjectBase hModule 0x%08x pid %d\n",hModule,GetCurrentProcessId());
		InjectBaseModuleInit(hModule);
		DEBUG_INFO("InjectBase Init Clean\n");
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DEBUG_INFO("InjectBase hModule 0x%08x pid %d\n",hModule,GetCurrentProcessId());
		InjectBaseModuleFini(hModule);
		DEBUG_INFO("InjectBase Fini Clean\n");
		break;
	}
	return TRUE;
}

