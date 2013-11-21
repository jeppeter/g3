
#include <iocapcommon.h>

class CDirectInputDevice8AHook;
class CDirectInputDevice8WHook;

typedef struct
{
    unsigned int m_Started;
    thread_control_t m_ThreadControl;
    unsigned int m_Bufnumm;
    unsigned int m_BufSectSize;
    unsigned char m_MemShareBaseName[IO_NAME_MAX_SIZE];
    HANDLE m_hMapFile;
    void* m_pMemShareBase;
    unsigned char m_FreeEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pFreeEvts;
    unsigned char m_InputEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pInputEvts;
    EVENT_LIST_t* m_pEventListArray;
    CRITICAL_SECTION m_ListCS;
    int m_ListCSInited;
    std::vector<EVENT_LIST_t*>* m_pFreeList;
} DETOUR_DIRECTINPUT_STATUS_t,*PDETOUR_DIRECTINPUT_STATUS_t;


static std::vector<IDirectInputDevice8A*> st_Key8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_Key8AHookVecs;
static std::vector<IDirectInputDevice8A*> st_Mouse8AVes;
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

#define IS_IID_MOUSE(riid)  ( (riid)	== GUID_SysMouse ||(riid) == GUID_SysMouseEm ||(riid) == GUID_SysMouseEm2 )
#define IS_IID_KEYBOARD(riid) ((riid) == GUID_SysKeyboard ||(riid) == GUID_SysKeyboardEm ||(riid) == GUID_SysKeyboardEm2)

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
            findidx = -1;
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




#define  DIRECT_INPUT_DEVICE_8A_IN()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8A_OUT()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)

#define  MAX_STATE_BUFFER_SIZE   256

#define  SET_BIT(c)  ((c)=0x80)
#define  CLEAR_BIT(c)  ((c)=0x00)


class CDirectInputDevice8AHook : public IDirectInputDevice8A
{
private:
    IDirectInputDevice8A* m_ptr;
    IID m_iid;
    unsigned char m_StateBuf[MAX_STATE_BUFFER_SIZE];
    unsigned int m_StateSize;
    CRITICAL_SECTION m_StateCS;
    std::vector<EVENT_LIST_t*> m_EventList;
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
            ret = ERROR_INVALID_PARAMETER;
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
            case MOUSE_EVNET_MOVING:
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
            CopyMemory(pData,this->m_StateBuf,this->m_StateSize);
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
            IoFreeEventList(pEventList);
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
        if(IS_IID_MOUSE(riid))
        {
            m_StateSize = sizeof(DIMOUSESTATE);
        }
        else if(IS_IID_KEYBOARD(riid))
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
        EnterCriticalSection(&(this->m_StateCS));
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
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
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
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
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
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
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

ULONG UnRegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8W_VECS();
    findidx = -1;
    for(i=0; i<st_Key8WVecs.size() ; i++)
    {
        if(st_Key8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_Key8WVecs.erase(st_Key8WVecs.begin() + findidx);
        st_Key8WHookVecs.erase(st_Key8WHookVecs.begin() + findidx);
    }
    else
    {
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(st_Mouse8WVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            st_Mouse8WVecs.erase(st_Mouse8WVecs.begin() + findidx);
            st_Mouse8WHookVecs.erase(st_Mouse8WHookVecs.begin() + findidx);
        }
        else
        {
            for(i=0; i<st_NotSet8WVecs.size() ; i++)
            {
                if(st_NotSet8WVecs[i] == ptr)
                {
                    findidx = i;
                    break;
                }
            }

            if(findidx >= 0)
            {
                st_NotSet8WVecs.erase(st_NotSet8WVecs.begin() + findidx);
                st_NotSet8WHookVecs.erase(st_NotSet8WHookVecs.begin() + findidx);
            }
            else
            {
                ERROR_INFO("<0x%p> not in the vectors\n",ptr);
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


#define  DIRECT_INPUT_DEVICE_8W_IN()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8W_OUT()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)


class CDirectInputDevice8WHook : public IDirectInputDevice8W
{
private:
    IDirectInputDevice8W* m_ptr;
    IID m_iid;
    unsigned char m_StateBuf[MAX_STATE_BUFFER_SIZE];
    unsigned int m_StateSize;
    CRITICAL_SECTION m_StateCS;
    std::vector<EVENT_LIST_t*> m_EventList;
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
            ret = ERROR_INVALID_PARAMETER;
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
            case MOUSE_EVNET_MOVING:
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
            CopyMemory(pData,this->m_StateBuf,this->m_StateSize);
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
            IoFreeEventList(pEventList);
            pEventList = NULL;
        }

        assert(HandledEventList.size() == 0);

        SetLastError(totalret);
        return hr;
    }
public:
    CDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
        ZeroMemory(m_StateBuf,sizeof(m_StateBuf));
        if(IS_IID_MOUSE(riid))
        {
            m_StateSize = sizeof(DIMOUSESTATE);
        }
        else if(IS_IID_KEYBOARD(riid))
        {
            m_StateSize = 0x100;
        }
        else
        {
            m_StateSize = 0;
        }
        InitializeCriticalSection(&m_StateCS);
    };
    ~CDirectInputDevice8WHook()
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
        EnterCriticalSection(&(this->m_StateCS));
        ZeroMemory(&(m_StateBuf),sizeof(m_StateBuf));
        LeaveCriticalSection(&(this->m_StateCS));
        return ;
    }
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

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8W_VECS();
    if(IS_IID_KEYBOARD(riid))
    {
        for(i=0; i<st_Key8WVecs.size() ; i++)
        {
            if(ptr == st_Key8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_Key8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_Key8WVecs.push_back(ptr);
            st_Key8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
    }
    else if(IS_IID_MOUSE(riid))
    {
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(ptr == st_Mouse8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_Mouse8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_Mouse8WVecs.push_back(ptr);
            st_Mouse8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
    }
    else
    {
        for(i=0; i<st_NotSet8WVecs.size() ; i++)
        {
            if(ptr == st_NotSet8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_NotSet8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_NotSet8WVecs.push_back(ptr);
            st_NotSet8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
    }

    LeaveCriticalSection(&st_Dinput8DeviceCS);

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

    uret = 1;
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

static HANDLE st_hDetourDinputSema=NULL;
static PDETOUR_DIRECTINPUT_STATUS_t st_pDinputStatus=NULL;
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
        if(st_pDinputStatus->m_pFreeList->At(i) == pEventList)
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


int DetourDirectInputChangeFreeToInput(PDETOUR_DIRECTINPUT_STATUS_t pStatus,DWORD idx)
{
    int ret = 0,findidx = -1,res;
    unsigned int i;
    PEVENT_LIST_t pEvent=NULL;
    if(pStatus->m_Started)
    {
        EnterCriticalSection(&(pStatus->m_ListCS));
        findidx = -1;
        for(i=0; i<pStatus->m_pFreeList->size() ; i++)
        {
            if(pStatus->m_pFreeList->At[i]->m_Idx == idx)
            {
                pEvent = pStatus->m_pFreeList[i];
                findidx = i;
                ret = 1;
                break;
            }
        }

        /*now we should change the status*/
        if(findidx >= 0)
        {
            pStatus->m_pFreeList->erase(pStatus->m_pFreeList->begin() + findidx);
            res = IoInjectInput(pEvent);
            if(res <= 0)
            {
                /*now it is not the right time to insert into the device ,so we input it back ,and it will give */
                pStatus->m_pFreeList->push_back(pEvent);
                SetEvent(pEvent->m_hFillEvt);
            }
        }
        LeaveCriticalSection(&(pStatus->m_ListCS));
    }
    else
    {
        /*it is started not ,so we should make it just handled it*/
        EnterCriticalSection(&(pStatus->m_ListCS));
        for(i=0; i<pStatus->m_pFreeList->size() ; i++)
        {
            if(pStatus->m_pFreeList[i]->m_Idx == idx)
            {
                pEvent = pStatus->m_pFreeList[i];
                findidx = i;
                ret = 1;
                break;
            }
        }

        if(findidx >= 0)
        {
            SetEvent(pEvent->m_hFillEvt);
        }

        LeaveCriticalSection(&(pStatus->m_ListCS));
    }
    return ret;
}

DWORD WINAPI DetourDirectInputThreadImpl(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
{
    HANDLE *pWaitHandles=NULL;
    unsigned int waitnum=0;
    DWORD dret,idx;
    int ret;
    int tries;

    assert(pStatus->m_Bufnumm > 0);
    /*for add into num*/
    waitnum = (pStatus->m_Bufnumm + 1);
    pWaitHandles = calloc(sizeof(*pWaitHandles),waitnum);
    if(pWaitHandles == NULL)
    {
        dret = LAST_ERROR_CODE();
        goto out;
    }

    CopyMemory(pWaitHandles,pStatus->m_pInputEvts,sizeof(*pWaitHandles)*(waitnum - 1));
    pWaitHandles[waitnum - 1] = pStatus->m_ThreadControl.exitevt;

    while(pStatus->m_ThreadControl.running)
    {
        dret = WaitForMultipleObjectsEx(waitnum,pWaitHandle,FALSE,INFINITE,TRUE);
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
                ERROR_INFO("Change[%d] wait\n",idx);
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


void __FreeDeviceEvents(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
{
    int ret;
    unsigned int i;
    int tries;
    if(pStatus == NULL)
    {
        return ;
    }

    /*now we should free events*/
    EnterCriticalSection(&(st_Dinput8DeviceCS));
    for(i=0; i<st_Key8AHookVecs.size() ; i++)
    {
        st_Key8AHookVecs[i]->FreeEventList();
    }

    for(i=0; i<st_Key8WHookVecs.size() ; i++)
    {
        st_Key8WHookVecs[i]->FreeEventList();
    }

    for(i=0; i<st_Mouse8AHookVecs.size() ; i++)
    {
        st_Mouse8AHookVecs[i]->FreeEventList();
    }

    for(i=0; i<st_Mouse8WHookVecs.size() ; i ++)
    {
        st_Mouse8WHookVecs[i]->FreeEventList();
    }

    for(i=0; i<st_NotSet8AHookVecs.size() ; i++)
    {
        st_NotSet8AHookVecs[i]->FreeEventList();
    }

    for(i=0; i<st_NotSet8WHookVecs.size(); i++)
    {
        st_NotSet8WHookVecs[i]->FreeEventList();
    }
    LeaveCriticalSection(&(st_Dinput8DeviceCS));


    return ;
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

void __UnMapMemBase(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
{

    if(pStatus == NULL)
    {
        return ;
    }
    UnMapFileBuffer(&(pStatus->m_pMemShareBase));
    CloseMapFileHandle(&(pStatus->m_hMapFile));
    ZeroMemory(pStatus->m_MemShareBaseName,sizeof(pStatus->m_MemShareBaseName));
    pStatus->m_Bufnumm = 0;
    pStatus->m_BufSectSize = 0;
    return ;
}

void __FreeDetourDinputStatus(PDETOUR_DIRECTINPUT_STATUS_t *ppStatus)
{
    PDETOUR_DIRECTINPUT_STATUS_t pStatus ;
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

PDETOUR_DIRECTINPUT_STATUS_t __AllocateDetourStatus()
{
    PDETOUR_DIRECTINPUT_STATUS_t pStatus=NULL;
    int ret;

    pStatus = calloc(sizeof(*pStatus),1);
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

int __MapMemBase(PDETOUR_DIRECTINPUT_STATUS_t pStatus,uint8_t* pMemName,uint32_t bufsectsize,uint32_t bufnum)
{
    int ret;
    __UnMapMemBase(pStatus);
    /*now we should first to get map file handle*/
    pStatus->m_hMapFile = CreateMapFile(pMemName,bufsectsize*bufnum,0);
    if(pStatus->m_hMapFile == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not createmapfile(%s) bufsectsize(%d) bufnum(%d) Error(%d)\n",
                   pMemName,bufsectsize,bufnum,ret);
        goto fail;
    }

    pStatus->m_pMemShareBase = MapFileBuffer(pStatus->m_hMapFile,bufsectsize*bufnum);
    if(pStatus->m_pMemShareBase == NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not mapfile buffer(%s) bufsectsize(%d) bufnum(%d) Error(%d)\n",
                   pMemName,bufsectsize,bufnum,ret);
        goto fail;
    }

    pStatus->m_Bufnumm = bufnum;
    pStatus->m_BufSectSize = bufsectsize;
    strncpy_s(pStatus->m_MemShareBaseName,sizeof(pStatus->m_MemShareBaseName),pMemName,_TRUNCATE);


    SetLastError(0);
    return 0;

fail:
    assert(ret > 0);
    __UnMapMemBase(pStatus);
    SetLastError(ret);
    return -ret;
}

int __AllocateFreeEvents(PDETOUR_DIRECTINPUT_STATUS_t pStatus,uint8_t* pFreeEvtBaseName)
{
    uint8_t fullname[IO_NAME_MAX_SIZE];
    int ret;
    uint32_t i;
    /*now we should allocate size*/
    pStatus->m_pFreeEvts = calloc(sizeof(pStatus->m_pFreeEvts[0]),pStatus->m_Bufnumm);
    if(pStatus->m_pFreeEvts == NULL)
    {
        ret= LAST_ERROR_CODE();
        goto fail;
    }

    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        _snprintf_s(fullname,sizeof(fullname),_TRUNCATE,"%s_%d",pFreeEvtBaseName,i);
        pStatus->m_pFreeEvts[i] = GetEvent(fullname,0);
        if(pStatus->m_pFreeEvts[i] == NULL)
        {
            ret=  LAST_ERROR_CODE();
            ERROR_INFO("GetFreeEvent (%s) Error(%d)\n",fullname,ret);
            goto fail;
        }
    }
    strncpy_s(pStatus->m_FreeEvtBaseName,sizeof(pStatus->m_FreeEvtBaseName),pFreeEvtBaseName,_TRUNCATE);


    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    __FreeDetourEvents(pStatus);
    SetLastError(ret);
    return -ret;
}

int __AllocateEventList(PDETOUR_DIRECTINPUT_STATUS_t pStatus)
{
    int ret;
    uint32_t i;
    /*now first to allocate event list array*/
    pStatus->m_pEventListArray = calloc(sizeof(pStatus->m_pEventListArray[0]),pStatus->m_Bufnumm);
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
    pStatus->m_pInputList = new std::vector<EVENT_LIST_t*>();
    if(pStatus->m_pInputList == NULL)
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

int __AllocateInputEvents(PDETOUR_DIRECTINPUT_STATUS_t pStatus,uint8_t* pInputEvtBaseName)
{
    uint8_t fullname[IO_NAME_MAX_SIZE];
    int ret;
    uint32_t i;
    /*now we should allocate size*/
    pStatus->m_pInputEvts = calloc(sizeof(pStatus->m_pInputEvts[0]),pStatus->m_Bufnumm);
    if(pStatus->m_pInputEvts == NULL)
    {
        ret= LAST_ERROR_CODE();
        goto fail;
    }

    for(i=0; i<pStatus->m_Bufnumm; i++)
    {
        _snprintf_s(fullname,sizeof(fullname),_TRUNCATE,"%s_%d",pInputEvtBaseName,i);
        pStatus->m_pInputEvts[i] = GetEvent(fullname,0);
        if(pStatus->m_pInputEvts[i] == NULL)
        {
            ret=  LAST_ERROR_CODE();
            ERROR_INFO("GetInputEvent (%s) Error(%d)\n",fullname,ret);
            goto fail;
        }
    }

    strncpy_s(pStatus->m_InputEvtBaseName,sizeof(pStatus->m_InputEvtBaseName),pInputEvtBaseName,_TRUNCATE);

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
    PDETOUR_DIRECTINPUT_STATUS_t pStatus=NULL;

    if(pControl == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    if(st_pDinputStatus)
    {
        SetLastError(0);
        return 0;
    }

    pStatus = __AllocateDetourStatus();
    if(pStatus == NULL)
    {
        ret=  LAST_ERROR_CODE();
        goto fail;
    }

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

    EnterCriticalSection();
    if(devtype == DEVICE_TYPE_KEYBOARD)
    {
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
    LeaveCriticalSection();

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

    EnterCriticalSection();
    if(devtype == DEVICE_TYPE_KEYBOARD)
    {
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
    LeaveCriticalSection();

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




