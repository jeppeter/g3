
#include "injectbase_window.h"

static CRITICAL_SECTION st_hWndCS;
static std::vector<HWND> st_hWndBaseVecs;
static std::vector<RECT> st_hWndBaseRectVecs;
static std::vector<HCURSOR> st_hWndClassCursorVecs;
static int st_SetCursorPosEnable=0;
static unsigned char st_BaseKeyState[MAX_STATE_BUFFER_SIZE] = {0};
static unsigned int st_KeyDownTimes[256] = {0};

#define   SHOWCURSOR_HIDE_REQ      -1
#define   SHOWCURSOR_NORMAL_REQ     1

static CRITICAL_SECTION st_ShowCursorCS;
static HCURSOR st_hNoMouseCursor=NULL;
static int st_ShowCursorCount=0;
static int st_ShowCursorInit=0;
static int st_ShowCursorHideMode=0;
static int st_ShowCursorRequest=0;


typedef int (WINAPI *ShowCursorFunc_t)(BOOL bShow);
typedef HCURSOR(WINAPI *SetCursorFunc_t)(HCURSOR hCursor);
typedef ULONG_PTR(WINAPI *SetClassLongPtrFunc_t)(HWND hWnd,int nIndex,LONG_PTR dwNewLong);
typedef ULONG_PTR(WINAPI *GetClassLongPtrFunc_t)(HWND hWnd,int nIndex);


ShowCursorFunc_t ShowCursorNext=ShowCursor;
SetCursorFunc_t SetCursorNext=SetCursor;
SetClassLongPtrFunc_t SetClassLongPtrANext=SetClassLongPtrA;
SetClassLongPtrFunc_t SetClassLongPtrWNext=SetClassLongPtrW;
GetClassLongPtrFunc_t GetClassLongPtrANext=GetClassLongPtrA;
GetClassLongPtrFunc_t GetClassLongPtrWNext=GetClassLongPtrW;

int ShowCursorHandle()
{
    int count,handle=0,curcount;
    HCURSOR hCursor=NULL;
    CURSORINFO cursorinfo;
    BOOL bret;
    BYTE CursorMaskAND[] = { 0xFF };
    BYTE CursorMaskXOR[] = { 0x00 };
    HWND hwnd=NULL;
    UINT i;

    if(st_ShowCursorInit)
    {
        EnterCriticalSection(&st_hWndCS);
        if(st_ShowCursorRequest == SHOWCURSOR_HIDE_REQ)
        {

            if(st_hNoMouseCursor == NULL)
            {
                st_hNoMouseCursor= CreateCursor(NULL, 0,0,1,1, CursorMaskAND, CursorMaskXOR);
            }

            if(st_hNoMouseCursor)
            {

                for(i=0; i<st_hWndBaseVecs.size() ; i++)
                {
                    hwnd = st_hWndBaseVecs[i];
                    if(st_ShowCursorHideMode == 0)
                    {
                        st_hWndClassCursorVecs[i] = (HCURSOR) GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
                    }
                    hCursor = (HCURSOR) SetClassLongPtrANext(hwnd,GCLP_HCURSOR,(LONG)st_hNoMouseCursor);
                }
            }
            else
            {
            	ERROR_INFO("could not create nomouse cursor Error(%d)\n",LAST_ERROR_CODE());
                goto outunlock;
            }

            count = ShowCursorNext(FALSE);
            if(count > -1)
            {
                while(1)
                {
                    count = ShowCursorNext(FALSE);
                    if(count < 0)
                    {
                        break;
                    }
                }
            }
            else if(count < -1)
            {
                while(1)
                {
                    count = ShowCursorNext(TRUE);
                    if(count == -1)
                    {
                        break;
                    }
                }
            }
            st_ShowCursorHideMode = 1;
            handle = 1;
        }
        else if(st_ShowCursorRequest == SHOWCURSOR_NORMAL_REQ)
        {
            for(i=0; i<st_hWndBaseVecs.size() ; i++)
            {
                hwnd = st_hWndBaseVecs[i];
                SetClassLongPtrACallBack(hwnd,GCLP_HCURSOR,(LONG)st_hWndClassCursorVecs[i]);
            }
            curcount = ShowCursorNext(TRUE);
            if(curcount > st_ShowCursorCount)
            {
                while(1)
                {
                    curcount = ShowCursorNext(FALSE);
                    if(curcount == st_ShowCursorCount)
                    {
                        break;
                    }
                }
            }
            else if(curcount < st_ShowCursorCount)
            {
                while(1)
                {
                    curcount = ShowCursorNext(TRUE);
                    if(curcount == st_ShowCursorCount)
                    {
                        break;
                    }
                }
            }
            st_ShowCursorHideMode = 0;
            handle = 1;
        }
outunlock:
        st_ShowCursorRequest = 0;
        LeaveCriticalSection(&st_hWndCS);
    }
    return handle;
}

int SetShowCursorHide()
{
    int ret=0,wait;

    if(st_ShowCursorInit)
    {
        do
        {
            wait = 1;
            EnterCriticalSection(&st_hWndCS);
            if(st_ShowCursorRequest == 0)
            {
                st_ShowCursorRequest = SHOWCURSOR_HIDE_REQ;
                wait = 0;
            }
            LeaveCriticalSection(&st_hWndCS);
            if(wait)
            {
                SchedOut();
            }
        }
        while(wait);
        ret = 1;
    }
    return ret;
}



int SetShowCursorNormal()
{
    int ret=0,wait=0;

    if(st_ShowCursorInit)
    {
        do
        {
            wait = 1;
            EnterCriticalSection(&st_hWndCS);
            if(st_ShowCursorRequest == 0)
            {
                st_ShowCursorRequest = SHOWCURSOR_NORMAL_REQ;
                wait = 0;
            }
            LeaveCriticalSection(&st_hWndCS);
            if(wait)
            {
                SchedOut();
            }
        }
        while(wait);
        ret = 1;
    }
    return ret;
}


ULONG_PTR WINAPI GetClassLongPtrACallBack(HWND hwnd,int nIndex)
{
    ULONG_PTR pRet=NULL;
    int findidx= -1;
    UINT i;

    if(hwnd && nIndex == GCLP_HCURSOR)
    {

        EnterCriticalSection(&(st_hWndCS));
        for(i=0; i<st_hWndBaseVecs.size() ; i++)
        {
            if(hwnd == st_hWndBaseVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            if(st_ShowCursorHideMode == 0)
            {
                st_hWndClassCursorVecs[findidx]= (HCURSOR) GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
            }
            pRet = st_hWndClassCursorVecs[findidx];
        }
        else
        {
            ERROR_INFO("[0x%08x]not find in the hwnd vecs\n",hwnd);
            pRet = (HCURSOR)(HCURSOR) GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
        }
        LeaveCriticalSection(&(st_hWndCS));
    }
    else
    {
        pRet = GetClassPtrNext(hwnd,nIndex);
    }
    return pRet;
}



ULONG_PTR WINAPI SetClassLongPtrACallBack(HWND hwnd,int nIndex,LONG_PTR dwNewLong)
{
    ULONG_PTR pRet=NULL;
    UINT i;
    int findidx = -1;

    if(hwnd && nIndex == GCLP_HCURSOR)
    {

        EnterCriticalSection(&(st_hWndCS));
        for(i=0; i<st_hWndBaseVecs.size() ; i++)
        {
            if(hwnd == st_hWndBaseVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            if(st_ShowCursorHideMode == 0)
            {
                st_hWndClassCursorVecs[findidx]= (HCURSOR)dwNewLong;
                pRet = (HCURSOR) SetClassLongPtrANext(hwnd,GCLP_HCURSOR,dwNewLong);
            }
            else
            {
                pRet = st_hWndClassCursorVecs[findidx];
                st_hWndClassCursorVecs[findidx]= dwNewLong;
            }
        }
        else
        {
            ERROR_INFO("[0x%08x] not found in the hwnd vecs\n",hwnd);
            pRet = (HCURSOR) SetClassLongPtrANext(hwnd,GCLP_HCURSOR,dwNewLong);
        }
        LeaveCriticalSection(&(st_ShowCursorCS));
    }
    else
    {
        pRet = SetClassPtrNext(hwnd,nIndex,dwNewLong);
    }
    return pRet;
}


ULONG_PTR WINAPI GetClassLongPtrWCallBack(HWND hwnd,int nIndex)
{
    UINT i;
    int findidx = -1;
    ULONG_PTR pRet=NULL;
    if(hwnd && nIndex == GCLP_HCURSOR)
    {
        if(st_InjectModuleInited)
        {
            EnterCriticalSection(&st_hWndCS);
            for(i=0; i<st_hWndBaseVecs.size() ; i++)
            {
                if(hwnd == st_hWndBaseVecs[i])
                {
                    findidx = i;
                    break;
                }
            }

            if(findidx >= 0)
            {
                if(st_ShowCursorHideMode)
                {
                    pRet = (ULONG_PTR) st_hWndClassCursorVecs[findidx];
                }
                else
                {
                    pRet = GetClassLongPtrWNext(hwnd,nIndex);
                    if(pRet !=(ULONG_PTR) st_hWndClassCursorVecs[findidx])
                    {
                        ERROR_INFO("[0x%08x] window cursor class 0x%08x  not equal 0x%08x\n",hwnd,pRet,st_hWndClassCursorVecs[findidx]);
                        st_hWndClassCursorVecs[findidx] =(HCURSOR) pRet;
                    }
                }
            }
            else
            {
                ERROR_INFO("[0x%08x] not found in the hwnd vecs\n",hwnd);
                pRet = GetClassLongPtrWNext(hwnd,nIndex);
            }
            LeaveCriticalSection(&st_hWndCS);
        }
        else
        {
            pRet = GetClassLongPtrWNext(hwnd,nIndex);
        }
    }
    else
    {
        pRet = GetClassLongPtrWNext(hwnd,nIndex);
    }
    return pRet;
}

ULONG_PTR WINAPI SetClassLongPtrWCallBack(HWND hwnd,int nIndex,LONG_PTR dwNewLong)
{
    UINT i;
    int findidx = -1;
    ULONG_PTR pRet = NULL;
    ULONG_PTR pOldPtr;

    if(hwnd && nIndex == GCLP_HCURSOR)
    {
        if(st_InjectModuleInited)
        {
            EnterCriticalSection(&st_hWndCS);
            for(i=0; i<st_hWndBaseVecs.size(); i++)
            {
                if(hwnd == st_hWndBaseVecs[i])
                {
                    findidx = i;
                    break;
                }
            }
            if(findidx >= 0)
            {
                if(st_ShowCursorHideMode)
                {
                }
                else
                {
                    pOldPtr = (ULONG_PTR)st_hWndClassCursorVecs[findidx];
                    st_hWndClassCursorVecs[findidx] =(HCURSOR) dwNewLong;
                    pRet = SetClassLongPtrWNext(hwnd,nIndex,dwNewLong);
                    if(pOldPtr != pRet)
                    {
                        ERROR_INFO("[0x%08x]hwnd GCLP_HCURSOR oldptr 0x%08x != pRet 0x%08x\n",hwnd,
                                   pOldPtr,pRet);
                    }
                }
            }
            else
            {
                ERROR_INFO("[0x%08x] not found in hwnd vecs\n",hwnd);
                pRet = SetClassLongPtrWNext(hwnd,nIndex,dwNewLong);
            }
            LeaveCriticalSection(&st_hWndCS);
        }
        else
        {
            pRet = SetClassLongPtrWNext(hwnd,nIndex,dwNewLong);
        }
    }
    else
    {
        pRet = SetClassLongPtrWNext(hwnd,nIndex,dwNewLong);
    }
    return pRet;
}

int WINAPI ShowCursorCallBack(BOOL bShow)
{
    int count=0;
    if(st_ShowCursorInit)
    {
        EnterCriticalSection(&st_hWndCS);
        if(st_ShowCursorHideMode)
        {
            if(bShow)
            {
                st_ShowCursorCount ++ ;

            }
            else
            {
                st_ShowCursorCount --;
            }
        }
        else
        {
            st_ShowCursorCount = ShowCursorNext(bShow);
        }
        count = st_ShowCursorCount;
        LeaveCriticalSection(&st_hWndCS);
    }
    else
    {
        count = ShowCursorNext(bShow);
    }
    return count;
}

HCURSOR WINAPI SetCursorCallBack(HCURSOR hCursor)
{
    HCURSOR hRetCursor=NULL;

    if(st_ShowCursorCount)
    {
        EnterCriticalSection(&st_hWndCS);
        if(st_ShowCursorHideMode)
        {
            hRetCursor = st_hCursor;
            st_hCursor = hCursor;
        }
        else
        {
            st_hCursor = hCursor;
            hRetCursor = SetCursorNext(hCursor);
        }
        LeaveCriticalSection(&st_hWndCS);
    }
    else
    {
        hRetCursor = SetCursorNext(hCursor);
    }

    return hRetCursor;
}

int DetourShowCursorFunction(void)
{
    InitializeCriticalSection(&st_hWndCS);
    DEBUG_BUFFER_FMT(ShowCursorNext,10,"Before ShowCursorNext (0x%p)",ShowCursorNext);
    DEBUG_BUFFER_FMT(SetCursorNext,10,"Before SetCursorNext (0x%p)",SetCursorNext);
    DEBUG_BUFFER_FMT(SetClassLongPtrANext,10,"Before SetClassLongPtrANext (0x%p)",SetClassLongPtrANext);
    DEBUG_BUFFER_FMT(GetClassLongPtrANext,10,"Before GetClassLongPtrANext (0x%p)",GetClassLongPtrANext);
    DEBUG_BUFFER_FMT(SetClassLongPtrWNext,10,"Before SetClassLongPtrWNext (0x%p)",SetClassLongPtrWNext);
    DEBUG_BUFFER_FMT(GetClassLongPtrWNext,10,"Before GetClassLongPtrWNext (0x%p)",GetClassLongPtrWNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&ShowCursorNext,ShowCursorCallBack);
    DetourAttach((PVOID*)&SetCursorNext,SetCursorCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(ShowCursorNext,10,"After ShowCursorNext (0x%p)",ShowCursorNext);
    DEBUG_BUFFER_FMT(SetCursorNext,10,"After SetCursorNext (0x%p)",SetCursorNext);
    DEBUG_BUFFER_FMT(SetClassLongPtrANext,10,"After SetClassLongPtrANext (0x%p)",SetClassLongPtrANext);
    DEBUG_BUFFER_FMT(GetClassLongPtrANext,10,"After GetClassLongPtrANext (0x%p)",GetClassLongPtrANext);
    DEBUG_BUFFER_FMT(SetClassLongPtrWNext,10,"After SetClassLongPtrWNext (0x%p)",SetClassLongPtrWNext);
    DEBUG_BUFFER_FMT(GetClassLongPtrWNext,10,"After GetClassLongPtrWNext (0x%p)",GetClassLongPtrWNext);


    st_ShowCursorInit = 1;
    return 0;
}


