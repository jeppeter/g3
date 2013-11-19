#ifndef __DLL_INSERT_H__
#define __DLL_INSERT_H__

#include <windows.h>

extern "C" int LoadInsert(const char* pExec,const char* pCommandLine,const char* pDllFullName,const char* pDllName);
extern "C" int CaptureFile(DWORD processid,const char* pDllName,const char* pFuncName,const char* bmpfile);
extern "C" int GetRemoteProcAddress(unsigned int processid,const char* pDllName,const char* pProcName,void** ppFnAddr);
extern "C" int CallRemoteFunc(unsigned int processid,void* pFnAddr,const char* pParam,int timeout,void** ppRetVal);
extern "C" int CallRemoteFuncRemoteParam(unsigned int processid,void* pFnAddr,LPVOID pRemoteAddr,int timeout,void**ppRetVal);
extern "C" ULONGLONG GetCurrentTick(ULONGLONG *pCtime);
extern "C" int TimeExpire(ULONGLONG ctime,ULONGLONG etime);

#endif  /*__DLL_INSERT_H__*/