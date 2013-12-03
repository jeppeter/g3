
/**************************************
this file is the file to
**************************************/
#ifndef  __IO_INJECT_THREAD_H__
#define __IO_INJECT_THREAD_H__

#include <funclist.h>

#ifdef __cplusplus
extern "C" {
#endif

    int IoInjectThreadInit(HMODULE hModule);
    void IoInjectThreadFini(HMODULE hModule);

    int RegisterEventListHandler(FuncCall_t pFunc,LPVOID pParam);
    int UnRegisterEventListHandler(FuncCall_t pFunc);

#ifdef __cplusplus
};
#endif


#endif  /*__IO_INJECT_THREAD_H__*/


