

#ifndef  __EVT_H__
#define  __EVT_H__

#include <Windows.h>

#ifdef INJECT_BASE_DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECT_BASE_DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECT_BASE_DLL_EXPORT*/


EXPORT_C_FUNC HANDLE GetEvent(const char* pName,int create);
#endif /*__EVT_H__*/

