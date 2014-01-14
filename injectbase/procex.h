
#ifndef __PROC_EX_H__
#define __PROC_EX_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef INJECTBASE_API
#define   INJECTBASE_API  extern "C"  __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef INJECTBASE_API
#define   INJECTBASE_API  extern "C" __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/


INJECTBASE_API int GetModuleInsertedProcess(const char* pPartDll,unsigned int **ppPids,int *pPidsSize);
/*0 for success negative for error code*/
INJECTBASE_API int EnableCurrentDebugPriv();
/*0 for success  negative for error code*/
INJECTBASE_API int DisableCurrentDebugPriv();


#endif /*__PROC_EX_H__*/

