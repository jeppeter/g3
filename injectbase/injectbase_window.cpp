
#include "injectbase_window.h"

static CRITICAL_SECTION st_hWndCS;
static std::vector<HWND> st_hWndBaseVecs;
static std::vector<RECT> st_hWndBaseRectVecs;
static std::vector<HCURSOR> st_hWndClassCursorVecs;
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
static UINT st_MouseZPoint=0;


#define   SHOWCURSOR_HIDE_REQ      -1
#define   SHOWCURSOR_NORMAL_REQ     1

static CRITICAL_SECTION st_ShowCursorCS;
static HCURSOR st_hNoMouseCursor=NULL;
static int st_ShowCursorCount=0;
static int st_ShowCursorInit=0;
static int st_ShowCursorHideMode=0;
static int st_ShowCursorRequest=0;
static int st_SetCursorPosEnable=0;


typedef int (WINAPI *ShowCursorFunc_t)(BOOL bShow);
typedef HCURSOR(WINAPI *SetCursorFunc_t)(HCURSOR hCursor);
typedef ULONG_PTR(WINAPI *SetClassLongPtrFunc_t)(HWND hWnd,int nIndex,LONG_PTR dwNewLong);
typedef ULONG_PTR(WINAPI *GetClassLongPtrFunc_t)(HWND hWnd,int nIndex);
typedef BOOL (WINAPI *SetCursorPosFunc_t)(int x,int y);
typedef BOOL (WINAPI *GetCursorPosFunc_t)(LPPOINT lpPoint);


ShowCursorFunc_t ShowCursorNext=ShowCursor;
SetCursorFunc_t SetCursorNext=SetCursor;
SetClassLongPtrFunc_t SetClassLongPtrANext=SetClassLongPtrA;
SetClassLongPtrFunc_t SetClassLongPtrWNext=SetClassLongPtrW;
GetClassLongPtrFunc_t GetClassLongPtrANext=GetClassLongPtrA;
GetClassLongPtrFunc_t GetClassLongPtrWNext=GetClassLongPtrW;
SetCursorPosFunc_t SetCursorPosNext=SetCursorPos;
GetCursorPosFunc_t GetCursorPosNext=GetCursorPos;

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
                SetClassLongPtrANext(hwnd,GCLP_HCURSOR,(LONG)st_hWndClassCursorVecs[i]);
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

    if(hwnd && nIndex == GCLP_HCURSOR && st_ShowCursorInit)
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
                /*if not hide ,so we should get this ok*/
                pRet =  GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
                if(pRet != (ULONG_PTR)st_hWndClassCursorVecs[findidx])
                {
                    ERROR_INFO("[0x%08x] wnd class OldRet 0x%08x != 0x%08x\n",hwnd,
                               pRet,st_hWndClassCursorVecs[findidx]);
                    st_hWndClassCursorVecs[findidx] = (HCURSOR)pRet;
                }
            }
            else
            {
                pRet = (ULONG_PTR)st_hWndClassCursorVecs[findidx];
            }
        }
        else
        {
            ERROR_INFO("[0x%08x]not find in the hwnd vecs\n",hwnd);
            pRet = GetClassLongPtrANext(hwnd,GCLP_HCURSOR);
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
    ULONG_PTR pRet=NULL,pOldRet;
    UINT i;
    int findidx = -1;

    if(hwnd && nIndex == GCLP_HCURSOR && st_ShowCursorInit)
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
                pOldRet = (ULONG_PTR)st_hWndClassCursorVecs[findidx];
                st_hWndClassCursorVecs[findidx]= (HCURSOR)dwNewLong;
                pRet = (HCURSOR) SetClassLongPtrANext(hwnd,GCLP_HCURSOR,dwNewLong);
                if(pOldRet != pRet)
                {
                    ERROR_INFO("[0x%08x] OldRet 0x%08x != Ret 0x%08x\n",hwnd,pOldRet,pRet);
                }
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
    HWND hActiveWnd=NULL;
    UINT i;
    int findidx=-1;
    HCURSOR oldcursor;

    if(st_ShowCursorCount)
    {

        hActiveWnd = GetCurrentProcessActiveWindow();

        EnterCriticalSection(&st_hWndCS);
        for(i=0; i<st_hWndBaseVecs.size(); i++)
        {
            if(hActiveWnd == st_hWndBaseVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            if(st_ShowCursorHideMode)
            {
                hRetCursor = st_hWndClassCursorVecs[findidx];
                st_hWndClassCursorVecs[findidx] = hCursor;
            }
            else
            {

                oldcursor = st_hWndClassCursorVecs[findidx];
                st_hWndClassCursorVecs[findidx] = hCursor;
                hRetCursor = SetCursorNext(hCursor);
                if(oldcursor != hRetCursor)
                {
                    ERROR_INFO("[0x%08x] active wnd oldcursor (0x%08x) != retcursor (0x%08x)\n",
                               hActiveWnd,oldcursor,hRetCursor);
                }
            }
        }
        else
        {
            ERROR_INFO("could not get active window state 0x%08x\n",hActiveWnd);
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
    EnterCriticalSection(&st_hWndCS);
    cnt = st_KeyDownTimes[scancode];
    LeaveCriticalSection(&st_hWndCS);
    return cnt;
}
int __ReCalculateMaxWindowRectNoLock()
{
    UINT i;
    int pickidx=0;
    LONG hw,hh,mw,mh;

    if(st_hWndBaseVecs.size() == 0)
    {
        st_MaxRect.top = 0;
        st_MaxRect.left = 0;
        st_MaxRect.right = 2;
        st_MaxRect.bottom = 2;
        return 0;
    }

    assert(st_hWndBaseVecs.size() > 0);
    for(i=0; i<st_hWndBaseVecs.size() ; i++)
    {
        hw = (st_hWndBaseRectVecs[i].right - st_hWndBaseRectVecs[i].left);
        hh = (st_hWndBaseRectVecs[i].bottom - st_hWndBaseRectVecs[i].top);
        mw = (st_hWndBaseRectVecs[pickidx].right - st_hWndBaseRectVecs[pickidx].left);
        mh = (st_hWndBaseRectVecs[pickidx].bottom - st_hWndBaseRectVecs[pickidx].top);
        if((hw > mw) &&
                (hh > mh))
        {
            DEBUG_INFO("picked %d\n",i);
            pickidx = i;
        }
    }

    /*now max window size is rect*/
    CopyMemory(&st_MaxRect,&(st_hWndBaseRectVecs[pickidx]),sizeof(st_MaxRect));

    if(st_MaxRect.left < 0)
    {
        st_MaxRect.left = 0;
    }

    if(st_MaxRect.right < 0 || st_MaxRect.right <= st_MaxRect.left)
    {
        st_MaxRect.right = st_MaxRect.left + 1;
    }

    if(st_MaxRect.top < 0)
    {
        st_MaxRect.top = 0;
    }

    if(st_MaxRect.bottom < 0 || st_MaxRect.bottom <= st_MaxRect.top)
    {
        st_MaxRect.bottom = st_MaxRect.top + 1;
    }


    return pickidx;
}

int __ReCalculateMousePointNoLock(int check)
{
    //DEBUG_INFO("st_MousePoint (%d-%d) maxrect (%d-%d:%d-%d)\n",
    //           st_MousePoint.x,st_MousePoint.y,
    //           st_MaxRect.left,st_MaxRect.top,
    //           st_MaxRect.right,st_MaxRect.bottom);
    if(st_MousePoint.x <= st_MaxRect.left)
    {
        DEBUG_INFO("reset x (%d + 1)\n",st_MaxRect.left);
        st_MousePoint.x = st_MaxRect.left + 1;
    }
    else if(st_MousePoint.x >= st_MaxRect.right)
    {
        DEBUG_INFO("reset x (%d - 1)\n",st_MaxRect.right);
        st_MousePoint.x = st_MaxRect.right - 1;
    }

    if(st_MousePoint.y  <= st_MaxRect.top)
    {
        DEBUG_INFO("reset y (%d + 1)\n",st_MaxRect.top);
        st_MousePoint.y = st_MaxRect.top + 1;
    }
    else if(st_MousePoint.y >= st_MaxRect.bottom)
    {
        DEBUG_INFO("reset y (%d - 1)\n",st_MaxRect.bottom);
        st_MousePoint.y = st_MaxRect.bottom - 1;
    }

    if(st_MousePoint.x < 1)
    {
        st_MousePoint.x = 1;
    }

    if(st_MousePoint.y < 1)
    {
        st_MousePoint.y = 1;
    }

    if(check && (st_MousePoint.x != st_MouseLastPoint.x ||
                 st_MousePoint.y != st_MouseLastPoint.y))
    {
        ERROR_INFO("Should MousePoint(x:%d:y:%d) But MousePoint(x:%d:y:%d)\n",
                   st_MousePoint.x,
                   st_MousePoint.y,
                   st_MouseLastPoint.x,
                   st_MouseLastPoint.y);
    }

    /*now we should make sure the point for the last mouse point*/
    st_MouseLastPoint.x = st_MousePoint.x;
    st_MouseLastPoint.y = st_MousePoint.y;

    return 0;
}

int __MoveMouseRelativeNoLock(int x,int y)
{
    int ret= 0;
    st_MousePoint.x += x;
    st_MousePoint.y += y;
    __ReCalculateMousePointNoLock(0);
    return 0;
}

int __MoveMouseAbsoluteNoLock(int clientx,int clienty)
{
    int ret = 0;
    st_MousePoint.x = st_MaxRect.left + clientx;
    st_MousePoint.y = st_MaxRect.top + clienty;
    __ReCalculateMousePointNoLock(1);
    return ret;
}

int __SetBaseMouseBtnNoLock(UINT btn,int down)
{
    int ret;

    if(btn > MOUSE_MAX_BTN || btn < MOUSE_MIN_BTN)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    if(down)
    {
        st_MouseBtnState[btn - 1] = 0x1;
    }
    else
    {
        st_MouseBtnState[ btn - 1] = 0x0;
    }

    return 0;
}

int BaseSetWindowRectState(HWND hwnd)
{
    int findidx = -1;
    UINT i;
    int refreshed = 0;
    BOOL bret;
    RECT rRect,wRect;
    POINT pt;
    int ret;

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
        bret = ::GetClientRect(hwnd,&rRect);
        if(bret)
        {
            pt.x = rRect.left;
            pt.y = rRect.top;
            bret = ::ClientToScreen(hwnd,&pt);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("ClientToScreen(%d-%d) Error(%d)\n",pt.x,pt.y,ret);
                goto unlock;
            }

            wRect.left = pt.x;
            wRect.top = pt.y;

            pt.x = rRect.right;
            pt.y = rRect.bottom;
            bret = ::ClientToScreen(hwnd,&pt);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("ClientToScreen(%d-%d) Error(%d)\n",pt.x,pt.y,ret);
                goto unlock;
            }

            wRect.right = pt.x;
            wRect.bottom = pt.y;
            if(st_hWndBaseRectVecs[findidx].top != wRect.top  ||
                    st_hWndBaseRectVecs[findidx].left != wRect.left ||
                    st_hWndBaseRectVecs[findidx].right != wRect.right ||
                    st_hWndBaseRectVecs[findidx].bottom != wRect.bottom)
            {
                DEBUG_INFO("hwnd(0x%08x) (%d:%d)=>(%d:%d) Set (%d:%d)=>(%d:%d) wRect(%d:%d)=>(%d:%d)\n",
                           st_hWndBaseVecs[findidx],
                           st_hWndBaseRectVecs[findidx].left,
                           st_hWndBaseRectVecs[findidx].top,
                           st_hWndBaseRectVecs[findidx].right,
                           st_hWndBaseRectVecs[findidx].bottom,
                           rRect.left,
                           rRect.top,
                           rRect.right,
                           rRect.bottom,
                           wRect.left,
                           wRect.top,
                           wRect.right,
                           wRect.bottom);
                st_hWndBaseRectVecs[findidx] = wRect;
                refreshed ++;
            }
        }
    }
    else
    {
        for(i=0; i<st_hWndBaseRectVecs.size(); i++)
        {
            bret = ::GetClientRect(st_hWndBaseRectVecs[i],&rRect);
            if(bret)
            {
                pt.x = rRect.left;
                pt.y = rRect.top;
                bret = ::ClientToScreen(st_hWndBaseRectVecs[i],&pt);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("ClientToScreen(%d-%d) Error(%d)\n",pt.x,pt.y,ret);
                    continue;
                }

                wRect.left = pt.x;
                wRect.top = pt.y;

                pt.x = rRect.right;
                pt.y = rRect.bottom;
                bret = ::ClientToScreen(st_hWndBaseRectVecs[i],&pt);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                    ERROR_INFO("ClientToScreen(%d-%d) Error(%d)\n",pt.x,pt.y,ret);
                    continue;
                }

                wRect.right = pt.x;
                wRect.bottom = pt.y;
                if(st_hWndBaseRectVecs[i].top != wRect.top  ||
                        st_hWndBaseRectVecs[i].left != wRect.left ||
                        st_hWndBaseRectVecs[i].right != wRect.right ||
                        st_hWndBaseRectVecs[i].bottom != wRect.bottom)
                {
                    DEBUG_INFO("hwnd(0x%08x) (%d:%d)=>(%d:%d) Set (%d:%d)=>(%d:%d)  wRect(%d:%d)=>(%d:%d)\n",
                               st_hWndBaseRectVecs[i],
                               st_hWndBaseRectVecs[i].left,
                               st_hWndBaseRectVecs[i].top,
                               st_hWndBaseRectVecs[i].right,
                               st_hWndBaseRectVecs[i].bottom,
                               rRect.left,
                               rRect.top,
                               rRect.right,
                               rRect.bottom,
                               wRect.left,
                               wRect.top,
                               wRect.right,
                               wRect.bottom);
                    st_hWndBaseRectVecs[i] = wRect;
                    GetWindowRect(st_hWndBaseRectVecs[i],&wRect);
                    DEBUG_INFO("hwnd(0x%08x) wRect(%d:%d)=>(%d:%d)\n",
                               st_hWndBaseRectVecs[i],
                               wRect.left,
                               wRect.top,
                               wRect.right,
                               wRect.bottom);
                    refreshed ++;
                }
            }
        }
    }
unlock:
    if(refreshed > 0)
    {
        /*we have refreshed window ,so recalculate the window*/
        __ReCalculateMaxWindowRectNoLock();
        __ReCalculateMousePointNoLock(0);
    }
    LeaveCriticalSection(&st_hWndCS);
    return refreshed;
}


int __BaseSetKeyStateNoLock(LPDEVICEEVENT pDevEvent)
{
    int ret;
    int scancode;

    if(pDevEvent->devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("<0x%p>Keyboard devid(%d) invalid\n",pDevEvent,pDevEvent->devid);
        SetLastError(ret);
        return -ret;
    }

    /*not check for the code*/

    if(pDevEvent->event.keyboard.code >= KEYBOARD_CODE_NULL)
    {
        ret= ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p>Keyboard code(%d) invalid\n",pDevEvent,pDevEvent->event.keyboard.code);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.keyboard.event >= KEYBOARD_EVENT_MAX)
    {
        ret= ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p>Keyboard event(%d) invalid\n",pDevEvent,pDevEvent->event.keyboard.event);
        SetLastError(ret);
        return -ret;
    }


    scancode = st_CodeMapDik[pDevEvent->event.keyboard.code];
    if(scancode == DIK_NULL)
    {
        ret= ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p>Keyboard code(%d) TO DIK_NULL invalid\n",pDevEvent,pDevEvent->event.keyboard.code);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN)
    {
        st_BaseKeyState[scancode] = 0x80;
        __BasePressKeyDownNoLock(scancode);
    }
    else
    {
        st_BaseKeyState[scancode] = 0x00;
        __BasePressKeyUpNoLock(scancode);
    }
    return 0;
}

int __BaseSetMouseStateNoLock(LPDEVICEEVENT pDevEvent)
{
    int ret;
    BOOL bret;

    if(pDevEvent->devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("<0x%p>Mouse devid(%d) invalid\n",pDevEvent,pDevEvent->devid);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.mouse.code >= MOUSE_CODE_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p>Mouse code(%d) invalid\n",pDevEvent,pDevEvent->event.mouse.code);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.mouse.event >= MOUSE_EVENT_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p>Mouse event(%d) invalid\n",pDevEvent,pDevEvent->event.mouse.event);
        SetLastError(ret);
        return -ret;
    }


    if(pDevEvent->event.mouse.code == MOUSE_CODE_MOUSE)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVNET_MOVING)
        {
            /*this is relative one*/
            __MoveMouseRelativeNoLock(pDevEvent->event.mouse.x,pDevEvent->event.mouse.y);
            //DEBUG_INFO("x %d y %d mousepoint(%d:%d)\n",pDevEvent->event.mouse.x,pDevEvent->event.mouse.y,
            //           st_MousePoint.x,st_MousePoint.y);
            if(st_SetCursorPosEnable)
            {
                bret = SetCursorPosNext(st_MousePoint.x,st_MousePoint.y);
                if(!bret)
                {
                    ret=  LAST_ERROR_CODE();
                    ERROR_INFO("SetMouse Moving[%d:%d] To MousePoint[%d:%d] Error(%d)\n",
                               pDevEvent->event.mouse.x,
                               pDevEvent->event.mouse.y,
                               st_MousePoint.x,st_MousePoint.y,
                               ret);
                }
            }


        }
        else if(pDevEvent->event.mouse.event ==  MOUSE_EVENT_SLIDE)
        {
            st_MouseZPoint += pDevEvent->event.mouse.x;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_ABS_MOVING)
        {
            __MoveMouseAbsoluteNoLock(pDevEvent->event.mouse.x,pDevEvent->event.mouse.y);
            if(st_SetCursorPosEnable)
            {
                bret = SetCursorPosNext(st_MousePoint.x,st_MousePoint.y);
                if(!bret)
                {
                    ret=  LAST_ERROR_CODE();
                    ERROR_INFO("SetMouse AbsoluteMove[%d:%d] To MousePoint[%d:%d] Error(%d)\n",
                               pDevEvent->event.mouse.x,
                               pDevEvent->event.mouse.y,
                               st_MousePoint.x,st_MousePoint.y,
                               ret);
                }
            }
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p>Mouse Invalid code(%d) event(%d)\n",pDevEvent,
                       pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            SetLastError(ret);
            return -ret;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            __SetBaseMouseBtnNoLock(MOUSE_LEFT_BTN,1);
            DEBUG_INFO("MouseLeftDown Point(%d:%d)\n",st_MousePoint.x,st_MousePoint.y);
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            __SetBaseMouseBtnNoLock(MOUSE_LEFT_BTN,0);
            DEBUG_INFO("MouseLeftUp Point(%d:%d)\n",st_MousePoint.x,st_MousePoint.y);
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p>Mouse Invalid code(%d) event(%d)\n",pDevEvent,
                       pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            SetLastError(ret);
            return -ret;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            __SetBaseMouseBtnNoLock(MOUSE_RIGHT_BTN,1);
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            __SetBaseMouseBtnNoLock(MOUSE_RIGHT_BTN,0);
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p>Mouse Invalid code(%d) event(%d)\n",pDevEvent,
                       pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            SetLastError(ret);
            return -ret;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            __SetBaseMouseBtnNoLock(MOUSE_MIDDLE_BTN,1);
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            __SetBaseMouseBtnNoLock(MOUSE_MIDDLE_BTN,0);
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p>Mouse Invalid code(%d) event(%d)\n",pDevEvent,
                       pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            SetLastError(ret);
            return -ret;
        }
    }
    else
    {
        /*we check before*/
        assert(0!=0);
    }

    return 0;
}


int BaseSetKeyMouseState(LPVOID pParam,LPVOID pInput)
{
    int ret;
    LPDEVICEEVENT pDevEvent=(LPDEVICEEVENT)pInput;
    if(pDevEvent->devtype != DEVICE_TYPE_KEYBOARD &&
            pDevEvent->devtype != DEVICE_TYPE_MOUSE)
    {
        ret = ERROR_NOT_SUPPORTED;
        ERROR_INFO("<0x%p> Not Supported devtype(%d)\n",pDevEvent,pDevEvent->devtype);
        SetLastError(ret);
        return -ret;
    }

    EnterCriticalSection(&st_hWndCS);
    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        ret= __BaseSetKeyStateNoLock(pDevEvent);
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        ret=  __BaseSetMouseStateNoLock(pDevEvent);
    }
    LeaveCriticalSection(&st_hWndCS);

    if(ret != 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
    }
    return -ret;
}

int BaseMouseBtnDown(UINT btn)
{
    int ret;

    if(btn > MOUSE_MAX_BTN || btn < MOUSE_MIN_BTN)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }
    EnterCriticalSection(&st_hWndCS);
    ret = st_MouseBtnState[btn - 1];
    LeaveCriticalSection(&st_hWndCS);

    return ret;
}

int BaseSetWindowsRect(HWND hWnd)
{
    return BaseSetWindowRectState(hWnd);
}

int BaseScreenMousePoint(HWND hwnd,POINT* pPoint)
{
    /*we test for the client point of this window*/
    UINT i;
    int findidx = -1;
    EnterCriticalSection(&st_hWndCS);
    /*now first to make sure */
    for(i=0; i<st_hWndBaseVecs.size(); i++)
    {
        if(st_hWndBaseVecs[i] == hwnd)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        /*now to make sure this mouse point is in the kernel*/
        if(st_MousePoint.x > st_hWndBaseRectVecs[findidx].left && st_MousePoint.x < st_hWndBaseRectVecs[findidx].right &&
                st_MousePoint.y > st_hWndBaseRectVecs[findidx].top && st_MousePoint.y < st_hWndBaseRectVecs[findidx].bottom)
        {
            pPoint->x = (st_MousePoint.x - st_hWndBaseRectVecs[findidx].left);
            pPoint->y = (st_MousePoint.y - st_hWndBaseRectVecs[findidx].top);
        }
        else
        {
            ERROR_INFO("Mouse(x:%d:y:%d) [%d] Rect(top-left:%d-%d  bottom-right:%d-%d)\n",
                       st_MousePoint.x,st_MousePoint.y,
                       findidx,
                       st_hWndBaseRectVecs[findidx].top,
                       st_hWndBaseRectVecs[findidx].left,
                       st_hWndBaseRectVecs[findidx].bottom,
                       st_hWndBaseRectVecs[findidx].right);
            if(st_MousePoint.x <= st_hWndBaseRectVecs[findidx].left)
            {
                pPoint->x = 1;
            }
            else if(st_MousePoint.x >= st_hWndBaseRectVecs[findidx].right)
            {
                pPoint->x = (st_hWndBaseRectVecs[findidx].right - st_hWndBaseRectVecs[findidx].left - 1);
            }
            else
            {
                pPoint->x = (st_MousePoint.x - st_hWndBaseRectVecs[findidx].left);
            }

            if(st_MousePoint.y <= st_hWndBaseRectVecs[findidx].top)
            {
                pPoint->y = 1;
            }
            else if(st_MousePoint.y >= st_hWndBaseRectVecs[findidx].bottom)
            {
                pPoint->y = (st_hWndBaseRectVecs[findidx].bottom - st_hWndBaseRectVecs[findidx].top - 1);
            }
            else
            {
                pPoint->y = (st_MousePoint.y -st_hWndBaseRectVecs[findidx].top);
            }
        }
    }
    else
    {
        /*not find ,so we put it in the top-left point for the max rect*/
        if(st_MousePoint.x <= st_MaxRect.left)
        {
            pPoint->x = 1;
        }
        else if(st_MousePoint.x >= st_MaxRect.right)
        {
            pPoint->x = (st_MaxRect.right - st_MaxRect.left -1);
        }
        else
        {
            pPoint->x = (st_MousePoint.x - st_MaxRect.left);
        }

        if(st_MousePoint.y <= st_MaxRect.top)
        {
            pPoint->y = 1;
        }
        else if(st_MousePoint.y >= st_MaxRect.bottom)
        {
            pPoint->y = (st_MaxRect.bottom - st_MaxRect.top - 1);
        }
        else
        {
            pPoint->y = (st_MousePoint.y - st_MaxRect.top);
        }
    }
    LeaveCriticalSection(&st_hWndCS);

    return 0;
}

int BaseGetMousePointAbsolution(POINT *pPoint)
{
    int ret=0;

    EnterCriticalSection(&st_hWndCS);
    pPoint->x = st_MousePoint.x;
    pPoint->y = st_MousePoint.y;
    LeaveCriticalSection(&st_hWndCS);
    return ret;
}

int GetBaseMouseState(UINT *pMouseBtnState,UINT btns,POINT *pPoint,UINT* pMouseZ)
{
    int ret=0;
    int i;
    int cpsize=0;

    EnterCriticalSection(&st_hWndCS);
    if(btns >= MOUSE_MAX_BTN)
    {
        cpsize = sizeof(*pMouseBtnState) * MOUSE_MAX_BTN;
    }
    else
    {
        cpsize = sizeof(*pMouseBtnState)*btns;
    }
    CopyMemory(pMouseBtnState,st_MouseBtnState,cpsize);
    pPoint->x = st_MousePoint.x;
    pPoint->y = st_MousePoint.y;
    *pMouseZ = st_MouseZPoint;
    LeaveCriticalSection(&st_hWndCS);
    return ret;
}

int GetBaseKeyState(unsigned char *pKeyState,UINT keys)
{
    int ret=0;
    int cpsize=0;

    EnterCriticalSection(&st_hWndCS);
    if(keys >= MAX_STATE_BUFFER_SIZE)
    {
        cpsize = sizeof(*pKeyState)*MAX_STATE_BUFFER_SIZE;
    }
    else
    {
        cpsize = sizeof(*pKeyState)*keys;
    }
    CopyMemory(pKeyState,st_BaseKeyState,cpsize);
    LeaveCriticalSection(&st_hWndCS);
    return ret;
}


BOOL WINAPI GetCursorPosCallBack(LPPOINT lpPoint)
{
    BOOL bret=TRUE;

    if(st_ShowCursorInit)
    {
        EnterCriticalSection(&st_hWndCS);
        lpPoint->x = st_MousePoint.x;
        lpPoint->y = st_MousePoint.y;
        LeaveCriticalSection(&st_hWndCS);
    }
    else
    {
        bret = GetCursorPosNext(lpPoint);
    }
    return bret;
}

BOOL WINAPI SetCursorPosCallBack(int x,int y)
{
    BOOL bret=TRUE;

    if(st_ShowCursorInit)
    {
        EnterCriticalSection(&st_hWndCS);
        /*now we should use abosulte position*/
        st_MousePoint.x = x;
        st_MousePoint.y = y;

        __ReCalculateMousePointNoLock(0);
        if(st_SetCursorPosEnable)
        {
            /*we set the cursor real position for it */
            bret = SetCursorPosNext(st_MousePoint.x, st_MousePoint.y);
        }

        LeaveCriticalSection(&st_hWndCS);
    }
    else
    {
        bret = SetCursorPosNext(x,y);
    }
    return bret;
}

int EnableSetCursorPos(void)
{
    int ret=0;

    EnterCriticalSection(&st_hWndCS);
    ret = st_SetCursorPosEnable;
    st_SetCursorPosEnable = 1;
    LeaveCriticalSection(&st_hWndCS);
    return ret;
}

int DisableSetCursorPos(void)
{
    int ret=0;

    EnterCriticalSection(&st_hWndCS);
    ret = st_SetCursorPosEnable;
    st_SetCursorPosEnable = 0;
    LeaveCriticalSection(&st_hWndCS);
    return ret;
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
    DEBUG_BUFFER_FMT(GetCursorPosNext,10,"Before GetCursorPosNext (0x%p)",GetCursorPosNext);
    DEBUG_BUFFER_FMT(SetCursorPosNext,10,"Before SetCursorPosNext (0x%p)",SetCursorPosNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&ShowCursorNext,ShowCursorCallBack);
    DetourAttach((PVOID*)&SetCursorNext,SetCursorCallBack);
    DetourAttach((PVOID*)&SetClassLongPtrANext,SetClassLongPtrACallBack);
    DetourAttach((PVOID*)&GetClassLongPtrANext,GetClassLongPtrACallBack);
    DetourAttach((PVOID*)&SetClassLongPtrWNext,SetClassLongPtrWCallBack);
    DetourAttach((PVOID*)&GetClassLongPtrWNext,GetClassLongPtrWCallBack);
    DetourAttach((PVOID*)&SetCursorPosNext,SetCursorPosCallBack);
    DetourAttach((PVOID*)&GetCursorPosNext,GetCursorPosCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(ShowCursorNext,10,"After ShowCursorNext (0x%p)",ShowCursorNext);
    DEBUG_BUFFER_FMT(SetCursorNext,10,"After SetCursorNext (0x%p)",SetCursorNext);
    DEBUG_BUFFER_FMT(SetClassLongPtrANext,10,"After SetClassLongPtrANext (0x%p)",SetClassLongPtrANext);
    DEBUG_BUFFER_FMT(GetClassLongPtrANext,10,"After GetClassLongPtrANext (0x%p)",GetClassLongPtrANext);
    DEBUG_BUFFER_FMT(SetClassLongPtrWNext,10,"After SetClassLongPtrWNext (0x%p)",SetClassLongPtrWNext);
    DEBUG_BUFFER_FMT(GetClassLongPtrWNext,10,"After GetClassLongPtrWNext (0x%p)",GetClassLongPtrWNext);
    DEBUG_BUFFER_FMT(GetCursorPosNext,10,"After GetCursorPosNext (0x%p)",GetCursorPosNext);
    DEBUG_BUFFER_FMT(SetCursorPosNext,10,"After SetCursorPosNext (0x%p)",SetCursorPosNext);


    st_ShowCursorInit = 1;
    return 0;
}


