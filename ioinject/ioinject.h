
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

#define  IO_INJECT_STOP            0
#define  IO_INJECT_START           1

#define  IO_NAME_MAX_SIZE          256

typedef struct
{
	int opcode;
	unsigned char memsharename[IO_NAME_MAX_SIZE];
	int memsharesize;	
	int memsharesectsize;                       /*section size*/
	int memsharenum;
	unsigned char freeevtbasename[IO_NAME_MAX_SIZE];
	unsigned char inputevtbasename[IO_NAME_MAX_SIZE];
} IO_CAP_CONTROL_t,*PIO_CAP_CONTROL_t;

IOINJECT_API int IoInjectControl(PIO_CAP_CONTROL_t pControl);

#endif /*__IO_INJECT_H__*/

