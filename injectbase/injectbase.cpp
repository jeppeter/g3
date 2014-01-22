// injectbase.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include <assert.h>
#include <output_debug.h>
#include <detours/detours.h>
#include <psapi.h>
#include <uniansi.h>
#include <injectbase.h>
#include <injectbase_window.h>

#pragma comment(lib,"psapi.lib")

#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))


static std::vector<char*> st_InsertDllFullNames;
static std::vector<char*> st_InsertDllPartNames;
static CRITICAL_SECTION st_DllNameCS;
static int st_InjectModuleInited=0;



#define INSERT_DLL_NAME_ASSERT() \
do\
{\
	assert(st_InsertDllFullNames.size() == st_InsertDllPartNames.size());\
}while(0)

static int InsertDllNames(const char* pFullName,const char* pPartName)
{
    int ret=0;
    int findidx=-1;
    unsigned int i;
    char *pAllocFullName=NULL,*pAllocPartName=NULL;

    pAllocFullName = _strdup(pFullName);
    pAllocPartName = _strdup(pPartName);
    if(pAllocFullName == NULL || pAllocPartName == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    EnterCriticalSection(&st_DllNameCS);

    INSERT_DLL_NAME_ASSERT();
    for(i=0; i<st_InsertDllPartNames.size(); i++)
    {
        if(strcmp(pAllocPartName,st_InsertDllPartNames[i])==0)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        st_InsertDllFullNames.push_back(pAllocFullName);
        st_InsertDllPartNames.push_back(pAllocPartName);
        ret = 1;
    }

    LeaveCriticalSection(&st_DllNameCS);

    if(ret == 0)
    {
        assert(pAllocFullName && pAllocPartName);
        free(pAllocFullName);
        pAllocFullName = NULL;
        free(pAllocPartName);
        pAllocPartName = NULL;
    }

    return ret;

fail:
    if(pAllocFullName)
    {
        free(pAllocFullName);
    }
    pAllocFullName = NULL;

    if(pAllocPartName)
    {
        free(pAllocPartName);
    }
    pAllocPartName = NULL;
    SetLastError(ret);
    return -ret;
}





static int ClearDllNames(const char* pPartName)
{
    char* pFreePartName=NULL;
    char* pFreeFullName=NULL;
    int findidx=-1;
    unsigned int i;
    int ret =0;

    EnterCriticalSection(&st_DllNameCS);
    INSERT_DLL_NAME_ASSERT();
    for(i=0; i<st_InsertDllPartNames.size(); i++)
    {
        if(pPartName == NULL || strcmp(pPartName,st_InsertDllPartNames[i])==0)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >=0)
    {
        pFreePartName = st_InsertDllPartNames[findidx];
        pFreeFullName = st_InsertDllFullNames[findidx];
        st_InsertDllFullNames.erase(st_InsertDllFullNames.begin() + findidx);
        st_InsertDllPartNames.erase(st_InsertDllPartNames.begin() + findidx);
        ret = 1;
    }

    LeaveCriticalSection(&st_DllNameCS);
    if(ret > 0)
    {
        assert(pFreePartName && pFreeFullName);
        free(pFreePartName);
        free(pFreeFullName);
        pFreePartName = NULL;
        pFreeFullName = NULL;
    }
    return ret;
}

static void ClearAllDllNames()
{
    int ret;

    while(1)
    {
        ret = ClearDllNames(NULL);
        if(ret == 0)
        {
            break;
        }
    }
    return ;
}

static int GetDllNames(int idx,char**ppFullName,char**ppPartName)
{
    int ret = 0;
    int overflow ;
    int fullmallocsize=1024,fullsize;
    char* pFullName=*ppFullName;
    char* pPartName=*ppPartName;

    if(pFullName || pPartName)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    do
    {
        if(pFullName)
        {
            free(pFullName);
        }
        pFullName = NULL;
        if(pPartName)
        {
            free(pPartName);
        }
        pPartName = NULL;
        pFullName = (char*)malloc(fullmallocsize);
        pPartName = (char*)malloc(fullmallocsize);
        if(pFullName == NULL || pPartName == NULL)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }

        ret = 0;
        overflow = 0;
        EnterCriticalSection(&st_DllNameCS);
        INSERT_DLL_NAME_ASSERT();
        if(st_InsertDllFullNames.size() > (unsigned int)idx)
        {
            fullsize = strlen(st_InsertDllFullNames[idx]);
            if(fullsize < fullmallocsize)
            {
                ret = 1;
                strncpy_s(pFullName,fullmallocsize,st_InsertDllFullNames[idx],_TRUNCATE);
                strncpy_s(pPartName,fullmallocsize,st_InsertDllPartNames[idx],_TRUNCATE);
            }
            else
            {
                overflow = 1;
                fullmallocsize = fullsize + 1;
            }

        }
        LeaveCriticalSection(&st_DllNameCS);
    }
    while(overflow);

    if(ret == 0)
    {
        if(pFullName)
        {
            free(pFullName);
        }
        pFullName = NULL;
        if(pPartName)
        {
            free(pPartName);
        }
        pPartName = NULL;
    }
    else
    {
        *ppFullName = pFullName;
        *ppPartName = pPartName;
    }

    return ret;

fail:
    if(pFullName)
    {
        free(pFullName);
    }
    pFullName = NULL;
    if(pPartName)
    {
        free(pPartName);
    }
    pPartName = NULL;
    SetLastError(ret);
    return -ret;
}


BOOL InsertDlls(HANDLE hProcess)
{
    int ret;
    BOOL bret;
    int i;
    char* pFullName=NULL,*pPartName=NULL;
    LPCSTR rlpDlls[2];
    i = 0;
    while(1)
    {
        assert(pFullName == NULL && pPartName == NULL);
        ret = GetDllNames(i,&pFullName,&pPartName);
        if(ret < 0)
        {
            assert(pFullName == NULL && pPartName == NULL);
            ERROR_INFO("Get [%d] name error(%d)\n",i,ret);
            ret = LAST_ERROR_CODE();
            goto fail;
        }
        else if(ret == 0)
        {
            break;
        }

        rlpDlls[0] = pFullName;
        rlpDlls[1] = pPartName;
        DEBUG_INFO("update (%s:%s)\n",pFullName,pPartName);

        bret = UpdateImports(hProcess,rlpDlls,2);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Import Dll(%s:%s) error(%d)\n",pFullName,pPartName,ret);
        }

        free(pFullName);
        free(pPartName);
        pFullName = NULL;
        pPartName = NULL;
        i ++;

    }
    if(pFullName)
    {
        free(pFullName);
    }
    pFullName = NULL;
    if(pPartName)
    {
        free(pPartName);
    }
    pPartName = NULL;

    return TRUE;
fail:
    if(pFullName)
    {
        free(pFullName);
    }
    pFullName = NULL;
    if(pPartName)
    {
        free(pPartName);
    }
    pPartName = NULL;
    SetLastError(ret);
    return FALSE;
}


typedef BOOL(WINAPI *CreateProcessWFunc_t)(LPCWSTR lpApplicationName,LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,LPCWSTR lpCurrentDirectory,LPSTARTUPINFO lpStartupInfo,LPPROCESS_INFORMATION lpProcessInformation);
static CreateProcessWFunc_t CreateProcessWNext=CreateProcessW;

BOOL WINAPI CreateProcessWCallBack(LPCWSTR lpApplicationName,
                                   LPWSTR lpCommandLine,
                                   LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                   LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                   BOOL bInheritHandles,
                                   DWORD dwCreationFlags,
                                   LPVOID lpEnvironment,
                                   LPCWSTR lpCurrentDirectory,
                                   LPSTARTUPINFO lpStartupInfo,
                                   LPPROCESS_INFORMATION lpProcessInformation)
{
    DWORD dwMyCreationFlags = (dwCreationFlags | CREATE_SUSPENDED);
    PROCESS_INFORMATION pi;
    int ret,res;
    BOOL bret;
    HANDLE hdupproc=NULL;
    int pid=0;

    DEBUG_INFO("Current  Process (%d)\n",GetCurrentProcessId());
    if(!CreateProcessWNext(lpApplicationName,
                           lpCommandLine,
                           lpProcessAttributes,
                           lpThreadAttributes,
                           bInheritHandles,
                           dwMyCreationFlags,
                           lpEnvironment,
                           lpCurrentDirectory,
                           lpStartupInfo,
                           &pi))
    {
        ret = LAST_ERROR_CODE();
        DEBUG_INFO("lasterror %d\n",ret);
        SetLastError(ret);
        return FALSE;
    }

    DEBUG_INFO("\n");

    if(!InsertDlls(pi.hProcess))
    {
        ret = LAST_ERROR_CODE();
        bret = TerminateProcess(pi.hProcess,3);
        if(!bret)
        {
            pid = GetProcessPid(pi.hProcess);
            res = EnableCurrentDebugPriv();
            if(res < 0)
            {
                res = LAST_ERROR_CODE();
                ERROR_INFO("could not enable debug priv Error(%d)\n",res);
            }
            else
            {
                hdupproc = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
                if(hdupproc != NULL)
                {
                    bret = TerminateProcess(hdupproc,3);
                    if(!bret)
                    {
                        res = LAST_ERROR_CODE();
                        ERROR_INFO("could not Terminate Process %d\n",res);
                    }
					CloseHandle(hdupproc);
					hdupproc = NULL;
                }
                else
                {
                    res = LAST_ERROR_CODE();
                    ERROR_INFO("could not open process(%d) Error(%d)\n",pid,ret);
                }
            }
        }
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
        SetLastError(ret);
        return FALSE;
    }
    DEBUG_INFO("\n");

    if(lpProcessInformation)
    {
        CopyMemory(lpProcessInformation, &pi, sizeof(pi));
    }

    if(!(dwCreationFlags & CREATE_SUSPENDED))
    {
        ResumeThread(pi.hThread);
    }
    DEBUG_INFO("\n");
    return TRUE;

}



typedef BOOL(WINAPI *CreateProcessAFunc_t)(LPCSTR lpApplicationName,LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,LPCSTR lpCurrentDirectory,
        LPSTARTUPINFOA lpStartupInfo,LPPROCESS_INFORMATION lpProcessInformation);
static CreateProcessAFunc_t CreateProcessANext=CreateProcessA;

BOOL WINAPI CreateProcessACallBack(LPCSTR lpApplicationName,
                                   LPSTR lpCommandLine,
                                   LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                   LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                   BOOL bInheritHandles,
                                   DWORD dwCreationFlags,
                                   LPVOID lpEnvironment,
                                   LPCSTR lpCurrentDirectory,
                                   LPSTARTUPINFOA lpStartupInfo,
                                   LPPROCESS_INFORMATION lpProcessInformation)
{
    DWORD dwMyCreationFlags = (dwCreationFlags | CREATE_SUSPENDED);
    PROCESS_INFORMATION pi;
    int ret;
	BOOL bret;
	int pid,res;
	HANDLE hdupproc = NULL;

    DEBUG_INFO("Current  Process (%d)\n",GetCurrentProcessId());
    if(!CreateProcessANext(lpApplicationName,
                           lpCommandLine,
                           lpProcessAttributes,
                           lpThreadAttributes,
                           bInheritHandles,
                           dwMyCreationFlags,
                           lpEnvironment,
                           lpCurrentDirectory,
                           lpStartupInfo,
                           &pi))
    {
        ret = LAST_ERROR_CODE();
        DEBUG_INFO("lasterror %d\n",ret);
        SetLastError(ret);
        return FALSE;
    }

    DEBUG_INFO("\n");

    if(!InsertDlls(pi.hProcess))
    {
        ret = LAST_ERROR_CODE();
        bret = TerminateProcess(pi.hProcess,3);
        if(!bret)
        {
            pid = GetProcessPid(pi.hProcess);
            res = EnableCurrentDebugPriv();
            if(res < 0)
            {
                res = LAST_ERROR_CODE();
                ERROR_INFO("could not enable debug priv Error(%d)\n",res);
            }
            else
            {
                hdupproc = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
                if(hdupproc != NULL)
                {
                    bret = TerminateProcess(hdupproc,3);
                    if(!bret)
                    {
                        res = LAST_ERROR_CODE();
                        ERROR_INFO("could not Terminate Process %d\n",res);
                    }
					CloseHandle(hdupproc);
					hdupproc = NULL;
                }
                else
                {
                    res = LAST_ERROR_CODE();
                    ERROR_INFO("could not open process(%d) Error(%d)\n",pid,ret);
                }
            }
        }
		CloseHandle(pi.hProcess);
		pi.hProcess = NULL;
        SetLastError(ret);
        return FALSE;
    }
    DEBUG_INFO("\n");

    if(lpProcessInformation)
    {
        CopyMemory(lpProcessInformation, &pi, sizeof(pi));
    }

    if(!(dwCreationFlags & CREATE_SUSPENDED))
    {
        ResumeThread(pi.hThread);
    }
    DEBUG_INFO("\n");
    return TRUE;

}


int InsertModuleFileName(HMODULE hModule)
{
    HANDLE hProcess=NULL;
    DWORD dret;
#ifdef _UNICODE
    wchar_t *pModuleFullNameW=NULL;
    int modulefullnamesize=0;
#else
    char *pModuleFullNameA=NULL;
#endif
    char* pModuleFullName=NULL,*pModulePartName=NULL;
    int lasterr=0;
    int ret;
    unsigned int fullnamesize=1024;

    if(st_InjectModuleInited == 0)
    {
        return -ERROR_NOT_SUPPORTED;
    }

    hProcess = GetCurrentProcess();
    do
    {
        lasterr = 0;
#ifdef _UNICODE
        pModuleFullNameW = (wchar_t*)calloc(fullnamesize,sizeof(*pModuleFullNameW));
        if(pModuleFullNameW == NULL)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }
        dret = GetModuleFileNameEx(hProcess,hModule,pModuleFullNameW,fullnamesize);
#else
        pModuleFullNameA = calloc(fullnamesize,sizeof(*pModuleFullNameA));
        if(pModuleFullNameA == NULL)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }
        dret = GetModuleFileNameEx(hProcess,hModule,pModuleFullNameA,fullnamesize);
#endif


        if(dret == 0 || dret >= fullnamesize)
        {
            lasterr = LAST_ERROR_CODE();
#ifdef _UNICODE
            free(pModuleFullNameW);
            pModuleFullNameW = NULL;
#else
            free(pModuleFullNameA);
            pModuleFullNameA = NULL;
#endif
            fullnamesize <<= 1;
        }
    }
    while(lasterr == ERROR_INSUFFICIENT_BUFFER);

    if(dret == 0 || dret >= fullnamesize)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("can not get [0x%x:0x%x] modulefilename error(%d)\n",hProcess,hModule,ret);
        goto fail;
    }

#ifdef _UNICODE
    ret = UnicodeToAnsi(pModuleFullNameW,&pModuleFullName,&modulefullnamesize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
#else
    pModuleFullName = pModuleFullNameA;
#endif

    /*now get the name so we should*/
    pModulePartName = strrchr(pModuleFullName,'\\');
    if(pModulePartName)
    {
        pModulePartName ++;
    }
    else
    {
        pModulePartName = pModuleFullName;
    }

    /*now insert the dlls*/
    DEBUG_INFO("Insert into(%s:%s)\n",pModuleFullName,pModulePartName);
    ret = InsertDllNames(pModuleFullName,pModulePartName);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not insert (%s:%s) error(%d)\n",pModuleFullName,pModulePartName,ret);
        goto fail;
    }

    DEBUG_INFO("Insert (%s:%s) succ\n",pModuleFullName,pModulePartName);


#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pModuleFullName,&modulefullnamesize);
    if(pModuleFullNameW)
    {
        free(pModuleFullNameW);
    }
    pModuleFullNameW = NULL;
    pModulePartName = NULL;
#else
    if(pModuleFullNameA)
    {
        free(pModuleFullNameA);
    }
    pModuleFullNameA = NULL;
    pModuleFullName = NULL;
    pModulePartName = NULL;
#endif

    fullnamesize = 0;

    return 0;

fail:
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pModuleFullName,&modulefullnamesize);
    if(pModuleFullNameW)
    {
        free(pModuleFullNameW);
    }
    pModuleFullNameW = NULL;
    pModulePartName = NULL;
#else
    if(pModuleFullNameA)
    {
        free(pModuleFullNameA);
    }
    pModuleFullNameA = NULL;
    pModuleFullName = NULL;
    pModulePartName = NULL;
#endif
    fullnamesize = 0;
    SetLastError(ret);
    return -ret;

}


static int DetourCreateProcessFunctions()
{
    PVOID OldCreatW=NULL,OldCreateA=NULL;

#if 0
    CreateProcessNext =(CreateProcessFunc_t) GetProcAddress(hModule,(LPCSTR)TEXT("CreateProcess"));
    if(CreateProcessNext == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not get process addr for CreateProcess error(%d)\n",ret);
        goto fail;
    }
#endif
    OldCreatW = (PVOID)CreateProcessWNext;
    DEBUG_INFO("CreateProcess Code");
    DEBUG_BUFFER(OldCreatW,5);

    OldCreateA = (PVOID)CreateProcessANext;
    DEBUG_BUFFER(OldCreateA,5);
    DEBUG_BUFFER_FMT(CreateProcessWNext,10,"Before CreateProcessWNext (0x%p)",CreateProcessWNext);
    DEBUG_BUFFER_FMT(CreateProcessANext,10,"Before CreateProcessANext (0x%p)",CreateProcessANext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&CreateProcessWNext,CreateProcessWCallBack);
    DetourAttach((PVOID*)&CreateProcessANext,CreateProcessACallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(CreateProcessANext,10,"After CreateProcessANext (0x%p)",CreateProcessANext);
    DEBUG_BUFFER_FMT(CreateProcessWNext,10,"After CreateProcessWNext (0x%p)",CreateProcessWNext);
    return 0;
}

LONG WINAPI DetourApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
    StackWalker sw;
    EXCEPTION_RECORD *xr = pException->ExceptionRecord;
    CONTEXT *xc = pException->ContextRecord;
    DEBUG_INFO("Eip 0x%08x\n",xc->Eip);
    sw.ShowCallstack(GetCurrentThread(), pException->ContextRecord,NULL,NULL);
    abort();
    return EXCEPTION_EXECUTE_HANDLER;
}


static int DetourDestroyWindow();
static int DetourCreateWindow();

int InjectBaseModuleInit(HMODULE hModule)
{
    int ret;
    InitializeCriticalSection(&st_DllNameCS);

    DEBUG_INFO("\n");

    ret = DetourShowCursorFunction();
    if(ret < 0)
    {
        return 0;
    }

    ret = DetourCreateProcessFunctions();
    if(ret < 0)
    {
        return 0;
    }


    ret = DetourDestroyWindow();
    if(ret < 0)
    {
        return 0;
    }

    ret = DetourCreateWindow();
    if(ret < 0)
    {
        return 0;
    }

    DEBUG_INFO("\n");
    st_InjectModuleInited = 1;

    ret = InsertModuleFileName(hModule);
    if(ret < 0)
    {
        return 0;
    }
    DEBUG_INFO("\n");
    SetUnHandlerExceptionDetour();


    return 0;

}

void SetUnHandlerExceptionDetour()
{
    //SetUnhandledExceptionFilter(DetourApplicationCrashHandler);
}

void InjectBaseModuleFini(HMODULE hModule)
{
    if(st_InjectModuleInited)
    {
        ClearAllDllNames();
        //SetUnhandledExceptionFilter(NULL);
    }
    return;
}

CFuncList st_DestroyFuncList;
typedef BOOL (WINAPI *DestroyWindowFunc_t)(HWND hWnd);

static DestroyWindowFunc_t DestroyWindowNext=DestroyWindow;

BOOL WINAPI DestroyWindowCallBack(HWND hwnd)
{
    BOOL bret;

    bret = DestroyWindowNext(hwnd);
    if(bret)
    {
        st_DestroyFuncList.CallList(hwnd);
        RemoveHwnd(hwnd);
    }
    return bret;
}

CFuncList st_CreateWindowFuncList;
typedef HWND (WINAPI *CreateWindowExAFunc_t)(DWORD dwExStyle,
        LPCSTR lpClassName,
        LPCSTR lpWindowName,
        DWORD dwStyle,
        int x,
        int y,
        int nWidth,
        int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam);

typedef HWND (WINAPI *CreateWindowExWFunc_t)(DWORD dwExStyle,
        LPCWSTR lpClassName,
        LPCWSTR lpWindowName,
        DWORD dwStyle,
        int x,
        int y,
        int nWidth,
        int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam);

static CreateWindowExAFunc_t CreateWindowExANext=CreateWindowExA;
static CreateWindowExWFunc_t CreateWindowExWNext=CreateWindowExW;


HWND WINAPI CreateWindowExACallBack(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
)
{
    HWND hWnd = NULL;

    hWnd = CreateWindowExANext(dwExStyle,
                               lpClassName,
                               lpWindowName,
                               dwStyle,
                               x,
                               y,
                               nWidth,
                               nHeight,
                               hWndParent,
                               hMenu,
                               hInstance,
                               lpParam
                              );
    if(hWnd != NULL)
    {
        DEBUG_INFO("hWnd (0x%08x) ThreadId(%d) dwStyle 0x%08x dwExStyle 0x%08x\n",hWnd,GetCurrentThreadId(),
                   dwStyle,dwExStyle);
        /*only visible window ,we put it when call*/
        InsertHwnd(hWnd);
        st_CreateWindowFuncList.CallList(hWnd);
    }
    return hWnd;
}

HWND WINAPI CreateWindowExWCallBack(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
)
{
    HWND hWnd = NULL;

    hWnd = CreateWindowExWNext(dwExStyle,
                               lpClassName,
                               lpWindowName,
                               dwStyle,
                               x,
                               y,
                               nWidth,
                               nHeight,
                               hWndParent,
                               hMenu,
                               hInstance,
                               lpParam
                              );
    if(hWnd != NULL)
    {
        DEBUG_INFO("hWnd (0x%08x) ThreadId(%d) dwStyle 0x%08x dwExStyle 0x%08x\n",hWnd,GetCurrentThreadId(),
                   dwStyle,dwExStyle);
        /*only visible window, we put it ok*/
        InsertHwnd(hWnd);
        st_CreateWindowFuncList.CallList(hWnd);
    }
    return hWnd;
}




static int DetourDestroyWindow()
{
    DEBUG_BUFFER_FMT(DestroyWindowNext,10,"Before DestroyWindowNext (0x%p)",DestroyWindowNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&DestroyWindowNext,DestroyWindowCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(DestroyWindowNext,10,"After DestroyWindowNext (0x%p)",DestroyWindowNext);
    return 0;
}

static int DetourCreateWindow()
{
    DEBUG_BUFFER_FMT(CreateWindowExANext,10,"Before CreateWindowExANext (0x%p)",CreateWindowExANext);
    DEBUG_BUFFER_FMT(CreateWindowExWNext,10,"Before CreateWindowExWNext (0x%p)",CreateWindowExWNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&CreateWindowExANext,CreateWindowExACallBack);
    DetourAttach((PVOID*)&CreateWindowExWNext,CreateWindowExWCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(CreateWindowExANext,10,"After CreateWindowExANext (0x%p)",CreateWindowExANext);
    DEBUG_BUFFER_FMT(CreateWindowExWNext,10,"After CreateWindowExWNext (0x%p)",CreateWindowExWNext);
    return 0;
}



int RegisterCreateWindowFunc(FuncCall_t pFunc,LPVOID pParam,int prior)
{
    return st_CreateWindowFuncList.AddFuncList(pFunc,pParam,prior);
}

int UnRegisterCreateWindowFunc(FuncCall_t pFunc)
{
    return st_CreateWindowFuncList.RemoveFuncList(pFunc);
}

int RegisterDestroyWindowFunc(FuncCall_t pFunc,LPVOID pParam,int prior)
{
    return st_DestroyFuncList.AddFuncList(pFunc,pParam,prior);
}

int UnRegisterDestroyWindowFunc(FuncCall_t pFunc)
{
    return st_DestroyFuncList.RemoveFuncList(pFunc);
}


