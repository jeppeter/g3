

#include <vector>
#define   DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

#define  DEVICE_GET_INFO   0x1
#define  DEVICE_GET_NAMEA  0x2
#define  DEVICE_GET_NAMEW  0x3

#define  KEY_STATE_SIZE   256

#define  RAW_INPUT_MAX_INPUT_SIZE  20

static CRITICAL_SECTION st_EmulationRawinputCS;
static std::vector<RAWINPUT*> st_KeyRawInputVecs;
static RID_DEVICE_INFO* st_KeyRawInputHandle=NULL;
static uint8_t *st_KeyRawInputName=NULL;
static wchar_t *st_KeyRawInputNameWide=NULL;
static int st_KeyLastInfo=DEVICE_GET_INFO;
static std::vector<RAWINPUT*> st_MouseRawInputVecs;
static RID_DEVICE_INFO* st_MouseRawInputHandle=NULL;
static uint8_t *st_MouseRawInputName=NULL;
static wchar_t *st_MouseRawInputNameWide=NULL;
static int st_MouseLastInfo=DEVICE_GET_INFO;


static CRITICAL_SECTION  st_KeyStateEmulationCS;
static uint16_t st_KeyStateArray[KEY_STATE_SIZE]= {0};
static uint16_t st_AsyncKeyStateArray[KEY_STATE_SIZE]= {0};

RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;
GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoANext=GetRawInputDeviceInfoA;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoWNext=GetRawInputDeviceInfoW;
GetRawInputDeviceListFunc_t GetRawInputDeviceListNext=GetRawInputDeviceList;
GetKeyStateFunc_t GetKeyStateNext=GetKeyState;
GetAsyncKeyStateFunc_t GetAsyncKeyStateNext=GetAsyncKeyState;

#define  DETOURRAWINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  DETOURRAWINPUT_DEBUG_INFO      DEBUG_INFO

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


#define  ASYNC_KEY_PRESSED_STATE   0x8000
#define  ASYNC_KEY_TOGGLED_STATE   0x1

#define  KEY_PRESSED_STATE         0x8000
#define  KEY_TOGGLE_STATE          0x1

int SetKeyState(UINT vsk,int keydown)
{
    UINT vk;
    vk = MapVirtualKey(vsk,MAPVK_VSC_TO_VK_EX);
    if(vk == 0)
    {
        /*if not return ,just not set*/
        return 0;
    }
    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(keydown)
    {
        st_KeyStateArray[vk] |= KEY_PRESSED_STATE;

        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_PRESSED_STATE;
        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_PRESSED_STATE;
    }
    else
    {
        st_KeyStateArray[vk] &=~(KEY_PRESSED_STATE) ;
        if(st_KeyStateArray[vk] & KEY_TOGGLE_STATE)
        {
            st_KeyStateArray[vk] &= ~(KEY_TOGGLE_STATE);
        }
        else
        {
            st_KeyStateArray[vk] |= KEY_TOGGLE_STATE;
        }

        st_AsyncKeyStateArray[vk] &= ~(ASYNC_KEY_PRESSED_STATE);
        st_AsyncKeyStateArray[vk] |= ASYNC_KEY_TOGGLED_STATE;

    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return 0;
}


USHORT __InnerGetKeyState(UINT vk)
{
    USHORT uret;
    UINT uvk[2];
    UINT exvk[2];
    int expanded=0;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }

    if(vk == VK_CONTROL)
    {
        expanded = 1;
        exvk[0] = VK_LCONTROL;
        exvk[1] = VK_RCONTROL;
    }
    else if(vk == VK_MENU)
    {
        expanded = 1;
        exvk[0] = VK_LMENU;
        exvk[1] = VK_RMENU;
    }
    else if(vk == VK_SHIFT)
    {
        expanded = 1;
        exvk[0] = VK_LSHIFT;
        exvk[1] = VK_RSHIFT;
    }

    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(expanded)
    {
        uvk[0] = st_KeyStateArray[exvk[0]];
        uvk[1] = st_KeyStateArray[exvk[1]];

        /*
        	we call the last more key value ,so it will return for more
        */
        if(uvk[0] > uvk[1])
        {
            uret = uvk[0];
        }
        else
        {
            uret = uvk[1];
        }

    }
    else
    {
        uret = st_KeyStateArray[vk];
    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return uret;
}


USHORT __InnerGetAsynState(UINT vk)
{
    USHORT uret;
    UINT uvk[2];
    UINT exvk[2];
    int expanded=0;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }
    if(vk == VK_CONTROL)
    {
        expanded = 1;
        exvk[0] = VK_LCONTROL;
        exvk[1] = VK_RCONTROL;
    }
    else if(vk == VK_MENU)
    {
        expanded = 1;
        exvk[0] = VK_LMENU;
        exvk[1] = VK_RMENU;
    }
    else if(vk == VK_SHIFT)
    {
        expanded = 1;
        exvk[0] = VK_LSHIFT;
        exvk[1] = VK_RSHIFT;
    }
    EnterCriticalSection(&st_KeyStateEmulationCS);
    if(expanded)
    {
        uvk[0] = (st_AsyncKeyStateArray[exvk[0]] & 0xffff);
        uvk[1] = (st_AsyncKeyStateArray[exvk[1]] & 0xffff);
        st_AsyncKeyStateArray[exvk[0]] &= ~(ASYNC_KEY_TOGGLED_STATE);
        st_AsyncKeyStateArray[exvk[1]] &= ~(ASYNC_KEY_TOGGLED_STATE);

        if(uvk[0] > uvk[1])
        {
            uret = uvk[0];
        }
        else
        {
            uret = uvk[1];
        }
    }
    else
    {
        uret = st_AsyncKeyStateArray[vk];
        st_AsyncKeyStateArray[vk] &= ~(ASYNC_KEY_TOGGLED_STATE);
    }
    LeaveCriticalSection(&st_KeyStateEmulationCS);
    return uret;
}


SHORT WINAPI GetKeyStateCallBack(
    int nVirtKey
)
{
    SHORT sret;

    sret = __InnerGetKeyState(nVirtKey);
    return sret;
}

SHORT WINAPI GetAsyncKeyStateCallBack(
    int vKey
)
{
    SHORT sret;

    sret = __InnerGetAsynState(vKey);
    return sret;
}


LONG __InsertKeyboardInput(RAWINPUT* pInput)
{
    LONG lret=0;
    RAWINPUT *pRemove=NULL;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_KeyRawInputHandle)
    {
        pInput->header.hDevice = st_KeyRawInputHandle;
        pInput->header.wParam = RIM_INPUT;
        lret =(LONG) st_KeyRawInputHandle;
        if(st_KeyRawInputVecs.size() >= RAW_INPUT_MAX_INPUT_SIZE)
        {
            pRemove = st_KeyRawInputVecs[0];
            st_KeyRawInputVecs.erase(st_KeyRawInputVecs.begin());
        }
        st_KeyRawInputVecs.push_back(pInput);
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(pRemove)
    {
        free(pRemove);
    }
    pRemove = NULL;
    return lret;
}


int __RawInputInsertKeyboardEvent(LPDEVICEEVENT pDevEvent)
{
    RAWINPUT* pKeyInput=NULL;
    int ret;
    int scank;
    int vk;
    LONG lparam;
    MSG InputMsg= {0};

    if(pDevEvent->event.keyboard.code >= 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> code (%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.code);
        goto fail;
    }

    pKeyInput = (RAWINPUT*)calloc(sizeof(*pKeyInput),1);
    if(pKeyInput == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pKeyInput->header.dwType = RIM_TYPEKEYBOARD;
    pKeyInput->header.dwSize = sizeof(pKeyInput->header) + sizeof(pKeyInput->data.keyboard);

    scank = st_CodeMapDik[pDevEvent->event.keyboard.code];
    if(scank == DIK_NULL)
    {
        /*it is not valid ,so we do not insert into it*/
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> code (%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.code);
        goto fail;
    }

    vk = MapVirtualKey(scank,MAPVK_VSC_TO_VK_EX);
    if(vk == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> code (%d) => scank (%d) => vk not valid\n",pDevEvent,pDevEvent->event.keyboard.code,scank);
        goto fail;
    }

    /*now we put the key into the keyboard*/
    pKeyInput->MakeCode = scank;
    if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN)
    {
        pKeyInput->Flags = RI_KEY_MAKE;
        pKeyInput->Message = WM_KEYDOWN ;
    }
    else if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_UP)
    {
        pKeyInput->Flags = RI_KEY_BREAK;
        pKeyInput->Message = WM_KEYUP ;
    }
    else
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> event (%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.event);
        goto fail;
    }
	/*if we are not successful in the insertkeyboardinput ,we can record the keydown or keyup event*/
    SetKeyState(vk,pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN ? 1 : 0);

    pKeyInput->Reserved = 0;
    pKeyInput->VKey = vk;
    pKeyInput->ExtraInformation = 0;

    lparam = __InsertKeyboardInput(pKeyInput);
    if(lparam == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        goto fail;
    }

    /*now to input key state*/

    pKeyInput = NULL;
    InputMsg.hwnd = NULL ;
    InputMsg.message = WM_INPUT;
    InputMsg.wParam = RIM_INPUT;
    InputMsg.lParam = lparam;
    InputMsg.time = GetTickCount();
    InputMsg.pt.x = 0;
    InputMsg.pt.y = 0;

    ret = InsertEmulationMessageQueue(&InputMsg,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
	
    return 0;
fail:
    assert(ret > 0);
    if(pKeyInput)
    {
        free(pKeyInput);
    }
    pKeyInput = NULL;
    SetLastError(ret);
    return -ret;
}

LONG __InsertMouseInput(RAWINPUT * pInput)
{
    LONG lret=0;
    RAWINPUT *pRemove=NULL;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_MouseRawInputHandle)
    {
        lret = (LONG) st_MouseRawInputHandle;
        pInput->hDevice = (HANDLE) st_MouseRawInputHandle;
        pInput->wParam = RIM_INPUT;
        if(st_MouseRawInputVecs.size() >= RAW_INPUT_MAX_INPUT_SIZE)
        {
            pRemove = st_MouseRawInputVecs[0];
            st_MouseRawInputVecs.erase(st_MouseRawInputVecs.begin());
        }
        st_MouseRawInputVecs.push_back(pInput);
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pRemove)
    {
        free(pRemove);
    }
    pRemove = NULL;
    return lret;
}

int __RawInputInsertMouseEvent(LPDEVICEEVENT pDevEvent)
{
    RAWINPUT* pMouseInput=NULL;
    int ret;
    LONG lparam;
    MSG InputMsg= {0};
    POINT pt;


    pMouseInput = calloc(sizeof(*pMouseInput),1);
    if(pMouseInput == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pMouseInput->header.dwType = RIM_TYPEMOUSE;
    pMouseInput->header.dwSize = sizeof(pMouseInput->header) + sizeof(pMouseInput->mouse);

    pMouseInput->mouse.usFlags = MOUSE_MOVE_ABSOLUTE;
    pMouseInput->mouse.ulButtons = 0;
    DetourDinput8GetMousePointAbsolution(&pt);
    if(pDevEvent->event.mouse.code == MOUSE_CODE_MOUSE)
    {
        /*no buttons push*/
        if(pDevEvent->event.mouse.event == MOUSE_EVNET_MOVING ||
                pDevEvent->event.mouse.event == MOUSE_EVENT_ABS_MOVING)
        {
            pMouseInput->mouse.usButtonFlags = 0;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_SLIDE)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_WHEEL;
            pMouseInput->mouse.usButtonData = pDevEvent->event.mouse.x;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_DOWN;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_UP;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_DOWN;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_UP;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_DOWN;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_UP;
            pMouseInput->mouse.usButtonData = 0;
            pMouseInput->mouse.ulRawButtons = 0;
            pMouseInput->mouse.lLastX = pt.x;
            pMouseInput->mouse.lLastY = pt.y;
            pMouseInput->mouse.ulExtraInformation = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }

    lparam = __InsertMouseInput(pMouseInput);
    if(lparam == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        goto fail;
    }
    pMouseInput = NULL;

    InputMsg.hwnd = NULL ;
    InputMsg.message = WM_INPUT;
    InputMsg.wParam = RIM_INPUT;
    InputMsg.lParam = lparam;
    InputMsg.time = GetTickCount();
    InputMsg.pt.x = 0;
    InputMsg.pt.y = 0;

    ret = InsertEmulationMessageQueue(&InputMsg,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    return 0;
fail:
    assert(ret > 0);
    if(pMouseInput)
    {
        free(pMouseInput);
    }
    pMouseInput = NULL;
    SetLastError(ret);
    return -ret;

}


static int RawInputEmulationInsertEventList(LPVOID pParam,LPVOID pInput)
{
    LPDEVICEEVENT pDevEvent = (LPDEVICEEVENT)pInput;
    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        return __RawInputInsertKeyboardEvent(pDevEvent);
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        return __RawInputInsertMouseEvent(pDevEvent);
    }
    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("<0x%p>devtype %d not supported\n",pDevEvent,pDevEvent->devtype);
    SetLastError(ret);
    return -ret;

}

void __UnRegisterKeyboardHandle()
{
    RID_DEVICE_INFO *pKeyboardInfo=NULL;
    uint8_t *pKeyName=NULL;
    wchar_t *pKeyUnicode=NULL;
    std::vector<RAWINPUT*> removekeyrawinput;
    RAWINPUT *pRemoveInput=NULL;
    EnterCriticalSection(&st_EmulationRawinputCS);
    pKeyboardInfo = st_KeyRawInputHandle;
    pKeyName = st_KeyRawInputName;
    pKeyUnicode = st_KeyRawInputNameWide;
    st_KeyRawInputHandle = NULL;
    st_KeyRawInputName = NULL;
    st_KeyRawInputNameWide = NULL;
    removekeyrawinput = st_KeyRawInputVecs;
    st_KeyRawInputVecs.clear();
    LeaveCriticalSection(&st_EmulationRawinputCS);

    while(removekeyrawinput.size() > 0)
    {
        pRemoveInput = removekeyrawinput[0];
        removekeyrawinput.erase(removekeyrawinput.begin());
        free(pRemoveInput);
        pRemoveInput = NULL;
    }
    if(pKeyboardInfo)
    {
        free(pKeyboardInfo);
    }
    pKeyboardInfo = NULL;
    if(pKeyName)
    {
        free(pKeyName);
    }
    pKeyName = NULL;
    if(pKeyUnicode)
    {
        free(pKeyUnicode);
    }
    pKeyUnicode = NULL;
    return ;
}

HANDLE __RegisterKeyboardHandle()
{
    RID_DEVICE_INFO *pKeyboardInfo=NULL,*pAllocInfo=NULL;
    uint8_t *pKeyName=NULL;
    wchar_t *pKeyUnicode=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pKeyboardInfo = st_KeyRawInputHandle;
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pKeyboardInfo == NULL)
    {
        pAllocInfo = calloc(sizeof(*pAllocInfo),1);
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pKeyName = calloc(1,256);
        if(pKeyName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s(pKeyName,256,"\\\\?\\KeyBoard_Emulate",_TRUNCATE);
        pKeyUnicode = calloc(2,256);
        if(pKeyUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pKeyName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pKeyName,256,L"\\\\?\\KeyBoard_Emulate",_TRUNCATE);

        pAllocInfo->cbSize = sizeof(pAllocInfo->header) + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEKEYBOARD;
        pAllocInfo->keyboard.dwType = 7;
        pAllocInfo->keyboard.dwSubType = 0;
        pAllocInfo->keyboard.dwKeyboardMode = 1;
        pAllocInfo->keyboard.dwNumberOfFunctionKeys = 12;
        pAllocInfo->keyboard.dwNumberOfIndicators = 3;
        pAllocInfo->keyboard.dwNumberOfKeysTotal = 101;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_KeyRawInputHandle == NULL)
        {
            inserted = 1;
            st_KeyRawInputHandle = pAllocInfo;
            pKeyboardInfo = pAllocInfo;
            st_KeyRawInputName = pKeyName;
            st_KeyRawInputNameWide = pKeyUnicode;
        }
        else
        {
            pKeyboardInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pKeyboardInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
            free(pKeyName);
            pKeyName = NULL;
            free(pKeyUnicode);
            pKeyUnicode = NULL;
        }
    }

    return pKeyboardInfo;
}


void __UnRegisterMouseHandle()
{
    RID_DEVICE_INFO *pMouseInfo=NULL;
    uint8_t *pMouseName=NULL;
    wchar_t *pMouseUnicode=NULL;
    std::vector<RAWINPUT*> removemouserawinput;
    RAWINPUT *pRemoveInput=NULL;
    EnterCriticalSection(&st_EmulationRawinputCS);
    pMouseInfo = st_MouseRawInputHandle;
    pMouseName = st_MouseRawInputName;
    pMouseUnicode = st_MouseRawInputNameWide;
    st_MouseRawInputHandle = NULL;
    st_MouseRawInputName = NULL;
    st_MouseRawInputNameWide = NULL;
    removemouserawinput = st_MouseRawInputVecs;
    st_MouseRawInputVecs.clear();
    LeaveCriticalSection(&st_EmulationRawinputCS);

    while(removemouserawinput.size() > 0)
    {
        pRemoveInput = removemouserawinput[0];
        removemouserawinput.erase(removemouserawinput.begin());
        free(pRemoveInput);
        pRemoveInput = NULL;
    }
    if(pMouseInfo)
    {
        free(pMouseInfo);
    }
    pMouseInfo = NULL;
    if(pMouseName)
    {
        free(pMouseName);
    }
    pMouseName = NULL;
    if(pMouseUnicode)
    {
        free(pMouseUnicode);
    }
    pMouseUnicode = NULL;
    return ;
}

HANDLE __RegisterMouseHandle()
{
    RID_DEVICE_INFO *pMouseInfo=NULL,*pAllocInfo=NULL;
    uint8_t *pMouseName=NULL;
    wchar_t *pMouseUnicode=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pMouseInfo = st_MouseRawInputHandle;
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pMouseInfo == NULL)
    {
        pAllocInfo = calloc(sizeof(*pAllocInfo),1);
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pMouseName = calloc(1,256);
        if(pMouseName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s(pMouseName,256,"\\\\?\\Mouse_Emulate",_TRUNCATE);
        pMouseUnicode = calloc(2,256);
        if(pMouseUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pMouseName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pMouseName,256,L"\\\\?\\Mouse_Emulate",_TRUNCATE);

        pAllocInfo->cbSize = sizeof(pAllocInfo->header) + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEMOUSE;
        pAllocInfo->mouse.dwId = 256;
        pAllocInfo->mouse.dwNumberOfButtons = 3;
        pAllocInfo->mouse.dwSampleRate = 0;
        pAllocInfo->mouse.fHasHorizontalWheel = 0;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_MouseRawInputHandle == NULL)
        {
            inserted = 1;
            st_MouseRawInputHandle = pAllocInfo;
            pMouseInfo = pAllocInfo;
            st_MouseRawInputName = pMouseName;
            st_MouseRawInputNameWide = pMouseUnicode;
        }
        else
        {
            pMouseInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pMouseInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
            free(pMouseName);
            pMouseName = NULL;
            free(pMouseUnicode);
            pMouseUnicode = NULL;
        }
    }

    return pMouseInfo;
}

int __CopyKeyboardDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret=0;
    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_KeyRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_KeyRawInputHandle;
        pRawList->dwType = RIM_TYPEKEYBOARD;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;


}

int __CopyMouseDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret=0;

    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_MouseRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_MouseRawInputHandle;
        pRawList->dwType = RIM_TYPEMOUSE;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;
}


BOOL __GetDeviceInfoNoLock(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    RID_DEVICE_INFO *pGetInfo;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        ret = 0 ;
        bret = TRUE;
        st_KeyLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = sizeof(pInfo->header) + sizeof(pInfo->keyboard);
        pInfo->dwType = RIM_TYPEKEYBOARD;
        CopyMemory(&(pInfo->keyboard),&(st_KeyRawInputHandle->keyboard),sizeof(pInfo->keyboard));
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        ret = 0;
        bret = TRUE;
        st_MouseLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = sizeof(pInfo->header) + sizeof(pInfo->mouse);
        pInfo->dwType = RIM_TYPEMOUSE;
        CopyMemory(&(pInfo->mouse),&(st_MouseRawInputHandle->mouse),sizeof(pInfo->mouse));
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceInfo(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceInfoNoLock(hDevice,pInfo);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameANoLock(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen(st_KeyRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen(st_KeyRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen(st_KeyRawInputName) + 1;
            strncpy_s(pData,*pcbSize,st_KeyRawInputName,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen(st_MouseRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen(st_MouseRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen(st_MouseRawInputName) + 1;
            strncpy_s(pData,*pcbSize,st_MouseRawInputName,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceNameA(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameWNoLock(HANDLE hDevice,void * pData,UINT * pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_KeyRawInputName)*2 +1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_KeyRawInputName)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_KeyRawInputName)*2 + 2;
            wcsncpy_s(pData,(*pcbSize)/2,st_KeyRawInputName,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_MouseRawInputName)*2+1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_MouseRawInputName)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_MouseRawInputName)*2 + 2;
            wcsncpy_s(pData,(*pcbSize)/2,st_MouseRawInputName,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}

int __GetRawInputDeviceNum()
{
    int retnum=0;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_KeyRawInputHandle)
    {
        retnum ++;
    }
    if(st_MouseRawInputHandle)
    {
        retnum ++;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return retnum;
}


BOOL __GetDeviceNameW(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

int __GetDeviceInfoLast(HANDLE hDevice,void* pData,UINT* pcbSize)
{
    BOOL bret = FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    int copiedlen=-1;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(hDevice == (HANDLE) st_KeyRawInputHandle)
    {
        if(st_KeyLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = strlen(pData);
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = wcslen(pData)*2;
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                }
                else
                {
                    copiedlen = sizeof(RID_DEVICE_INFO);
                    ret = 0;
                }
            }
        }
        else
        {
            /*we could not arrive here*/
            assert(0!=0);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        if(st_MouseLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = strlen(pData);
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = wcslen(pData)*2;
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                }
                else
                {
                    copiedlen = sizeof(RID_DEVICE_INFO);
                    ret = 0;
                }
            }
        }
        else
        {
            /*we could not arrive here*/
            assert(0!=0);
        }
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(copiedlen < 0)
    {
        SetLastError(ret);
    }
    return copiedlen;
}

BOOL WINAPI RegisterRawInputDevicesCallBack(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
)
{
    RAWINPUTDEVICE *pDevice=NULL;
    int ret;
    UINT i;
    RID_DEVICE_INFO *pMouse=NULL,*pKeyBoard=NULL;



    /*now first check for device is supported*/
    if(cbSize != sizeof(*pDevice) || pRawInputDevices == NULL || uiNumDevices == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    for(i=0; i<uiNumDevices; i++)
    {
        pDevice = &(pRawInputDevices[i]);
        /*we only supported keyboard and mouse ,others are not supported*/
        if(pDevice->usUsagePage != 0x1 || (pDevice->usUsage != 0x2 && pDevice->usUsage != 0x6))
        {
            ret = ERROR_NOT_SUPPORTED;
            SetLastError(ret);
            return FALSE;
        }

        if(pDevice->usUsage == 2)
        {
            pMouse = __RegisterMouseHandle();
            if(pMouse == NULL)
            {
                ret = LAST_ERROR_CODE();
                if(pKeyBoard)
                {
                    __UnRegisterKeyboardHandle();
                }
                pKeyBoard = NULL;
                SetLastError(ret);
                return FALSE;
            }
        }

        if(pDevice->usUsage == 0x6)
        {
            pKeyBoard = __RegisterKeyboardHandle();
            if(pKeyBoard== NULL)
            {
                ret = LAST_ERROR_CODE();
                if(pMouse)
                {
                    __UnRegisterMouseHandle();
                }
                pMouse = NULL;
                SetLastError(ret);
                return FALSE;
            }
        }
    }

    /*now all is ok  ,so we do not need any more*/
    return TRUE;
}


UINT WINAPI GetRawInputDeviceListCallBack(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize
)
{
    UINT uret;
    int ret;
    int retnum=0;
    int num=0;

    /*now check for the input */
    if(puiNumDevices == NULL || cbSize != sizeof(*pRawInputDeviceList))
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    num = *puiNumDevices;

    if(*puiNumDevices < 2 || pRawInputDeviceList == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        *puiNumDevices = 2;
        SetLastError(ret);
        return (UINT) -1;
    }

    retnum = __GetRawInputDeviceNum();
    *puiNumDevices = retnum;
    if(retnum == 0)
    {
        return 0;
    }

    num = 0;
    /*now we should copy the memory*/
    ret = __CopyKeyboardDeviceList(&(pRawInputDeviceList[num]));
    if(ret >= 0)
    {
        num ++;
    }

    if(num >= retnum)
    {
        ret = ERROR_INSUFFICIENT_BUFFER;
        *puiNumDevices = num + 1;
        SetLastError(ret);
        return (UINT) -1;
    }

    ret = __CopyMouseDeviceList(&(pRawInputDeviceList[num]));
    if(ret >=  0)
    {
        num ++;
    }
    *puiNumDevices = num;

    return num;
}

UINT WINAPI GetRawInputDeviceInfoACallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    BOOL bret;
    int ret;

    if(pcbSize == NULL || hDevice == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }
    /*now first to copy the data*/
    if(uiCommand == RIDI_DEVICENAME)
    {
        bret= __GetDeviceNameA(hDevice,pData,pcbSize);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return strlen(pData);
    }
    else if(uiCommand == RIDI_DEVICEINFO)
    {
        if(*pcbSize != sizeof(RID_DEVICE_INFO))
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }

        bret= __GetDeviceInfo(hDevice,(RID_DEVICE_INFO*)pData);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return sizeof(RID_DEVICE_INFO);
    }
    else if(uiCommand == RIDI_PREPARSEDDATA)
    {
        ret = __GetDeviceInfoLast(hDevice,pData,pcbSize);
        return (UINT)ret;
    }

    /*now nothing find ,so we should return not supported*/
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return (UINT) -1;
}


UINT WINAPI GetRawInputDeviceInfoWCallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    BOOL bret;
    int ret;

    if(pcbSize == NULL || hDevice == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    /*now first to copy the data*/
    if(uiCommand == RIDI_DEVICENAME)
    {
        bret= __GetDeviceNameW(hDevice,pData,pcbSize);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return (wcslen(pData)*2);
    }
    else if(uiCommand == RIDI_DEVICEINFO)
    {
        if(*pcbSize != sizeof(RID_DEVICE_INFO))
        {
            ret = ERROR_INVALID_PARAMETER;
            *pcbSize = sizeof(RID_DEVICE_INFO);
            SetLastError(ret);
            return (UINT) -1;
        }

        bret= __GetDeviceInfo(hDevice,(RID_DEVICE_INFO*)pData);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return sizeof(RID_DEVICE_INFO);
    }
    else if(uiCommand == RIDI_PREPARSEDDATA)
    {
        ret = __GetDeviceInfoLast(hDevice,pData,pcbSize);
        return (UINT)ret;
    }

    /*now nothing find ,so we should return not supported*/
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return (UINT) -1;
}

UINT __GetRawInputDataNoLock(HRAWINPUT hRawInput,
                             UINT uiCommand,
                             LPVOID pData,
                             PUINT pcbSize,
                             UINT cbSizeHeader)
{
    RAWINPUT *pRawInput=NULL;
    int ret;
    if(pData == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        if(uiCommand == RID_HEADER)
        {
            *pcbSize = sizeof(pRawInput->header);
        }
        else if(uiCommand == RID_INPUT)
        {
            if(hRawInput == (HRAWINPUT)st_KeyRawInputHandle)
            {
                *pcbSize = sizeof(pRawInput->header) + sizeof(pRawInput->keyboard);
            }
            else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
            {
                *pcbSize = sizeof(pRawInput->header) + sizeof(pRawInput->mouse);
            }
            else
            {
                ret = ERROR_DEV_NOT_EXIST;
            }
        }
        else
        {
            ret = ERROR_NOT_SUPPORTED;
        }
        SetLastError(ret);
        return (UINT) -1;
    }

    if(uiCommand == RID_HEADER)
    {
        if(*pcbSize < sizeof(pRawInput->header))
        {
            ret = ERROR_INSUFFIENT_BUFFER;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }

        if(cbSizeHeader != sizeof(pRawInput->header))
        {
            ret = ERROR_INVALID_PARAMETER;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }

        if(hRawInput == (HRAWINPUT) st_KeyRawInputHandle)
        {
            if(st_KeyRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No keyboard data for <0x%p>\n",st_KeyRawInputHandle);
                *pcbSize = sizeof(pRawInput->header);
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_KeyRawInputVecs[0];
            *pcbSize = sizeof(pRawInput->header);
            /*not to remove the vectors ,for next read*/
            CopyMemory(pData,&(pRawInput->header),sizeof(pRawInput->header));
            return sizeof(pRawInput->header);
        }
        else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
        {
            if(st_MouseRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No Mouse data for <0x%p>\n",st_MouseRawInputHandle);
                *pcbSize = sizeof(pRawInput->header);
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_MouseRawInputVecs[0];
            *pcbSize = sizeof(pRawInput->header);
            /*not to remove the vectors ,for next read*/
            CopyMemory(pData,&(pRawInput->header),sizeof(pRawInput->header));
            return sizeof(pRawInput->header);
        }
        else
        {
            ret = ERROR_DEV_NOT_EXIST;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }
    }
    else if(uiCommand == RID_INPUT)
    {

        if(hRawInput == (HRAWINPUT) st_KeyRawInputHandle)
        {
            if(*pcbSize < (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard)))
            {
                ret = ERROR_INSUFFIENT_BUFFER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(cbSizeHeader != (sizeof(pRawInput->header)))
            {
                ret = ERROR_INVALID_PARAMETER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(st_KeyRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No keyboard data for <0x%p>\n",st_KeyRawInputHandle);
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_KeyRawInputVecs[0];
            /*remove this input handle*/
            st_KeyRawInputVecs.erase(st_KeyRawInputVecs.begin());
            CopyMemory(pData,pRawInput,(sizeof(pRawInput->header)+sizeof(pRawInput->keyboard)));
            *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard));
            /*free memory ,and not let it memory leak*/
            free(pRawInput);
            pRawInput = NULL;
            return (sizeof(pRawInput->header)+sizeof(pRawInput->keyboard));
        }
        else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
        {
            if(*pcbSize < (sizeof(pRawInput->header)+sizeof(pRawInput->mouse)))
            {
                ret = ERROR_INSUFFIENT_BUFFER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(cbSizeHeader != (sizeof(pRawInput->header)))
            {
                ret = ERROR_INVALID_PARAMETER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(st_MouseRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No Mouse data for <0x%p>\n",st_MouseRawInputHandle);
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_MouseRawInputVecs[0];
            st_MouseRawInputVecs.erase(st_MouseRawInputVecs.begin());
            CopyMemory(pData,pRawInput,(sizeof(pRawInput->header)+sizeof(pRawInput->mouse)));
            *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->mouse));
            /*free memory ,and not let it memory leak*/
            free(pRawInput);
            pRawInput = NULL;
            return (sizeof(pRawInput->header)+sizeof(pRawInput->mouse));
        }
        else
        {
            ret = ERROR_DEV_NOT_EXIST;
            ERROR_INFO("hRawInput<0x%08x> not exist\n",hRawInput);
            SetLastError(ret);
            return (UINT) -1;
        }
    }
    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("Not Supported uiCommand 0x%08x\n",uiCommand);
    SetLastError(ret);
    return (UINT) -1;
}


UINT WINAPI GetRawInputDataCallBack(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader)
{
    UINT uret;
    int ret;

    if(pcbSize == NULL || hRawInput == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    EnterCriticalSection(&st_EmulationRawinputCS);
    uret = __GetRawInputDataNoLock(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return uret;
}


int __RawInputDetour(void)
{
    int ret;
    InitializeCriticalSection(&st_EmulationRawinputCS);
    InitializeCriticalSection(&st_KeyStateEmulationCS);
    ret = RegisterEventListHandler(RawInputEmulationInsertEventList,NULL,RAWINPUT_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register Rawinput Emulation Error(%d)\n",ret);
        return ret;
    }
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"Before RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"Before GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"Before GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"Before GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"Before GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"Before GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"Before GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&RegisterRawInputDevicesNext,RegisterRawInputDevicesCallBack);
    DetourAttach((PVOID*)&GetRawInputDataNext,GetRawInputDataCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoANext,GetRawInputDeviceInfoACallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoWNext,GetRawInputDeviceInfoWCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceListNext,GetRawInputDeviceListCallBack);
    DetourAttach((PVOID*)&GetKeyStateNext,GetKeyStateCallBack);
    DetourAttach((PVOID*)&GetAsyncKeyStateNext,GetAsyncKeyStateCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"After RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"After GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"After GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"After GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"After GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"After GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"After GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    return 0;
}



