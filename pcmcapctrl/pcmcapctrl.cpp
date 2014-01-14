#include "pcmcapctrl.h"
#include <dllinsert.h>
#include <output_debug.h>
#include <timeticks.h>
#include <assert.h>
#include <memshare.h>
#include <evt.h>
#include <sched.h>
#include <injectcommon.h>

#ifdef _DEBUG
#define  PCMCAP_DLL_NAME                 "pcmcapinjectd.dll"
#else
#define  PCMCAP_DLL_NAME                 "pcmcapinject.dll"
#endif
#define  PCMCAP_SET_OPERATION_FUNC_NAME  "HandleAudioOperation"

#define  MAP_FILE_OBJNAME_BASE           "PCMCAP_CAPPER_MAPFILE"
#define  FREE_EVENT_OBJNAME_BASE         "PCMCAP_CAPPER_FREEEVT"
#define  FILL_EVENT_OBJNAME_BASE         "PCMCAP_CAPPER_FILLEVT"
#define  START_EVENT_OBJNAME_BASE        "PCMCAP_CAPPER_START"
#define  STOP_EVENT_OBJNAME_BASE         "PCMCAP_CAPPER_STOP"

#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))

CPcmCapController::CPcmCapController()
{
    m_hProc = NULL;
    m_ProcessId = 0;
    m_iOperation = PCMCAPPER_OPERATION_NONE;
    m_pPcmCapperCb = NULL;
    m_lpParam = NULL;
    m_iBufNum = 0;
    m_iBufBlockSize = 0;

    m_tThreadControl.thread = NULL;
    m_tThreadControl.threadid = 0;
    m_tThreadControl.exitevt = NULL;
    m_tThreadControl.running = 0;
    m_tThreadControl.exited = 1;

    m_hMapFile = NULL;
    m_pMapBuffer = NULL;
    memset(&(m_strMapBaseName),0,sizeof(m_strMapBaseName));

    memset(&(m_strFreeEvtBaseName),0,sizeof(m_strFreeEvtBaseName));
    memset(&(m_strFillEvtBaseName),0,sizeof(m_strFillEvtBaseName));
    memset(&(m_strStartEvtBaseName),0,sizeof(m_strStartEvtBaseName));
    memset(&(m_strStopEvtBaseName),0,sizeof(m_strStopEvtBaseName));
    InitializeCriticalSection(&(m_PcmCS));
    assert(m_PcmIds.size() == 0);
    assert(m_PcmIdx.size() == 0);
    m_CurPcmIds = 0;
    m_hStartEvt = NULL;
    m_hStopEvt = NULL;
    m_pFreeEvt = NULL;
    m_pFillEvt = NULL;
}

BOOL CPcmCapController::__SetOperationInner(pcmcap_control_t * pControl,DWORD *pRetCode)
{
    int ret,res;
    LPVOID pRemoteFunc=NULL,pRemoteAlloc=NULL;
    HANDLE hThread=NULL;
    BOOL bres,bret;
    SIZE_T retsize;
    DWORD threadid=0;
    DWORD dret;
    unsigned int stick,ctick,etick,lefttick;
    int timeout=4;
    DWORD retcode;

    DEBUG_INFO("operation %d\n",pControl->operation);

    pControl->timeout = timeout;
    /*now to call the */
    ret = GetRemoteProcAddress(this->m_ProcessId,PCMCAP_DLL_NAME,PCMCAP_SET_OPERATION_FUNC_NAME,&pRemoteFunc);
    if(ret < 0)
    {
        ret = -ret;
        goto fail;
    }

    /*now to allocate memory and we will put the memory*/
    pRemoteAlloc = VirtualAllocEx(this->m_hProc,NULL,sizeof(*pControl),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    if(pRemoteAlloc == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not allocate 0x%x (%d) size %d error(%d)\n",this->m_hProc,this->m_ProcessId,sizeof(*pControl),ret);
        goto fail;
    }

    DEBUG_INFO("startevt (%s) endevt (%s)\n",pControl->startevt_name, pControl->stopevt_name);
    bret = WriteProcessMemory(this->m_hProc,pRemoteAlloc,pControl,sizeof(*pControl),&retsize);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not write 0x%x (%d) addr 0x%p size %d error(%d)\n",
                   this->m_hProc,this->m_ProcessId,pRemoteAlloc,sizeof(*pControl),ret);
        goto fail;
    }

    if(retsize != sizeof(*pControl))
    {
        ret = ERROR_INSUFFICIENT_BUFFER;
        ERROR_INFO("could not write return %d != %d\n",retsize ,sizeof(*pControl));
        goto fail;
    }

    DEBUG_INFO("\n");
    /*now to call remote address*/
    hThread = CreateRemoteThread(this->m_hProc,NULL,0,(LPTHREAD_START_ROUTINE)pRemoteFunc,pRemoteAlloc,0,&threadid);
    if(hThread == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create (0x%x:%d) process thread error(%d)\n",this->m_hProc,this->m_ProcessId,ret);
        goto fail;
    }

    timeout = 4;
    bret = InitializeTicks(&stick,&ctick,&etick,timeout);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    /*now to do the job handle*/
    while(1)
    {
        lefttick = LeftTicks(&stick,&ctick,&etick,timeout);
        if(lefttick == 0)
        {
            ERROR_INFO("thread 0x%x timeout %d\n",hThread,timeout);
            ret = WAIT_TIMEOUT;
            goto fail;
        }

        DEBUG_INFO("\n");
        dret = WaitForSingleObject(hThread,lefttick);
        DEBUG_INFO("dret %d\n",dret);
        if(dret == WAIT_OBJECT_0)
        {
            bret = GetExitCodeThread(hThread,&retcode);
            if(bret)
            {
                break;
            }
        }
        else if(dret == WAIT_FAILED)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }

        GetCurrentTicks(&ctick);
    }

    *pRetCode = retcode;

    assert(hThread);
    CloseHandle(hThread);
    hThread = NULL;

    assert(pRemoteAlloc);
    //bres = VirtualFreeEx(this->m_hProc,pRemoteAlloc,sizeof(*pControl),MEM_DECOMMIT);
    bres = VirtualFreeEx(this->m_hProc,pRemoteAlloc,0,MEM_RELEASE);
    if(!bres)
    {
        res = LAST_ERROR_CODE();
        //ERROR_INFO("could not free (0x%x) process %d handle remoteaddr 0x%p size %d error(%d)\n",
        //           this->m_hProc,this->m_ProcessId,pRemoteAlloc,sizeof(*pControl),ret);
        ERROR_INFO("could not free (0x%x) process %d handle remoteaddr 0x%p size %d error(%d)\n",
                   this->m_hProc,this->m_ProcessId,pRemoteAlloc,0,ret);
    }

    pRemoteAlloc = NULL;


    return TRUE;
fail:
    if(hThread)
    {
        /*now to wait for a while and at last to kill it*/
        timeout = 2;
        bres = InitializeTicks(&stick,&ctick,&etick,timeout);
        if(bres)
        {
            while(1)
            {
                lefttick = LeftTicks(&stick,&ctick,&etick,timeout);
                if(lefttick == 0)
                {
                    ERROR_INFO("could not wait for thread handle 0x%x timeout\n",hThread);
                    break;
                }
                dret = WaitForSingleObject(hThread,lefttick);
                if(dret == WAIT_OBJECT_0)
                {
                    bres = GetExitCodeThread(hThread,&retcode);
                    if(bres)
                    {
                        break;
                    }
                }
                else if(dret == WAIT_FAILED)
                {
                    break;
                }

                GetCurrentTicks(&ctick);
            }
        }
        else
        {
            ERROR_INFO("could not get ticks\n");
        }
        CloseHandle(hThread);
    }
    hThread = NULL;
    if(pRemoteAlloc)
    {
        //bres = VirtualFreeEx(this->m_hProc,pRemoteAlloc,sizeof(*pControl),MEM_DECOMMIT);
        bres = VirtualFreeEx(this->m_hProc,pRemoteAlloc,0,MEM_RELEASE);
        if(!bres)
        {
            res = LAST_ERROR_CODE();
            //ERROR_INFO("could not free (0x%x) handle remoteaddr 0x%p size %d error(%d)\n",
            //           this->m_hProc,pRemoteAlloc,sizeof(*pControl),ret);
            ERROR_INFO("could not free (0x%x) handle remoteaddr 0x%p size %d error(%d)\n",
                       this->m_hProc,pRemoteAlloc,0,ret);
        }
    }
    pRemoteAlloc = NULL;
    pRemoteFunc = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CPcmCapController::__SetOperationNone()
{
    pcmcap_control_t* pControl=NULL;
    BOOL bret;
    DWORD retcode=0;

    if(this->m_hProc == NULL)
    {
        /*it is not set*/
        return TRUE;
    }

    pControl = (pcmcap_control_t*)malloc(sizeof(*pControl));
    if(pControl == NULL)
    {
        return FALSE;
    }

    memset(pControl,0,sizeof(*pControl));
    pControl->operation = PCMCAPPER_OPERATION_NONE;

    bret = this->__SetOperationInner(pControl,&retcode);


    free(pControl);
    pControl = NULL;

    if(!bret)
    {
        return FALSE;
    }
    if(retcode != 0)
    {
        SetLastError(ERROR_FATAL_APP_EXIT);
        return FALSE;
    }



    return TRUE;

}


BOOL CPcmCapController::Stop()
{
    return this->__StopOperation(PCMCAPPER_OPERATION_NONE);
}

#define PCM_IDS_EQUAL()  \
do\
{\
	assert(this->m_PcmIds.size() == this->m_PcmIdx.size());\
}while(0)

void CPcmCapController::__DestroyPcmIds()
{
    EnterCriticalSection(&(this->m_PcmCS));
    PCM_IDS_EQUAL();
    this->m_CurPcmIds = 0;
    this->m_PcmIds.clear();
    this->m_PcmIdx.clear();
    LeaveCriticalSection(&(this->m_PcmCS));
    return ;
}

BOOL CPcmCapController::__StopOperation(int iOperation)
{
    int lasterr = 0;
    BOOL bret,totalbret=TRUE;

    if(this->m_hProc)
    {
        /*now to set the */
        assert(iOperation == PCMCAPPER_OPERATION_NONE || iOperation == PCMCAPPER_OPERATION_RENDER);
        switch(iOperation)
        {
        case PCMCAPPER_OPERATION_NONE:
            bret = this->__SetOperationNone();
            break;
        case PCMCAPPER_OPERATION_RENDER:
            bret = this->__SetOperationRender();
            break;

        }
        if(!bret)
        {
            totalbret = FALSE;
            lasterr = lasterr ? lasterr : (LAST_ERROR_CODE());
        }
    }
    this->m_iOperation = iOperation;

    this->__StopThread();
    this->__DestroyEvent();

    this->__DestroyPcmIds();
    this->__DestroyMap();
    SetLastError(lasterr);
    return totalbret;

}

CPcmCapController::~CPcmCapController()
{
    this->Stop();
    this->m_hProc = NULL;
    this->m_iBufNum = 0;
    this->m_iBufBlockSize = 0;
    this->m_pPcmCapperCb = NULL;
    this->m_lpParam = NULL;
    DeleteCriticalSection(&(this->m_PcmCS));
}

void CPcmCapController::__DestroyMap()
{
    UnMapFileBuffer(&(this->m_pMapBuffer));
    CloseMapFileHandle(&(this->m_hMapFile));
    memset(&(this->m_strMapBaseName),0,sizeof(this->m_strMapBaseName));
    return ;
}

int CPcmCapController::__CreateMap()
{
    int ret;
    unsigned int mapsize;
    _snprintf_s((char*)this->m_strMapBaseName,sizeof(this->m_strMapBaseName),_TRUNCATE,"%s%d",MAP_FILE_OBJNAME_BASE,this->m_ProcessId);
    mapsize = this->m_iBufBlockSize * this->m_iBufNum;
    this->m_hMapFile = CreateMapFile((char*)this->m_strMapBaseName,mapsize ,1);
    if(this->m_hMapFile == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create (%s) (%d:0x%x) error (%d)\n",this->m_strMapBaseName,mapsize,mapsize,ret);
        this->__DestroyMap();
        return -ret;
    }

    this->m_pMapBuffer = MapFileBuffer(this->m_hMapFile,mapsize);
    if(this->m_pMapBuffer == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not mapfilebuffer %s (%d:0x%x) error(%d)\n",this->m_strMapBaseName,mapsize,mapsize,ret);
        this->__DestroyMap();
        return -ret;
    }

    return 0;
}


void CPcmCapController::__DestroyEvent()
{
    unsigned int i;
    for(i=0; i<this->m_iBufNum; i++)
    {
        /*now to close handle*/
        if(this->m_pFillEvt)
        {
            if(this->m_pFillEvt[i])
            {
                CloseHandle(this->m_pFillEvt[i]);
            }
            this->m_pFillEvt[i] = NULL;
        }

        if(this->m_pFreeEvt)
        {
            if(this->m_pFreeEvt[i])
            {
                CloseHandle(this->m_pFreeEvt[i]);
            }
            this->m_pFreeEvt[i] = NULL;
        }
    }

    memset(&(this->m_strFillEvtBaseName),0,sizeof(this->m_strFillEvtBaseName));
    memset(&(this->m_strFreeEvtBaseName),0,sizeof(this->m_strFreeEvtBaseName));
    memset(&(this->m_strStartEvtBaseName),0,sizeof(this->m_strStartEvtBaseName));
    memset(&(this->m_strStopEvtBaseName),0,sizeof(this->m_strStopEvtBaseName));

    if(this->m_hStartEvt)
    {
        CloseHandle(this->m_hStartEvt);
    }
    this->m_hStartEvt = NULL;

    if(this->m_hStopEvt)
    {
        CloseHandle(this->m_hStopEvt);
    }
    this->m_hStopEvt = NULL;

    if(this->m_pFillEvt)
    {
        free(this->m_pFillEvt);
    }
    this->m_pFillEvt = NULL;

    if(this->m_pFreeEvt)
    {
        free(this->m_pFreeEvt);
    }
    this->m_pFreeEvt = NULL;
    return ;
}

int CPcmCapController::__CreateEvent()
{
    int ret;
    unsigned int i;
    unsigned char evname[128];
    _snprintf_s((char*)this->m_strFreeEvtBaseName,sizeof(this->m_strFreeEvtBaseName),_TRUNCATE,"%s%d",FREE_EVENT_OBJNAME_BASE,this->m_ProcessId);
    _snprintf_s((char*)this->m_strFillEvtBaseName,sizeof(this->m_strFillEvtBaseName),_TRUNCATE,"%s%d",FILL_EVENT_OBJNAME_BASE,this->m_ProcessId);
    _snprintf_s((char*)this->m_strStartEvtBaseName,sizeof(this->m_strStartEvtBaseName),_TRUNCATE,"%s_%d",START_EVENT_OBJNAME_BASE,this->m_ProcessId);
    _snprintf_s((char*)this->m_strStopEvtBaseName,sizeof(this->m_strStopEvtBaseName),_TRUNCATE,"%s_%d",STOP_EVENT_OBJNAME_BASE,this->m_ProcessId);


    assert(this->m_pFreeEvt == NULL);
    assert(this->m_pFillEvt == NULL);
    assert(this->m_hStartEvt == NULL);
    assert(this->m_hStopEvt == NULL);
    assert(this->m_iBufNum > 0);
    assert(this->m_iBufBlockSize >= 0x1000);

    this->m_pFreeEvt =(HANDLE*) calloc(this->m_iBufNum,sizeof(this->m_pFreeEvt[0]));
    if(this->m_pFreeEvt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not allocate size %d error (%d)\n",sizeof(this->m_pFreeEvt[0])*this->m_iBufNum,ret);
        this->__DestroyEvent();
        SetLastError(ret);
        return -ret;
    }

    for(i=0; i<this->m_iBufNum; i++)
    {
        _snprintf_s((char*)evname,sizeof(evname),_TRUNCATE,"%s_%d",this->m_strFreeEvtBaseName,i);
        this->m_pFreeEvt[i] = GetEvent((const char*)evname,1);
        if(this->m_pFreeEvt[i] == NULL)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could not create %s event error(%d)\n",evname,ret);
            this->__DestroyEvent();
            SetLastError(ret);
            return -ret;
        }
    }

    this->m_pFillEvt =(HANDLE*) calloc(this->m_iBufNum,sizeof(this->m_pFillEvt[0]));
    if(this->m_pFillEvt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not allocate size %d error (%d)\n",sizeof(this->m_pFillEvt[0])*this->m_iBufNum,ret);
        this->__DestroyEvent();
        SetLastError(ret);
        return -ret;
    }

    for(i=0; i<this->m_iBufNum; i++)
    {
        _snprintf_s((char*)evname,sizeof(evname),_TRUNCATE,"%s_%d",this->m_strFillEvtBaseName,i);
        this->m_pFillEvt[i] = GetEvent((const char*)evname,1);
        if(this->m_pFillEvt[i] == NULL)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("could not create %s event error(%d)\n",evname,ret);
            this->__DestroyEvent();
            SetLastError(ret);
            return -ret;
        }
    }

    this->m_hStartEvt = GetEvent((const char*)this->m_strStartEvtBaseName,1);
    if(this->m_hStartEvt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create startevent (%s) error(%d)\n",this->m_strStartEvtBaseName,ret);
        this->__DestroyEvent();
        SetLastError(ret);
        return -ret;
    }

    this->m_hStopEvt = GetEvent((const char*)this->m_strStopEvtBaseName,1);
    if(this->m_hStopEvt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create stopevent (%s) error(%d)\n",this->m_strStopEvtBaseName,ret);
        this->__DestroyEvent();
        SetLastError(ret);
        return -ret;
    }

    return 0;
}

int CPcmCapController::__FreeAllEvent()
{
    unsigned int i;
    int ret;
    BOOL bret;

    assert(this->m_pFreeEvt);
    assert(this->m_iBufNum > 0);

    for(i=0; i<this->m_iBufNum; i++)
    {
        assert(this->m_pFreeEvt[i]);
        bret = SetEvent(this->m_pFreeEvt[i]);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("[%d] setevent error(%d)\n",i,ret);
            goto fail;
        }
    }

    SetLastError(0);
    return 0;

fail:
    SetLastError(ret);
    return -ret;
}

BOOL CPcmCapController::Start(HANDLE hProc,int iOperation,int iBufNum,int iBlockSize,IPcmCapControllerCallback * pPcc,LPVOID lpParam)
{

    if(iOperation != PCMCAP_AUDIO_BOTH &&
            iOperation != PCMCAP_AUDIO_NONE &&
            iOperation != PCMCAP_AUDIO_CAPTURE &&
            iOperation != PCMCAP_AUDIO_RENDER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(iBufNum < 1 || iBlockSize < 0x1000 || hProc == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    this->__StopOperation(PCMCAPPER_OPERATION_NONE);

    this->m_hProc = hProc;
    this->m_iBufNum = iBufNum;
    this->m_iBufBlockSize = iBlockSize;
    this->m_ProcessId = GetProcessId(hProc);
    this->m_pPcmCapperCb = pPcc;
    this->m_lpParam = lpParam;

    return this->SetAudioOperation(iOperation);
}

void CPcmCapController::__StopThread()
{
    BOOL bret;
    int ret;
    if(this->m_tThreadControl.thread)
    {
        assert(this->m_tThreadControl.exitevt);
        this->m_tThreadControl.running = 0;
        while(this->m_tThreadControl.exited == 0)
        {
            bret = SetEvent(this->m_tThreadControl.exitevt);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("could not set exitevt error(%d)\n",ret);
            }
            SchedOut();
        }

        /*now we should thread control*/
        CloseHandle(this->m_tThreadControl.thread);
    }

    this->m_tThreadControl.thread = NULL;
    this->m_tThreadControl.exited = 1;
    if(this->m_tThreadControl.exitevt)
    {
        CloseHandle(this->m_tThreadControl.exitevt);
    }
    this->m_tThreadControl.exitevt = NULL;
    this->m_tThreadControl.running = 0;
    this->m_tThreadControl.threadid = 0;
    return ;
}

int CPcmCapController::__StartThread()
{
    int ret;

    assert(this->m_tThreadControl.thread == NULL);
    assert(this->m_tThreadControl.exitevt == NULL);
    assert(this->m_tThreadControl.exited == 1);
    assert(this->m_tThreadControl.running == 0);
    assert(this->m_tThreadControl.threadid == 0);

    this->m_tThreadControl.exitevt = GetEvent(NULL,1);
    if(this->m_tThreadControl.exitevt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create exit event error(%d)\n",ret);
        goto fail;
    }

    /*now for the thread running state*/
    this->m_tThreadControl.exited = 0;
    this->m_tThreadControl.running = 1;

    this->m_tThreadControl.thread = CreateThread(NULL,0,CPcmCapController::ThreadFunc,this,0,&(this->m_tThreadControl.threadid));
    if(this->m_tThreadControl.thread == NULL)
    {
        this->m_tThreadControl.thread = NULL;
        this->m_tThreadControl.exited = 1;
        this->m_tThreadControl.running = 0;
        this->m_tThreadControl.threadid = 0;
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create thread func error(%d)\n",ret);
        goto fail;
    }

    /*ok we should start thread*/

    return 0;
fail:
    this->__StopThread();
    return -ret;
}

DWORD WINAPI CPcmCapController::ThreadFunc(void * arg)
{
    CPcmCapController* pThis = (CPcmCapController*)arg;
    return pThis->__ThreadImpl();
}

DWORD CPcmCapController::__ThreadImpl()
{
    int ret;
    DWORD dret;
    HANDLE *pWaitHandle=NULL;
    unsigned int i;
    unsigned int bufnum;

    /*to include the exit num*/
    assert(this->m_iBufNum > 0);
    assert(this->m_tThreadControl.exitevt);
    bufnum = this->m_iBufNum;
    pWaitHandle = (HANDLE*)calloc(bufnum+3,sizeof(*pWaitHandle));
    if(pWaitHandle == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not allocate %d handles error(%d)\n",(bufnum+3)*sizeof(*pWaitHandle),ret);
        goto out;
    }

    for(i=0; i<bufnum; i++)
    {
        assert(this->m_pFillEvt);
        assert(this->m_pFreeEvt);
        assert(this->m_pFillEvt[i]);
        assert(this->m_pFreeEvt[i]);
        pWaitHandle[i] = this->m_pFillEvt[i];
    }

    pWaitHandle[bufnum] = this->m_tThreadControl.exitevt;
    pWaitHandle[bufnum+1] = this->m_hStartEvt;
    pWaitHandle[bufnum+2] = this->m_hStopEvt;


    while(this->m_tThreadControl.running)
    {
        dret = WaitForMultipleObjects(bufnum + 3,pWaitHandle,FALSE,INFINITE);
        if(dret <= (WAIT_OBJECT_0+bufnum-1) && dret >= WAIT_OBJECT_0)
        {
            this->__AudioRenderBuffer(dret - WAIT_OBJECT_0);
        }
        else if(dret == (WAIT_OBJECT_0 + bufnum))
        {
            /*nothing to handle*/
            ;
        }
        else if(dret == (WAIT_OBJECT_0 + bufnum + 1))
        {
            /*this is start calling*/
            this->__AudioStartCall();
        }
        else if(dret == (WAIT_OBJECT_0+bufnum + 2))
        {
            /*this is stop calling*/
            this->__AudioStopCall();
        }
        else if(dret == WAIT_FAILED)
        {
            ret = -LAST_ERROR_CODE();
            ERROR_INFO("wait for num %d error(%d)\n",bufnum,ret);
            goto out;
        }
    }

    ret = 0;
out:
    if(pWaitHandle)
    {
        free(pWaitHandle);
    }
    pWaitHandle = NULL;
    this->m_tThreadControl.exited = 1;
    return ret;
}


void CPcmCapController::__AudioRenderBuffer(int idx)
{
    pcmcap_buffer_t* pItem;
    BOOL bret;
    int ret;
    int cont = 0;
    int curidx;
    int findidx = -1;
    UINT i;
    assert(this->m_pMapBuffer);
    assert(idx >= 0 && idx < (int)this->m_iBufNum);

    pItem = (pcmcap_buffer_t*)((ptr_t)this->m_pMapBuffer + (idx)*this->m_iBufBlockSize);

    cont = 0;
    EnterCriticalSection(&(this->m_PcmCS));
    if((this->m_CurPcmIds +1) == pItem->pcmid)
    {
        cont = 1;
        this->m_CurPcmIds ++;
        curidx = idx;
    }
    else
    {
        /*now to find the pcmid*/
        PCM_IDS_EQUAL();
        findidx = -1;
        for(i=0; i<this->m_PcmIds.size(); i++)
        {
            if(this->m_PcmIds[i] > pItem->pcmid)
            {
                findidx = i;
                break;
            }
        }
        if(findidx >= 0)
        {
            this->m_PcmIds.insert(this->m_PcmIds.begin() + findidx,pItem->pcmid);
            this->m_PcmIdx.insert(this->m_PcmIdx.begin() + findidx,idx);
        }
        else
        {
            this->m_PcmIds.push_back(pItem->pcmid);
            this->m_PcmIdx.push_back(idx);
        }
    }
    LeaveCriticalSection(&(this->m_PcmCS));

    if(cont)
    {

        if(this->m_pPcmCapperCb)
        {
            this->m_pPcmCapperCb->WaveInCb(pItem,this->m_lpParam);
        }
        assert(this->m_pFreeEvt);
        assert(this->m_pFreeEvt[curidx]);
        bret = SetEvent(this->m_pFreeEvt[curidx]);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("setevent %d error(%d)\n",
                       curidx,ret);

        }

        do
        {
            cont = 0;
            EnterCriticalSection(&(this->m_PcmCS));
            PCM_IDS_EQUAL();
            if(this->m_PcmIds.size() > 0)
            {
                if((this->m_CurPcmIds + 1) == this->m_PcmIds[0])
                {
                    curidx = this->m_PcmIdx[0];
                    pItem = (pcmcap_buffer_t*)((ptr_t)this->m_pMapBuffer + (curidx)*this->m_iBufBlockSize);
                    this->m_CurPcmIds ++;
                    this->m_PcmIds.erase(this->m_PcmIds.begin());
                    this->m_PcmIdx.erase(this->m_PcmIdx.begin());
                    cont = 1;
                }
            }
            LeaveCriticalSection(&(this->m_PcmCS));

            if(cont)
            {
                if(this->m_pPcmCapperCb)
                {
                    this->m_pPcmCapperCb->WaveInCb(pItem,this->m_lpParam);
                }
                assert(this->m_pFreeEvt);
                assert(this->m_pFreeEvt[curidx]);
                bret = SetEvent(this->m_pFreeEvt[curidx]);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("setevent %d error(%d)\n",
                               curidx,ret);

                }
            }
        }
        while(cont);
    }

    /*now to set the event*/

    return ;

}

BOOL CPcmCapController::__SetOperationBoth()
{
    pcmcap_control_t* pControl=NULL;
    int ret;
    BOOL bret;
    DWORD retcode;
    assert(this->m_hProc);
    assert(this->m_ProcessId);
    pControl = (pcmcap_control_t*)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    pControl->operation = PCMCAPPER_OPERATION_BOTH;
    strncpy_s((char*)pControl->mem_sharename,sizeof(pControl->mem_sharename),(const char*)this->m_strMapBaseName,_TRUNCATE);
    pControl->mem_sharesize = this->m_iBufNum * this->m_iBufBlockSize;
    pControl->packsize = this->m_iBufBlockSize;
    pControl->packnum = this->m_iBufNum;
    strncpy_s((char*)pControl->freelist_semnamebase, sizeof(pControl->freelist_semnamebase), (const char*)this->m_strFreeEvtBaseName, _TRUNCATE);
    strncpy_s((char*)pControl->filllist_semnamebase, sizeof(pControl->filllist_semnamebase), (const char*)this->m_strFillEvtBaseName, _TRUNCATE);
    strncpy_s((char*)pControl->startevt_name, sizeof(pControl->startevt_name), (const char*)this->m_strStartEvtBaseName, _TRUNCATE);
    DEBUG_INFO("control start %s start evtname %s\n", pControl->startevt_name, this->m_strStartEvtBaseName);
    strncpy_s((char*)pControl->stopevt_name, sizeof(pControl->stopevt_name), (const char*)this->m_strStopEvtBaseName, _TRUNCATE);

    bret = this->__SetOperationInner(pControl,&retcode);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not set operation BOTH error (%d)\n",ret);
        goto fail;
    }
    if(retcode != 0)
    {
        ret = ERROR_FATAL_APP_EXIT;
        ERROR_INFO("set both error code %d\n",retcode);
        goto fail;
    }

    free(pControl);
    pControl = NULL;

    SetLastError(0);
    return TRUE;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CPcmCapController::__SetOperationCapture()
{
    pcmcap_control_t* pControl=NULL;
    int ret;
    BOOL bret;
    DWORD retcode;
    /**/
    pControl = (pcmcap_control_t*)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    pControl->operation = PCMCAPPER_OPERATION_CAPTURE;
    strncpy_s((char*)pControl->mem_sharename,sizeof(pControl->mem_sharename),(const char*)this->m_strMapBaseName,_TRUNCATE);
    pControl->mem_sharesize = this->m_iBufNum * this->m_iBufBlockSize;
    pControl->packsize = this->m_iBufBlockSize;
    pControl->packnum = this->m_iBufNum;
    strncpy_s((char*)pControl->freelist_semnamebase, sizeof(pControl->freelist_semnamebase), (const char*)this->m_strFreeEvtBaseName, _TRUNCATE);
    strncpy_s((char*)pControl->filllist_semnamebase, sizeof(pControl->filllist_semnamebase), (const char*)this->m_strFillEvtBaseName, _TRUNCATE);
    strncpy_s((char*)pControl->startevt_name, sizeof(pControl->startevt_name), (const char*)this->m_strStartEvtBaseName, _TRUNCATE);
    DEBUG_INFO("control start %s start evtname %s\n", pControl->startevt_name, this->m_strStartEvtBaseName);
    strncpy_s((char*)pControl->stopevt_name, sizeof(pControl->stopevt_name), (const char*)this->m_strStopEvtBaseName, _TRUNCATE);

    bret = this->__SetOperationInner(pControl,&retcode);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not set operation BOTH error (%d)\n",ret);
        goto fail;
    }
    if(retcode != 0)
    {
        ret = ERROR_FATAL_APP_EXIT;
        ERROR_INFO("set both error code %d\n",retcode);
        goto fail;
    }

    free(pControl);
    pControl = NULL;

    SetLastError(0);
    return TRUE;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CPcmCapController::__SetOperationRender()
{
    pcmcap_control_t* pControl=NULL;
    int ret;
    BOOL bret;
    DWORD retcode;

    if(this->m_hProc == NULL)
    {
        return TRUE;
    }
    /**/
    pControl = (pcmcap_control_t*)calloc(1,sizeof(*pControl));
    if(pControl == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    pControl->operation = PCMCAPPER_OPERATION_RENDER;

    bret = this->__SetOperationInner(pControl,&retcode);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not set operation BOTH error (%d)\n",ret);
        goto fail;
    }
    if(retcode != 0)
    {
        ret = ERROR_FATAL_APP_EXIT;
        ERROR_INFO("set both error code %d\n",retcode);
        goto fail;
    }

    free(pControl);
    pControl = NULL;

    SetLastError(0);
    return TRUE;
fail:
    if(pControl)
    {
        free(pControl);
    }
    pControl = NULL;
    SetLastError(ret);
    return FALSE;
}

BOOL CPcmCapController::SetAudioOperation(int iOperation)
{
    BOOL bret=TRUE;

    switch(iOperation)
    {
    case PCMCAPPER_OPERATION_NONE:
    case PCMCAPPER_OPERATION_RENDER:
        this->__StopOperation(iOperation);
        break;

    case PCMCAPPER_OPERATION_BOTH:
        this->__StopOperation(PCMCAPPER_OPERATION_NONE);
        if(this->m_hProc == NULL || this->m_ProcessId == 0 || this->m_iBufBlockSize < 0x1000 ||
                this->m_iBufNum < 1)
        {
            bret = FALSE;
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        bret = this->__StartOperation(PCMCAPPER_OPERATION_BOTH);
        break;
    case PCMCAPPER_OPERATION_CAPTURE:
        this->__StopOperation(PCMCAPPER_OPERATION_NONE);
        if(this->m_hProc == NULL || this->m_ProcessId == 0 || this->m_iBufBlockSize < 0x1000 ||
                this->m_iBufNum < 1)
        {
            bret = FALSE;
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }
        bret = this->__StartOperation(PCMCAPPER_OPERATION_CAPTURE);
        break;
    default:
        bret = FALSE;
        SetLastError(ERROR_NOT_SUPPORTED);
        break;
    }

    if(!bret)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL CPcmCapController::__StartOperation(int iOperation)
{
    int ret;
    BOOL bret;
    /*now we should allocate the buffer and the event*/
    ret = this->__CreateMap();
    if(ret < 0)
    {
        this->Stop();
        SetLastError(-ret);
        return FALSE;
    }

    ret = this->__CreateEvent();
    if(ret < 0)
    {
        this->Stop();
        SetLastError(-ret);
        return FALSE;
    }


    ret = this->__FreeAllEvent();
    if(ret < 0)
    {
        this->Stop();
        SetLastError(-ret);
        return FALSE;
    }

    ret = this->__StartThread();
    if(ret < 0)
    {
        this->Stop();
        SetLastError(-ret);
        return FALSE;
    }

    switch(iOperation)
    {
    case PCMCAPPER_OPERATION_BOTH:
        bret= this->__SetOperationBoth();
        break;
    case PCMCAPPER_OPERATION_CAPTURE:
        bret = this->__SetOperationCapture();
        break;
    }

    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        this->Stop();
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}

void CPcmCapController::__AudioStartCall()
{
    if(this->m_pPcmCapperCb)
    {
        this->m_pPcmCapperCb->WaveOpenCb(this->m_lpParam);
    }
    return ;
}

void CPcmCapController::__AudioStopCall()
{
    if(this->m_pPcmCapperCb)
    {
        this->m_pPcmCapperCb->WaveCloseCb(this->m_lpParam);
    }
    return ;
}

