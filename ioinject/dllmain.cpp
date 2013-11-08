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
	DEBUG_INFO("\n");
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitIoInject(hModule);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		FiniIoInject(hModule);
		break;
	}
	return TRUE;
}



