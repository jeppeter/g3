

#include "dllinsert.h"
#include "uniansi.h"
#include "detours/detours.h"
#include <windows.h>
#include <TlHelp32.h>
#include <assert.h>
#include "output_debug.h"
#include <Psapi.h>

#define LAST_ERROR_RETURN()  ((int)(GetLastError() ? GetLastError() : 1))

#define REMOTE_OFFSET_OF(ptr,typestruct,member) ((unsigned long)(ptr) + (unsigned long)(((typestruct*)0)->member))

int __LoadInsert(const char* pExec,const char* pCommandLine,const char* pDllFullName,const char* pDllName)
{
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    int ret;
    si.cb = sizeof(si);
#ifdef _UNICODE
    LPWSTR pExecWide=NULL;
    LPWSTR pCommandWide=NULL;
    int execsize=0,commandsize = 0;
    int execlen=0,commandlen = 0;
    if(pExec)
    {
        execlen = AnsiToUnicode((char*)pExec,&pExecWide,&execsize);
        if(execlen < 0)
        {
            return execlen;
        }
    }
    if(pCommandLine)
    {
        commandlen = AnsiToUnicode((char*)pCommandLine,&pCommandWide,&commandsize);
        if(commandlen < 0)
        {
            AnsiToUnicode(NULL,&pExecWide,&execsize);
            return commandlen;
        }
    }

    ret = DetourCreateProcessWithDllW(pExecWide,pCommandWide,NULL,NULL,TRUE,CREATE_DEFAULT_ERROR_MODE,
                                      NULL,NULL,
                                      &si,&pi,pDllFullName,pDllName,NULL);


    AnsiToUnicode(NULL,&pCommandWide,&commandsize);
    AnsiToUnicode(NULL,&pExecWide,&execsize);

#else
    ret = DetourCreateProcessWithDllA(pExec,pCommandLine,NULL,NULL,TRUE,CREATE_DEFAULT_ERROR_MODE
                                      NULL,NULL,
                                      &si,&pi,pDllFullName,pDllName,NULL);
#endif

    if(ret < 0)
    {
        return ret;
    }
    return ret;
}

extern "C" int LoadInsert(const char* pExec,const char* pCommandLine,const char* pDllFullName,const char* pDllName)
{

    DEBUG_INFO("load %s exe command(%s) with fullname (%s) dll (%s)\n",pExec ? pExec : "null",pCommandLine ? pCommandLine : "null",pDllFullName,pDllName);
    return __LoadInsert(pExec,pCommandLine,pDllFullName,pDllName);
}



int LowerCaseName(const char* pName)
{
    char* pCurPtr=(char*)pName;

    while((*pCurPtr) != '\0')
    {
        if((*pCurPtr) >= 'A' && (*pCurPtr) <= 'Z')
        {
            *pCurPtr -= 'A' ;
            *pCurPtr += 'a';
        }
        pCurPtr ++;
    }
    return (pCurPtr - pName);
}


PVOID __GetModuleBaseAddr(unsigned int processid,const char* pDllName)
{
    /*first to open process*/
    HANDLE hProc=NULL;
    BOOL bret;
    int ret;
    PVOID pBaseAddr=NULL;
    HMODULE *pHModules=NULL;
    DWORD modulesize=1024;
    DWORD modulelen=0,moduleret=0;
    DWORD i;
    MODULEINFO modinfo;
    int namesize=1024;
#ifdef _UNICODE
    wchar_t *pNameWide=NULL;
    char *pNameAnsi=NULL;
    int ansisize = 0;
#else
    char *pNameAnsi=NULL;
#endif
    char *pPartName=NULL;



    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,processid);
    if(hProc == NULL)
    {
        ret = LAST_ERROR_RETURN();
        ERROR_INFO("Could not Open(%d) error(%d)\n",processid,ret);
        goto fail;
    }
    pHModules = (HMODULE*)calloc(sizeof(*pHModules),modulesize);
    if(pHModules == NULL)
    {
        ret = LAST_ERROR_RETURN();
        goto fail;
    }


    bret = EnumProcessModules(hProc,pHModules,modulesize*sizeof(*pHModules),&moduleret);
    if(!bret)
    {
        ret = LAST_ERROR_RETURN();
        ERROR_INFO("[%d] get modulesize %d error(%d)\n",processid,modulesize,ret);
        free(pHModules);
        pHModules = NULL;
        modulesize = ((moduleret +1)/ sizeof(*pHModules));
        pHModules = (HMODULE*)calloc(sizeof(*pHModules),modulesize);
        if(pHModules == NULL)
        {
            ret = LAST_ERROR_RETURN();
            goto fail;
        }
        bret = EnumProcessModules(hProc,pHModules,modulesize*sizeof(*pHModules),&moduleret);
        if(!bret)
        {
            ret =LAST_ERROR_RETURN();
            ERROR_INFO("[%d] get modulesize %d error(%d)\n",processid,modulesize,ret);
            goto fail;
        }
    }

    modulelen = moduleret / sizeof(*pHModules);

#ifdef _UNICODE
    pNameWide = (wchar_t*)calloc(sizeof(*pNameWide),namesize);
    if(pNameWide == NULL)
    {
        ret =LAST_ERROR_RETURN();
        goto fail;
    }
#else  /*_UNICODE*/
    pNameAnsi = (char*)calloc(sizeof(*pNameAnsi),namesize);
    if(pNameAnsi == NULL)
    {
        ret = LAST_ERROR_RETURN();
        goto fail;
    }
#endif  /*_UNICODE*/

    for(i=0; i<modulelen; i++)
    {
#ifdef _UNICODE
        bret = GetModuleFileNameEx(hProc,pHModules[i],pNameWide,namesize);
#else
        bret = GetModuleFileNameEx(hProc,pHModules[i],pNameAnsi,namesize);
#endif
        if(!bret)
        {
            ret =LAST_ERROR_RETURN();
            ERROR_INFO("[%d] process->[%d]module(0x%08x) error(%d)\n",processid,i,pHModules[i],ret);
            continue;
        }

#ifdef _UNICODE
        ret = UnicodeToAnsi(pNameWide,&pNameAnsi,&ansisize);
        if(ret < 0)
        {
            ret = LAST_ERROR_RETURN();
            goto fail;
        }
#endif
        pPartName = strrchr(pNameAnsi,'\\');
        if(pPartName)
        {
            pPartName += 1;
        }
        else
        {
            pPartName = pNameAnsi;
        }

        if(pDllName == NULL || _stricmp(pPartName,pDllName)==0)
        {
            /*now get the module information*/
            bret = GetModuleInformation(hProc,pHModules[i],&modinfo,sizeof(modinfo));
            if(!bret)
            {
                ret = LAST_ERROR_RETURN();
                ERROR_INFO("[%d]process->[%d](0x%08x) dll(%s) get info error(%d)\n",
                           processid,i,pHModules[i],pNameAnsi,ret);
                goto fail;
            }

            pBaseAddr = modinfo.lpBaseOfDll;
            break;
        }

    }

    if(pBaseAddr==NULL)
    {
        ret = ERROR_MOD_NOT_FOUND;
        ERROR_INFO("[%d] not found %s\n",processid,pDllName);
        goto fail;
    }

#ifdef _UNICODE
    if(pNameWide)
    {
        free(pNameWide);
    }
    pNameWide = NULL;
    UnicodeToAnsi(NULL,&pNameAnsi,&ansisize);
#else  /*_UNICODE*/
    if(pNameAnsi)
    {
        free(pNameAnsi);
    }
    pNameAnsi = NULL;
#endif  /*_UNICODE*/
    namesize = 0;

    if(pHModules)
    {
        free(pHModules);
    }
    pHModules = NULL;
    modulelen = 0;
    modulesize = 0;
    moduleret = 0;

    if(hProc)
    {
        CloseHandle(hProc);
    }
    hProc = NULL;


    SetLastError(0);
    return pBaseAddr;
fail:
    assert(ret > 0);

#ifdef _UNICODE
    if(pNameWide)
    {
        free(pNameWide);
    }
    pNameWide = NULL;
    UnicodeToAnsi(NULL,&pNameAnsi,&ansisize);
#else  /*_UNICODE*/
    if(pNameAnsi)
    {
        free(pNameAnsi);
    }
    pNameAnsi = NULL;
#endif  /*_UNICODE*/
    namesize = 0;

    if(pHModules)
    {
        free(pHModules);
    }
    pHModules = NULL;
    modulelen = 0;
    modulesize = 0;
    moduleret = 0;

    if(hProc)
    {
        CloseHandle(hProc);
    }
    hProc = NULL;
    SetLastError(ret);
    return NULL;
}



BOOL __ReallocateSize(PVOID* ppBuffer,int buflen,int*pBufsize)
{
    PVOID pBuffer = NULL;
    int bufsize=*pBufsize;
    BOOL bret;

    if(buflen > 0)
    {
        pBuffer = VirtualAllocEx(GetCurrentProcess(),NULL,buflen,MEM_COMMIT,PAGE_READWRITE);
        if(pBuffer == NULL)
        {
            return FALSE;
        }
    }

    if(*ppBuffer)
    {
        SetLastError(0);
        //bret = VirtualFreeEx(GetCurrentProcess(),*ppBuffer,bufsize,MEM_DECOMMIT);
        bret = VirtualFreeEx(GetCurrentProcess(),*ppBuffer,0,MEM_RELEASE);
        if(!bret)
        {
            //DEBUG_INFO("Free 0x%p [%d] error(%d)\n",*ppBuffer,bufsize,GetLastError());
            DEBUG_INFO("Free 0x%p [%d] error(%d)\n",*ppBuffer,0,GetLastError());
        }
    }
    *ppBuffer = pBuffer;
    *pBufsize = buflen;
    return TRUE;
}


DWORD __GetExportTableAddr(HANDLE hProcess,PVOID pModBase)
{
    PIMAGE_DOS_HEADER pDosHeader=NULL;
    PIMAGE_FILE_HEADER pFileHeader=NULL;
    PIMAGE_OPTIONAL_HEADER32 pOptional32=NULL;
    PIMAGE_NT_HEADERS pNtHeader=NULL;
    PIMAGE_DATA_DIRECTORY pDataDir=NULL;
    PVOID pBuffer=NULL;
    DWORD pTableAddr=NULL;
    int bufsize=0;
    int buflen=0;
    int ret;
    BOOL bret;
    unsigned char* pRPtr=NULL;
    SIZE_T rsize;
    bufsize = 0;
    buflen = sizeof(*pDosHeader);
    if(buflen > bufsize)
    {
        bufsize = buflen;
    }
    buflen = sizeof(*pNtHeader);
    if(buflen > bufsize)
    {
        bufsize = buflen;
    }

    bret = __ReallocateSize(&pBuffer,buflen,&bufsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    /*now we should read the memory of DOS header*/
    pRPtr =(unsigned char*) pModBase;
    pDosHeader = (PIMAGE_DOS_HEADER) pBuffer;
    bret = ReadProcessMemory(hProcess,pRPtr,pDosHeader,sizeof(*pDosHeader),&rsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        goto fail;
    }
    if(rsize != sizeof(*pDosHeader))
    {
        ret = ERROR_INVALID_BLOCK;
        goto fail;
    }

    /*header of MZ*/
    if(pDosHeader->e_magic != 0x5a4d)
    {
        ret = ERROR_INVALID_BLOCK;
        goto fail;
    }

    pRPtr += pDosHeader->e_lfanew;
    pNtHeader =(PIMAGE_NT_HEADERS) pBuffer;
    bret = ReadProcessMemory(hProcess,pRPtr,pNtHeader,sizeof(*pNtHeader),&rsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        goto fail;
    }

    if(rsize != sizeof(*pNtHeader))
    {
        ret = ERROR_INVALID_BLOCK;
        goto fail;
    }

    /*header of PE\0\0*/
    if(pNtHeader->Signature != 0x00004550)
    {
        ret = ERROR_INVALID_BLOCK;
        goto fail;
    }

    /*now we get the PE header so we should get the export table*/
    pOptional32 = &(pNtHeader->OptionalHeader);
    if(pOptional32->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        ret = ERROR_INVALID_BLOCK;
        goto fail;
    }

    pDataDir = &(pOptional32->DataDirectory[0]);

    pTableAddr = pDataDir->VirtualAddress;
    __ReallocateSize(&pBuffer,0,&bufsize);

    return pTableAddr;


fail:
    __ReallocateSize(&pBuffer,0,&bufsize);
    SetLastError(ret);
    return NULL;
}

int __ReadName(HANDLE hProcess,PVOID pModBase,DWORD rva,PVOID *ppBuffer,int skipbyte,int* pBufSize)
{
    int done;
    int readlen ;
    SIZE_T rsize;
    unsigned char* pCurPtr=NULL;
    PVOID pTmpBuf=NULL;
    unsigned char* pCurAddr;
    int tmpsize=0,tmplen=0;
    BOOL bret;
    int ret;
    unsigned char* pChar=NULL;

    done = 0;
    readlen = 0;
    /*now first to read for the readnum*/

    pCurPtr =(unsigned char*) *ppBuffer;
    pCurAddr = (unsigned char*)pModBase + rva+skipbyte;

    for(;;)
    {
        if(readlen >= *pBufSize)
        {
            /*we will expand the buffer size*/
            tmplen = (*pBufSize) << 1 ? (*pBufSize) << 1 : 0x1000;
            assert(tmpsize == 0);
            assert(pTmpBuf == NULL);
            bret = __ReallocateSize(&pTmpBuf,tmplen,&tmpsize);
            if(!bret)
            {
                ret = GetLastError() ? GetLastError() : 1;
                goto fail;
            }

            /*now to copy memory*/
            if(readlen > 0)
            {
                memcpy(pTmpBuf,*ppBuffer,readlen);
            }
            /*free buffer*/
            __ReallocateSize(ppBuffer,0,pBufSize);
            /*to replace the buffer*/
            *ppBuffer = pTmpBuf;
            *pBufSize = tmpsize;
            pTmpBuf = NULL;
            tmpsize = 0;
            pCurPtr = ((unsigned char*)(*ppBuffer) + readlen);
        }

        bret = ReadProcessMemory(hProcess,pCurAddr,pCurPtr,1,&rsize);
        if(!bret)
        {
            ret = GetLastError() ? GetLastError() : 1;
            goto fail;
        }
        if(rsize != 1)
        {
            ret = ERROR_INVALID_BLOCK;
            goto fail;
        }
        if((*pCurPtr) == '\0')
        {
            done =1;
            break;
        }
        pCurPtr += 1;
        readlen += 1;
        pCurAddr += 1;
    }

    assert(pTmpBuf == NULL);
    return readlen;

fail:
    __ReallocateSize(&pTmpBuf,0,&tmpsize);
    SetLastError(ret);
    return -ret;
}


PVOID __GetProcAddr(HANDLE hProcess,PVOID pModBase,DWORD tablerva,const char* pDllName,const char* pProcName)
{
    int buflen=0,bufsize=0;
    PVOID pBuffer=NULL;
    int namelen = 0,namesize=0;
    PVOID pNameBuf=NULL;
    int ret;
    SIZE_T rsize;
    BOOL bret;
    int fntablesize=0,fntablelen=0;
    PVOID pFnAddr = NULL;
    DWORD *pFnTable=NULL;
    int nametablesize=0,nametablelen=0;
    DWORD *pNameTable=NULL;
    PIMAGE_EXPORT_DIRECTORY pExportDir=NULL;
    unsigned int i;
    int findidx,readlen;
    DWORD rva;

    buflen = 0x1000;
    bret = __ReallocateSize(&pBuffer,buflen,&bufsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }
    namelen = 0x1000;
    bret = __ReallocateSize(&pNameBuf,namelen,&namesize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }
    buflen = sizeof(*pExportDir);
    if(buflen > bufsize)
    {
        bret = __ReallocateSize(&pBuffer,buflen,&bufsize);
        if(!bret)
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("\n");
            goto fail;
        }
    }

    /*now we should get the table address */
    pExportDir = (PIMAGE_EXPORT_DIRECTORY)pBuffer;
    bret = ReadProcessMemory(hProcess,(unsigned char*)pModBase + tablerva,pExportDir,sizeof(*pExportDir),&rsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }
    if(rsize != sizeof(*pExportDir))
    {
        ret = ERROR_INVALID_BLOCK;
        DEBUG_INFO("\n");
        goto fail;
    }

    /*now we should compare module name ,this will give it check*/
    rva = pExportDir->Name;
    readlen = __ReadName(hProcess,pModBase,rva,&pNameBuf,0,&namesize);
    if(readlen < 0)
    {
        ret = -(int)readlen;
        DEBUG_INFO("\n");
        goto fail;
    }

    LowerCaseName((const char*)pNameBuf);
    /*now to compare the name*/
    if(_stricmp((const char*)pNameBuf,pDllName)!= 0)
    {
        ret = ERROR_INVALID_NAME;
        DEBUG_INFO("dllname (%s) pNameBuf (%s)\n",pDllName,pNameBuf);
        goto fail;
    }

    /*now we should get for the functions scanning*/
    fntablelen = sizeof(DWORD)*pExportDir->NumberOfFunctions;
    bret = __ReallocateSize((PVOID*)&pFnTable,fntablelen,&fntablesize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    rva = pExportDir->AddressOfFunctions;
    bret = ReadProcessMemory(hProcess,(unsigned char*)pModBase+rva ,pFnTable,fntablelen,&rsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    if(rsize != fntablelen)
    {
        ret = ERROR_INVALID_BLOCK;
        DEBUG_INFO("\n");
        goto fail;
    }

    /*name table is less than the functions ,if it has no name ,the value is 0 ,so we should set NumberOfFunctions not NumberOfNames*/
    nametablelen = sizeof(DWORD)*pExportDir->NumberOfFunctions;
    bret = __ReallocateSize((PVOID*)&pNameTable,nametablelen,&nametablesize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    rva = pExportDir->AddressOfNames;
    bret = ReadProcessMemory(hProcess,(unsigned char*)pModBase+rva ,pNameTable,nametablelen,&rsize);
    if(!bret)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    if(rsize != nametablelen)
    {
        ret = ERROR_INVALID_BLOCK;
        DEBUG_INFO("\n");
        goto fail;
    }

    findidx = -1;
    /*now to get the name and */
    for(i = 0; i<pExportDir->NumberOfFunctions; i++)
    {
        /*now to test if */
        if(pNameTable[i] == 0)
        {
            continue;
        }

        /*now it is not has name ,so we should search for it*/
        rva = pNameTable[i];
        ret = __ReadName(hProcess,pModBase,rva,&pNameBuf,0,&namesize);
        if(ret < 0)
        {
            ret = GetLastError() ? GetLastError() : 1;
            goto fail;
        }
        /*now to compare the jobs*/
        if(strcmp((const char*)pNameBuf,pProcName)==0)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        ret = ERROR_INVALID_NAME;
        DEBUG_INFO("\n");
        goto fail;
    }

    /*ok we should set the function pointer*/
    pFnAddr = (unsigned char*)pModBase + pFnTable[findidx];

    __ReallocateSize((PVOID*)&pNameTable,0,&nametablesize);
    __ReallocateSize((PVOID*)&pFnTable,0,&fntablesize);
    __ReallocateSize(&pNameBuf,0,&namesize);
    __ReallocateSize(&pBuffer,0,&bufsize);

    return pFnAddr;
fail:
    __ReallocateSize((PVOID*)&pNameTable,0,&nametablesize);
    __ReallocateSize((PVOID*)&pFnTable,0,&fntablesize);
    __ReallocateSize(&pNameBuf,0,&namesize);
    __ReallocateSize(&pBuffer,0,&bufsize);
    SetLastError(ret);
    return NULL;
}

extern "C" int GetRemoteProcAddress(unsigned int processid,const char* pDllName,const char* pProcName,void** ppFnAddr)
{
    HANDLE hProcess=NULL;
    PVOID pBaseAddr=NULL,pFuncAddr=NULL;
    DWORD exporttablerva = 0;
    int ret;

    pBaseAddr = __GetModuleBaseAddr(processid,pDllName);
    if(pBaseAddr == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    hProcess = OpenProcess(PROCESS_VM_OPERATION |
                           PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE,processid);
    if(hProcess == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }


    exporttablerva = __GetExportTableAddr(hProcess,pBaseAddr);
    if(exporttablerva == 0)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    pFuncAddr = __GetProcAddr(hProcess,pBaseAddr,exporttablerva,pDllName,pProcName);
    if(pFuncAddr == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }
    *ppFnAddr = pFuncAddr;

    assert(hProcess != NULL);
    CloseHandle(hProcess);
    DEBUG_INFO("%s.%s address 0x%p\n",pDllName,pProcName,pFuncAddr);

    return 0;
fail:
    if(hProcess)
    {
        CloseHandle(hProcess);
    }
    hProcess = NULL;
    SetLastError(ret);
    return -ret;
}


int TimeExpire(ULONGLONG ctime,ULONGLONG etime)
{
    if(ctime >= etime)
    {
        return 1;
    }
    return 0;
}

#define  BIT32_MASK  0xffffffff

ULONGLONG GetCurrentTick(ULONGLONG *pCtime)
{
    ULONGLONG ctime = GetTickCount() & BIT32_MASK;
    ULONGLONG lastltime = *pCtime & BIT32_MASK;
    ULONGLONG lasthtime = ((*pCtime) >> 32) & BIT32_MASK;

    /*this means overflow */
    if(ctime < lastltime)
    {
        lasthtime +=1;
    }
    *pCtime = (lasthtime << 32) | ctime;
    return *pCtime;

}

extern "C" int CallRemoteFunc(unsigned int processid,void* pFnAddr,const char* pParam,int timeout,void** ppRetVal)
{
    PVOID pRemoteAddr=NULL;
    char* pNullPtr="\x00";
    int remotesize=0;
    int len;
    HANDLE hProcess = NULL;
    HANDLE hThread=NULL;
    int ret;
    BOOL bret;
    SIZE_T wsize;
    DWORD threadid=0,waitmils,wret,retcode;
    ULONGLONG stime,etime,ctime;

    /*first to allocate the memory for it*/
    hProcess = OpenProcess(PROCESS_VM_OPERATION |
                           PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD ,FALSE,processid);
    if(hProcess == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }
    if(pParam)
    {
        len = strlen(pParam);
    }
    else
    {
        len = 0;
    }
    if(len > 0)
    {
        remotesize = len+1;
        pRemoteAddr = VirtualAllocEx(hProcess,NULL,remotesize,MEM_COMMIT,PAGE_READWRITE);
        if(pRemoteAddr == NULL)
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("\n");
            goto fail;
        }

        bret = WriteProcessMemory(hProcess,pRemoteAddr,pParam,len,&wsize);
        if(!bret)
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("write %s=> 0x%p size %d error (%d)\n",pParam,pRemoteAddr,len,ret);
            goto fail;
        }
        if(wsize != (len))
        {
            ret = ERROR_INVALID_BLOCK;
            DEBUG_INFO("\n");
            goto fail;
        }

        bret = WriteProcessMemory(hProcess,(unsigned char*)pRemoteAddr+len,pNullPtr,1,&wsize);
        if(!bret)
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("\n");
            goto fail;
        }
        if(wsize != 1)
        {
            ret = ERROR_INVALID_BLOCK;
            DEBUG_INFO("\n");
            goto fail;
        }
    }

    hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)pFnAddr,pRemoteAddr,0,&threadid);
    if(hThread == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    /**/
    stime = GetTickCount();
    etime = stime + timeout* 1000;
    ctime = stime;

    while(TimeExpire(ctime, etime)== 0|| timeout == 0)
    {
        waitmils = 2000;
        if(timeout)
        {
            if((etime - ctime) < waitmils)
            {
                waitmils =(DWORD)(etime - ctime);
            }
        }

        wret = WaitForSingleObject(hThread,waitmils);
        if(wret == WAIT_OBJECT_0)
        {
            bret = GetExitCodeThread(hThread,&retcode);
            if(bret)
            {
                break;
            }
            else if(GetLastError() != STILL_ACTIVE)
            {
                ret = GetLastError() ? GetLastError() : 1;
                DEBUG_INFO("\n");
                goto fail;
            }
            /*still alive ,continue*/
        }
        else if(wret == WAIT_TIMEOUT)
        {
            /*wait timeout*/
            ;
        }
        else
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("wait error %d\n",ret);
            goto fail;
        }
        GetCurrentTick(&ctime);
    }

    if(ctime >= etime && timeout > 0)
    {
        ret = WAIT_TIMEOUT;
        DEBUG_INFO("\n");
        goto fail;
    }


    *ppRetVal = (void*)retcode;
    DEBUG_INFO("call 0x%p with param %s retcode(%d)\n",pFnAddr,pParam,retcode);

    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread = NULL;
    if(pRemoteAddr)
    {
        //bret = VirtualFreeEx(hProcess,pRemoteAddr,remotesize,MEM_DECOMMIT);
        bret = VirtualFreeEx(hProcess,pRemoteAddr,0,MEM_RELEASE);
        if(!bret)
        {
            //DEBUG_INFO("could not free 0x%p (%d) (%d)\n",pRemoteAddr,remotesize,GetLastError());
            DEBUG_INFO("could not free 0x%p (%d) (%d)\n",pRemoteAddr,0,GetLastError());
        }
    }
    pRemoteAddr = NULL;
    remotesize = 0;
    if(hProcess)
    {
        CloseHandle(hProcess);
    }
    hProcess = NULL;

    return 0;
fail:
    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread = NULL;
    if(pRemoteAddr)
    {
        //bret = VirtualFreeEx(hProcess,pRemoteAddr,remotesize,MEM_DECOMMIT);
        bret = VirtualFreeEx(hProcess,pRemoteAddr,0,MEM_RELEASE);
        if(!bret)
        {
            //DEBUG_INFO("could not free 0x%p (%d) (%d)\n",pRemoteAddr,remotesize,GetLastError());
            DEBUG_INFO("could not free 0x%p (%d) (%d)\n",pRemoteAddr,0,GetLastError());
        }
    }
    pRemoteAddr = NULL;
    remotesize = 0;
    if(hProcess)
    {
        CloseHandle(hProcess);
    }
    hProcess = NULL;
    SetLastError(ret);
    return -ret;
}


int CallRemoteFuncRemoteParam(unsigned int processid,void* pFnAddr,LPVOID pRemoteAddr,int timeout,void**ppRetVal)
{
    HANDLE hProcess = NULL;
    HANDLE hThread=NULL;
    int ret;
    BOOL bret;
    DWORD threadid=0,waitmils,wret,retcode;
    ULONGLONG stime,etime,ctime;

    /*first to allocate the memory for it*/
    hProcess = OpenProcess(PROCESS_VM_OPERATION |
                           PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD ,FALSE,processid);
    if(hProcess == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        ERROR_INFO("Open Process(%d) Error(%d)\n",processid,ret);
        goto fail;
    }

    hThread = CreateRemoteThread(hProcess,NULL,0,(LPTHREAD_START_ROUTINE)pFnAddr,pRemoteAddr,0,&threadid);
    if(hThread == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("\n");
        goto fail;
    }

    /**/
    stime = GetTickCount();
    etime = stime + timeout* 1000;
    ctime = stime;

    while(TimeExpire(ctime, etime)== 0|| timeout == 0)
    {
        waitmils = 2000;
        if(timeout)
        {
            if((etime - ctime) < waitmils)
            {
                waitmils =(DWORD)(etime - ctime);
            }
        }

        wret = WaitForSingleObject(hThread,waitmils);
        if(wret == WAIT_OBJECT_0)
        {
            bret = GetExitCodeThread(hThread,&retcode);
            if(bret)
            {
                break;
            }
            else if(GetLastError() != STILL_ACTIVE)
            {
                ret = GetLastError() ? GetLastError() : 1;
                DEBUG_INFO("\n");
                goto fail;
            }
            /*still alive ,continue*/
        }
        else if(wret == WAIT_TIMEOUT)
        {
            /*wait timeout*/
            ;
        }
        else
        {
            ret = GetLastError() ? GetLastError() : 1;
            DEBUG_INFO("wait error %d\n",ret);
            goto fail;
        }
        GetCurrentTick(&ctime);
    }

    if(ctime >= etime && timeout > 0)
    {
        ret = WAIT_TIMEOUT;
        DEBUG_INFO("\n");
        goto fail;
    }


    *ppRetVal = (void*)retcode;
    DEBUG_INFO("call 0x%p with param 0x%p retcode(%d)\n",pFnAddr,pRemoteAddr,retcode);

    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread = NULL;
    if(hProcess)
    {
        CloseHandle(hProcess);
    }
    hProcess = NULL;

    return 0;
fail:
    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread = NULL;
    if(hProcess)
    {
        CloseHandle(hProcess);
    }
    hProcess = NULL;
    SetLastError(ret);
    return -ret;
}




