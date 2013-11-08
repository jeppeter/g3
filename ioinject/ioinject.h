
#ifndef __IO_INJECT_H__
#define __IO_INJECT_H__

#include <Windows.h>


#ifdef IOINJECT_EXPORTS
#ifndef IOINJECT_API
#define IOINJECT_API  extern "C" __declspec(dllexport)
#endif
#else  /*IOINJECT_EXPORTS*/
#ifndef IOINJECT_API
#define IOINJECT_API  extern "C" __declspec(dllimport)
#endif
#endif /*IOINJECT_EXPORTS*/


void FiniIoInject(HMODULE hModule);
BOOL InitIoInject(HMODULE hModule);

IOINJECT_API BOOL IoInjectDummyExport(void);

#endif /*__IO_INJECT_H__*/

