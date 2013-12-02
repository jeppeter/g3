
/**************************************
this file is the file to 
**************************************/
#ifndef  __IO_INJECT_THREAD_H__
#define __IO_INJECT_THREAD_H__

#include <funclist.h>

int InitIoInjectThread();
void FiniIoInjectThread();

int RegisterEventListHandler(FuncCall_t pFunc,LPVOID pParam);
int UnRegisterEventListHandler(FuncCall_t pFunc);


#endif  /*__IO_INJECT_THREAD_H__*/


