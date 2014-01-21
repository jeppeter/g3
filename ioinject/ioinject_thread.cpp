
#include "ioinject_thread.h"
#include <injectbase.h>
#include <assert.h>

typedef struct
{
    thread_control_t m_ThreadControl;
    uint32_t m_Bufnumm;
    uint32_t m_BufSectSize;
    uint8_t m_MemShareBaseName[IO_NAME_MAX_SIZE];
    HANDLE m_hMapFile;
    ptr_t m_pMemShareBase;
    uint8_t m_FreeEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pFreeEvts;
    uint8_t m_InputEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pInputEvts;
    EVENT_LIST_t* m_pEventListArray;
    unsigned long long m_CurSeqId;
    std::vector<DWORD>* m_pBufIdxVecs;
    std::vector<unsigned long long>* m_pSeqIdVecs;
} DETOUR_THREAD_STATUS_t,*PDETOUR_THREAD_STATUS_t;

CFuncList st_EventHandlerFuncList;
CFuncList st_EventInitFuncList;
static DETOUR_THREAD_STATUS_t *st_pDetourStatus=NULL;
static HANDLE st_hIoInjectControlSema=NULL;

static int st_UnPressedLastKey=-1;


int __HandleStatusEventReal(PDETOUR_THREAD_STATUS_t pStatus,DWORD idx)
{
    int totalret=0;
    LPDEVICEEVENT pDevEvent=NULL;
    LPSEQ_DEVICEEVENT pSeqEvent=NULL;
    int ret;
    /*now we should handle this function*/
    EVENT_LIST_t* pEventList=NULL;
    BOOL bret;
    pEventList = &(pStatus->m_pEventListArray[idx]);
    //DEBUG_INFO("[%d]Base 0x%x offset 0x%x\n",idx,pEventList->m_BaseAddr,pEventList->m_Offset);
    pSeqEvent= (LPSEQ_DEVICEEVENT)((ptr_t)pEventList->m_BaseAddr + (ptr_t)pEventList->m_Offset);
    pDevEvent = &(pSeqEvent->devevent);

    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD && pDevEvent->devid == 0)
    {
        DEBUG_INFO("Keyboard Input Events[%d]\n",idx);
        if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN)
        {
            st_UnPressedLastKey = -1;
        }
        else if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_UP)
        {
            if(st_UnPressedLastKey == pDevEvent->event.keyboard.code)
            {
                DEBUG_INFO("[%d]<0x%p>UnPressed key double(0x%08x:%d)\n",idx,pDevEvent,st_UnPressedLastKey,st_UnPressedLastKey);
                bret = SetEvent(pEventList->m_hFillEvt);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("<0x%p>SetEvent(0x%08x) Error(%d)\n",pEventList,pEventList->m_hFillEvt,ret);
                }
                return 0;
            }
            st_UnPressedLastKey = pDevEvent->event.keyboard.code;
        }
    }

    totalret = st_EventHandlerFuncList.CallList(pDevEvent);

    /*now we should get the point */

    bret = SetEvent(pEventList->m_hFillEvt);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%p>SetEvent(0x%08x) Error(%d)\n",pEventList,pEventList->m_hFillEvt,ret);
    }
    return totalret;
}

int __HandleStatusEvent(PDETOUR_THREAD_STATUS_t pStatus,DWORD idx)
{
    int ret;
    LPSEQ_DEVICEEVENT pSeqEvent=NULL;
    int findidx=-1;
    UINT i;
    int cnt = 0;
    DWORD curidx=idx;
    BOOL bret;
    EVENT_LIST_t* pEventList=NULL;
    assert(idx < pStatus->m_Bufnumm);
    pEventList = &(pStatus->m_pEventListArray[idx]);
    //DEBUG_INFO("[%d]Base 0x%x offset 0x%x\n",idx,pEventList->m_BaseAddr,pEventList->m_Offset);
    pSeqEvent= (LPSEQ_DEVICEEVENT)((ptr_t)pEventList->m_BaseAddr + (ptr_t)pEventList->m_Offset);

    if(pSeqEvent->seqid == (pStatus->m_CurSeqId + 1))
    {
        ret = __HandleStatusEventReal(pStatus,curidx);
        if(ret < 0)
        {
            ERROR_INFO("Handle[%d] Error(%d)\n",curidx,ret);
        }
        pStatus->m_CurSeqId += 1;
        cnt ++;

        while(1)
        {
            assert(pStatus->m_pSeqIdVecs->size() == pStatus->m_pBufIdxVecs->size());
            if(pStatus->m_pSeqIdVecs->size() == 0)
            {
                break;
            }

            if(pStatus->m_pSeqIdVecs->at(0) != (pStatus->m_CurSeqId + 1))
            {
                break;
            }

            curidx = pStatus->m_pBufIdxVecs->at(0);
            ret = __HandleStatusEventReal(pStatus,curidx);
            if(ret < 0)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("Handle[%d] Error(%d)\n",curidx,ret);
            }

            pStatus->m_pBufIdxVecs->erase(pStatus->m_pBufIdxVecs->begin());
            pStatus->m_pSeqIdVecs->erase(pStatus->m_pSeqIdVecs->begin());
            pStatus->m_CurSeqId += 1;
            cnt ++;
        }
    }
    else if((pStatus->m_CurSeqId) < (pSeqEvent->seqid))
    {

        if(pStatus->m_pSeqIdVecs->size() >= pStatus->m_Bufnumm)
        {
            ERROR_INFO("wait seqids %d\n",pStatus->m_pSeqIdVecs->size());
        }
        /*now to push find the index*/
        for(i=0; i<pStatus->m_pSeqIdVecs->size(); i++)
        {
            if(pStatus->m_pSeqIdVecs->at(i) > pSeqEvent->seqid)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pStatus->m_pSeqIdVecs->insert(pStatus->m_pSeqIdVecs->begin() + findidx,pSeqEvent->seqid);
            pStatus->m_pBufIdxVecs->insert(pStatus->m_pBufIdxVecs->begin() + findidx,idx);
        }
        else
        {
            pStatus->m_pSeqIdVecs->push_back(pSeqEvent->seqid);
            pStatus->m_pBufIdxVecs->push_back(idx);
        }
        assert(pStatus->m_pSeqIdVecs->size() == pStatus->m_pBufIdxVecs->size());
    }
    else
    {
        ERROR_INFO("[%d] seqid %lld but curseqid (%lld)\n",idx,pSeqEvent->seqid,pStatus->m_CurSeqId);
        cnt ++;
        bret = SetEvent(pEventList->m_hFillEvt);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("<0x%p>SetEvent(0x%08x) Error(%d)\n",pEventList,pEventList->m_hFillEvt,ret);
        }
    }

    return cnt;
}



DWORD WINAPI DetourIoInjectThreadThreadImpl(LPVOID pParam)
{
    PDETOUR_THREAD_STATUS_t pStatus = (PDETOUR_THREAD_STATUS_t)pParam;
    HANDLE *pWaitHandles=NULL;
    unsigned int waitnum=0;
    DWORD dret,idx;

    assert(pStatus->m_Bufnumm > 0);
    /*for add into num exit handle to wait*/
    waitnum = (pStatus->m_Bufnumm + 1);
    pWaitHandles = (HANDLE*)calloc(waitnum,sizeof(*pWaitHandles));
    if(pWaitHandles == NULL)
    {
        dret = LAST_ERROR_CODE();
        goto out;
    }

    DEBUG_INFO("pStatus base 0x%p\n",pStatus->m_pMemShareBase);
    CopyMemory(pWaitHandles,pStatus->m_pInputEvts,sizeof(*pWaitHandles)*(waitnum - 1));
    pWaitHandles[waitnum - 1] = pStatus->m_ThreadControl.exitevt;

    while(pStatus->m_ThreadControl.running)
    {
        dret = WaitForMultipleObjectsEx(waitnum,pWaitHandles,FALSE,INFINITE,TRUE);
        if((dret >= WAIT_OBJECT_0) && (dret <= (WAIT_OBJECT_0+waitnum - 2)))
        {
            idx = dret - WAIT_OBJECT_0;
            __HandleStatusEvent(pStatus,idx);
        }
        else if((dret == (WAIT_OBJECT_0+waitnum - 1)))
        {
            ERROR_INFO("Exit Event Notify");
        }
        else if(dret == WAIT_FAILED)
        {
            dret = LAST_ERROR_CODE();
            ERROR_INFO("Wait error(%d)\n",dret);
            goto out;
        }
    }

    dret =0;


out:
    if(pWaitHandles)
    {
        free(pWaitHandles);
    }
    pWaitHandles = NULL;
    SetLastError(dret);
    pStatus->m_ThreadControl.exited = 1;
    return dret;
}


void __ClearEventList(PDETOUR_THREAD_STATUS_t pStatus)
{
    int tries = 0;
    int fullfreelist=0;
    if(pStatus == NULL)
    {
        return ;
    }

    if(pStatus->m_pEventListArray)
    {
        free(pStatus->m_pEventListArray);
    }
    pStatus->m_pEventListArray = NULL;


    return ;
}

void __ClearSeqId(PDETOUR_THREAD_STATUS_t pStatus)
{
    if(pStatus->m_pBufIdxVecs)
    {
        pStatus->m_pBufIdxVecs->clear();
        delete pStatus->m_pBufIdxVecs;
    }
    pStatus->m_pBufIdxVecs = NULL;

    if(pStatus->m_pSeqIdVecs)
    {
        pStatus->m_pSeqIdVecs->clear();
        delete pStatus->m_pSeqIdVecs;
    }
    pStatus->m_pSeqIdVecs = NULL;
    pStatus->m_CurSeqId = 0;
    return ;
}


void __FreeDetourEvents(PDETOUR_THREAD_STATUS_t pStatus)
{
    unsigned int i;
    BOOL bret;
    int ret;

    if(pStatus == NULL)
    {
        return ;
    }

    if(pStatus->m_pFreeEvts)
    {
        for(i=0; i<pStatus->m_Bufnumm; i++)
        {
            if(pStatus->m_pFreeEvts[i])
            {
                bret = CloseHandle(pStatus->m_pFreeEvts[i]);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("[%d] FreeEvent(0x%08x) Close Error(%d)\n",
                               i,pStatus->m_pFreeEvts[i],ret);
                }
            }
            pStatus->m_pFreeEvts[i] = NULL;
        }

        free(pStatus->m_pFreeEvts);
    }
    pStatus->m_pFreeEvts = NULL;
    ZeroMemory(pStatus->m_FreeEvtBaseName,sizeof(pStatus->m_FreeEvtBaseName));

    if(pStatus->m_pInputEvts)
    {
        for(i=0; i<pStatus->m_Bufnumm; i++)
        {
            if(pStatus->m_pInputEvts[i])
            {
                bret = CloseHandle(pStatus->m_pInputEvts[i]);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("[%d] InputEvent(0x%08x) Close Error(%d)\n",
                               i,pStatus->m_pInputEvts[i],ret);
                }
            }
            pStatus->m_pInputEvts[i] = NULL;
        }
        free(pStatus->m_pInputEvts);
    }
    pStatus->m_pInputEvts = NULL;
    ZeroMemory(pStatus->m_InputEvtBaseName,sizeof(pStatus->m_InputEvtBaseName));
    return ;
}

void __UnMapMemBase(PDETOUR_THREAD_STATUS_t pStatus)
{

    if(pStatus == NULL)
    {
        return ;
    }
    UnMapFileBuffer((unsigned char**)&(pStatus->m_pMemShareBase));
    CloseMapFileHandle(&(pStatus->m_hMapFile));
    ZeroMemory(pStatus->m_MemShareBaseName,sizeof(pStatus->m_MemShareBaseName));
    pStatus->m_Bufnumm = 0;
    pStatus->m_BufSectSize = 0;
    return ;
}

void __FreeIoInjectThreadStatus(PDETOUR_THREAD_STATUS_t *ppStatus)
{
    PDETOUR_THREAD_STATUS_t pStatus ;
    if(ppStatus == NULL || *ppStatus == NULL)
    {
        return;
    }
    pStatus = *ppStatus;
    /*now first to stop thread */
    StopThreadControl(&(pStatus->m_ThreadControl));

    __ClearSeqId(pStatus);

    /*now to delete all the free event*/
    __ClearEventList(pStatus);
    __FreeDetourEvents(pStatus);


    /*now to unmap memory*/
    __UnMapMemBase(pStatus);
    st_EventInitFuncList.CallList(NULL);

    free(pStatus);
    *ppStatus = NULL;

    return ;
}


int __DetourIoInjectThreadStop(PIO_CAP_CONTROL_t pControl)
{
    __FreeIoInjectThreadStatus(&st_pDetourStatus);
    st_UnPressedLastKey = -1;
    SetLastError(0);
    return 0;
}


PDETOUR_THREAD_STATUS_t __AllocateDetourStatus()
{
    PDETOUR_THREAD_STATUS_t pStatus=NULL;
    int ret;

    pStatus =(PDETOUR_THREAD_STATUS_t) calloc(1,sizeof(*pStatus));
    if(pStatus == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return NULL;
    }

    /*to make it ok for exited*/
    pStatus->m_ThreadControl.exited = 1;
    return pStatus;
}


int __MapMemBase(PDETOUR_THREAD_STATUS_t pStatus,uint8_t* pMemName,uint32_t bufsectsize,uint32_t bufnum)
{
    int ret;
    __UnMapMemBase(pStatus);
    /*now we should first to get map file handle*/
    pStatus->m_hMapFile = CreateMapFile((const char*)pMemName,bufsectsize*bufnum,0);
    if(pStatus->m_hMapFile == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not createmapfile(%s) bufsectsize(%d) bufnum(%d) Error(%d)\n",
                   pMemName,bufsectsize,bufnum,ret);
        goto fail;
    }

    pStatus->m_pMemShareBase = (ptr_t)MapFileBuffer(pStatus->m_hMapFile,bufsectsize*bufnum);
    if(pStatus->m_pMemShareBase == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not mapfile buffer(%s) bufsectsize(%d) bufnum(%d) Error(%d)\n",
                   pMemName,bufsectsize,bufnum,ret);
        goto fail;
    }

    pStatus->m_Bufnumm = bufnum;
    pStatus->m_BufSectSize = bufsectsize;
    strncpy_s((char*)pStatus->m_MemShareBaseName,sizeof(pStatus->m_MemShareBaseName),(const char*)pMemName,_TRUNCATE);


    DEBUG_INFO("memshare(%s)0x%p bufnum %d bufsectsize %d\n",pStatus->m_MemShareBaseName,pStatus->m_pMemShareBase,pStatus->m_Bufnumm,pStatus->m_BufSectSize);
    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    __UnMapMemBase(pStatus);
    SetLastError(ret);
    return -ret;
}

int __AllocateSeqId(PDETOUR_THREAD_STATUS_t pStatus)
{
    __ClearSeqId(pStatus);
    pStatus->m_CurSeqId = 0;
    pStatus->m_pBufIdxVecs = new std::vector<DWORD>();
    pStatus->m_pSeqIdVecs = new std::vector<unsigned long long>();

    return 0;
}


int __AllocateFreeEvents(PDETOUR_THREAD_STATUS_t pStatus,uint8_t* pFreeEvtBaseName)
{
    uint8_t fullname[IO_NAME_MAX_SIZE];
    int ret;
    uint32_t i;
    assert(pStatus->m_Bufnumm > 0);
    /*now we should allocate size*/
    pStatus->m_pFreeEvts = (HANDLE*)calloc(pStatus->m_Bufnumm,sizeof(pStatus->m_pFreeEvts[0]));
    if(pStatus->m_pFreeEvts == NULL)
    {
        ret= LAST_ERROR_CODE();
        goto fail;
    }

    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        _snprintf_s((char*)fullname,sizeof(fullname),_TRUNCATE,"%s_%d",pFreeEvtBaseName,i);
        pStatus->m_pFreeEvts[i] = GetEvent((const char*)fullname,0);
        if(pStatus->m_pFreeEvts[i] == NULL)
        {
            ret=  LAST_ERROR_CODE();
            ERROR_INFO("GetFreeEvent (%s) Error(%d)\n",fullname,ret);
            goto fail;
        }
        DEBUG_INFO("[%d] GetFreeEvts (%s)\n",i,fullname);
    }
    strncpy_s((char*)pStatus->m_FreeEvtBaseName,sizeof(pStatus->m_FreeEvtBaseName),(const char*)pFreeEvtBaseName,_TRUNCATE);


    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    __FreeDetourEvents(pStatus);
    SetLastError(ret);
    return -ret;
}

int __AllocateEventList(PDETOUR_THREAD_STATUS_t pStatus)
{
    int ret;
    uint32_t i;
    assert(pStatus->m_Bufnumm);
    /*now first to allocate event list array*/
    pStatus->m_pEventListArray = (EVENT_LIST_t*)calloc(pStatus->m_Bufnumm,sizeof(pStatus->m_pEventListArray[0]));
    if(pStatus->m_pEventListArray == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        pStatus->m_pEventListArray[i].m_BaseAddr =(ptr_t) pStatus->m_pMemShareBase;
        pStatus->m_pEventListArray[i].m_Error = 0;
        pStatus->m_pEventListArray[i].m_hFillEvt = pStatus->m_pFreeEvts[i];
        pStatus->m_pEventListArray[i].m_Idx = i;
        pStatus->m_pEventListArray[i].m_Offset = (pStatus->m_BufSectSize * i);
        pStatus->m_pEventListArray[i].size = pStatus->m_BufSectSize;
        DEBUG_INFO("[%d]base 0x%08x offset 0x%08x\n",i,pStatus->m_pEventListArray[i].m_BaseAddr,pStatus->m_pEventListArray[i].m_Offset);
    }


    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    __ClearEventList(pStatus);
    SetLastError(ret);
    return -ret;
}

int __AllocateInputEvents(PDETOUR_THREAD_STATUS_t pStatus,uint8_t* pInputEvtBaseName)
{
    uint8_t fullname[IO_NAME_MAX_SIZE];
    int ret;
    uint32_t i;
    assert(pStatus->m_Bufnumm > 0);
    /*now we should allocate size*/
    pStatus->m_pInputEvts = (HANDLE*)calloc(pStatus->m_Bufnumm,sizeof(pStatus->m_pInputEvts[0]));
    if(pStatus->m_pInputEvts == NULL)
    {
        ret= LAST_ERROR_CODE();
        goto fail;
    }

    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        _snprintf_s((char*)fullname,sizeof(fullname),_TRUNCATE,"%s_%d",pInputEvtBaseName,i);
        pStatus->m_pInputEvts[i] = GetEvent((const char*)fullname,0);
        if(pStatus->m_pInputEvts[i] == NULL)
        {
            ret=  LAST_ERROR_CODE();
            ERROR_INFO("GetInputEvent (%s) Error(%d)\n",fullname,ret);
            goto fail;
        }
        DEBUG_INFO("[%d] GetInput Evts(%s)\n",i,fullname);
    }

    strncpy_s((char*)pStatus->m_InputEvtBaseName,sizeof(pStatus->m_InputEvtBaseName),(const char*)pInputEvtBaseName,_TRUNCATE);

    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    __FreeDetourEvents(pStatus);
    SetLastError(ret);
    return -ret;
}


int __DetourIoInjectThreadStart(PIO_CAP_CONTROL_t pControl)
{
    int ret;
    PDETOUR_THREAD_STATUS_t pStatus=NULL;

    if(pControl == NULL || pControl->memsharenum == 0 ||
            pControl->memsharesectsize == 0 ||
            pControl->memsharesize != (pControl->memsharenum * pControl->memsharesectsize) ||
            strlen((const char*)pControl->memsharename) == 0 ||
            strlen((const char*)pControl->inputevtbasename) == 0 ||
            strlen((const char*)pControl->freeevtbasename)== 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("Invalid Parameter\n");
        SetLastError(ret);
        return -ret;
    }

    if(st_pDetourStatus)
    {
        ret = ERROR_ALREADY_EXISTS;
        ERROR_INFO("Already Exist Start\n");
        SetLastError(ret);
        return -ret;
    }

    /*to call when use to init*/
    st_EventInitFuncList.CallList(NULL);

    pStatus = __AllocateDetourStatus();
    if(pStatus == NULL)
    {
        ret=  LAST_ERROR_CODE();
        goto fail;
    }

    /*we put here for it will let the start ok*/

    ret = __MapMemBase(pStatus,pControl->memsharename,pControl->memsharesectsize,pControl->memsharenum);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    ret = __AllocateFreeEvents(pStatus,pControl->freeevtbasename);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    ret = __AllocateInputEvents(pStatus,pControl->inputevtbasename);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }


    ret = __AllocateEventList(pStatus);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    ret = __AllocateSeqId(pStatus);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    ret = StartThreadControl(&(pStatus->m_ThreadControl),DetourIoInjectThreadThreadImpl,pStatus,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    st_pDetourStatus = pStatus;



    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    __FreeIoInjectThreadStatus(&pStatus);
    SetLastError(ret);
    return -ret;
}

int __DetourIoInjectThreadAddDevice(PIO_CAP_CONTROL_t pControl)
{
    int ret=ERROR_NOT_FOUND;
    uint32_t devtype;
    uint32_t devid;
    uint32_t count =0;

    devtype = pControl->devtype;
    devid = pControl->devid;

    if(devtype != DEVICE_TYPE_KEYBOARD && devtype != DEVICE_TYPE_MOUSE)
    {
        ret = ERROR_NOT_SUPPORTED;
        ERROR_INFO("devtype (%d) not supported\n",devtype);
        goto fail;
    }

    if(devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("devid(%d) outof range\n",devid);
        goto fail;
    }

    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}

int __DetourIoInjectThreadRemoveDevice(PIO_CAP_CONTROL_t pControl)
{
    int ret=ERROR_NOT_FOUND;
    uint32_t devtype;
    uint32_t devid;
    uint32_t count =0;

    devtype = pControl->devtype;
    devid = pControl->devid;
    if(devtype != DEVICE_TYPE_KEYBOARD && devtype != DEVICE_TYPE_MOUSE)
    {
        ret = ERROR_NOT_SUPPORTED;
        ERROR_INFO("devtype (%d) not supported\n",devtype);
        goto fail;
    }

    if(devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("devid(%d) outof range\n",devid);
        goto fail;
    }

    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}

int DetourIoInjectThreadControl(PIO_CAP_CONTROL_t pControl)
{
    int ret;
    DWORD dret;

    if(st_hIoInjectControlSema == NULL)
    {
        ret = ERROR_SEM_NOT_FOUND;
        SetLastError(ret);
        return -ret;
    }

    dret = WaitForSingleObjectEx(st_hIoInjectControlSema,5000,TRUE);
    if(dret != WAIT_OBJECT_0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("WaitFor InputSema Return(%d) Error(%d)\n",dret,ret);
        SetLastError(ret);
        return -ret;
    }

    switch(pControl->opcode)
    {
    case IO_INJECT_STOP:
        ret = __DetourIoInjectThreadStop(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Stop Device Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_START:
        ret = __DetourIoInjectThreadStart(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Start Device Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_ADD_DEVICE:
        ret = __DetourIoInjectThreadAddDevice(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Add Device (%d) Error(%d)\n",
                       pControl->devtype,ret);
            goto fail;
        }
        break;
    case IO_INJECT_REMOVE_DEVICE:
        ret = __DetourIoInjectThreadRemoveDevice(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Remove Device (%d:%d) Error(%d)\n",
                       pControl->devtype,pControl->devid,ret);
            goto fail;
        }
        break;

    case IO_INJECT_HIDE_CURSOR:
        ret = SetShowCursorHide();
        if(ret < 0)
        {
            ret=  LAST_ERROR_CODE();
            ERROR_INFO("Hide Cursor Error(%d)\n",ret);
            goto fail;
        }
        break;

    case IO_INJECT_NORMAL_CURSOR:
        ret = SetShowCursorNormal();
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Normal Cursor Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_ENABLE_SET_POS:
        ret = EnableSetCursorPos();
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Enable SetCursorPos Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_DISABLE_SET_POS:
        ret = DisableSetCursorPos();
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Disable SetCursorPos Error(%d)\n",ret);
            goto fail;
        }
        break;
    default:
        ret=  ERROR_INVALID_PARAMETER;
        ERROR_INFO("Invalid code (%d)\n",pControl->opcode);
        goto fail;
    }


    ReleaseSemaphore(st_hIoInjectControlSema,1,NULL);
    SetLastError(0);
    return 0;

fail:
    ReleaseSemaphore(st_hIoInjectControlSema,1,NULL);
    SetLastError(ret);
    return -ret;


}

int RegisterEventListInit(FuncCall_t pFunc,LPVOID pParam,int prior)
{
    return st_EventInitFuncList.AddFuncList(pFunc,pParam,prior);
}

int UnRegisterEventListInit(FuncCall_t pFunc)
{
    return st_EventInitFuncList.RemoveFuncList(pFunc);
}

int RegisterEventListHandler(FuncCall_t pFunc,LPVOID pParam,int prior)
{
    return st_EventHandlerFuncList.AddFuncList(pFunc,pParam,prior);
}

int UnRegisterEventListHandler(FuncCall_t pFunc)
{
    return st_EventHandlerFuncList.RemoveFuncList(pFunc);
}

void IoInjectThreadFini(HMODULE hModule)
{
    UnRegisterEventListHandler(BaseSetKeyMouseState);
    if(st_hIoInjectControlSema)
    {
        CloseHandle(st_hIoInjectControlSema);
    }
    st_hIoInjectControlSema = NULL;
    return ;
}

int IoInjectThreadInit(HMODULE hModule)
{
    int ret;
    st_hIoInjectControlSema = CreateSemaphore(NULL,1,10,NULL);
    if(st_hIoInjectControlSema == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create Semaphore Error(%d)\n",ret);
        goto fail;
    }
    ret = RegisterEventListHandler(BaseSetKeyMouseState,NULL,0);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Register BaseSetKeyMouseState Error(%d)\n",ret);
        goto fail;
    }

    SetLastError(0);
    return 0;
fail:
    UnRegisterEventListHandler(BaseSetKeyMouseState);

    assert(ret > 0);
    if(st_hIoInjectControlSema)
    {
        CloseHandle(st_hIoInjectControlSema);
    }
    st_hIoInjectControlSema = NULL;

    SetLastError(ret);
    return -ret;
}

