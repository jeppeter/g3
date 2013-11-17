

class CDirectInputDevice8AHook;
class CDirectInputDevice8WHook;


static std::vector<IDirectInputDevice8A*> st_Key8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_Key8AHookVecs;
static std::vector<IDirectInputDevcie8A*> st_Mouse8AVes;
static std::vector<CDirectInputDevice8AHook*> st_Mouse8AHookVecs;
static std::vector<IDirectInputDevice8A*> st_NotSet8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_NotSet8AHookVecs;


static std::vector<IDirectInputDevice8W*> st_Key8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_Key8WHookVecs;
static std::vector<IDirectInputDevice8W*> st_Mouse8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_Mouse8WHookVecs;
static std::vector<IDirectInputDevice8W*> st_NotSet8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_NotSet8WHookVecs;

static CRITICAL_SECTION st_Dinput8DeviceCS;

#define IS_IID_MOUSE(riid)  (( (riid)	== GUID_SysMouse ||(riid) == GUID_SysMouseEm ||(riid) == GUID_SysMouseEm2 ))
#define IS_IID_KEYBOARD(riid) (((riid) == GUID_SysKeyboard ||(riid) == GUID_SysKeyboardEm ||(riid) == GUID_SysKeyboardEm2))

#define EQUAL_DEVICE_8A_VECS() \
do\
{\
	assert(st_Key8AVecs.size() == st_Key8AHookVecs.size());\
	assert(st_Mouse8AVes.size() == st_Mouse8AHookVecs.size());\
	assert(st_NotSet8AVecs.size() == st_NotSet8AHookVecs.size());\
}while(0)


#define  EQUAL_DEVICE_8W_VECS()  \
do\
{\
	assert(st_Key8WVecs.size() == st_Key8WHookVecs.size());\
	assert(st_Mouse8WVecs.size() == st_Mouse8WHookVecs.size());\
	assert(st_NotSet8WVecs.size() == st_NotSet8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInputDevice8AHook(IDirectInputDevice8A* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8A_VECS();

    findidx = -1;
    for(i=0; i<st_Key8AVecs.size() ; i++)
    {
        if(st_Key8AVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_Key8AHookVecs.erase(st_Key8AHookVecs.begin() + findidx);
        st_Key8AVecs.erase(st_Key8AVecs.begin() + findidx);
    }
    else
    {
        findidx = -1;
        for(i=0; i<st_Mouse8AVes.size() ; i++)
        {
            if(st_Mouse8AVes[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            st_Mouse8AVes.erase(st_Mouse8AVes.begin() + findidx);
            st_Mouse8AHookVecs.erase(st_Mouse8AHookVecs.begin() + findidx);
        }
        else
        {
            for(i=0; i<st_NotSet8AVecs.size(); i++)
            {
                if(st_NotSet8AVecs[i] == ptr)
                {
                    findidx = i;
                    break;
                }
            }

            if(findidx >= 0)
            {
                st_NotSet8AVecs.erase(st_NotSet8AVecs.begin() + findidx);
                st_NotSet8AHookVecs.erase(st_NotSet8AHookVecs.begin() + findidx);
            }
            else
            {
                ERROR_INFO("<0x%p> not found for dinput device\n",ptr);
            }
        }
    }
    LeaveCriticalSection(&st_Dinput8DeviceCS);


    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}

static void IoFreeEventList(EVENT_LIST_t* pEventList)
{
    return ;
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
    DIK_COMMA         ,DIK_PERIOD         ,DIK_SLASH          ,DIK_MULTIPLY       ,DIK_RSHIFT         ,  /*55*/
    DIK_LMENU         ,DIK_SPACE          ,DIK_CAPITAL        ,DIK_F1             ,DIK_F2             ,  /*60*/
    DIK_F3            ,DIK_F4             ,DIK_F5             ,DIK_F6             ,DIK_F7             ,  /*65*/
    DIK_F8            ,DIK_F9             ,DIK_F10            ,DIK_F11            ,DIK_F12            ,  /*70*/
    DIK_NUMLOCK       ,DIK_SCROLL         ,DIK_NUMPAD7        ,DIK_NUMPAD8        ,DIK_NUMPAD9        ,  /*75*/
    DIK_SUBTRACT      ,DIK_NUMPAD4        ,DIK_NUMPAD5        ,DIK_NUMPAD6        ,DIK_ADD            ,  /*80*/
    DIK_NUMPAD1       ,DIK_NUMPAD2        ,DIK_NUMPAD3        ,DIK_NUMPAD0        ,DIK_DECIMAL        ,  /*85*/
    DIK_OEM_102       ,DIK_F13            ,DIK_F14            ,DIK_F15            ,DIK_KANA           ,  /*90*/
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
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,
    DIK_NULL
};


static int st_DIKMapCode[256] =
{
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_ESCAPE              ,KEYBOARD_CODE_1                      ,KEYBOARD_CODE_2                 ,KEYBOARD_CODE_3                   ,  /*5*/
    KEYBOARD_CODE_4                  ,KEYBOARD_CODE_5                   ,KEYBOARD_CODE_6                      ,KEYBOARD_CODE_7                 ,KEYBOARD_CODE_8                   ,  /*10*/
    KEYBOARD_CODE_9                  ,KEYBOARD_CODE_0                   ,KEYBOARD_CODE_MINUS                  ,KEYBOARD_CODE_EQUALS            ,KEYBOARD_CODE_BACK                ,  /*15*/
    KEYBOARD_CODE_TAB                ,KEYBOARD_CODE_Q                   ,KEYBOARD_CODE_W                      ,KEYBOARD_CODE_E                 ,KEYBOARD_CODE_R                   ,  /*20*/
    KEYBOARD_CODE_T                  ,KEYBOARD_CODE_Y                   ,KEYBOARD_CODE_U                      ,KEYBOARD_CODE_I                 ,KEYBOARD_CODE_O                   ,  /*25*/
    KEYBOARD_CODE_P                  ,KEYBOARD_CODE_LBRACKET            ,KEYBOARD_CODE_RBRACKET               ,KEYBOARD_CODE_RETURN            ,KEYBOARD_CODE_LCONTROL            ,  /*30*/
    KEYBOARD_CODE_A                  ,KEYBOARD_CODE_S                   ,KEYBOARD_CODE_D                      ,KEYBOARD_CODE_F                 ,KEYBOARD_CODE_G                   ,  /*35*/
    KEYBOARD_CODE_H                  ,KEYBOARD_CODE_J                   ,KEYBOARD_CODE_K                      ,KEYBOARD_CODE_L                 ,KEYBOARD_CODE_SEMICOLON           ,  /*40*/
    KEYBOARD_CODE_APOSTROPHE         ,KEYBOARD_CODE_GRAVE               ,KEYBOARD_CODE_LSHIFT                 ,KEYBOARD_CODE_BACKSLASH         ,KEYBOARD_CODE_Z                   ,  /*45*/
    KEYBOARD_CODE_X                  ,KEYBOARD_CODE_C                   ,KEYBOARD_CODE_V                      ,KEYBOARD_CODE_B                 ,KEYBOARD_CODE_N                   ,  /*50*/
    KEYBOARD_CODE_M                  ,KEYBOARD_CODE_COMMA               ,KEYBOARD_CODE_PERIOD                 ,KEYBOARD_CODE_SLASH             ,KEYBOARD_CODE_RSHIFT              ,  /*55*/
    KEYBOARD_CODE_NUM_MULTIPLY       ,KEYBOARD_CODE_LALT                ,KEYBOARD_CODE_SPACE                  ,KEYBOARD_CODE_CAPITAL           ,KEYBOARD_CODE_F1                  ,  /*60*/
    KEYBOARD_CODE_F2                 ,KEYBOARD_CODE_F3                  ,KEYBOARD_CODE_F4                     ,KEYBOARD_CODE_F5                ,KEYBOARD_CODE_F6                  ,  /*65*/
    KEYBOARD_CODE_F7                 ,KEYBOARD_CODE_F8                  ,KEYBOARD_CODE_F9                     ,KEYBOARD_CODE_F10               ,KEYBOARD_CODE_NUMLOCK             ,  /*70*/
    KEYBOARD_CODE_SCROLL             ,KEYBOARD_CODE_NUM_7               ,KEYBOARD_CODE_NUM_8                  ,KEYBOARD_CODE_NUM_9             ,KEYBOARD_CODE_SUBTRACT            ,  /*75*/
    KEYBOARD_CODE_NUM_4              ,KEYBOARD_CODE_NUM_5               ,KEYBOARD_CODE_NUM_6                  ,KEYBOARD_CODE_NUM_ADD           ,KEYBOARD_CODE_NUM_1               ,  /*80*/
    KEYBOARD_CODE_NUM_2              ,KEYBOARD_CODE_NUM_3               ,KEYBOARD_CODE_NUM_0                  ,KEYBOARD_CODE_DECIMAL           ,KEYBOARD_CODE_NULL                ,  /*85*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_OEM_102             ,KEYBOARD_CODE_F11                    ,KEYBOARD_CODE_F12               ,KEYBOARD_CODE_NULL                ,  /*90*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*95*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*100*/
    KEYBOARD_CODE_F13                ,KEYBOARD_CODE_F14                 ,KEYBOARD_CODE_F15                    ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*105*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*110*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_KANA                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*115*/
    KEYBOARD_CODE_ABNT_C1            ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*120*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_CONVERT             ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NONCONVERT        ,KEYBOARD_CODE_NULL                ,  /*125*/
    KEYBOARD_CODE_YEN                ,KEYBOARD_CODE_ABNT_C2             ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*130*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*135*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*140*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NUM_EQUALS          ,KEYBOARD_CODE_PREV_TRACK             ,KEYBOARD_CODE_AT                ,KEYBOARD_CODE_COLON               ,  /*145*/
    KEYBOARD_CODE_UNDERLINE          ,KEYBOARD_CODE_KANJI               ,KEYBOARD_CODE_STOP                   ,KEYBOARD_CODE_AX                ,KEYBOARD_CODE_UNLABELED           ,  /*150*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NEXT_TRACK          ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NUM_ENTER           ,  /*155*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_RCONTROL               ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*160*/
    KEYBOARD_CODE_MUTE               ,KEYBOARD_CODE_CALCULATOR          ,KEYBOARD_CODE_PLAY_PAUSE             ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_MEDIA_STOP          ,  /*165*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*170*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_VOLUME_DOWN         ,  /*175*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_VOLUME_UP           ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_WEB_HOME          ,KEYBOARD_CODE_NUM_COMMA           ,  /*180*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NUM_DIVIDE             ,KEYBOARD_CODE_SYSRQ             ,KEYBOARD_CODE_RALT                ,  /*185*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*190*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*195*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_PAUSE                  ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_HOME                ,  /*200*/
    KEYBOARD_CODE_UP                 ,KEYBOARD_CODE_PRIOR               ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_LEFT              ,KEYBOARD_CODE_NULL                ,  /*205*/
    KEYBOARD_CODE_RIGHT              ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_END                    ,KEYBOARD_CODE_DOWN              ,KEYBOARD_CODE_NEXT                ,  /*210*/
    KEYBOARD_CODE_INSERT             ,KEYBOARD_CODE_DELETE              ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*215*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_LWIN                ,  /*220*/
    KEYBOARD_CODE_RWIN               ,KEYBOARD_CODE_APPMENU             ,KEYBOARD_CODE_POWER                  ,KEYBOARD_CODE_SLEEP             ,KEYBOARD_CODE_NULL                ,  /*225*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_WAKE                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_WEB_SEARCH          ,  /*230*/
    KEYBOARD_CODE_WEB_FAVORITES      ,KEYBOARD_CODE_WEB_REFRESH         ,KEYBOARD_CODE_WEB_STOP               ,KEYBOARD_CODE_WEB_FORWARD       ,KEYBOARD_CODE_WEB_BACK            ,  /*235*/
    KEYBOARD_CODE_MY_COMPUTER        ,KEYBOARD_CODE_MAIL                ,KEYBOARD_CODE_MEDIA_SELECT           ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*240*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*245*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*250*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*255*/
    KEYBOARD_CODE_NULL
};


#define  DIRECT_INPUT_DEVICE_8A_IN()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8A_OUT()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)

#define  MAX_STATE_BUFFER_SIZE   256

#define  LEFTBUTTON_IDX      0
#define  RIGHTBUTTON_IDX     1
#define  MIDBUTTON_IDX       2
#define  SET_BIT(c)  ((c)=0x80)
#define  CLEAR_BIT(c)  ((c)=0x00)


class CDirectInputDevice8AHook : public IDirectInputDevice8A
{
private:
    IDirectInputDevice8A* m_ptr;
    IID m_iid;
    unsigned char m_StateBuf[MAX_STATE_BUFFER_SIZE];
    int m_StateSize;
    CRITICAL_SECTION m_StateCS;
    std::vector<EVENT_LIST_t> m_EventList;
private:
    int __IsMouseDevice()
    {
        int ret = 0;
        if(IS_IID_MOUSE((this->m_iid)))
        {
            ret = 1;
        }

        return ret;
    }

    int __IsKeyboardDevice()
    {
        int ret = 0;
        if(IS_IID_KEYBOARD(this->m_iid))
        {
            ret = 1;
        }
        return ret;
    }
    EVENT_LIST_t* __GetEventList()
    {
        EVENT_LIST_t *pEventList=NULL;
        EnterCriticalSection(&(this->m_StateCS));
        if(this->m_EventList.size() > 0)
        {
            pEventList = this->m_EventList[0];
            this->m_EventList.erase(this->m_EventList.begin());
        }
        LeaveCriticalSection(&(this->m_StateCS));
        return pEventList;
    }

    int __InsertEventList(EVENT_LIST_t* pEventList,int insertback)
    {
        int ret = 0;
        EnterCriticalSection(&(this->m_StateCS));
        if(insertback)
        {
            this->m_EventList.push_back(pEventList);
        }
        else
        {
            this->m_EventList.insert(this->m_EventList.begin(),pEventList);
        }
        ret = 1;
        LeaveCriticalSection(&(this->m_StateCS));
        return ret;
    }

    int __UpdateMouseEventStateNoLock(EVENT_LIST_t* pEventList)
    {
        DIMOUSESTATE *pMouseState=(DIMOUSESTATE*)this->m_StateBuf;
        LPDEVICEEVENT pDevEvent = NULL;
        int ret;

        /*now to make the event list handle*/
        if(pEventList->size < sizeof(*pDevEvent))
        {
            ret = ERROR_INVALID_PARAMTER;
            ERROR_INFO("eventsize %d < sizeof(%d)\n",pEventList->size,sizeof(*pEventList));
            SetLastError(ret);
            return -ret;
        }

        pDevEvent = (LPDEVICEEVENT)(pEventList->m_BaseAddr + pEventList->m_Offset);
        __try
        {
            assert(pDevEvent->devtype == DEVICE_TYPE_MOUSE);
            switch(pDevEvent->event.mouse.event)
            {
            case MOUSE_CODE_MOUSE:
                pMouseState->lX += pDevEvent->event.mouse.x;
                pMouseState->lY += pDevEvent->event.mouse.y;
                break;
            case MOUSE_EVENT_KEYDOWN:
                switch(pDevEvent->event.mouse.code)
                {
                case MOUSE_CODE_LEFTBUTTON:
                    SET_BIT(pMouseState->rgbButtons[LEFTBUTTON_IDX]);
                    break;
                case MOUSE_CODE_RIGHTBUTTON:
                    SET_BIT(pMouseState->rgbButtons[RIGHTBUTTON_IDX]);
                    break;
                case MOUSE_CODE_MIDDLEBUTTON:
                    SET_BIT(pMouseState->rgbButtons[MIDBUTTON_IDX]);
                    break;
                default:
                    ret = ERROR_INVALID_PARAMETER;
                    ERROR_INFO("Mouse KeyDown code %d\n",pDevEvent->event.mouse.code);
                    goto fail;
                }
                break;
            case MOUSE_EVENT_KEYUP:
                switch(pDevEvent->event.mouse.code)
                {
                case MOUSE_CODE_LEFTBUTTON:
                    CLEAR_BIT(pMouseState->rgbButtons[LEFTBUTTON_IDX]);
                    break;
                case MOUSE_CODE_RIGHTBUTTON:
                    CLEAR_BIT(pMouseState->rgbButtons[RIGHTBUTTON_IDX]);
                    break;
                case MOUSE_CODE_MIDDLEBUTTON:
                    CLEAR_BIT(pMouseState->rgbButtons[MIDBUTTON_IDX]);
                    break;
                default:
                    ret = ERROR_INVALID_PARAMETER;
                    ERROR_INFO("Mouse KeyUp code %d\n",pDevEvent->event.mouse.code);
                    goto fail;
                }
                break;
            case MOUSE_EVENT_SLIDE:
                pMouseState->lZ += pDevEvent->event.mouse.x;
                break;
            default:
                ret = ERROR_INVALID_PARAMETER;
                ERROR_INFO("Mouse error event(%d)\n",pDevEvent->event.mouse.event);
                goto fail;
            }
        }

        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }

        SetLastError(0);
        return 0;


fail:
        assert(ret > 0);
        SetLastError(ret);
        return -ret;
    }

    int __UpdateKeyboardEventStateNoLock(EVENT_LIST_t* pEventList)
    {
        unsigned char* pKeyboardState = this->m_StateBuf;
        LPDEVICEEVENT pDevEvent= NULL;
        int idx;
        int ret;

        /*now we  should test for the size*/
        if(pEventList->size < sizeof(*pDevEvent))
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return -ret;
        }

        pDevEvent = (LPDEVICEEVENT)(pEventList->m_BaseAddr + pEventList->m_Offset);

        __try
        {
            if(pDevEvent->event.keyboard.code > KEYBOARD_CODE_NULL)
            {
                ret = ERROR_INVALID_PARAMETER;
                ERROR_INFO("keyboard code (%d)\n",pDevEvent->event.keyboard.code);
                goto fail;
            }

            idx = st_CodeMapDik[pDevEvent->event.keyboard.code];
            assert(idx <= DIK_NULL);
            switch(pDevEvent->event.keyboard.event)
            {
            case KEYBOARD_EVENT_DOWN:
                SET_BIT(pKeyboardState[idx]);
                break;
            case KEYBOARD_EVENT_UP:
                CLEAR_BIT(pKeyboardState[idx]);
                break;
            default:
                ret = ERROR_INVALID_PARAMETER;
                ERROR_INFO("<0x%p> keyboard event (%d)\n",this->m_ptr,pDevEvent->event.keyboard.event);
                goto fail;
            }

        }

        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }

        SetLastError(0);
        return 0;


fail:
        assert(ret > 0);
        SetLastError(ret);
        return -ret;
    }

    int __UpdateMouseAfter()
    {
        DIMOUSESTATE *pMouseState=(DIMOUSESTATE*)this->m_StateBuf;

        EnterCriticalSection(&(this->m_StateCS));
        pMouseState->lX = 0;
        pMouseState->lY = 0;
        pMouseState->lZ = 0;
        LeaveCriticalSection(&(this->m_StateCS));
        return 0;
    }



    int __UpdateEventStateNoLock(EVENT_LIST_t* pEventList)
    {
        int ret = 0;

        if(this->__IsMouseDevice())
        {
            /*now if the device is mouse ,so we should make sure it is*/
            return this->__UpdateMouseEventStateNoLock(pEventList);
        }
        else if(this->__IsKeyboardDevice())
        {
            return this->__UpdateKeyboardEventStateNoLock(pEventList);
        }

        return ret;
    }

    HRESULT __UpdateEventState(DWORD cbData,PVOID pData)
    {
        int ret;
        int totalret=0;
        HRESULT hr=DI_OK;
        std::vector<EVENT_LIST_t*> HandledEventList;
        EVENT_LIST_t* pEventList=NULL;

        EnterCriticalSection(&(this->m_StateCS));
        while(this->m_EventList.size() > 0)
        {
            assert(pEventList == NULL);
            pEventList = this->m_EventList[0];
            this->m_EventList.erase(this->m_EventList.begin());
            ret = this->__UpdateEventStateNoLock(pEventList);
            if(ret < 0)
            {
                totalret = LAST_ERROR_CODE();
                hr = E_PENDING;
                ERROR_INFO("could not update <0x%p> eventlist error(%d)\n",pEventList,totalret);
            }
            HandledEventList.push_back(pEventList);
            pEventList = NULL;
        }

        /*now we should update it*/
        if(cbData < this->m_StateSize)
        {
            totalret = ERROR_INSUFFICIENT_BUFFER;
            hr = DIERR_INVALIDPARAM;
            ERROR_INFO("<0x%p> cbData %d size(%d)\n",this->m_ptr,cbData,this->m_StateSize);
        }
        else if(this->m_StateSize > 0)
        {
            CopyMemory(pData,this->m_StateBuf,cbData);
        }
        else
        {
            /*we pretend it is ok*/
            ZeroMemory(pData,cbData);
        }
        LeaveCriticalSection(&(this->m_StateCS));

        /*now we should free event*/
        while(HandledEventList.size() > 0)
        {
            assert(pEventList == NULL);
            pEventList = HandledEventList[0];
            HandledEventList.erase(HandledEventList.begin());
            ret = IoFreeEventList(pEventList);
            /*we should event to handle this*/
            assert(ret >= 0);
            pEventList = NULL;
        }

        assert(HandledEventList.size() == 0);

        SetLastError(totalret);
        return hr;
    }

public:
    CDirectInputDevice8AHook(IDirectInputDevice8A* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
        ZeroMemory(m_StateBuf,sizeof(m_StateBuf));
        if(riid == GUID_SysMouse ||
                riid == GUID_SysMouseEm ||
                riid == GUID_SysMouseEm2)
        {
            m_StateSize = sizeof(DIMOUSESTATE);
        }
        else if(riid == GUID_SysKeyboard ||
                riid == GUID_SysKeyboardEm ||
                riid == GUID_SysKeyboardEm2)
        {
            m_StateSize = 0x100;
        }
        else
        {
            m_StateSize = 0;
        }
        InitializeCriticalSection(&m_StateCS);
    };

    ~CDirectInputDevice8AHook()
    {
        this->FreeEventList();
        DeleteCriticalSection(&(m_StateCS));
        ZeroMemory(&(m_StateBuf),sizeof(m_StateBuf));
        m_StateSize = 0;
        m_iid = IID_NULL;
    }


    int PutEventList(EVENT_LIST_t* pEventList)
    {
        return this->__InsertEventList(pEventList,1);
    }


    void FreeEventList()
    {
        EVENT_LIST_t* pEventList=NULL;

        while(1)
        {
            pEventList = this->__GetEventList();
            if(pEventList == NULL)
            {
                break;
            }
            IoFreeEventList(pEventList);
        }
        /*to make the buffer not set*/
        EnterCritcalSection(&(this->m_StateCS));
        ZeroMemory(&(m_StateBuf),sizeof(m_StateBuf));
        LeaveCriticalSection(&(this->m_StateCS));
        return ;
    }
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8A_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8A_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_DEVICE_8A_OUT();
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInputDevice8AHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,GetCapabilities)(THIS_ LPDIDEVCAPS lpDIDevCaps)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetCapabilities(lpDIDevCaps);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumObjects(lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetProperty)(THIS_ REFGUID rguidProp,LPDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetProperty)(THIS_ REFGUID rguidProp,LPCDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Acquire();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Unacquire();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceState)(THIS_ DWORD cbData,LPVOID lpvData)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        if(this->__IsMouseDevice() || this->__IsKeyboardDevice())
        {
            hr = this->__UpdateEventState(cbData,lpvData);
            if(hr == DI_OK)
            {
                if(this->__IsMouseDevice())
                {
                    /*if mouse device will doing ,so we should do this handle*/
                    this->__UpdateMouseAfter();
                }
            }
        }
        else
        {
            hr = this->m_ptr->GetDeviceState(cbData,lpvData);
        }
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetDataFormat)(THIS_ LPCDIDATAFORMAT lpdf)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetDataFormat(lpdf);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetEventNotification)(THIS_ HANDLE hEvent)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetEventNotification(hEvent);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetCooperativeLevel(hwnd,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA pdidoi,DWORD dwObj,DWORD dwHow)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetObjectInfo(pdidoi,dwObj,dwHow);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA pdidi)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetDeviceInfo(pdidi);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_ HINSTANCE hinst,DWORD dwVersion,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Initialize(hinst,dwVersion,rguid);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,CreateEffect)(THIS_ REFGUID rguid,LPCDIEFFECT lpeff,LPDIRECTINPUTEFFECT * ppdeff,LPUNKNOWN punkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->CreateEffect(rguid,lpeff,ppdeff,punkOuter);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA lpCallback,LPVOID pvRef,DWORD dwEffType)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumEffects(lpCallback,pvRef,dwEffType);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,GetEffectInfo)(THIS_ LPDIEFFECTINFOA pdei,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetEffectInfo(pdei,rguid);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetForceFeedbackState)(THIS_  LPDWORD pdwOut)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetForceFeedbackState(pdwOut);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendForceFeedbackCommand)(THIS_ DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SendForceFeedbackCommand(dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumCreatedEffectObjects)(THIS_  LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,LPVOID pvRef,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumCreatedEffectObjects(lpCallback,pvRef,fl);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Escape)(THIS_ LPDIEFFESCAPE pesc)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Escape(pesc);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Poll)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Poll();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendDeviceData)(THIS_ DWORD cbObjectData,LPCDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SendDeviceData(cbObjectData,rgdod,pdwInOut,fl);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffectsInFile)(THIS_ LPCSTR lpszFileName,LPDIENUMEFFECTSINFILECALLBACK pec,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumEffectsInFile(lpszFileName,pec,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,WriteEffectToFile)(THIS_ LPCSTR lpszFileName,DWORD dwEntries,LPDIFILEEFFECT rgDiFileEft,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->WriteEffectToFile(lpszFileName,dwEntries,rgDiFileEft,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,BuildActionMap)(THIS_ LPDIACTIONFORMATA lpdiaf,LPCSTR lpszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->BuildActionMap(lpdiaf,lpszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetActionMap)(THIS_  LPDIACTIONFORMATA lpdiActionFormat,LPCSTR lptszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetActionMap(lpdiActionFormat,lptszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetImageInfo)(THIS_  LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetImageInfo(lpdiDevImageInfoHeader);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

};


#define  IS_IID_MOUSE(riid) (((riid) == ))

CDirectInputDevice8AHook* RegisterDirectInputDevice8AHook(IDirectInputDevice8A* ptr,REFIID riid)
{
    CDirectInputDevice8AHook* pHookA=NULL;
    int findidx = -1;
    unsigned int i;


    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8A_VECS();
    if(IS_IID_MOUSE(riid))
    {
        for(i=0; i<st_Mouse8AVes.size(); i++)
        {
            if(st_Mouse8AVes[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_Mouse8AHookVecs[findidx];
        }
        else
        {
            pHookA = CDirectInputDevice8AHook(ptr,riid);
            st_Mouse8AVes.push_back(ptr);
            st_Mouse8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
    }
    else if(IS_IID_KEYBOARD(riid))
    {
        for(i=0; i<st_Key8AVecs.size(); i++)
        {
            if(st_Key8AVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_Key8AHookVecs[findidx];
        }
        else
        {
            pHookA = CDirectInputDevice8AHook(ptr,riid);
            st_Key8AVecs.push_back(ptr);
            st_Key8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
    }
    else
    {
        for(i=0; i<st_NotSet8AVecs.size(); i++)
        {
            if(st_NotSet8AVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_NotSet8AHookVecs[findidx];
        }
        else
        {
            pHookA = CDirectInputDevice8AHook(ptr,riid);
            st_NotSet8AVecs.push_back(ptr);
            st_NotSet8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
    }

    LeaveCriticalSection(&st_Dinput8DeviceCS);

    return pHookA;
}



/*****************************************
*  to make the IDirectInputDevice8W hook
*
*****************************************/
class CDirectInputDevice8WHook;

static std::vector<IDirectInputDevice8W*> st_DIDevice8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_CDIDevice8WHookVecs;
static CRITICAL_SECTION st_DIDevice8WCS;

#define EQUAL_DI_DEVICE_8W_VECS() \
do\
{\
	assert(st_DIDevice8WVecs.size() == st_CDIDevice8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DIDevice8WCS);
    EQUAL_DI_DEVICE_8W_VECS();
    for(i=0; i<st_DIDevice8WVecs.size() ; i++)
    {
        if(st_DIDevice8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DIDevice8WVecs.erase(st_DIDevice8WVecs.begin()+findidx);
        st_CDIDevice8WHookVecs.erase(st_CDIDevice8WHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("could not find 0x%p DirectInputDevice8W\n",ptr);
    }
    LeaveCriticalSection(&st_DIDevice8WCS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}


#define  DIRECT_INPUT_DEVICE_8W_IN()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8W_OUT()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)


class CDirectInputDevice8WHook : public IDirectInputDevice8W
{
private:
    IDirectInputDevice8W* m_ptr;
    IID m_iid;
public:
    CDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
    };
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8W_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8W_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_DEVICE_8W_OUT();
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInputDevice8WHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,GetCapabilities)(THIS_ LPDIDEVCAPS lpDIDevCaps)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetCapabilities(lpDIDevCaps);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumObjects(lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetProperty)(THIS_ REFGUID rguidProp,LPDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetProperty)(THIS_ REFGUID rguidProp,LPCDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Acquire();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Unacquire();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceState)(THIS_ DWORD cbData,LPVOID lpvData)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceState(cbData,lpvData);
        if(hr == DI_OK)
        {
            if(this->m_iid == GUID_SysMouse)
            {
#if 0
                DIMOUSESTATE* pMouseState = (DIMOUSESTATE*)lpvData;
                DINPUT_DEBUG_INFO("<0x%p> SysMouse data\n",this->m_ptr);
                if(cbData >= sizeof(*pMouseState))
                {
                    DINPUT_DEBUG_INFO("lx %ld ly %ld lz %ld rgbbutton[0] 0x%02x rgbbutton[1] 0x%02x rgbbutton[2] 0x%02x rgbbutton[3] 0x%02x\n",
                                      pMouseState->lX,
                                      pMouseState->lY,
                                      pMouseState->lZ,
                                      pMouseState->rgbButtons[0],
                                      pMouseState->rgbButtons[1],
                                      pMouseState->rgbButtons[2],
                                      pMouseState->rgbButtons[3]);
                }
                DINPUT_DEBUG_BUFFER_FMT(lpvData,cbData,NULL);
#endif
            }
            else if(this->m_iid == GUID_SysKeyboard)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpvData,cbData,"<0x%p> SysKeyboard data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_Joystick)
            {
                DINPUT_DEBUG_INFO("<0x%p> Joystick data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm2 data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm2 data\n",this->m_ptr);
            }
        }
        else
        {
            DINPUT_DEBUG_INFO("<0x%p> size(0x%08x) return 0x%08x\n",this->m_ptr,cbData,hr);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetDataFormat)(THIS_ LPCDIDATAFORMAT lpdf)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetDataFormat(lpdf);
        if(hr == DI_OK)
        {
            if(this->m_iid == GUID_SysMouse)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouse Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboard)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboard Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_Joystick)
            {
                DINPUT_DEBUG_INFO("<0x%p> Joystick Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm2 Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm2 Format\n",this->m_ptr);
            }

            if(lpdf == &c_dfDIMouse)
            {
                DINPUT_DEBUG_INFO("c_dfDIMouse\n");
            }
            else if(lpdf == &c_dfDIMouse2)
            {
                DINPUT_DEBUG_INFO("c_dfDIMouse2\n");
            }
            else if(lpdf == &c_dfDIKeyboard)
            {
                DINPUT_DEBUG_INFO("c_dfDIKeyboard\n");
            }
            else if(lpdf == &c_dfDIJoystick)
            {
                DINPUT_DEBUG_INFO(" c_dfDIJoystick\n");
            }
            else if(lpdf == &c_dfDIJoystick2)
            {
                DINPUT_DEBUG_INFO(" c_dfDIJoystick2\n");
            }
            if(lpdf->dwSize)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpdf,lpdf->dwSize,"<0x%p> format",this->m_ptr);
            }

            if(lpdf->rgodf && lpdf->dwObjSize)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpdf->rgodf,lpdf->dwObjSize,"<0x%p> flag 0x%08x datasize 0x%08x numobjs 0x%08x",this->m_ptr,lpdf->dwFlags,lpdf->dwDataSize,lpdf->dwNumObjs);
            }


        }
        else
        {
            ERROR_INFO("<0x%p> SetDataFormat Error\n",this->m_ptr);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetEventNotification)(THIS_ HANDLE hEvent)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetEventNotification(hEvent);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetCooperativeLevel(hwnd,dwFlags);
        if(hr == DI_OK)
        {
            DINPUT_DEBUG_INFO("<0x%p> hwnd 0x%08x dwLevel 0x%08x\n",this->m_ptr,
                              hwnd,dwFlags);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW pdidoi,DWORD dwObj,DWORD dwHow)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetObjectInfo(pdidoi,dwObj,dwHow);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW pdidi)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceInfo(pdidi);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_ HINSTANCE hinst,DWORD dwVersion,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Initialize(hinst,dwVersion,rguid);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,CreateEffect)(THIS_ REFGUID rguid,LPCDIEFFECT lpeff,LPDIRECTINPUTEFFECT * ppdeff,LPUNKNOWN punkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->CreateEffect(rguid,lpeff,ppdeff,punkOuter);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW lpCallback,LPVOID pvRef,DWORD dwEffType)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumEffects(lpCallback,pvRef,dwEffType);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,GetEffectInfo)(THIS_ LPDIEFFECTINFOW pdei,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetEffectInfo(pdei,rguid);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetForceFeedbackState)(THIS_  LPDWORD pdwOut)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetForceFeedbackState(pdwOut);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendForceFeedbackCommand)(THIS_ DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SendForceFeedbackCommand(dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumCreatedEffectObjects)(THIS_  LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,LPVOID pvRef,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumCreatedEffectObjects(lpCallback,pvRef,fl);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Escape)(THIS_ LPDIEFFESCAPE pesc)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Escape(pesc);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Poll)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Poll();
        DINPUT_DEBUG_INFO("<0X%p> Poll Return 0x%08x\n",this->m_ptr,hr);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendDeviceData)(THIS_ DWORD cbObjectData,LPCDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SendDeviceData(cbObjectData,rgdod,pdwInOut,fl);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffectsInFile)(THIS_ LPCWSTR lpszFileName,LPDIENUMEFFECTSINFILECALLBACK pec,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumEffectsInFile(lpszFileName,pec,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,WriteEffectToFile)(THIS_ LPCWSTR lpszFileName,DWORD dwEntries,LPDIFILEEFFECT rgDiFileEft,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->WriteEffectToFile(lpszFileName,dwEntries,rgDiFileEft,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,BuildActionMap)(THIS_ LPDIACTIONFORMATW lpdiaf,LPCTSTR lpszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->BuildActionMap(lpdiaf,lpszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetActionMap)(THIS_	LPDIACTIONFORMATW lpdiActionFormat,LPCTSTR lptszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetActionMap(lpdiActionFormat,lptszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetImageInfo)(THIS_	LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetImageInfo(lpdiDevImageInfoHeader);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

};

CDirectInputDevice8WHook* RegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid)
{
    CDirectInputDevice8WHook* pHookW=NULL;
    int findidx = -1;
    unsigned int i;

    EnterCriticalSection(&st_DIDevice8WCS);
    EQUAL_DI_DEVICE_8W_VECS();
    for(i=0; i<st_DIDevice8WVecs.size() ; i++)
    {
        if(st_DIDevice8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        pHookW = st_CDIDevice8WHookVecs[findidx];
    }
    else
    {
        pHookW = new CDirectInputDevice8WHook(ptr,riid);
        st_CDIDevice8WHookVecs.push_back(pHookW);
        st_DIDevice8WVecs.push_back(ptr);
        /*to add reference ,it will give release ok*/
        ptr->AddRef();
    }

    LeaveCriticalSection(&st_DIDevice8WCS);

    return pHookW;
}



/*****************************************
*   to make the DirectInput8A hook
*
*****************************************/
class CDirectInput8AHook;

static std::vector<IDirectInput8A*> st_DI8AVecs;
static std::vector<CDirectInput8AHook*> st_CDI8AHookVecs;
static CRITICAL_SECTION st_DI8ACS;

#define EQUAL_DI8A_VECS() \
do\
{\
	assert(st_DI8AVecs.size() == st_CDI8AHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInput8AHook(IDirectInput8A* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DI8ACS);
    EQUAL_DI8A_VECS();
    for(i=0; i<st_DI8AVecs.size() ; i++)
    {
        if(st_DI8AVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DI8AVecs.erase(st_DI8AVecs.begin()+findidx);
        st_CDI8AHookVecs.erase(st_CDI8AHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("could not find 0x%p DirectInput8A\n",ptr);
    }
    LeaveCriticalSection(&st_DI8ACS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}

#define  DIRECT_INPUT_8A_IN() do{DINPUT_DEBUG_INFO("Input8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_8A_OUT() do{DINPUT_DEBUG_INFO("Input8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)
class CDirectInput8AHook :public IDirectInput8A
{
private:
    IDirectInput8A* m_ptr;

public:
    CDirectInput8AHook(IDirectInput8A* ptr):m_ptr(ptr) {};
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8A_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_8A_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8A_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_8A_OUT();
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInput8AHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,CreateDevice)(THIS_ REFGUID rguid,
                                     LPDIRECTINPUTDEVICE8A * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        if(SUCCEEDED(hr))
        {
            CDirectInputDevice8AHook* pHookA=NULL;
            if(rguid == GUID_SysMouse)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouse 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboard)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboard 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_Joystick)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("joystick 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysMouseEm)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysMouseEm2)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboardEm)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboardEm2)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }


            DINPUT_DEBUG_INFO("rguid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              rguid.Data1,
                              rguid.Data2,
                              rguid.Data3,
                              rguid.Data4[0],
                              rguid.Data4[1],
                              rguid.Data4[2],
                              rguid.Data4[3],
                              rguid.Data4[4],
                              rguid.Data4[5],
                              rguid.Data4[6],
                              rguid.Data4[7]);

            if(pHookA)
            {
                /*if we make the hook object ,just replace it to the upper caller*/
                *lplpDirectInputDevice = pHookA;
            }
        }
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACKA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceStatus)(THIS_  REFGUID rguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->GetDeviceStatus(rguidInstance);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_  HINSTANCE hinst,DWORD dwVersion)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->Initialize(hinst,dwVersion);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,FindDevice)(THIS_ REFGUID rguidClass,LPCSTR ptszName,LPGUID pguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->FindDevice(rguidClass,ptszName,pguidInstance);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevicesBySemantics)(THIS_ LPCSTR ptszUserName,LPDIACTIONFORMATA lpdiActionFormat,LPDIENUMDEVICESBYSEMANTICSCBA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->EnumDevicesBySemantics(ptszUserName,lpdiActionFormat,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMSA lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->ConfigureDevices(lpdiCallback,lpdiCDParams,dwFlags,pvRefData);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

};

CDirectInput8AHook* RegisterDirectInput8AHook(IDirectInput8A* ptr)
{
    int findidx=-1;
    unsigned int i;
    CDirectInput8AHook* pHook=NULL;

    EnterCriticalSection(&st_DI8ACS);
    EQUAL_DI8A_VECS();
    for(i=0; i<st_DI8AVecs.size() ; i++)
    {
        if(st_DI8AVecs[i]==ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        pHook =new CDirectInput8AHook(ptr);
        st_DI8AVecs.push_back(ptr);
        st_CDI8AHookVecs.push_back(pHook);
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
    }
    else
    {
        pHook = st_CDI8AHookVecs[findidx];
    }
    LeaveCriticalSection(&st_DI8ACS);
    return pHook;
}


/*****************************************
*   to make the DirectInput8W hook
*
*****************************************/


class CDirectInput8WHook;

static std::vector<IDirectInput8W*> st_DI8WVecs;
static std::vector<CDirectInput8WHook*> st_CDI8WHookVecs;
static CRITICAL_SECTION st_DI8WCS;

#define EQUAL_DI8W_VECS() \
do\
{\
	assert(st_DI8WVecs.size() == st_CDI8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInput8WHook(IDirectInput8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DI8WCS);
    EQUAL_DI8W_VECS();
    for(i=0; i<st_DI8WVecs.size() ; i++)
    {
        if(st_DI8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DI8WVecs.erase(st_DI8WVecs.begin()+findidx);
        st_CDI8WHookVecs.erase(st_CDI8WHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("Not Register 0x%p\n",ptr);
    }
    LeaveCriticalSection(&st_DI8WCS);

    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}



#define  DIRECT_INPUT_8W_IN()  do{DINPUT_DEBUG_INFO("Input8W::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_8W_OUT()  do{DINPUT_DEBUG_INFO("Input8W::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)

class CDirectInput8WHook : IDirectInput8W
{
private:
    IDirectInput8W* m_ptr;
public:
    CDirectInput8WHook(IDirectInput8W* ptr) : m_ptr(ptr) {};
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8W_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_8W_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8W_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_8W_OUT();
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInput8WHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,CreateDevice)(THIS_ REFGUID rguid,
                                     LPDIRECTINPUTDEVICE8W * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        SetUnHandlerExceptionDetour();
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        if(SUCCEEDED(hr))
        {
            CDirectInputDevice8WHook* pHookW=NULL;
            if(rguid == GUID_SysMouse)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouse 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboard)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboard 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_Joystick)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("joystick 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysMouseEm)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysMouseEm2)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboardEm)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboardEm2)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }


            DINPUT_DEBUG_INFO("rguid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              rguid.Data1,
                              rguid.Data2,
                              rguid.Data3,
                              rguid.Data4[0],
                              rguid.Data4[1],
                              rguid.Data4[2],
                              rguid.Data4[3],
                              rguid.Data4[4],
                              rguid.Data4[5],
                              rguid.Data4[6],
                              rguid.Data4[7]);

            if(pHookW)
            {
                /*if we make the hook object ,just replace it to the upper caller*/
                *lplpDirectInputDevice = pHookW;
            }
        }
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACKW lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceStatus)(THIS_  REFGUID rguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->GetDeviceStatus(rguidInstance);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_  HINSTANCE hinst,DWORD dwVersion)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->Initialize(hinst,dwVersion);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,FindDevice)(THIS_ REFGUID rguidClass,LPCTSTR ptszName,LPGUID pguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->FindDevice(rguidClass,ptszName,pguidInstance);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevicesBySemantics)(THIS_ LPCTSTR ptszUserName,LPDIACTIONFORMAT lpdiActionFormat,LPDIENUMDEVICESBYSEMANTICSCB lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->EnumDevicesBySemantics(ptszUserName,lpdiActionFormat,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMSW lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->ConfigureDevices(lpdiCallback,lpdiCDParams,dwFlags,pvRefData);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

};

CDirectInput8WHook* RegisterDirectInput8WHook(IDirectInput8W* ptr)
{
    int findidx=-1;
    unsigned int i;
    CDirectInput8WHook* pHookW=NULL;

    EnterCriticalSection(&st_DI8WCS);
    EQUAL_DI8W_VECS();
    for(i=0; i<st_DI8WVecs.size() ; i++)
    {
        if(st_DI8WVecs[i]==ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        pHookW =new CDirectInput8WHook(ptr);
        st_DI8WVecs.push_back(ptr);
        st_CDI8WHookVecs.push_back(pHookW);
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
    }
    else
    {
        pHookW = st_CDI8WHookVecs[findidx];
    }
    LeaveCriticalSection(&st_DI8WCS);
    return pHookW;
}





