
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
} DETOUR_DIRECTINPUT_STATUS_t,*PDETOUR_DIRECTINPUT_STATUS_t;


static DETOUR_THREAD_STATUS_t *st_pDetourStatus=NULL;

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
    PDETOUR_DIRECTINPUT_STATUS_t pStatus = (PDETOUR_DIRECTINPUT_STATUS_t)pParam;
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


void __ClearEventList(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
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


void __FreeDetourEvents(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
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

