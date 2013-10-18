
#ifndef  __INJECT_BASE_H__
#define  __INJECT_BASE_H__



#ifdef DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC
#endif
#endif   /*DLL_EXPORT*/

#include <detours/detours.h>

EXPORT_C_FUNC int Capture3DBackBuffer(const char* filetosave);
EXPORT_C_FUNC int Capture2DBackBufferD11(const char* filetosave);



#endif /*__INJECT_BASE_H__*/

