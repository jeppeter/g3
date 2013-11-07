
#include <dllinsert.h>
#include <detours/detours.h>
#include <imgcapcommon.h>
#include <output_debug.h>

#define LAST_ERROR_RETURN()  ((int)(GetLastError() ? GetLastError() : 1))


extern "C" int CaptureFile(DWORD processid,const char* pDllName,const char* pFuncName,const char* bmpfile)
{
    int ret;
    PVOID pFnAddr=NULL;
    PVOID pRetVal;

    ret = GetRemoteProcAddress(processid,pDllName,pFuncName,&pFnAddr);
    if(ret < 0)
    {
        return ret;
    }

    ret = CallRemoteFunc(processid,pFnAddr,bmpfile,0,&pRetVal);
    if(ret < 0)
    {
        return ret;
    }

    ret = (int)pRetVal;
    if(ret != 0)
    {
        ret = ret > 0 ? 0 : ret;
    }

    return ret;
}



/******************************************
D3DHook_HookProcess :
param :
           hProc   HANDLE for process insert
           strDllName  name must be fullpath ,if not ,it assume this is in the current directory

return value:
           0 for success
           otherwise ,negative error code

remarks :
           this will give error
******************************************/
int D3DHook_HookProcess(HANDLE hProc, char * strDllName)
{
    int ret=-1;
    char* pPartDllName=NULL;
    BOOL bret;
    char *pDllFullName=NULL,*pDllStripName=NULL;
    char* dllNames[2];


    pPartDllName = strrchr(strDllName,'\\');
    if(pPartDllName == NULL)
    {
        pPartDllName = strDllName;
    }
    else
    {
        /*skip the name*/
        pPartDllName ++;
    }

    pDllFullName = _strdup(strDllName);
    if(pDllFullName == NULL)
    {
        ret = GetLastError();
        goto out;
    }

    pDllStripName = _strdup(pPartDllName);
    if(pDllStripName == NULL)
    {
        ret = GetLastError();
        goto out;
    }
    dllNames[0] = pDllFullName;
    dllNames[1] = pDllStripName;

    bret = UpdateImports(hProc,(LPCSTR*)dllNames,2);
    if(!bret)
    {
        ret = GetLastError();
        goto out;
    }

    /*all is ok*/
    ret = 0;
out:
    if(pDllFullName)
    {
        free(pDllFullName);
    }
    pDllFullName = NULL;
    if(pDllStripName)
    {
        free(pDllStripName);
    }
    pDllStripName = NULL;
    SetLastError(ret);
    return -ret;
}



/******************************************
D3DHook_CaptureImageBuffer:
            capture image buffer ,we will copy the format

parameter:
		hProc  process handle that the D3DHook_HookProcess
		strDllName  name of dll to insert last time
		data   data to copy
             len   length of the data
             format      format of the data please see capture.h
             width        the width of the picture
             height       height of the picture

return value:
             filled length of the buffer is success
             otherwise the negative error code

remark :
             this will give the timeout to copy buffer
             and the format will give

******************************************/
extern "C" int D3DHook_CaptureImageBuffer(HANDLE hProc,char* strDllName,char * data, int len, int * format, int * width, int * height)
{
    int ret;
    char* pDllStripName=NULL;
    imgcap_buffer_t *pCaptureBuffer=NULL,*pCurCaptureBuffer=NULL;
    unsigned int capturesize=sizeof(*pCaptureBuffer);
    HANDLE hHandleProc=NULL;
    unsigned int processid=0;
    BOOL bret;
    SIZE_T curret;
    void* pFnAddr=NULL;
    int getlen=0;
    HANDLE hThread=NULL;
    DWORD threadid=0;
    DWORD stime,etime,ctime,wtime;
    DWORD dret;
    int timeout=3;
    DWORD retcode=(DWORD)-1;


    DEBUG_INFO("\n");
    pDllStripName = strrchr(strDllName,'\\');
    if(pDllStripName == NULL)
    {
        pDllStripName = strDllName;
    }
    else
    {
        pDllStripName ++;
    }
    DEBUG_INFO("\n");

    processid = GetProcessId(hProc);
    DEBUG_INFO("get hProc 0x%08lx processid (%d)\n",hProc,processid);


	hHandleProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, FALSE, processid);
    if(hHandleProc == NULL)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("OpenProcess (%d) error (%d)\n",processid,ret);
        goto fail;
    }

    DEBUG_INFO("\n");
    pCaptureBuffer = (imgcap_buffer_t*)VirtualAllocEx(hHandleProc,NULL,capturesize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    if(pCaptureBuffer == NULL)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("virtualAllocEx (0x%08x) error(%d)\n",hHandleProc,ret);
        goto fail;
    }
    DEBUG_INFO("\n");

    pCurCaptureBuffer =(imgcap_buffer_t*) calloc(sizeof(*pCurCaptureBuffer),1);
    if(pCurCaptureBuffer == NULL)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("\n");
        goto fail;
    }

    DEBUG_INFO("current 0x%08lx data 0x%p len(0x%08x)\n",GetCurrentProcess(),data,len);
    pCurCaptureBuffer->data = data;
    pCurCaptureBuffer->datalen = len;
    pCurCaptureBuffer->processid = GetCurrentProcessId();

    /*now copy the memory*/
    bret = WriteProcessMemory(hHandleProc,pCaptureBuffer,pCurCaptureBuffer,sizeof(*pCaptureBuffer),&curret);
    if(!bret)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("\n");
        goto fail;
    }

    DEBUG_INFO("\n");
    if(curret != sizeof(*pCaptureBuffer))
    {
        ret = ERROR_INVALID_PARAMETER;
        DEBUG_INFO("\n");
        goto fail;
    }

    DEBUG_INFO("\n");
    /*now to create remote thread*/
    ret = GetRemoteProcAddress(processid,pDllStripName,"CaptureBuffer",&pFnAddr);
    if(ret < 0)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("\n");
        goto fail;
    }
    DEBUG_INFO("CaptureBuffer function address 0x%p\n",pFnAddr);

    hThread=CreateRemoteThread(hHandleProc,NULL,0,(LPTHREAD_START_ROUTINE)pFnAddr,pCaptureBuffer,0,&threadid);
    if(hThread == NULL)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("\n");
        goto fail;
    }
    DEBUG_INFO("\n");

    stime = GetTickCount();
    etime = stime + timeout* 1000;
    ctime = stime;

    while(TimeExpire(ctime,etime)==0)
    {
        wtime = 2000;
        if((etime - ctime) < wtime)
        {
            wtime = (etime - ctime);
        }
        dret = WaitForSingleObject(hThread,wtime);
        if(dret == WAIT_OBJECT_0)
        {
            bret = GetExitCodeThread(hThread,&retcode);
            if(bret)
            {
                break;
            }
            else if(GetLastError() != STILL_ACTIVE)
            {
                ret = LAST_ERROR_RETURN();
                DEBUG_INFO("\n");
                goto fail;
            }
            /*still alive ,continue*/
        }
        else if(dret == WAIT_TIMEOUT)
        {
            ;
        }
        else
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("\n");
            goto fail;
        }
        ctime= GetTickCount();
    }

    if(TimeExpire(ctime, etime))
    {
        ret = WAIT_TIMEOUT;
        DEBUG_INFO("\n");
        goto fail;
    }

    ret =(int) retcode;
    if(ret < 0)
    {
        ret = -ret;
        DEBUG_INFO("\n");
        goto fail;
    }

    /*get the length*/
    getlen = ret;

    /*now to read from the memory as the  result*/
    bret = ReadProcessMemory(hHandleProc,pCaptureBuffer,pCurCaptureBuffer,sizeof(*pCurCaptureBuffer),&curret);
    if(!bret)
    {
        ret = LAST_ERROR_RETURN();
        DEBUG_INFO("\n");
        goto fail;
    }

    if(curret != sizeof(*pCurCaptureBuffer))
    {
        ret = ERROR_INVALID_OPERATION;
        DEBUG_INFO("\n");
        goto fail;
    }


    *format = pCurCaptureBuffer->format;
    *width = pCurCaptureBuffer->width;
    *height = pCurCaptureBuffer->height;


    /*all is ok ,so we should do this*/
    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread=NULL;
    if(pCaptureBuffer)
    {
        bret = VirtualFreeEx(hHandleProc,pCaptureBuffer,capturesize,MEM_DECOMMIT);
        if(!bret)
        {
            ERROR_INFO("could not free %p size %d on %x error (%d)\n",pCaptureBuffer,capturesize,hHandleProc,GetLastError());
        }
    }
    if(pCurCaptureBuffer)
    {
        free(pCurCaptureBuffer);
    }
    pCurCaptureBuffer = NULL;
    if(hHandleProc)
    {
        CloseHandle(hHandleProc);
    }
    hHandleProc = NULL;

    return getlen;

fail:
    if(hThread)
    {
        CloseHandle(hThread);
    }
    hThread=NULL;
    if(pCaptureBuffer)
    {
        bret = VirtualFreeEx(hHandleProc,pCaptureBuffer,capturesize,MEM_DECOMMIT);
        if(!bret)
        {
            ERROR_INFO("could not free %p size %d on %x error (%d)\n",pCaptureBuffer,capturesize,hHandleProc,GetLastError());
        }
    }
    if(pCurCaptureBuffer)
    {
        free(pCurCaptureBuffer);
    }
    pCurCaptureBuffer = NULL;
    if(hHandleProc)
    {
        CloseHandle(hHandleProc);
    }
    hHandleProc = NULL;
    SetLastError(ret);
    return -ret;

}

