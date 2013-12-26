
#include <vector>
#include "ioinject_thread.h"
#include <Windows.h>
#define   DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#include "detourdinput.h"
#include <assert.h>
#include "detourrawinput.h"

#include "detourmessage_emulation_keystate.cpp"

static GetMessageFunc_t GetMessageANext= GetMessageA;
static PeekMessageFunc_t PeekMessageANext=PeekMessageA;
static GetMessageFunc_t GetMessageWNext= GetMessageW;
static PeekMessageFunc_t PeekMessageWNext=PeekMessageW;

static CRITICAL_SECTION st_MessageEmulationCS;
static std::vector<LPMSG> st_MessageEmulationQueue;
static int st_MessageQuit=0;
static UINT st_MaxMessageEmulationQueue=20;



static int st_MessageEmualtionInited=0;

#define  EMULATIONMESSAGE_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  EMULATIONMESSAGE_DEBUG_INFO      DEBUG_INFO

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


/*return 1 for double click will insert ,
0 for not
negative for error ,not insert into it*/
int __InsertMessageQueue(LPMSG lpMsg,int back)
{
    int ret;
    LPMSG lpRemove=NULL;
    if(st_MessageEmualtionInited)
    {
        uint32_t curtick = GetTickCount();
        ret = 0;
        EnterCriticalSection(&st_MessageEmulationCS);
        if(back)
        {

            if(st_MessageQuit==0)
            {
                if(st_MessageEmulationQueue.size() > st_MaxMessageEmulationQueue)
                {
                    lpRemove = st_MessageEmulationQueue[0];
                    st_MessageEmulationQueue.erase(st_MessageEmulationQueue.begin());
                }
                st_MessageEmulationQueue.push_back(lpMsg);
            }
            else
            {
                ret = 0;
            }
        }
        else
        {
            st_MessageEmulationQueue.insert(st_MessageEmulationQueue.begin(),lpMsg);
        }
        LeaveCriticalSection(&st_MessageEmulationCS);

        if(lpRemove)
        {
            EMULATIONMESSAGE_DEBUG_INFO("remove 0x%p Message Code(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d)\n",
                                        lpRemove,lpRemove->message,lpRemove->message,
                                        lpRemove->wParam,lpRemove->wParam,
                                        lpRemove->lParam,lpRemove->lParam);
            free(lpRemove);
        }
        lpRemove = NULL;
        return ret;
    }
    ret = ERROR_APP_INIT_FAILURE;
    SetLastError(ret);
    return -1;
}

int InsertEmulationMessageQueue(LPMSG lpMsg,int back)
{
    int ret=-ERROR_NOT_SUPPORTED;
    LPMSG lcpMsg=NULL;
    HWND hwnd;
    int keyrawinput=0,mouserawinput=0;

    if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
    {
        keyrawinput = IsRawInputKeyboardRegistered();
        if(keyrawinput > 0)
        {
            /*if keyboard is registered ,so no message for keyboard will insert*/
			if(lpMsg->hwnd)
			{
				PostMessage(lpMsg->hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
			}
			else
			{
				hwnd = GetCurrentProcessActiveWindow();
				if(hwnd)
				{
					PostMessage(hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
				}
			}
            return 0;
        }
    }
    else if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST)
    {
        mouserawinput = IsRawInputMouseRegistered();
        if(mouserawinput > 0)
        {
        	/*if mouse raw input registered ,so no message for mouse will insert*/
			if(lpMsg->hwnd)
			{
				PostMessage(lpMsg->hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
			}
			else
			{
				hwnd = GetCurrentProcessActiveWindow();
				if(hwnd)
				{
					PostMessage(hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
				}
			}
            return 0;
        }
    }


    if(st_MessageEmualtionInited)
    {
        lcpMsg = (LPMSG)calloc(sizeof(*lcpMsg),1);
        if(lcpMsg == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return -ret;
        }
        CopyMemory(lcpMsg,lpMsg,sizeof(*lpMsg));
        ret = __InsertMessageQueue(lcpMsg,back);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            free(lcpMsg);
            return -ret;
        }
        lcpMsg = NULL;
        if(lpMsg->hwnd)
        {
            PostMessage(lpMsg->hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
        }
        else
        {
            hwnd = GetCurrentProcessActiveWindow();
            if(hwnd)
            {
                PostMessage(hwnd,lpMsg->message,lpMsg->wParam,lpMsg->lParam);
            }
        }

        SetLastError(0);
        return 0;
    }
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return -ret;
}

int __PrepareMouseButtonMessage(LPMSG lpMsg,UINT message)
{
    lpMsg->hwnd = NULL;
    lpMsg->message = message;

    /*now we should check for the state of*/
    lpMsg->wParam = 0;
    /*check for button down*/
    if(DetourDinputPressKeyDownTimes(DIK_RCONTROL) ||
            DetourDinputPressKeyDownTimes(DIK_LCONTROL))
    {
        lpMsg->wParam |= MK_CONTROL;
    }

    if(DetourDinputMouseBtnDown(MOUSE_LEFT_BTN))
    {
        lpMsg->wParam |= MK_LBUTTON;
    }

    if(DetourDinputMouseBtnDown(MOUSE_RIGHT_BTN))
    {
        lpMsg->wParam |= MK_RBUTTON;
    }

    if(DetourDinputMouseBtnDown(MOUSE_MIDDLE_BTN))
    {
        lpMsg->wParam |= MK_MBUTTON;
    }

    if(DetourDinputPressKeyDownTimes(DIK_RSHIFT) ||
            DetourDinputPressKeyDownTimes(DIK_LSHIFT))
    {
        lpMsg->wParam |= MK_SHIFT;
    }

    /*now to change the client we change this in the time when get this message*/
    lpMsg->lParam = 0;

    /*now we should set the time and point*/
    lpMsg->time = GetTickCount();
    lpMsg->pt.x = 0;
    lpMsg->pt.y = 0;

    return 0;

}

int __PrepareKeyPressMessage(LPMSG lpMsg,UINT scancode,int keydown)
{
    int ret = 0;
    UINT vk;
    int repeat;
    lpMsg->hwnd = NULL;
    if(keydown)
    {
        lpMsg->message = WM_KEYDOWN;
    }
    else
    {
        lpMsg->message = WM_KEYUP;
    }

    lpMsg->wParam = 0;
    vk = MapVirtualKeyEmulation(scancode);
    if(vk == 0)
    {
        /*can not find virtual key ,so we should return error*/
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("scancode (0x%08x:%d) can not find virtual key\n",scancode,scancode);
        SetLastError(ret);
        return -ret;
    }

    lpMsg->wParam = vk;

    lpMsg->lParam = 0;
    if(keydown)
    {
        repeat = DetourDinputPressKeyDownTimes(scancode);
        if(repeat == 0)
        {
            repeat = 1;
        }
    }
    else
    {
        repeat = 1;
    }

    lpMsg->lParam |= (repeat & 0xffff);
    lpMsg->lParam |= (scancode & 0xff) << 16;

    if(scancode == DIK_RCONTROL || scancode == DIK_RMENU)
    {
        lpMsg->lParam |= (1 << 24);
    }
    /*if it is keydown and repeats*/
    if((repeat > 1 && keydown)|| keydown == 0)
    {
        lpMsg->lParam |= (1 << 30);
    }

    if(keydown == 0)
    {
        lpMsg->lParam |= (1 << 31);
    }

    lpMsg->time = GetTickCount();

    lpMsg->pt.x = 0;
    lpMsg->pt.y = 0;
    return 0;

}

int __InsertKeyboardMessageDevEvent(LPDEVICEEVENT pDevEvent)
{
    int ret;
    UINT scancode;
    int keydown;
    int vk;
    int cnt;
    std::vector<MSG> msgs;
    int i;

    if(pDevEvent->devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("KeyBoard(%d) not exist\n",pDevEvent->devid);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.keyboard.event >= KEYBOARD_EVENT_MAX ||
            pDevEvent->event.keyboard.code >= KEYBOARD_CODE_NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> event(%d) code (%d) not valid\n",
                   pDevEvent->event.keyboard.event,
                   pDevEvent->event.keyboard.code);
        SetLastError(ret);
        return -ret;
    }


    scancode = st_CodeMapDik[pDevEvent->event.keyboard.code];
    keydown = (pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN) ? 1 : 0;

    SetLastError(0);
    vk = MapVirtualKeyEmulation(scancode);
    if(vk == 0)
    {
        /*can not find virtual key ,so we should return error*/
        ret = LAST_ERROR_CODE();
        ERROR_INFO("scancode (0x%08x:%d) can not find virtual key Error(%d)\n",scancode,scancode,ret);
        SetLastError(ret);
        return -ret;
    }

    ret =    GetKeyMessage(vk,keydown,msgs);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Virtualkey %d keydown %d Error(%d)\n",vk,keydown,ret);
        SetLastError(ret);
        return -ret;
    }

    cnt = ret;
    assert((int)msgs.size() >= cnt);

    for(i=0; i<cnt; i++)
    {
        ret= InsertEmulationMessageQueue(&(msgs[i]),1);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }
    }

    return cnt;
fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}


int __InsertMouseMessageDevEvent(LPDEVICEEVENT pDevEvent)
{
    MSG Msg= {0};
    UINT message;
    int ret;

    if(pDevEvent->devid != 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        ERROR_INFO("<0x%p> Mouse devid(%d)\n",pDevEvent,pDevEvent->devid);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.mouse.event >= MOUSE_EVENT_MAX ||
            pDevEvent->event.mouse.code >= MOUSE_CODE_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> Mouse event(%d) code(%d) not valid\n",
                   pDevEvent,pDevEvent->event.mouse.event,
                   pDevEvent->event.mouse.code);
        SetLastError(ret);
        return -ret;
    }

    if(pDevEvent->event.mouse.code == MOUSE_CODE_MOUSE)
    {
        if((pDevEvent->event.mouse.event == MOUSE_EVNET_MOVING ||
                pDevEvent->event.mouse.event == MOUSE_EVENT_ABS_MOVING))
        {
            message = WM_MOUSEMOVE;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_SLIDE)
        {
            message = WM_MOUSEHWHEEL;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse event(%d) code(%d) not valid\n",
                       pDevEvent,pDevEvent->event.mouse.event,
                       pDevEvent->event.mouse.code);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            message = WM_LBUTTONDOWN;
            DEBUG_INFO("message leftdown 0x%08x\n",message);
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            message = WM_LBUTTONUP;
            DEBUG_INFO("message leftup 0x%08x\n",message);
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse event(%d) code(%d) not valid\n",
                       pDevEvent,pDevEvent->event.mouse.event,
                       pDevEvent->event.mouse.code);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            message = WM_RBUTTONDOWN;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            message = WM_RBUTTONUP;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse event(%d) code(%d) not valid\n",
                       pDevEvent,pDevEvent->event.mouse.event,
                       pDevEvent->event.mouse.code);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            message = WM_RBUTTONDOWN;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            message = WM_RBUTTONUP;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse event(%d) code(%d) not valid\n",
                       pDevEvent,pDevEvent->event.mouse.event,
                       pDevEvent->event.mouse.code);
            goto fail;
        }
    }
    else
    {
        assert(0!=0);
    }

    ret = __PrepareMouseButtonMessage(&Msg,message);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%p>Prepare code (%d) event(%d) message(0x%08x) Error(%d)\n",
                   pDevEvent,pDevEvent->event.mouse.code,
                   pDevEvent->event.mouse.event,
                   message,ret);
        goto fail;
    }

    if(message == WM_MOUSEHWHEEL)
    {
        int wdelta;
        /*it is wheel ,so we should do this for wheel handle*/
        Msg.wParam &= (0xffff);
        wdelta = (pDevEvent->event.mouse.x << 16);
        Msg.wParam |= (wdelta);
    }

    //DEBUG_INFO("hwnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time(%d) pt.x(%d) pt.y(%d)\n",
    //           Msg.hwnd,
    //           Msg.message,Msg.message,
    //           Msg.wParam,Msg.wParam,
    //           Msg.lParam,Msg.lParam,
    //           Msg.time,Msg.pt.x,Msg.pt.y);

    ret = InsertEmulationMessageQueue(&Msg,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    return 1;
fail:
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}


/*return 0 for not insert ,1 for insert ,negative for error*/
int InsertMessageDevEvent(LPVOID pParam,LPVOID pInput)
{
    LPDEVICEEVENT pDevEvent = (LPDEVICEEVENT)pInput;
    int ret;
    HWND hwnd= NULL;
    /*now to test for the dev event*/
    /*we post message ,for it will give the thread to get message ok*/
    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        return __InsertKeyboardMessageDevEvent(pDevEvent);
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        return  __InsertMouseMessageDevEvent(pDevEvent);
    }

    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("<0x%p>Not Supported devtype(%d)\n",pDevEvent,pDevEvent->devtype);
    SetLastError(ret);
    return -ret;
}

LPMSG __GetEmulationMessageQueue()
{
    LPMSG lGetMsg=NULL;
    static int st_getmessagedebug=0;
    if(st_MessageEmualtionInited)
    {
        EnterCriticalSection(&st_MessageEmulationCS);
        if(st_MessageEmulationQueue.size() > 0)
        {
            lGetMsg = st_MessageEmulationQueue[0];
            st_MessageEmulationQueue.erase(st_MessageEmulationQueue.begin());
        }
        LeaveCriticalSection(&st_MessageEmulationCS);
    }
    else
    {
        return NULL;
    }

    return lGetMsg;

}

int DetourMessageInit(LPVOID pParam,LPVOID pInput)
{
    LPMSG lpMsg=NULL;

    while(1)
    {
        lpMsg = __GetEmulationMessageQueue();
        if(lpMsg == NULL)
        {
            break;
        }
        free(lpMsg);
        lpMsg = NULL;
    }
    return 0;
}


int __SetMessageQuit()
{
    int ret = -1;
    if(st_MessageEmualtionInited)
    {
        EnterCriticalSection(&st_MessageEmulationCS);
        ret = 0;
        st_MessageQuit = 1;
        LeaveCriticalSection(&st_MessageEmulationCS);
    }

    return ret;
}



int __GetKeyMouseMessage(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax,UINT remove)
{
    LPMSG lGetMsg=NULL;
    int ret = 0,res;
    POINT pt;
    static int st_GetMessageCount=0;

    lGetMsg = __GetEmulationMessageQueue();
    if(lGetMsg == NULL)
    {
        ret = 0;
        goto out;
    }
    if(lGetMsg->message == WM_LBUTTONDOWN)
    {
        EMULATIONMESSAGE_DEBUG_INFO("LBUTTONDOWN\n");
    }
    else if(lGetMsg->message == WM_LBUTTONUP)
    {
        EMULATIONMESSAGE_DEBUG_INFO("LBUTTONUP\n");
    }

    //EMULATIONMESSAGE_DEBUG_INFO("lGetMsg 0x%p Message Code(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d)\n",
    //                            lGetMsg,lGetMsg->message,lGetMsg->message,
    //                            lGetMsg->wParam,lGetMsg->wParam,
    //                            lGetMsg->lParam,lGetMsg->lParam);

    /*now to compare whether it is the ok*/
    if(wMsgFilterMin == 0 && wMsgFilterMax == 0)
    {
        CopyMemory(lpMsg,lGetMsg,sizeof(*lGetMsg));
        if(!(remove & PM_REMOVE) && lGetMsg->message != WM_QUIT)
        {
            /*not remove ,so we put back*/
            DEBUG_INFO("ReInsertMsg hwnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d)\n",
                       lGetMsg->hwnd,lGetMsg->message,lGetMsg->message,
                       lGetMsg->wParam,lGetMsg->wParam,
                       lGetMsg->lParam,lGetMsg->lParam);
            res = InsertEmulationMessageQueue(lGetMsg,0);
            assert(res >= 0);
        }
        ret = 1;
    }
    else if((lGetMsg->message >= wMsgFilterMin && lGetMsg->message <= wMsgFilterMax) || lGetMsg->message == WM_QUIT)
    {
        CopyMemory(lpMsg,lGetMsg,sizeof(*lGetMsg));
        if(!(remove & PM_REMOVE) && lGetMsg->message != WM_QUIT)
        {
            /*not remove ,so we put back*/
            res = InsertEmulationMessageQueue(lGetMsg,0);
            assert(res >= 0);
        }
        ret = 1;
    }
    else
    {
        /*now in it ,so we do not use this*/
        DEBUG_INFO("FilterMin %d FilterMax %d\n",wMsgFilterMin,wMsgFilterMax);
        res = InsertEmulationMessageQueue(lGetMsg,0);
        assert(res >= 0);
        ret = 0;
        goto out;
    }

    /*now we should get the mouse value*/
    if(lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST && ret > 0)
    {
        /*we put the mouse pointer here */
        lpMsg->lParam = 0;
        res = DetourDinputScreenMousePoint(hWnd,&pt);
        if(res < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("hWnd(0x%08x) Could not GetScreen mouse point Error(%d)\n",hWnd,ret);
            goto fail;
        }
        lpMsg->lParam |= (0xffff & pt.x);
        lpMsg->lParam |= ((0xffff & pt.y) << 16);
        lpMsg->pt.x = pt.x;
        lpMsg->pt.y = pt.y;
    }
out:
    if(ret > 0)
    {
        if(lpMsg->hwnd == NULL)
        {
            lpMsg->hwnd = GetCurrentProcessActiveWindow();
        }
        //DEBUG_INFO("[%d] hwnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time(%d) pt.x(%d) pt.y(%d)\n",
        //           st_GetMessageCount,lpMsg->hwnd,
        //           lpMsg->message,lpMsg->message,
        //           lpMsg->wParam,lpMsg->wParam,
        //           lpMsg->lParam,lpMsg->lParam,
        //           lpMsg->time,lpMsg->pt.x,lpMsg->pt.y);
    }
    if(lGetMsg)
    {
        free(lGetMsg);
    }
    lGetMsg = NULL;
    //DEBUG_INFO("ret %d\n",ret);
    SetLastError(0);
    return ret;

fail:
    assert(ret > 0);
    if(lGetMsg)
    {
        if(lGetMsg->message != WM_QUIT)
        {
            res = InsertEmulationMessageQueue(lGetMsg,0);
            assert(res >= 0);
        }
        free(lGetMsg);
    }
    lGetMsg = NULL;
    SetLastError(ret);
    return -ret;
}

int __SetWindowRect(HWND hWnd)
{
    return DetourDinputSetWindowsRect(hWnd);
}

BOOL WINAPI GetMessageACallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
)
{
    BOOL bret;
    int ret;

    if(lpMsg == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    __SetWindowRect(hWnd);
try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,PM_REMOVE);
    if(ret > 0)
    {
#if 1
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
#endif
        return TRUE;
    }


    ZeroMemory(lpMsg,sizeof(*lpMsg));
    bret = GetMessageANext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        //DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
        //                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
        //                 lpMsg->wParam,lpMsg->wParam,
        //                 lpMsg->lParam,lpMsg->lParam,
        //                 lpMsg->time,lpMsg->time,
        //                 lpMsg->pt.x,lpMsg->pt.x,
        //                 lpMsg->pt.y,lpMsg->pt.y);
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST) ||
                lpMsg->message == WM_INPUT)
        {
            /*we discard this message*/
            goto try_again;
        }
    }
    return bret;
}


BOOL WINAPI PeekMessageACallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg
)
{
    BOOL bret;
    int ret;

    if(lpMsg == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }
    __SetWindowRect(hWnd);

try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
    if(ret > 0)
    {
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        return TRUE;
    }

    ZeroMemory(lpMsg,sizeof(*lpMsg));
    bret = PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        //DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageA hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
        //                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
        //                 lpMsg->wParam,lpMsg->wParam,
        //                 lpMsg->lParam,lpMsg->lParam,
        //                 lpMsg->time,lpMsg->time,
        //                 lpMsg->pt.x,lpMsg->pt.x,
        //                 lpMsg->pt.y,lpMsg->pt.y);
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST) ||
                lpMsg->message == WM_INPUT)
        {
            if(!(wRemoveMsg & PM_REMOVE))
            {
                /*this means not remove ,so we should do this removed*/
                PeekMessageANext(lpMsg,hWnd,wMsgFilterMin,
                                 wMsgFilterMax,wRemoveMsg|PM_REMOVE);

            }
            goto try_again;
        }
    }


    return bret;
}


BOOL WINAPI GetMessageWCallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
)
{
    BOOL bret;
    int ret;

    if(lpMsg == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    __SetWindowRect(hWnd);
try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,PM_REMOVE);
    if(ret > 0)
    {
        if(0)
            //if(lpMsg->message == WM_LBUTTONDOWN || lpMsg->message == WM_LBUTTONUP)
            //if(lpMsg->message == WM_INPUT)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
        return TRUE;
    }

    ZeroMemory(lpMsg,sizeof(*lpMsg));
    bret = GetMessageWNext(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
    if(bret)
    {
        //DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"GetMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
        //                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
        //                 lpMsg->wParam,lpMsg->wParam,
        //                 lpMsg->lParam,lpMsg->lParam,
        //                 lpMsg->time,lpMsg->time,
        //                 lpMsg->pt.x,lpMsg->pt.x,
        //                 lpMsg->pt.y,lpMsg->pt.y);
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST) ||
                lpMsg->message == WM_INPUT)
        {
            /*we discard this message ,so get the next one*/
            goto try_again;
        }
    }
    return bret;
}


BOOL WINAPI PeekMessageWCallBack(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg
)
{
    BOOL bret;
    int ret;

    if(lpMsg == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    __SetWindowRect(hWnd);
try_again:
    ret = __GetKeyMouseMessage(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax,wRemoveMsg);
    if(ret > 0)
    {
#if 1
        if(lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)
        {
            DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
                             lpMsg->hwnd,lpMsg->message,lpMsg->message,
                             lpMsg->wParam,lpMsg->wParam,
                             lpMsg->lParam,lpMsg->lParam,
                             lpMsg->time,lpMsg->time,
                             lpMsg->pt.x,lpMsg->pt.x,
                             lpMsg->pt.y,lpMsg->pt.y);
        }
#endif
        return TRUE;
    }

    ZeroMemory(lpMsg,sizeof(*lpMsg));
    bret = PeekMessageWNext(lpMsg,hWnd,wMsgFilterMin,
                            wMsgFilterMax,wRemoveMsg);
    if(bret)
    {
        //DEBUG_BUFFER_FMT(lpMsg,sizeof(*lpMsg),"PeekMessageW hWnd(0x%08x) message(0x%08x:%d) wParam(0x%08x:%d) lParam(0x%08x:%d) time (0x%08x:%d) pt(x:0x%08x:%d:y:0x%08x:%d)",
        //                 lpMsg->hwnd,lpMsg->message,lpMsg->message,
        //                 lpMsg->wParam,lpMsg->wParam,
        //                 lpMsg->lParam,lpMsg->lParam,
        //                 lpMsg->time,lpMsg->time,
        //                 lpMsg->pt.x,lpMsg->pt.x,
        //                 lpMsg->pt.y,lpMsg->pt.y);
        if((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST)||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST) ||
                lpMsg->message == WM_INPUT)
        {
            if(!(wRemoveMsg & PM_REMOVE))
            {
                /*this means not remove ,so we should do this removed*/
                PeekMessageWNext(lpMsg,hWnd,wMsgFilterMin,
                                 wMsgFilterMax,wRemoveMsg|PM_REMOVE);

            }
            goto try_again;
        }


    }
    return bret;
}



int __MessageDetour(void)
{
    int ret;
    InitializeCriticalSection(&st_MessageEmulationCS);
    ret = EmulationKeyStateInit();
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("EmulationKeyStateInit Error(%d)\n",ret);
        return -ret;
    }

    ret = RegisterEventListHandler(InsertMessageDevEvent,NULL,MESSAGE_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register EventHandler Error(%d)\n",ret);
        return -ret;
    }

    ret = RegisterEventListInit(DetourMessageInit,NULL,MESSAGE_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register Event Init Error(%d)\n",ret);
        return -ret;
    }

    DEBUG_BUFFER_FMT(GetMessageANext,10,"Before GetMessageANext(0x%p)",GetMessageANext);
    DEBUG_BUFFER_FMT(PeekMessageANext,10,"Before PeekMessageANext(0x%p)",PeekMessageANext);
    DEBUG_BUFFER_FMT(GetMessageWNext,10,"Before GetMessageWNext(0x%p)",GetMessageWNext);
    DEBUG_BUFFER_FMT(PeekMessageWNext,10,"Before PeekMessageWNext(0x%p)",PeekMessageWNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&PeekMessageANext,PeekMessageACallBack);
    DetourAttach((PVOID*)&GetMessageANext,GetMessageACallBack);
    DetourAttach((PVOID*)&PeekMessageWNext,PeekMessageWCallBack);
    DetourAttach((PVOID*)&GetMessageWNext,GetMessageWCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(GetMessageANext,10,"After GetMessageANext(0x%p)",GetMessageANext);
    DEBUG_BUFFER_FMT(PeekMessageANext,10,"After PeekMessageANext(0x%p)",PeekMessageANext);
    DEBUG_BUFFER_FMT(GetMessageWNext,10,"After GetMessageWNext(0x%p)",GetMessageWNext);
    DEBUG_BUFFER_FMT(PeekMessageWNext,10,"After PeekMessageWNext(0x%p)",PeekMessageWNext);
    st_MessageEmualtionInited = 1;
    return 0;
}

