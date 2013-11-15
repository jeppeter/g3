
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


typedef struct
{
    HANDLE m_hFillEvt;
    int m_Error;
    int m_Idx;
    pcmcap_ptr_t m_BaseAddr;
    pcmcap_ptr_t m_Offset;
    unsigned int size;
} EVENT_LIST_t;


INJECTBASE_API int InsertModuleFileName(HMODULE hModule);
INJECTBASE_API void SetUnHandlerExceptionDetour();



int InjectBaseModuleInit(HMODULE hModule);
void InjectBaseModuleFini(HMODULE hModule);


#endif /*__INJECT_BASE_H__*/

