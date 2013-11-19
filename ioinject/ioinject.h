
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
#define  IO_INJECT_ADD_DEVICE      2
#define  IO_INJECT_REMOVE_DEVICE   3

#define  IO_NAME_MAX_SIZE          256

typedef struct
{
    uint32_t opcode;
    uint32_t devtype;
    uint32_t devid;
    uint8_t memsharename[IO_NAME_MAX_SIZE];
    uint32_t memsharesize;
    uint32_t memsharesectsize;                       /*section size*/
    uint32_t memsharenum;
    uint8_t freeevtbasename[IO_NAME_MAX_SIZE];
    uint8_t inputevtbasename[IO_NAME_MAX_SIZE];
} IO_CAP_CONTROL_t,*PIO_CAP_CONTROL_t;

IOINJECT_API int IoInjectControl(PIO_CAP_CONTROL_t pControl);

#endif /*__IO_INJECT_H__*/

