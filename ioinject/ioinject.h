
#ifndef __IO_INJECT_H__
#define __IO_INJECT_H__

#include <Windows.h>
#include <iocapcommon.h>

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



IOINJECT_API int IoInjectControl(PIO_CAP_CONTROL_t pControl);

#endif /*__IO_INJECT_H__*/

