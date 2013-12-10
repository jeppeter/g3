// ioctrlclient.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ioctrlclient.h"
#include <output_debug.h>
#include <iocapcommon.h>
#include <memory>
#include <vector>
#include <Winsock.h>
#include <uniansi.h>
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ws2_32.lib")

#define MAX_LOADSTRING 100

#define WM_SOCKET   (WM_USER + 1)
#define SOCKET_TIMEOUT   3000
#define SOCKET_TM_EVENTID 10003

static char g_Host[256];
static int g_Port;
static int g_EscapeKey=DIK_RCONTROL;
static SOCKET g_Socket=INVALID_SOCKET;
static UINT_PTR g_SocketConnTimer=0;
static int g_Connected=0;
static int g_writesize=0;
static HWND g_hWnd=NULL;
static std::vector<DEVICEEVENT> st_DevEvent;

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
    if(g_Socket == INVALID_SOCKET)
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
        DEVICEEVENT devevent;
        devevent = event[0];
        st_DevEvent.push_back(devevent);
        event.erase(event.begin());
    }
    UpdateMouseBufferAfter(&g_LastdiMouseState);

    return TRUE;

}


// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR    lpCmdLine,
                       _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO:  在此放置代码。
    MSG msg;
    HACCEL hAccelTable;
    WSADATA wsaData;
    HRESULT hr;
    int ret;
    std::auto_ptr<TCHAR> pErrStr2(new TCHAR[256]);
    TCHAR *pErrStr=pErrStr2.get();

    hInst = hInstance;
    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_IOCTRLCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    ret = WSAStartup(MAKEWORD(2, 2),&wsaData);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
#ifdef _UNICODE
        _snwprintf_s(pErrStr,256,_TRUNCATE,TEXT("Init Sock Error(%d)"),ret);
#else
        _snprintf_s(pErrStr,256,_TRUNCATE,"Init Sock Error(%d)",ret);
#endif
        ::MessageBox(g_hWnd,pErrStr,TEXT("Error"),MB_OK);
        goto out;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IOCTRLCLIENT));

    hr = DirectInput_Init(g_hWnd,hInst);
    if(hr != S_OK)
    {
    	ret = LAST_ERROR_CODE();
#ifdef _UNICODE
        _snwprintf_s(pErrStr,256,_TRUNCATE,TEXT("Init Directinput Error(%d)"),ret);
#else
        _snprintf_s(pErrStr,256,_TRUNCATE,"Init Directinput Error(%d)",ret);
#endif
        ::MessageBox(g_hWnd,pErrStr,TEXT("Error"),MB_OK);
		goto out;
    }

    ZeroMemory(&msg,sizeof(msg));
    // 主消息循环:
    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        UpdateCodeMessage();
    }
	ret = 0;
out:
	DirectInput_Fini();
    WSACleanup();

    return ret;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IOCTRLCLIENT));
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IOCTRLCLIENT);
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    hInst = hInstance; // 将实例句柄存储在全局变量中

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if(!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_hWnd = hWnd;

    return TRUE;
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


BOOL InitializeConnectDialog(HWND hwnd)
{
    BOOL bret;
    int ret;
#ifdef _UNICODE
    std::auto_ptr<wchar_t> pChar2(new wchar_t[256]);
    wchar_t* pChar = pChar2.get();
#else
    std::auto_ptr<char> pChar2(new char[256]);
    char* pChar = pChar2.get();
#endif

    SprintfString(pChar,256,TEXT("3391"));
    bret = SetDialogItemString(hwnd,IDC_EDT_PORT,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set IDC_EDT_PORT Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RCONTROL"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,0,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RWIN"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,1,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SprintfString(pChar,256,TEXT("RSHIFT"));
    bret = InsertComboString(hwnd,IDC_COMBO_ESCAPE,2,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Insert IDC_COMBO_ESCAPE Error(%d)\n",ret);
        goto fail;
    }
    SetComboSel(hwnd,IDC_COMBO_ESCAPE,0);

    SetLastError(0);
    return TRUE;
fail:
    SetLastError(ret);
    return FALSE;
}

BOOL GetConnectParam(HWND hwnd)
{
    BOOL bret;
    int ret;
    char* pHostAnsi=NULL;
    int hostansisize=0;
    int port;
#ifdef _UNICODE
    std::auto_ptr<wchar_t> pChar2(new wchar_t[256]),pErrStr2(new wchar_t[256]);
    wchar_t* pChar = pChar2.get();
    wchar_t* pErrStr = pErrStr2.get();
#else
    std::auto_ptr<char> pChar2(new char[256]),pErrStr2(new char[256]);
    char* pChar = pChar2.get();
    char *pErrStr = pErrStr2.get();
#endif

    bret = GetDialogItemString(hwnd,IDC_EDT_HOST,pChar,256);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_EDT_HOST Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }
#ifdef _UNICODE
    ret = UnicodeToAnsi(pChar,&pHostAnsi,&hostansisize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
#else
    pHostAnsi = pChar;
    hostansisize = strlen(pHostAnsi) + 1;
#endif

    if(strlen(pHostAnsi) == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(pErrStr,256,TEXT("Host must specify"));
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

    strncpy_s(g_Host,sizeof(g_Host),pHostAnsi,_TRUNCATE);
    bret = GetDialogItemString(hwnd,IDC_EDT_PORT,pChar,256);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_EDT_PORT Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

#ifdef _UNICODE
    port = _wtoi(pChar);
#else
    port = atoi(pChar);
#endif
    if(port == 0 || port >= (1<<16))
    {
        ret = ERROR_INVALID_PARAMETER;
        SprintfString(pErrStr,256,TEXT("Port (%d) Not valid"),port);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }
    g_Port = port;


    port = GetComboSel(hwnd,IDC_COMBO_ESCAPE);
    if(port < 0)
    {
        ret =LAST_ERROR_CODE();
        SprintfString(pErrStr,256,TEXT("Get IDC_COMBO_ESCAPE Error(%d)"),ret);
        ::MessageBox(hwnd,pErrStr,TEXT("Error"),MB_OK);
        goto fail;
    }

    if(port == 0)
    {
        g_EscapeKey = DIK_RCONTROL;
    }
    else if(port == 1)
    {
        g_EscapeKey = DIK_RWIN;
    }
    else if(port ==2)
    {
        g_EscapeKey = DIK_RSHIFT;
    }



    SetLastError(ret);
    return TRUE;
fail:
#ifdef _UNICODE
    UnicodeToAnsi(NULL,&pHostAnsi,&hostansisize);
#else
    pHostAnsi = NULL;
    hostansisize = 0;
#endif
    SetLastError(ret);
    return FALSE;
}


BOOL CALLBACK ConnectDialogProc(HWND hwndDlg,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
    BOOL bret=FALSE;
    switch(message)
    {
    case WM_INITDIALOG:
        bret = InitializeConnectDialog(hwndDlg);
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        bret = TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg,wParam);
            bret = TRUE;
            break;
        case IDOK:
            bret = GetConnectParam(hwndDlg);
            if(bret)
            {
                EndDialog(hwndDlg,wParam);
                bret = TRUE;
            }
            break;
        }
        break;
    default:
        break;
    }

    return bret;
}



void StopConnect(HWND hwnd)
{
    BOOL bret;
    int ret;
    if(g_SocketConnTimer != 0)
    {
        bret = KillTimer(hwnd,g_SocketConnTimer);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("KillTimer hwnd(0x%08x)(%d) Error(%d)\n",hwnd,g_SocketConnTimer,ret);
        }
    }
    g_SocketConnTimer= 0;
    if(g_Socket != INVALID_SOCKET)
    {
        closesocket(g_Socket);
    }
    g_Socket= INVALID_SOCKET;
    g_Connected = 0;

    st_DevEvent.clear();
    return ;
}


BOOL StartConnect(HWND hwnd)
{
    int ret;
    struct sockaddr_in saddr;
    u_long nonblock;
    UINT_PTR timeret;
    StopConnect(hwnd);

    /*now first to make socket*/
    g_Socket = socket(AF_INET,SOCK_STREAM,0);
    if(g_Socket == INVALID_SOCKET)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Socket Stream Error(%d)\n",ret);
        goto fail;
    }
    nonblock = 1;
    ret = ioctlsocket(g_Socket,FIONBIO,&nonblock);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("ioctl nonblock Error(%d)\n",ret);
        goto fail;
    }

    ZeroMemory(&saddr,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(g_Host);
    saddr.sin_port = htons(g_Port);

    ret = connect(g_Socket,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret == SOCKET_ERROR)
    {
        ret=WSAGetLastError() ? WSAGetLastError() : 1;
        if(ret != WSAEWOULDBLOCK  &&
                ret != WSAEINPROGRESS)
        {
            ERROR_INFO("connect (%s:%d) Error(%d)\n",g_Host,g_Port,ret);
            goto fail;
        }
        else
        {
            timeret = SetTimer(hwnd,SOCKET_TM_EVENTID,SOCKET_TIMEOUT,NULL);
            if(timeret == 0)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("SetTimer hwnd(0x%08x) (%d) Error(%d)\n",hwnd,SOCKET_TM_EVENTID,ret);
                goto fail;
            }
            g_SocketConnTimer = timeret;
        }
    }
    else
    {
        /*we have connected*/
        g_Connected = 1;
    }

    ret = WSAAsyncSelect(g_Socket,hwnd,WM_SOCKET,FD_CONNECT|FD_WRITE|FD_CLOSE);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Set hwnd(0x%08x) Select Error(%d)\n",hwnd,ret);
        goto fail;
    }


    SetLastError(0);
    return TRUE;
fail:
    StopConnect(hwnd);
    SetLastError(ret);
    return FALSE;
}


BOOL CallConnectFunction(HWND hwnd)
{
    INT_PTR nRet;
    nRet = DialogBox(hInst,MAKEINTRESOURCE(IDD_DLG_CONNECT),hwnd,ConnectDialogProc);
    if(nRet == IDOK)
    {
        StartConnect(hwnd);
    }

    return TRUE;
}

void CallStopFunction(HWND hwnd)
{
    StopConnect(hwnd);
    return ;
}


int WriteDeviceEvent(HWND hwnd)
{
    int cnt=0;
    DEVICEEVENT devevent;
    char* pBuf=NULL;
    int ret;
    std::auto_ptr<TCHAR> pChar2(new TCHAR[256]);
    TCHAR *pChar=pChar2.get();

    if(g_Socket == INVALID_SOCKET || g_Connected == 0)
    {
        return 0;
    }

    if(st_DevEvent.size() > 20)
    {
        ERROR_INFO("DevEvent.size(%d)\n",st_DevEvent.size());
    }

    while(st_DevEvent.size() > 0)
    {
        devevent = st_DevEvent[0];
        pBuf = (char*)&(devevent);
        pBuf += g_writesize;
        ret = send(g_Socket,pBuf,(sizeof(devevent)-g_writesize),0);
        if(ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError() ? WSAGetLastError() : 1;
            if(ret != WSAEWOULDBLOCK)
            {
                StopConnect(hwnd);
                SprintfString(pChar,256,TEXT("Write(%s:%d) Error(%d)"),g_Host,g_Port,ret);
                ::MessageBox(hwnd,pChar,TEXT("Error"),MB_OK);
                SetLastError(ret);
                return -ret;
            }
            DEBUG_INFO("Write Device Block\n");
            return cnt;
        }
        g_writesize += ret;
        if(g_writesize >= sizeof(devevent))
        {
            st_DevEvent.erase(st_DevEvent.begin());
            g_writesize = 0;
            cnt ++;
        }
        else
        {
            /*we can not write any more ,so just return */
            return cnt;
        }
    }

    return cnt;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    DWORD event;
    BOOL bret;
    int ret,res;
    std::auto_ptr<TCHAR> pChar2(new TCHAR[256]);
    TCHAR *pChar=pChar2.get();

    switch(message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // 分析菜单选择:
        switch(wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_CONTROL_CONNECT:
            CallConnectFunction(hWnd);
            break;
        case ID_CONTROL_DISCONNECT:
            CallStopFunction(hWnd);
            break;
        case ID_CONTROL_EXIT:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO:  在此添加任意绘图代码...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if(wParam == g_SocketConnTimer && g_SocketConnTimer != 0)
        {
            ret = WSAGetLastError() ? WSAGetLastError() : 1;
            if(g_Socket != INVALID_SOCKET)
            {
                int error;
                int errsize;
                errsize = sizeof(error);
                error = 0;
                res = getsockopt(g_Socket,SOL_SOCKET,SO_ERROR,(char*)&error,&errsize);
                if(res == 0)
                {
                    ret = error;
                }
            }
            /*now we should disconnect*/
            StopConnect(hWnd);
            SprintfString(pChar,256,TEXT("Connect(%s:%d) Error(%d)"),g_Host,g_Port,ret);
            ::MessageBox(hWnd,pChar,TEXT("Error"),MB_OK);
        }
        break;
    case WM_SOCKET:
        event = WSAGETSELECTEVENT(lParam);
        if(event & FD_CONNECT)
        {
            if(g_Socket == INVALID_SOCKET || g_Connected == 1)
            {
                ERROR_INFO("Not valid socket for FD_CONNECT\n");
                break;
            }
            g_Connected = 1;
            bret =KillTimer(hWnd,g_SocketConnTimer);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("Kill Timer(0x%08x) (%d) Error(%d)\n",hWnd,g_SocketConnTimer,ret);
            }
            g_SocketConnTimer = 0;
        }
        else if(event & FD_CLOSE)
        {
            StopConnect(hWnd);
            SprintfString(pChar,256,TEXT("(%s:%d) Closed"),g_Host,g_Port);
            ::MessageBox(hWnd,pChar,TEXT("Error"),MB_OK);
        }
        else if(event & FD_WRITE)
        {
            WriteDeviceEvent(hWnd);
        }
        else
        {
            ERROR_INFO("Event is 0x%08x\n",event);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

