
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

typedef struct
{
	unsigned char memsharename[256];
	int memsharesize;	
	int memsharesectsize;                       /**/
} IO_CAP_CONTROL_t,*PIO_CAP_CONTROL_t;

IOINJECT_API BOOL IoInjectDummyExport(void);

#endif /*__IO_INJECT_H__*/

