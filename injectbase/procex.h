
#ifndef __PROC_EX_H__
#define __PROC_EX_H__

#ifdef INJECT_BASE_DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECT_BASE_DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECT_BASE_DLL_EXPORT*/


EXPORT_C_FUNC int GetModuleInsertedProcess(const char* pPartDll,unsigned int **ppPids,int *pPidsSize);


#endif /*__PROC_EX_H__*/

