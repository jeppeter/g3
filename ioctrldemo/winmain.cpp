
#include <Windows.h>


LPDIRECTINPUT8          g_pDirectInput      = NULL; //
LPDIRECTINPUTDEVICE8    g_pMouseDevice      = NULL;
int                     g_MouseAcquire      = 0;
DIMOUSESTATE            g_diMouseState      = {0};
DIMOUSESTATE            g_LastdiMouseState  = {0};
LPDIRECTINPUTDEVICE8    g_pKeyboardDevice   = NULL;
int                     g_KeyboardAcquire   = 0;
char                    g_pKeyStateBuffer[256] = {0};
char                    g_LastpKeyStateBuffer[256] = {0};
CIOController           *g_pIoController=NULL;
int                      g_EscapeKey =DIK_RCONTROL;


void DirectInput_Fini()
{
    ULONG uret;
    if(g_pMouseDevice)
    {
        if(g_MouseAcquire)
        {
            g_pMouseDevice->Unacquire();
        }
        g_MouseAcquire = 0;
        uret = g_pMouseDevice->Release();
        if(uret != 0)
        {
            ERROR_INFO("MouseDevice Release Not 0 (%d)\n",uret);
        }
    }
    g_pMouseDevice = NULL;

    if(g_pKeyboardDevice)
    {
        if(g_KeyboardAcquire)
        {
            g_pKeyboardDevice->Unacquire();
        }
        g_KeyboardAcquire = 0;
        uret = g_pKeyboardDevice->Release();
        if(uret != 0)
        {
            ERROR_INFO("KeyBoardDevice Release Not 0 (%d)\n",uret);
        }
    }
    g_pKeyboardDevice = NULL;

    if(g_pDirectInput)
    {
        uret = g_pDirectInput->Release();
        if(uret != 0)
        {
            ERROR_INFO("DirectInput Release Not 0 (%d)\n",uret);
        }
    }
    g_pDirectInput = NULL;
    return ;
}

HRESULT DirectInput_Init(HWND hwnd,HINSTANCE hInstance)
{
    HRESULT hr;
    int ret;

    hr = DirectInput8Create(hInstance,0x800,(void**)&g_pDirectInput,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create DirectInput8 error(%d)\n",ret);
        goto fail;
    }

    hr = g_pDirectInput->CreateDevice(GUID_SysKeyboard,&g_pKeyboardDevice,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create SysKeyboard error(%d)\n",ret);
        goto fail;
    }

    hr = g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Set Keyboard Format error(%d)\n",ret);
        goto fail;
    }
    hr = g_pKeyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not SetCooperativeLevel error(%d)\n",ret);
        goto fail;
    }

    hr = g_pKeyboardDevice->Acquire();
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Keyboard Acquire error(%d)\n",ret);
        goto fail;
    }


    hr = g_pDirectInput->CreateDevice(GUID_SysMouse,&g_pMouseDevice,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create SysMouse error(%d)\n",ret);
        goto fail;
    }

    hr = g_pMouseDevice->SetDataFormat(&c_dfDIMouse);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Set Mouse Data Format error(%d)\n",ret);
        goto fail;
    }

    hr = g_pMouseDevice->Acquire();
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Acquire Mouse error(%d)\n",ret);
        goto fail;
    }



    return S_OK;
fail:
    DirectInput_Fini();
    SetLastError(ret);
    return hr;
}


BOOL Device_Read(IDirectInputDevice8* pDevice,void* pBuffer,long lSize)
{
    HRESULT hr;

    while(1)
    {
        pDevice->Poll();
        pDevice->Acquire();
        hr = pDevice->GetDeviceState(lSize,pBuffer);
        if(hr == DI_OK)
        {
            break;
        }
        if(hr != DIERR_INPUTLOST && hr != DIERR_NOTACQUIRED)
        {
            return FALSE;
        }

        hr = pDevice->Acquire();
        if(hr != DI_OK)
        {
            return FALSE;
        }
    }
    return TRUE;
}

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


BOOL CompareKeyBuffer(unsigned char* pCurBuffer,unsigned char* pLastBuffer,std::vector<DEVICEEVENT>& event)
{
    DEVICEEVENT evt;
    unsigned int i;
    for(i=0; i<256; i++)
    {
        if(i == g_EscapeKey)
        {
            continue;
        }

        if(pCurBuffer[i] != pLastBuffer[i])
        {
            evt.devtype = DEVICE_TYPE_KEYBOARD;
            evt.devid = 0;
            evt.event.keyboard.code = st_DIKMapCode[i];
            if(pCurBuffer[i])
            {
                evt.event.keyboard.event = KEYBOARD_EVENT_DOWN;
            }
            else
            {
                evt.event.keyboard.event = KEYBOARD_EVENT_UP;
            }

            event.push_back(evt);
        }
    }

    CopyMemory(pLastBuffer,pCurBuffer,256);

    return TRUE;
}

BOOL CompareMouseBuffer(DIMOUSESTATE* pCurState,DIMOUSESTATE* pLastState,std::vector<DEVICEEVENT>& event)
{
    DEVICEEVENT evt;
    if(pCurState->lX != 0 || pCurState->lY != 0)
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_MOUSE;
        evt.event.mouse.event = MOUSE_EVNET_MOVING;
        evt.event.mouse.x = pCurState->lX ;
        evt.event.mouse.y = pCurState->lY ;
        event.push_back(evt);
    }

    if(pCurState->lZ != 0)
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_MOUSE;
        evt.event.mouse.event = MOUSE_EVENT_SLIDE;
        evt.event.mouse.x = pCurState->lZ;
        event.push_back(evt);
    }

    if(pCurState->rgbButtons[0] != pLastState->rgbButtons[0])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_LEFTBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[0])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_UP;
        }
        event.push_back(evt)
    }


    if(pCurState->rgbButtons[1] != pLastState->rgbButtons[1])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_RIGHTBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[1])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_UP;
        }
        event.push_back(evt)
    }

    if(pCurState->rgbButtons[2] != pLastState->rgbButtons[2])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_MIDDLEBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[1])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_UP;
        }
        event.push_back(evt)
    }

    CopyMemory(pLastState,pCurState,sizeof(*pCurState));
    return TRUE;
}

void UpdateMouseBufferAfter(DIMOUSESTATE* pLastState)
{
    pLastState->lX = 0;
    pLastState->lY = 0;
    pLastState->lZ = 0;
}

BOOL UpdateCodeMessage()
{
    std::vector<DEVICEEVENT> event;
    if(g_pIoController == NULL)
    {
        return TRUE;
    }

    /*now we should check if the specified key is pressed*/
    Device_Read(g_pKeyboardDevice,g_pKeyStateBuffer,256);
    Device_Read(g_pMouseDevice,g_diMouseState,sizeof(g_diMouseState));

    if(g_pKeyStateBuffer[g_EscapeKey])
    {
        /*it is the escape key pressed ,so we do not handle any more*/
        return;
    }

    /*now compare the key difference*/
    CompareKeyBuffer(g_pKeyStateBuffer,g_LastpKeyStateBuffer,event);
    CompareMouseBuffer(&g_diMouseState,&g_LastdiMouseState,event);

    while(event.size()>0)
    {
        g_pIoController->PushEvent(&(event[0]));
        event.erase(event.begin());
    }

    return TRUE;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    HMENU hMenu=NULL;
    HWND hwnd = NULL;
    HRESULT hr;
    MSG msg= {0};

    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX) ;
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra		= 0;
    wndClass.cbWndExtra		= 0;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = _T("IOControlDemo");

    if(!RegisterClassEx(&wndClass))
        return -1;
    hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN_MENU));
    if(hMenu == NULL)
    {
        return -3;
    }
    hwnd = CreateWindow(_T("IOControlDemo"),WINDOW_TITLE,
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH,
                        SCREEN_HEIGHT, NULL, hMenu, hInstance, NULL);

    hr = DirectInput_Init(hwnd,hInstance);
    if(hr != S_OK)
    {
        ERROR_INFO("Could not Init DirectInput\n");
        return -4;
    }


    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		UpdateCodeMessage();

    }

    DirectInput_Fini();

    UnregisterClass(_T("IOControlDemo"), wndClass.hInstance);
    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        switch(wParam)
        {
        case ID_START_IO_INJECT:
			CStartIoDlg dlg;
			
			
            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

