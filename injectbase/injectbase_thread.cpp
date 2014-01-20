
#include "injectbase.h"


void StopThreadControl(thread_control_t *pThrControl)
{
    if(pThrControl == NULL)
    {
        return ;
    }
    pThrControl->running = 0;
    while(pThrControl->exited == 0)
    {
        if(pThrControl->exitevt)
        {
            SetEvent(pThrControl->exitevt);
        }
        SchedOut();
    }

    if(pThrControl->exitevt)
    {
        CloseHandle(pThrControl->exitevt);
    }
    pThrControl->exitevt = NULL;

    if(pThrControl->thread)
    {
        CloseHandle(pThrControl->thread);
    }
    pThrControl->thread = NULL;

    pThrControl->threadid = 0;
    return ;
}

int StartThreadControl(thread_control_t* pThrControl,ThreadFunc_t pStartFunc,LPVOID pParam,int startnow)
{
    int ret;
    if(pThrControl == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    StopThreadControl(pThrControl);

    /*now first to make handle of event*/
    pThrControl->exitevt = GetEvent(NULL,1);
    if(pThrControl->exitevt == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not create event Error(%d)\n",ret);
        goto fail;
    }

    pThrControl->running = 1;
    pThrControl->exited = 0;

    /*now start thread*/
    pThrControl->thread = CreateThread(NULL,0,pStartFunc,pParam,startnow ? 0 : CREATE_SUSPENDED , &(pThrControl->threadid));
    if(pThrControl->thread == NULL)
    {
        ret = LAST_ERROR_CODE();
        /*we need make it exited*/
        pThrControl->exited = 1;
        ERROR_INFO("could not create thread (0x%p:0x%p) Error(%d)\n",
                   pStartFunc,pParam,ret);
        goto fail;
    }

    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    StopThreadControl(pThrControl);
    SetLastError(ret);
    return -ret;
}


int ResumeThreadControl(thread_control_t * pThrControl)
{
    int ret;
    DWORD dret;
    if(pThrControl == NULL || pThrControl->thread == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    dret = ResumeThread(pThrControl->thread);
    if(dret == (DWORD)-1)
    {
        ret = LAST_ERROR_CODE();
        /*we make sure this will exited ,when it */
        pThrControl->exited = 1;
        ERROR_INFO("Thread[0x%p] resume Error(%d)\n",pThrControl->thread,ret);
        SetLastError(ret);
        return -ret;
    }

    SetLastError(0);
    return dret;
}

