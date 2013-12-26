
/**************************************
this file is the file to
**************************************/
#ifndef  __IO_INJECT_THREAD_H__
#define __IO_INJECT_THREAD_H__

#include <funclist.h>
#include <iocapcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

    int IoInjectThreadInit(HMODULE hModule);
    void IoInjectThreadFini(HMODULE hModule);

	int RegisterEventListInit(FuncCall_t pFunc,LPVOID pParam,int prior);
	int UnRegisterEventListInit(FuncCall_t pFunc);
    int RegisterEventListHandler(FuncCall_t pFunc,LPVOID pParam,int prior);
    int UnRegisterEventListHandler(FuncCall_t pFunc);
	int DetourIoInjectThreadControl(PIO_CAP_CONTROL_t pControl);

#define  DINPUT_EMULATION_PRIOR          10
#define  RAWINPUT_EMULATION_PRIOR        20
#define  MESSAGE_EMULATION_PRIOR         30

#ifdef __cplusplus
};
#endif


#endif  /*__IO_INJECT_THREAD_H__*/


