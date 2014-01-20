
#include "injectbase_window.h"

static CRITICAL_SECTION st_hWndCS;
static CRITICAL_SECTION st_BaseKeyMouseStateCS;
static std::vector<HWND> st_hWndBaseVecs;
static std::vector<RECT> st_hWndBaseRectVecs;
static std::vector<HCURSOR> st_hWndClassCursorVecs;
static int st_SetCursorPosEnable=0;
static unsigned char st_BaseKeyState[MAX_STATE_BUFFER_SIZE] = {0};
static unsigned int st_KeyDownTimes[256] = {0};
/********************************************
.left 0
.top 0
.right 2
.bottom 2
********************************************/
static RECT st_MaxRect = {0,0,2,2};
static POINT st_MousePoint = { 1,1};
static POINT st_MouseLastPoint = {1,1};
static UINT st_MouseBtnState[MOUSE_MAX_BTN] = {0};
static UINT st_MouseLastBtnState[MOUSE_MAX_BTN] = {0};


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


BOOL InsertHwnd(HWND hwnd)
{
    BOOL bret=FALSE;
    int findidx=-1;
    UINT i;
    HCURSOR hCursor;
    if(st_InjectModuleInited)
    {
        EnterCriticalSection(&st_hWndCS);
        for(i=0; i<st_hWndBaseVecs.size() ; i++)
        {
            if(st_hWndBaseVecs[i] == hwnd)
            {
                findidx = i;
                break;
            }
        }

        if(findidx < 0)
        {
            bret = TRUE;
            st_hWndBaseVecs.push_back(hwnd);
            hCursor = (HCURSOR) GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
            st_hWndClassCursorVecs.push_back(hCursor);
            if(st_ShowCursorHideMode > 0)
            {
                /*this means we should hid cursor ,because when st_ShowCursorHideMode > 0 we have create hNoMouseCursor ,so assert it*/
                assert(st_hNoMouseCursor);
                SetClassLongPtrANext(hwnd,GCLP_HCURSOR,st_hNoMouseCursor);
            }
        }
        LeaveCriticalSection(&st_hWndCS);

        if(!bret)
        {
            SetLastError(ERROR_DUP_NAME);
        }
    }
    else
    {
        SetLastError(ERROR_BAD_ENVIRONMENT);
    }

    return bret;
}

BOOL RemoveHwnd(HWND hwnd)
{
    BOOL bret=FALSE;
    int findidx=-1;
    UINT i;
    if(st_InjectModuleInited)
    {
        EnterCriticalSection(&st_hWndCS);
        for(i=0; i<st_hWndBaseVecs.size() ; i++)
        {
            if(st_hWndBaseVecs[i] == hwnd)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            bret = TRUE;
            st_hWndBaseVecs.erase(st_hWndBaseVecs.begin() + findidx);
            st_hWndClassCursorVecs.erase(st_hWndClassCursorVecs.begin() + findidx);
        }
        LeaveCriticalSection(&st_hWndCS);

        if(!bret)
        {
            SetLastError(ERROR_NO_DATA);
        }
    }
    else
    {
        SetLastError(ERROR_BAD_ENVIRONMENT);
    }

    return bret;
}

#define DIK_NULL  0xff

static int st_CodeMapDik[256] =
{
    DIK_A             ,DIK_B              ,DIK_C              ,DIK_D              ,DIK_E              ,  /*5*/
    DIK_F             ,DIK_G              ,DIK_H              ,DIK_I              ,DIK_J              ,  /*10*/
    DIK_K             ,DIK_L              ,DIK_M              ,DIK_N              ,DIK_O              ,  /*15*/
    DIK_P             ,DIK_Q              ,DIK_R              ,DIK_S              ,DIK_T              ,  /*20*/
    DIK_U             ,DIK_V              ,DIK_W              ,DIK_X              ,DIK_Y              ,  /*25*/
    DIK_Z             ,DIK_0              ,DIK_1              ,DIK_2              ,DIK_3              ,  /*30*/
    DIK_4             ,DIK_5              ,DIK_6              ,DIK_7              ,DIK_8              ,  /*35*/
    DIK_9             ,DIK_ESCAPE         ,DIK_MINUS          ,DIK_EQUALS         ,DIK_BACK           ,  /*40*/
    DIK_TAB           ,DIK_LBRACKET       ,DIK_RBRACKET       ,DIK_RETURN         ,DIK_LCONTROL       ,  /*45*/
    DIK_SEMICOLON     ,DIK_APOSTROPHE     ,DIK_GRAVE          ,DIK_LSHIFT         ,DIK_BACKSLASH      ,  /*50*/
    DIK_COMMA         ,DIK_PERIOD         ,DIK_SLASH          ,DIK_RSHIFT         ,DIK_MULTIPLY       ,  /*55*/
    DIK_LMENU         ,DIK_SPACE          ,DIK_CAPITAL        ,DIK_F1             ,DIK_F2             ,  /*60*/
    DIK_F3            ,DIK_F4             ,DIK_F5             ,DIK_F6             ,DIK_F7             ,  /*65*/
    DIK_F8            ,DIK_F9             ,DIK_F10            ,DIK_F11            ,DIK_F12            ,  /*70*/
    DIK_F13           ,DIK_F14            ,DIK_F15            ,DIK_NUMLOCK        ,DIK_SCROLL         ,  /*75*/
    DIK_SUBTRACT      ,DIK_NUMPAD0        ,DIK_NUMPAD1        ,DIK_NUMPAD2        ,DIK_NUMPAD3        ,  /*80*/
    DIK_NUMPAD4       ,DIK_NUMPAD5        ,DIK_NUMPAD6        ,DIK_NUMPAD7        ,DIK_NUMPAD8        ,  /*85*/
    DIK_NUMPAD9       ,DIK_ADD            ,DIK_DECIMAL        ,DIK_OEM_102        ,DIK_KANA           ,  /*90*/
    DIK_ABNT_C1       ,DIK_CONVERT        ,DIK_NOCONVERT      ,DIK_YEN            ,DIK_ABNT_C2        ,  /*95*/
    DIK_NUMPADEQUALS  ,DIK_PREVTRACK      ,DIK_AT             ,DIK_COLON          ,DIK_UNDERLINE      ,  /*100*/
    DIK_KANJI         ,DIK_STOP           ,DIK_AX             ,DIK_UNLABELED      ,DIK_NEXTTRACK      ,  /*105*/
    DIK_NUMPADENTER   ,DIK_RCONTROL       ,DIK_MUTE           ,DIK_CALCULATOR     ,DIK_PLAYPAUSE      ,  /*110*/
    DIK_MEDIASTOP     ,DIK_VOLUMEDOWN     ,DIK_VOLUMEUP       ,DIK_WEBHOME        ,DIK_NUMPADCOMMA    ,  /*115*/
    DIK_DIVIDE        ,DIK_SYSRQ          ,DIK_RMENU          ,DIK_PAUSE          ,DIK_HOME           ,  /*120*/
    DIK_UP            ,DIK_PRIOR          ,DIK_LEFT           ,DIK_RIGHT          ,DIK_END            ,  /*125*/
    DIK_DOWN          ,DIK_NEXT           ,DIK_INSERT         ,DIK_DELETE         ,DIK_LWIN           ,  /*130*/
    DIK_RWIN          ,DIK_APPS           ,DIK_POWER          ,DIK_SLEEP          ,DIK_WAKE           ,  /*135*/
    DIK_WEBSEARCH     ,DIK_WEBFAVORITES   ,DIK_WEBREFRESH     ,DIK_WEBSTOP        ,DIK_WEBFORWARD     ,  /*140*/
    DIK_WEBBACK       ,DIK_MYCOMPUTER     ,DIK_MAIL           ,DIK_MEDIASELECT    ,DIK_NULL           ,  /*144*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*150*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*155*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*160*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*165*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*170*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*175*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*180*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*185*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*190*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*195*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*200*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*205*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*210*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*215*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*220*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*225*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*230*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*235*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*240*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*245*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*250*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*255*/
    DIK_NULL
};

int __BasePressKeyDownNoLock(UINT scancode)
{
    int cnt;

    if(scancode >= 256)
    {
        cnt = ERROR_INVALID_PARAMETER;
        SetLastError(cnt);
        return -cnt;
    }
    st_KeyDownTimes[scancode] ++;
    cnt = st_KeyDownTimes[scancode];
    return cnt;
}

int __BasePressKeyUpNoLock(UINT scancode)
{
    int cnt;

    if(scancode >= 256)
    {
        cnt = ERROR_INVALID_PARAMETER;
        SetLastError(cnt);
        return -cnt;
    }
    st_KeyDownTimes[scancode] =0;
    cnt = 0;
    return cnt;
}

int BasePressKeyDownTimes(UINT scancode)
{
    int cnt;

    if(scancode >= 256)
    {
        cnt = ERROR_INVALID_PARAMETER;
        SetLastError(cnt);
        return -cnt;
    }
    EnterCriticalSection(&st_BaseKeyMouseStateCS);
    cnt = st_KeyDownTimes[scancode];
    LeaveCriticalSection(&st_BaseKeyMouseStateCS);
    return cnt;
}

