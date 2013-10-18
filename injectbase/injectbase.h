
#ifndef  __INJECT_BASE_H__
#define  __INJECT_BASE_H__



#ifdef DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C
#endif
#endif   /*DLL_EXPORT*/

#include <detours/detours.h>





#endif /*__INJECT_BASE_H__*/

