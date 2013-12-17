
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>
#include <iocapctrl.h>
#include <output_debug.h>
#include "resource.h"
#include <uniansi.h>
#include <dllinsert.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


#ifdef _DEBUG
#pragma comment(lib,"injectctrld.lib")
#pragma comment(lib,"iocapctrld.lib")
#else
#pragma comment(lib,"injectctrl.lib")
#pragma comment(lib,"iocapctrl.lib")
#endif

LPDIRECTINPUT8          g_pKeyDirectInput      = NULL;
LPDIRECTINPUT8          g_pMouseDirectInput    = NULL;
LPDIRECTINPUTDEVICE8    g_pMouseDevice      = NULL;
int                     g_MouseAcquire      = 0;
DIMOUSESTATE            g_diMouseState      = {0};
DIMOUSESTATE            g_LastdiMouseState  = {0};
LPDIRECTINPUTDEVICE8    g_pKeyboardDevice   = NULL;
int                     g_KeyboardAcquire   = 0;
unsigned char                    g_KeyStateBuffer[256] = {0};
unsigned char                    g_LastpKeyStateBuffer[256] = {0};
CIOController           *g_pIoController=NULL;
HANDLE                   g_hProc = NULL;
int                      g_EscapeKey =DIK_RCONTROL;
int                      g_WaitTime = 0;
HINSTANCE                g_hInstance = NULL;


#define  MAX_STRING   256

#define  SCREEN_WIDTH  800
#define  SCREEN_HEIGHT 600

#ifdef _UNICODE
wchar_t  g_pExeStr[MAX_STRING];
wchar_t  g_pDllStr[MAX_STRING];
wchar_t  g_pParamStr[MAX_STRING];
wchar_t  g_pBufNumStr[MAX_STRING];
wchar_t  g_pBufSizeStr[MAX_STRING];
wchar_t  g_pWaitTimeStr[MAX_STRING];
#else
char  g_pExeStr[MAX_STRING];
char  g_pDllStr[MAX_STRING];
char  g_pParamStr[MAX_STRING];
char  g_pBufNumStr[MAX_STRING];
char  g_pBufSizeStr[MAX_STRING];
char  g_pWaitTimeStr[MAX_STRING];
#endif

void Process_Fini()
{
    if(g_pIoController)
    {
        delete g_pIoController ;
    }
    g_pIoController = NULL;
    if(g_hProc)
    {
        CloseHandle(g_hProc);
    }
    g_hProc = NULL;
}


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
    ZeroMemory(&g_diMouseState,sizeof(g_diMouseState));
    ZeroMemory(&g_LastdiMouseState,sizeof(g_LastdiMouseState));

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
    ZeroMemory(g_KeyStateBuffer,sizeof(g_KeyStateBuffer));
    ZeroMemory(g_LastpKeyStateBuffer,sizeof(g_LastpKeyStateBuffer));

    if(g_pKeyDirectInput)
    {
        uret = g_pKeyDirectInput->Release();
        if(uret != 0)
        {
            ERROR_INFO("KeyDirectInput Release Not 0 (%d)\n",uret);
        }
    }
    g_pKeyDirectInput = NULL;
    if(g_pMouseDirectInput)
    {
        uret = g_pMouseDirectInput->Release();
        if(uret != 0)
        {
            ERROR_INFO("MouseDirectInput Release Not 0 (%d)\n",uret);
        }
    }
    g_pMouseDirectInput = NULL;
    return ;
}

HRESULT DirectInput_Init(HWND hwnd,HINSTANCE hInstance)
{
    HRESULT hr;
    int ret;

    DirectInput_Fini();
    hr = DirectInput8Create(hInstance,0x800,IID_IDirectInput8,(void**)&g_pMouseDirectInput,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create DirectInput8 error(%d) (0x%08x)\n",ret,hr);
        goto fail;
    }

    hr = g_pMouseDirectInput->CreateDevice(GUID_SysMouse,&g_pMouseDevice,NULL);
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
    g_MouseAcquire = 1;

    hr = DirectInput8Create(hInstance,0x800,IID_IDirectInput8,(void**)&g_pKeyDirectInput,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create DirectInput8 error(%d) (0x%08x)\n",ret,hr);
        goto fail;
    }

    hr = g_pKeyDirectInput->CreateDevice(GUID_SysKeyboard,&g_pKeyboardDevice,NULL);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Create SysKeyboard error(%d) (0x%08x)\n",ret,hr);
        goto fail;
    }

    hr = g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Set Keyboard Format error(%d) (0x%08x)\n",ret,hr);
        goto fail;
    }
    hr = g_pKeyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if(hr != DI_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not SetCooperativeLevel error(%d) (0x%08x)\n",ret,hr);
        goto fail;
    }

    SetLastError(0);
    hr = g_pKeyboardDevice->Acquire();
    /*yes we may acquire this ,or not ,so do not mind this*/
    if(hr == DI_OK)
    {
        g_KeyboardAcquire = 1;
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
    ZeroMemory(pBuffer,lSize);

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

static IO_KEYBOARD_CODE_t st_DIKMapCode[256] =
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
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NUM_EQUALS          ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_PREV_TRACK          ,  /*145*/
    KEYBOARD_CODE_AT                 ,KEYBOARD_CODE_COLON               ,KEYBOARD_CODE_UNDERLINE              ,KEYBOARD_CODE_KANJI             ,KEYBOARD_CODE_STOP                ,  /*150*/
    KEYBOARD_CODE_AX                 ,KEYBOARD_CODE_UNLABELED           ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NEXT_TRACK        ,KEYBOARD_CODE_NULL                ,  /*155*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NUM_ENTER           ,KEYBOARD_CODE_RCONTROL               ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*160*/
    KEYBOARD_CODE_MUTE               ,KEYBOARD_CODE_CALCULATOR          ,KEYBOARD_CODE_PLAY_PAUSE             ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_MEDIA_STOP          ,  /*165*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_NULL                ,  /*170*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NULL                ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_NULL              ,KEYBOARD_CODE_VOLUME_DOWN         ,  /*175*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_VOLUME_UP           ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_WEB_HOME          ,KEYBOARD_CODE_NUM_COMMA           ,  /*180*/
    KEYBOARD_CODE_NULL               ,KEYBOARD_CODE_NUM_DIVIDE          ,KEYBOARD_CODE_NULL                   ,KEYBOARD_CODE_SYSRQ             ,KEYBOARD_CODE_RALT                ,  /*185*/
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

    if(pCurState->rgbButtons[LEFTBUTTON_IDX] != pLastState->rgbButtons[LEFTBUTTON_IDX])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_LEFTBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[LEFTBUTTON_IDX])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYUP;
        }
        event.push_back(evt);
    }


    if(pCurState->rgbButtons[RIGHTBUTTON_IDX] != pLastState->rgbButtons[RIGHTBUTTON_IDX])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_RIGHTBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[RIGHTBUTTON_IDX])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYUP;
        }
        event.push_back(evt);
    }

    if(pCurState->rgbButtons[MIDBUTTON_IDX] != pLastState->rgbButtons[MIDBUTTON_IDX])
    {
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_MIDDLEBUTTON;
        evt.event.mouse.x = 0;
        evt.event.mouse.y = 0;
        if(pCurState->rgbButtons[MIDBUTTON_IDX])
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYDOWN;
        }
        else
        {
            evt.event.mouse.event = MOUSE_EVENT_KEYUP;
        }
        event.push_back(evt);
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
    Device_Read(g_pKeyboardDevice,g_KeyStateBuffer,256);
    Device_Read(g_pMouseDevice,&g_diMouseState,sizeof(g_diMouseState));

    if(g_KeyStateBuffer[g_EscapeKey])
    {
        /*it is the escape key pressed ,so we do not handle any more*/
        return TRUE;
    }

    /*now compare the key difference*/
    CompareKeyBuffer(g_KeyStateBuffer,g_LastpKeyStateBuffer,event);
    CompareMouseBuffer(&g_diMouseState,&g_LastdiMouseState,event);

    while(event.size()>0)
    {
        g_pIoController->PushEvent(&(event[0]));
        event.erase(event.begin());
        UpdateMouseBufferAfter(&g_LastdiMouseState);
    }

    return TRUE;

}

BOOL StartExeProcess(HWND hwnd);
BOOL CALLBACK ShowDialogProc(HWND hwndDlg,
                             UINT message,
                             WPARAM wParam,
                             LPARAM lParam);

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
            INT_PTR nRet;
            nRet = DialogBox(g_hInstance,MAKEINTRESOURCE(IDD_START_DIALOG),hwnd,ShowDialogProc);
            if(nRet == IDOK)
            {
                StartExeProcess(hwnd);
            }

            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    HWND hwnd = NULL;
    HRESULT hr;
    MSG msg= {0};
    int ret;
    ATOM pAtom=NULL;

    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX) ;
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra		= 0;
    wndClass.cbWndExtra		= 0;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);
    wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
    wndClass.lpszClassName = TEXT("IOControlDemo");

    g_hInstance = hInstance;

    pAtom = RegisterClassEx(&wndClass);
    if(!pAtom)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register Class Error(%d)\n",ret);
        goto out;
    }
    hwnd = CreateWindow(TEXT("IOControlDemo"),TEXT("Demo Window"),
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH,
                        SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);

    hr = DirectInput_Init(hwnd,hInstance);
    if(hr != S_OK)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Could not Init DirectInput\n");
        goto out;
    }
    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        UpdateCodeMessage();
        Sleep(100);

    }

    ret = 0;
out:
    DirectInput_Fini();
    Process_Fini();

    if(pAtom)
    {
        UnregisterClass(TEXT("IOControlDemo"),hInstance);
    }
    pAtom = NULL;
    return -ret;
}


#ifdef _UNICODE
int StringLength(wchar_t* pString)
{
    return wcslen(pString);
}
BOOL GetDialogItemString(HWND hwndDlg,int nIDDlgItem,wchar_t *pString,int count)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ZeroMemory(pString,count*sizeof(pString[0]));
    SetLastError(0);
    ret = ::GetWindowText(hCtrlItem,pString,count);
    if(ret == 0 && GetLastError() != 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
    }

    return TRUE;

}

BOOL SetDialogItemString(HWND hwndDlg,int nIDDlgItem,wchar_t *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    BOOL bret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    bret = SetWindowText(hCtrlItem,pString);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}

BOOL InsertComboString(HWND hwndDlg,int nIDDlgItem,int idx,wchar_t *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ret = SendMessage(hCtrlItem, CB_INSERTSTRING,idx,(LPARAM)pString);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("InsertString <0x%08x>:%d  Error(%d) CB_ERR(%d)\n",hwndDlg,nIDDlgItem,idx,ret,CB_ERR);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}

BOOL SetComboSel(HWND hwndDlg,int nIDDlgItem,int idx)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ret = SendMessage(hCtrlItem,CB_SETCURSEL,idx,0);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        SendMessage(hCtrlItem,CB_SETCURSEL,0,0);
        ERROR_INFO("SetCurSel <0x%08x>:%d Idx(%d) CB_ERR(%d)  Error(%d)\n",hwndDlg,nIDDlgItem,idx,CB_ERR,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}


int GetComboSel(HWND hwndDlg,int nIDDlgItem)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return -1;
    }

    ret = SendMessage(hCtrlItem,CB_GETCURSEL,0,0);
    if(ret == CB_ERR)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("GetCurSel <0x%08x>  Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return -1;
    }

    return ret;
}



int SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...)
{
    int ret;
    va_list ap;

    va_start(ap,pfmt);

    ret =  _vsnwprintf_s(pString,count,_TRUNCATE,pfmt,ap);

    DEBUG_INFO("%S\n",pString);
    return ret;
}


#else

int StringLength(char * pString)
{
    return strlen(pString);
}

BOOL GetDialogItemString(HWND hwndDlg,int nIDDlgItem,char *pString,int count)
{
    HWND hCtrlItem=NULL;
    int ret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    ZeroMemory(pString,count*sizeof(pString[0]));
    SetLastError(0);
    ret = ::GetWindowText(hCtrlItem,pString,count);
    if(ret == 0 && GetLastError() != 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
    }

    return TRUE;
}

BOOL SetDialogItemString(HWND hwndDlg,int nIDDlgItem,char *pString)
{
    HWND hCtrlItem=NULL;
    int ret;
    BOOL bret;
    hCtrlItem = ::GetDlgItem(hwndDlg,nIDDlgItem);
    if(!hCtrlItem)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get <0x%08x>:%d DlgItem Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    bret = SetWindowText(hCtrlItem,pString);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set <0x%08x>:%d WindowText Error(%d)\n",hwndDlg,nIDDlgItem,ret);
        SetLastError(ret);
        return FALSE;
    }

    return TRUE;
}




void SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...)
{
    int ret;
    va_list ap;

    va_start(ap,fmt);

    ret = _vsnprintf_s(pString,count,_TRUNCATE,pfmt,ap);

    DEBUG_INFO("%s\n",pString);
    return ret;
}

#endif /*_UNICODE*/

BOOL StartExeProcess(HWND hwnd)
{
    char* pExeAnsi=NULL,*pDllAnsi=NULL,*pParamAnsi=NULL,*pCommandAnsi=NULL,*pPartDll=NULL;
#ifdef _UNICODE
    wchar_t errstr[MAX_STRING];
#else
    char errstr[MAX_STRING];
#endif
    uint32_t bufnum=0,bufsize=0;
    uint32_t keyboardid,mouseid;
    int ret;
    BOOL bret;
    uint32_t pid=0;
    int cmdsize=0;
#ifdef _UNICODE
    int exesize=0,dllsize=0,paramsize=0;
#endif
    /*now we should delete the */
    if(g_pIoController)
    {
        delete g_pIoController;
    }
    g_pIoController = NULL;
    if(g_hProc)
    {
        CloseHandle(g_hProc);
    }
    g_hProc = NULL;




    /*now first we should CreateProcess*/
#ifdef _UNICODE
    ret = UnicodeToAnsi(g_pExeStr,&pExeAnsi,&exesize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    ret = UnicodeToAnsi(g_pDllStr,&pDllAnsi,&dllsize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    ret = UnicodeToAnsi(g_pParamStr,&pParamAnsi,&paramsize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
#else
    pExeAnsi = (LPCSTR) g_pExeStr;
    pDllAnsi = (LPCSTR) g_pDllStr;
    pParamAnsi = (LPCSTR) g_pParamStr;
#endif

    pPartDll = strrchr(pDllAnsi,'\\');
    if(pPartDll == NULL)
    {
        pPartDll = pDllAnsi;
    }
    else
    {
        pPartDll += 1;
    }

    cmdsize = strlen(pExeAnsi);
    cmdsize += 10;
    cmdsize += strlen(pParamAnsi);

    pCommandAnsi = (char*)calloc(cmdsize,sizeof(*pCommandAnsi));
    if(pCommandAnsi == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    _snprintf_s(pCommandAnsi,cmdsize,_TRUNCATE,"%s %s",pExeAnsi,pParamAnsi);


    ret = LoadInsert(NULL,pCommandAnsi,pDllAnsi,pPartDll);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Could not Start exec(%s) param(%s) dll(%s) Error(%d)"),
                      pExeAnsi,pParamAnsi,pDllAnsi,ret);
        MessageBox(hwnd,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    pid = ret;

    DEBUG_INFO("Wait Before (%d)\n",g_WaitTime);
    if(g_WaitTime > 0)
    {
        Sleep(g_WaitTime * 1000);
    }
    DEBUG_INFO("Wait After (%d)\n",g_WaitTime);

    g_hProc = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
    if(g_hProc == NULL)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Could not OpenProcess(%d) Error(%d)"),
                      pid,ret);
        MessageBox(hwnd,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }


#if 1
    g_pIoController = new CIOController();
#ifdef _UNICODE
    bufnum = wcstoul(g_pBufNumStr, NULL, 10);
    bufsize = wcstoul(g_pBufSizeStr, NULL, 16);
#else
    bufnum = strtoul(g_pBufNumStr, NULL, 10);
    bufsize = strtoul(g_pBufSizeStr, NULL, 16);
#endif
    bret  = g_pIoController->Start(g_hProc,bufnum,bufsize);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Could not Start(0x%08x) bufnum(%d) bufsize(0x%08x) Error(%d)"),
                      g_hProc,bufnum,bufsize,ret);
        MessageBox(hwnd,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = g_pIoController->AddDevice(DEVICE_TYPE_KEYBOARD,&keyboardid);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Could not Add Keyboard Error(%d)"),
                      ret);
        MessageBox(hwnd,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = g_pIoController->AddDevice(DEVICE_TYPE_MOUSE,&mouseid);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Could not Add Mouse Error(%d)"),
                      ret);
        MessageBox(hwnd,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    DEBUG_INFO("Add Mouse %d KeyBoard %d\n",mouseid,keyboardid);
#endif

#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pExeAnsi,&exesize);
    UnicodeToAnsi(NULL,&pDllAnsi,&dllsize);
    UnicodeToAnsi(NULL,&pParamAnsi,&paramsize);
#else
    pExeAnsi = NULL;
    pDllAnsi = NULL;
    pParamAnsi = NULL;
#endif
    if(pCommandAnsi)
    {
        free(pCommandAnsi);
    }
    pCommandAnsi = NULL;



    return TRUE;

fail:
    assert(ret > 0);

    if(g_hProc)
    {
        CloseHandle(g_hProc);
    }
    g_hProc = NULL;
    if(g_pIoController)
    {
        delete g_pIoController;
    }
    g_pIoController = NULL;

#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pExeAnsi,&exesize);
    UnicodeToAnsi(NULL,&pDllAnsi,&dllsize);
    UnicodeToAnsi(NULL,&pParamAnsi,&paramsize);
#else
    pExeAnsi = NULL;
    pDllAnsi = NULL;
    pParamAnsi = NULL;
#endif
    if(pCommandAnsi)
    {
        free(pCommandAnsi);
    }
    pCommandAnsi = NULL;
    SetLastError(ret);
    return FALSE;
}


BOOL CheckDialogString(HWND hwndDlg)
{
    BOOL bret;
    int ret;
    wchar_t errstr[MAX_STRING];
    uint32_t bufnum,bufsize,waittime;
    bret = GetDialogItemString(hwndDlg,IDC_EDT_EXE,g_pExeStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get Exe Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = StringLength(g_pExeStr);
    if(ret == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(errstr,MAX_STRING,TEXT("Must Specify Exe"));
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = GetDialogItemString(hwndDlg,IDC_EDT_PARAM,g_pParamStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get Param Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = GetDialogItemString(hwndDlg,IDC_EDT_DLL,g_pDllStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get DLL Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }
    ret = StringLength(g_pDllStr);
    if(ret == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(errstr,MAX_STRING,TEXT("Must Specify Dll"));
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = GetDialogItemString(hwndDlg,IDC_EDT_BUFNUM,g_pBufNumStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get BufNum Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = GetDialogItemString(hwndDlg,IDC_EDT_BUFSIZE,g_pBufSizeStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get BufSize Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }
    bret = GetDialogItemString(hwndDlg,IDC_EDT_WAITTIME,g_pWaitTimeStr,MAX_STRING);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get WaitTime Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

#ifdef _UNICODE
    bufnum = wcstoul(g_pBufNumStr, NULL, 10);
    bufsize = wcstoul(g_pBufSizeStr, NULL, 16);
    waittime = wcstoul(g_pWaitTimeStr, NULL, 10);
#else
    bufnum = strtoul(g_pBufNumStr, NULL, 10);
    bufsize = strtoul(g_pBufSizeStr, NULL, 16);
    waittime = strtoul(g_pBufNumStr, NULL, 10);
#endif
    DEBUG_INFO("wait time %d (%S)\n",waittime,g_pWaitTimeStr);

    if(bufnum == 0 || bufsize < 32)
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(errstr,MAX_STRING,TEXT("Bufnum (%d  == 0) or BufSize(%d < 32)"),bufnum,bufsize);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    ret = GetComboSel(hwndDlg,IDC_COMBO_ESCAPE);
    if(ret == -1)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Get CurSel Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(ret == 0)
    {
        g_EscapeKey = DIK_RCONTROL;
    }
    else if(ret == 1)
    {
        g_EscapeKey = DIK_RWIN;
    }

    g_WaitTime = waittime;

    SetLastError(0);
    return TRUE;
fail:
    SetLastError(ret);
    return FALSE;
}



BOOL InitShowDialog(HWND hwndDlg)
{
    BOOL bret;
    int ret;
    wchar_t errstr[MAX_STRING];
    wchar_t selstr[MAX_STRING];
    bret = SetDialogItemString(hwndDlg,IDC_EDT_EXE,TEXT(""));
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Set Exe Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = SetDialogItemString(hwndDlg,IDC_EDT_PARAM,TEXT(""));
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Set Param Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = SetDialogItemString(hwndDlg,IDC_EDT_DLL,TEXT(""));
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get DLL Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    SprintfString(g_pBufNumStr,MAX_STRING,TEXT("10"));

    bret = SetDialogItemString(hwndDlg,IDC_EDT_BUFNUM,g_pBufNumStr);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Get BufNum Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    SprintfString(g_pBufSizeStr,MAX_STRING,TEXT("400"));
    bret = SetDialogItemString(hwndDlg,IDC_EDT_BUFSIZE,g_pBufSizeStr);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Set BufSize Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    SprintfString(selstr,MAX_STRING,TEXT("1"));
    bret = SetDialogItemString(hwndDlg,IDC_EDT_WAITTIME,selstr);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Set WaitTime Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    SprintfString(selstr,MAX_STRING,TEXT("RCONTROL"));
    bret = InsertComboString(hwndDlg,IDC_COMBO_ESCAPE,0,selstr);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Insert RCONTROL Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    SprintfString(selstr,MAX_STRING,TEXT("RWIN"));
    bret = InsertComboString(hwndDlg,IDC_COMBO_ESCAPE,1,selstr);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Insert RWIN Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }

    bret = SetComboSel(hwndDlg,IDC_COMBO_ESCAPE,0);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(errstr,MAX_STRING,TEXT("Can not Select 0 Error(%d)"),ret);
        MessageBox(hwndDlg,errstr,TEXT("Error"),MB_OK);
        goto fail;
    }


    SetLastError(0);
    return TRUE;
fail:
    SetLastError(ret);
    return FALSE;

}

TCHAR* FileDialogSelect(HWND hWnd,TCHAR *pFilter)
{
    OPENFILENAME ofn;
    std::auto_ptr<TCHAR> pBuffer2(new TCHAR[80*MAX_PATH]);
    TCHAR *pBuffer = pBuffer2.get();
    BOOL bret;
    int ret;
    UINT nLen;
    TCHAR *pReturn=NULL;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.Flags = OFN_EXPLORER ;
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = pBuffer;
    ofn.nMaxFile = 80*MAX_PATH*sizeof(TCHAR);
    ofn.lpstrFile[0] = '\0';
    ofn.lpstrFilter = pFilter;

    bret = GetOpenFileName(&ofn);
    if(bret)
    {
        nLen = lstrlen(pBuffer);
        pReturn = (TCHAR*)calloc(nLen + 1,sizeof(*pReturn));
        if(pReturn == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }
        lstrcpyn(pReturn,pBuffer,nLen + 1);
    }

    return pReturn;
}


BOOL SelectFileAndSetEdit(HWND hwnd,TCHAR* pFilter,UINT id)
{
    TCHAR* pFileName=NULL;
    HWND hEdt=NULL;
    BOOL bret=FALSE;
    int ret=0;

    pFileName = FileDialogSelect(hwnd,pFilter);
    if(pFileName == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    hEdt = ::GetDlgItem(hwnd,id);
    if(hEdt== NULL)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get (0x%08x) DlgItem(%d) Error(%d)\n",hwnd,id,ret);
        goto fail;
    }
    bret = ::SetWindowText(hEdt,pFileName);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set Window (0x%08x) Error(%d)\n",hEdt,ret);
        goto fail;
    }
    if(pFileName)
    {
        free(pFileName);
    }
    pFileName = NULL;

    SetLastError(0);
    return TRUE;
fail:
    assert(ret > 0);
    if(pFileName)
    {
        free(pFileName);
    }
    pFileName = NULL;
    SetLastError(ret);
    return FALSE;

}


BOOL CALLBACK ShowDialogProc(HWND hwndDlg,
                             UINT message,
                             WPARAM wParam,
                             LPARAM lParam)
{
    BOOL bret;
    switch(message)
    {
    case WM_INITDIALOG:
        InitShowDialog(hwndDlg);
        return TRUE;
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            bret = CheckDialogString(hwndDlg);
            if(bret)
            {
                EndDialog(hwndDlg,wParam);
                return TRUE;
            }
            break;
        case IDCANCEL:
            EndDialog(hwndDlg,wParam);
            return TRUE;
        case ID_BTN_EXE_SEL:
            SelectFileAndSetEdit(hwndDlg,TEXT("Exe File(*.exe)\0*.exe\0"),IDC_EDT_EXE);
            break;
        case ID_BTN_DLL_SEL:
            SelectFileAndSetEdit(hwndDlg,TEXT("Dll File(*.dll)\0*.dll\0"),IDC_EDT_DLL);
            break;
        }
        break;

    }

    return FALSE;
}

