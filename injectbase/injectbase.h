
#ifndef  __INJECT_BASE_H__
#define  __INJECT_BASE_H__



#ifdef INJECT_BASE_DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECT_BASE_DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECT_BASE_DLL_EXPORT*/

#include <detours/detours.h>
#include <uniansi.h>
#include <output_debug.h>
#include <evt.h>
#include <memshare.h>
#include <StackWalker.h>
#include <sched.h>
#include <procex.h>
#include <timeticks.h>



#endif /*__INJECT_BASE_H__*/

