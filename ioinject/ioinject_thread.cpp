
#include "ioinject_thread.h"

typedef struct
{
    uint32_t m_Started;
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
    CRITICAL_SECTION m_ListCS;
    int m_ListCSInited;
    std::vector<EVENT_LIST_t*>* m_pFreeList;
} DETOUR_THREAD_STATUS_t,*PDETOUR_THREAD_STATUS_t;


static DETOUR_THREAD_STATUS_t *st_pDetourStatus=NULL;
static HANDLE st_hDetourDinputSema=NULL;
static PDETOUR_THREAD_STATUS_t st_pDinputStatus=NULL;


int IoInjectInput(PEVENT_LIST_t pEvent)
{
    int ret=0,res;
    LPDEVICEEVENT pDevEvent=NULL;
    int findidx=-1;
    unsigned int i;
    int cnt=0;

    pDevEvent = (LPDEVICEEVENT)(pEvent->m_BaseAddr+pEvent->m_Offset);
    /*now make sure*/
    EnterCriticalSection(&(st_Dinput8DeviceCS));

    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        cnt = 0;
        for(i=0; i<st_Key8WHookVecs.size(); i++)
        {
            if(cnt == pDevEvent->devid)
            {
                findidx = i;
                res = st_Key8WHookVecs[i]->PutEventList(pEvent);
                assert(res > 0);
                ret = 1;
                break;
            }
            cnt +=1;
        }

        if(findidx < 0)
        {
            for(i=0; i<st_Key8AHookVecs.size(); i++)
            {
                if(cnt == pDevEvent->devid)
                {
                    findidx = i;
                    res = st_Key8AHookVecs[i]->PutEventList(pEvent);
                    assert(res > 0);
                    ret = 1;
                    break;
                }
                cnt += 1;
            }
        }
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        cnt = 0;
        for(i=0; i<st_Mouse8WHookVecs.size() ; i++)
        {
            if(cnt  == pDevEvent->devid)
            {
                findidx = i;
                res = st_Mouse8WHookVecs[i]->PutEventList(pEvent);
                assert(res > 0);
                ret = 1;
                break;
            }
            cnt += 1;
        }

        if(findidx < 0)
        {
            for(i=0; i<st_Mouse8AHookVecs.size(); i++)
            {
                if(cnt == pDevEvent->devid)
                {
                    findidx = i;
                    res = st_Mouse8AHookVecs[i]->PutEventList(pEvent);
                    assert(res > 0);
                    ret = 1;
                    break;
                }
                cnt += 1;
            }
        }
    }
    else
    {
        /*now we do not support this*/
        ERROR_INFO("not supported type %d\n",pDevEvent->devtype);
        ret = 0;
    }
    LeaveCriticalSection(&(st_Dinput8DeviceCS));
    return ret;
}


static void IoFreeEventList(EVENT_LIST_t* pEventList)
{
    unsigned int i;
    int findidx = -1;
    int ret = 0;
    int* pnullptr=NULL;
    if(st_pDinputStatus == NULL)
    {
        ERROR_INFO("FreeEventList<0x%p> no DinputStatus\n",pEventList);
        /*this may not be right ,but it notifies the free event ,so it will ok to get the event ok*/
        SetEvent(pEventList->m_hFillEvt);
        return;
    }

    EnterCriticalSection(&(st_pDinputStatus->m_ListCS));
    for(i=0; i<st_pDinputStatus->m_pFreeList->size(); i++)
    {
        if(st_pDinputStatus->m_pFreeList->at(i) == pEventList)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        ERROR_INFO("We have input the Event List<0x%p> idx(%d)\n",pEventList,pEventList->m_Idx);
    }
    else
    {
        st_pDinputStatus->m_pFreeList->push_back(pEventList);
        /*to notify the free list*/
        ret = 1;
    }
    LeaveCriticalSection(&(st_pDinputStatus->m_ListCS));
    if(ret > 0)
    {
        SetEvent(pEventList->m_hFillEvt);
    }

    return ;
}



DWORD WINAPI DetourDirectInputThreadImpl(LPVOID pParam)
{
    PDETOUR_THREAD_STATUS_t pStatus = (PDETOUR_THREAD_STATUS_t)pParam;
    HANDLE *pWaitHandles=NULL;
    unsigned int waitnum=0;
    DWORD dret,idx;
    int ret;
    int tries;

    assert(pStatus->m_Bufnumm > 0);
    /*for add into num exit handle to wait*/
    waitnum = (pStatus->m_Bufnumm + 1);
    pWaitHandles = (HANDLE*)calloc(sizeof(*pWaitHandles),waitnum);
    if(pWaitHandles == NULL)
    {
        dret = LAST_ERROR_CODE();
        goto out;
    }

    CopyMemory(pWaitHandles,pStatus->m_pInputEvts,sizeof(*pWaitHandles)*(waitnum - 1));
    pWaitHandles[waitnum - 1] = pStatus->m_ThreadControl.exitevt;

    while(pStatus->m_ThreadControl.running)
    {
        dret = WaitForMultipleObjectsEx(waitnum,pWaitHandles,FALSE,INFINITE,TRUE);
        if((dret >= WAIT_OBJECT_0) && (dret <= (WAIT_OBJECT_0+waitnum - 2)))
        {
            idx = dret - WAIT_OBJECT_0;
            tries = 0;
            while(1)
            {
                ret = DetourDirectInputChangeFreeToInput(pStatus,idx);
                if(ret > 0)
                {
                    break;
                }
                tries ++;
                if(tries > 5)
                {
                    dret =ERROR_TOO_MANY_MUXWAITERS ;
                    ERROR_INFO("could not change %d into free more than %d\n",idx,tries);
                    goto out;
                }

                SchedOut();
                ERROR_INFO("Change[%d] wait[%d]\n",idx,tries);
            }

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

    if(pStatus->m_pFreeList)
    {
        if(pStatus->m_ListCSInited)
        {
            /*now we should test if the free list is full ,if it is some free list is not in the free list,so we should wait for it*/
            tries = 0;
            while(1)
            {
                fullfreelist = 0;
                EnterCriticalSection(&(pStatus->m_ListCS));
                if(pStatus->m_pFreeList->size() == pStatus->m_Bufnumm)
                {
                    fullfreelist = 1;
                }
                LeaveCriticalSection(&(pStatus->m_ListCS));

                if(fullfreelist > 0)
                {
                    /*ok ,we have collect all free list ,so we can clear them*/
                    break;
                }

                /*now ,so we should wait for a while*/
                if(tries > 5)
                {
                    ERROR_INFO("Could not Get Free List At times (%d)\n",tries);
                    abort();
                }

                tries ++;
                SchedOut();
                ERROR_INFO("Wait Free List At Time(%d)\n",tries);
            }
        }

        pStatus->m_pFreeList->clear();
        delete pStatus->m_pFreeList;
    }
    pStatus->m_pFreeList = NULL;

    if(pStatus->m_pEventListArray)
    {
        free(pStatus->m_pEventListArray);
    }
    pStatus->m_pEventListArray = NULL;

    if(pStatus->m_ListCSInited)
    {
        DeleteCriticalSection(&(pStatus->m_ListCS));
    }
    pStatus->m_ListCSInited = 0;

    return ;
}


void __FreeDetourEvents(PDETOUR_THREAD_STATUS_t pStatus)
{
    unsigned int i;

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
                CloseHandle(pStatus->m_pFreeEvts[i]);
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
                CloseHandle(pStatus->m_pInputEvts[i]);
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

void __FreeDetourDinputStatus(PDETOUR_THREAD_STATUS_t *ppStatus)
{
    PDETOUR_THREAD_STATUS_t pStatus ;
    if(ppStatus == NULL || *ppStatus == NULL)
    {
        return;
    }
    pStatus = *ppStatus;
    /*make sure this is stopped ,so we can do things safe*/
    pStatus->m_Started = 0;
    /*now first to stop thread */
    StopThreadControl(&(pStatus->m_ThreadControl));

    /*now we should free all the events*/
    __FreeDeviceEvents(pStatus);

    /*now to delete all the free event*/
    __ClearEventList(pStatus);
    __FreeDetourEvents(pStatus);

    /*now to unmap memory*/
    __UnMapMemBase(pStatus);

    /*not make not started*/
    pStatus->m_Started = 0;
    free(pStatus);
    *ppStatus = NULL;

    return ;
}


int __DetourDirectInputStop(PIO_CAP_CONTROL_t pControl)
{
    __FreeDetourDinputStatus(&st_pDinputStatus);
    SetLastError(0);
    return 0;
}


PDETOUR_THREAD_STATUS_t __AllocateDetourStatus()
{
    PDETOUR_THREAD_STATUS_t pStatus=NULL;
    int ret;

    pStatus =(PDETOUR_THREAD_STATUS_t) calloc(sizeof(*pStatus),1);
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


    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    __UnMapMemBase(pStatus);
    SetLastError(ret);
    return -ret;
}


int __AllocateFreeEvents(PDETOUR_THREAD_STATUS_t pStatus,uint8_t* pFreeEvtBaseName)
{
    uint8_t fullname[IO_NAME_MAX_SIZE];
    int ret;
    uint32_t i;
    /*now we should allocate size*/
    pStatus->m_pFreeEvts = (HANDLE*)calloc(sizeof(pStatus->m_pFreeEvts[0]),pStatus->m_Bufnumm);
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
    /*now first to allocate event list array*/
    pStatus->m_pEventListArray = (EVENT_LIST_t*)calloc(sizeof(pStatus->m_pEventListArray[0]),pStatus->m_Bufnumm);
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
    }

    /*new vector*/
    pStatus->m_pFreeList = new std::vector<EVENT_LIST_t*>();
    if(pStatus->m_pFreeList == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    /*now put all the event list into the free list*/
    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        pStatus->m_pFreeList->push_back(&(pStatus->m_pEventListArray[i]));
    }

    /*initialize the critical section*/
    InitializeCriticalSection(&(pStatus->m_ListCS));
    pStatus->m_ListCSInited = 1;

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
    /*now we should allocate size*/
    pStatus->m_pInputEvts = (HANDLE*)calloc(sizeof(pStatus->m_pInputEvts[0]),pStatus->m_Bufnumm);
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


int __DetourDirectInputStart(PIO_CAP_CONTROL_t pControl)
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

    if(st_pDinputStatus)
    {
        ret = ERROR_ALREADY_EXISTS;
        ERROR_INFO("Already Exist Start\n");
        SetLastError(ret);
        return -ret;
    }

    pStatus = __AllocateDetourStatus();
    if(pStatus == NULL)
    {
        ret=  LAST_ERROR_CODE();
        goto fail;
    }

    /*we put here for it will let the start ok*/
    pStatus->m_Started = 1;

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

    pStatus->m_Started = 1;
    ret = StartThreadControl(&(pStatus->m_ThreadControl),DetourDirectInputThreadImpl,pStatus,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        pStatus->m_Started = 0;
        goto fail;
    }

    st_pDinputStatus = pStatus;

    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    __FreeDetourDinputStatus(&pStatus);
    SetLastError(ret);
    return -ret;
}

int __DetourDirectInputAddDevice(PIO_CAP_CONTROL_t pControl)
{
    int ret=ERROR_NOT_FOUND;
    uint32_t devtype;
    uint32_t devid;
    uint32_t i;
    uint32_t count =0;

    devtype = pControl->devtype;
    devid = pControl->devid;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    if(devtype == DEVICE_TYPE_KEYBOARD)
    {
        count = 0;
        for(i=0; i<st_Key8WVecs.size() ; i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }

        for(i=0; i<st_Key8AVecs.size(); i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }
    }
    else if(devtype == DEVICE_TYPE_MOUSE)
    {
        count = 0;
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }

        for(i=0; i<st_Mouse8AVes.size(); i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }
    }
    else
    {
        ret =ERROR_INVALID_PARAMETER;
        ERROR_INFO("Invalid devtype(%d)\n",devtype);
    }
unlock:
    LeaveCriticalSection(&st_Dinput8DeviceCS);

    if(ret > 0)
    {
        goto fail;
    }

    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}

int __DetourDirectInputRemoveDevice(PIO_CAP_CONTROL_t pControl)
{
    int ret=ERROR_NOT_FOUND;
    uint32_t devtype;
    uint32_t devid;
    uint32_t i;
    uint32_t count =0;

    devtype = pControl->devtype;
    devid = pControl->devid;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    if(devtype == DEVICE_TYPE_KEYBOARD)
    {
        count = 0;
        for(i=0; i<st_Key8WVecs.size() ; i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }

        for(i=0; i<st_Key8AVecs.size(); i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }
    }
    else if(devtype == DEVICE_TYPE_MOUSE)
    {
        count = 0;
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }

        for(i=0; i<st_Mouse8AVes.size(); i++)
        {
            if(count == devid)
            {
                ret = 0;
                goto unlock;
            }
            count ++;
        }
    }
    else
    {
        ret =ERROR_INVALID_PARAMETER;
        ERROR_INFO("Invalid devtype(%d)\n",devtype);
    }
unlock:
    LeaveCriticalSection(&st_Dinput8DeviceCS);

    if(ret > 0)
    {
        goto fail;
    }

    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl)
{
    int ret;
    DWORD dret;

    if(st_hDetourDinputSema == NULL)
    {
        ret = ERROR_SEM_NOT_FOUND;
        SetLastError(ret);
        return -ret;
    }

    dret = WaitForSingleObjectEx(st_hDetourDinputSema,5000,TRUE);
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
        ret = __DetourDirectInputStop(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Stop Device Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_START:
        ret = __DetourDirectInputStart(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Start Device Error(%d)\n",ret);
            goto fail;
        }
        break;
    case IO_INJECT_ADD_DEVICE:
        ret = __DetourDirectInputAddDevice(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Add Device (%d) Error(%d)\n",
                       pControl->devtype,ret);
            goto fail;
        }
        break;
    case IO_INJECT_REMOVE_DEVICE:
        ret = __DetourDirectInputRemoveDevice(pControl);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Remove Device (%d:%d) Error(%d)\n",
                       pControl->devtype,pControl->devid,ret);
            goto fail;
        }
        break;
    default:
        ret=  ERROR_INVALID_PARAMETER;
        ERROR_INFO("Invalid code (%d)\n",pControl->opcode);
        goto fail;
    }


    ReleaseSemaphore(st_hDetourDinputSema,1,NULL);
    SetLastError(0);
    return 0;

fail:
    ReleaseSemaphore(st_hDetourDinputSema,1,NULL);
    SetLastError(ret);
    return -ret;


}

