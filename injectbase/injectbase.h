
#ifndef  __INJECT_BASE_H__
#define  __INJECT_BASE_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef INJECTBASE_API
#define   INJECTBASE_API   extern "C" __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef INJECTBASE_API
#define   INJECTBASE_API  extern "C" __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/

#include <detours/detours.h>
#include <uniansi.h>
#include <output_debug.h>
#include <evt.h>
#include <memshare.h>
#include <StackWalker.h>
#include <sched.h>
#include <procex.h>
#include <timeticks.h>
#include <injectcommon.h>

typedef struct
{
    HANDLE m_hFillEvt;
    int m_Error;
    int m_Idx;
    ptr_t m_BaseAddr;
    ptr_t m_Offset;
    unsigned int size;
} EVENT_LIST_t,*PEVENT_LIST_t;



INJECTBASE_API int InsertModuleFileName(HMODULE hModule);
INJECTBASE_API void SetUnHandlerExceptionDetour();
INJECTBASE_API void StopThreadControl(thread_control_t* pThrControl);
INJECTBASE_API int StartThreadControl(thread_control_t* pThrControl,ThreadFunc_t pStartFunc,LPVOID pParam);




int InjectBaseModuleInit(HMODULE hModule);
void InjectBaseModuleFini(HMODULE hModule);


#endif /*__INJECT_BASE_H__*/

