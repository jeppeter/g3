// ioctrlclient.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ioctrlclient.h"
#include <output_debug.h>
#include <iocapcommon.h>
#include <memory>
#include <vector>
#include <Winsock2.h>
#include <uniansi.h>
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>
#include <assert.h>
#include <injectctrl.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Ws2_32.lib")

#define MAX_LOADSTRING 100

#define SOCKET_TIMEOUT   3000
#define SOCKET_TM_EVENTID 10003
#define SOCKET_WRITE_EVENTID  10004

static char g_Host[256];
static int g_Port;
static int g_EscapeKey=DIK_RCONTROL;
static HWND g_hWnd=NULL;
static CRITICAL_SECTION st_DevEventCS;
static std::vector<DEVICEEVENT> st_DevEvent;
static thread_control_t st_SockThreadCtrl= {0};
static SOCKET g_Socket=INVALID_SOCKET;
static int g_ResetMouse=0;

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
int                     g_LastPressKey = 0;
int                     g_LastUnPressKey = 0;
LONG              g_LastPressedTimes=0;
LONG              g_AbsPosX = 0;
LONG              g_AbsPosY = 0;
LONG              g_AbsPosXHex = 0;
LONG              g_AbsPosYHex = 0;
LONG              g_AbsLastSend =0;


int InsertDevEvent(DEVICEEVENT *pDevEvent,int back);
BOOL SetAbsPos(HWND hwnd);

#ifdef _UNICODE
int SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...);
#else
int SprintfString(char* pString ,int count,const char* pfmt,...);
#endif

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
    TCHAR errstr[256];
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
                g_LastPressKey = i;
                g_LastPressedTimes = 1;
                g_LastUnPressKey = 0;
            }
            else
            {
                if(g_LastUnPressKey == i)
                {
                    SprintfString(errstr,256,TEXT("Double up Key (0x%02x:%d)"),g_LastUnPressKey,g_LastUnPressKey);
                    ::MessageBox(g_hWnd,errstr,TEXT("Notice"),MB_OK);
                }
                evt.event.keyboard.event = KEYBOARD_EVENT_UP;
                g_LastPressKey = 0;
                g_LastPressedTimes = 0;
                g_LastUnPressKey = i;
            }
            pLastBuffer[i] = pCurBuffer[i];

            event.push_back(evt);
        }
        else if(pCurBuffer[i] && i == g_LastPressKey)
        {
            g_LastPressedTimes ++;
            if(g_LastPressedTimes >= 17)
            {
                evt.devtype = DEVICE_TYPE_KEYBOARD;
                evt.devid = 0;
                evt.event.keyboard.code = st_DIKMapCode[i];
                evt.event.keyboard.event = KEYBOARD_EVENT_DOWN;
                event.push_back(evt);
            }
        }
    }


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
    int ret;
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
        SetAbsPos(g_hWnd);
        g_KeyStateBuffer[g_EscapeKey] = 0;
        g_AbsLastSend = 1;
        return TRUE;
    }


    if(g_AbsLastSend)
    {
        /*we do not send the last send mouse move*/
        g_AbsLastSend = 0;
        return TRUE;
    }

    /*now compare the key difference*/
    CompareKeyBuffer(g_KeyStateBuffer,g_LastpKeyStateBuffer,event);
    CompareMouseBuffer(&g_diMouseState,&g_LastdiMouseState,event);


    if(event.size() > 20)
    {
        DEBUG_INFO("st_DevEvent size(%d) event.size(%d)\n",st_DevEvent.size(),event.size());
    }
    while(event.size()>0)
    {
        DEVICEEVENT devevent;
        devevent = event[0];
        ret = InsertDevEvent(&devevent,1);
        if(ret < 0)
        {
            ERROR_INFO("Could Not Insert devevent\n");
            return FALSE;
        }
        event.erase(event.begin());
    }
    UpdateMouseBufferAfter(&g_LastdiMouseState);

    return TRUE;

}

SOCKET ConnectSocket(char* pIp,int port,int *pConnected)
{
    SOCKET sock=INVALID_SOCKET;
    int ret;
    u_long nonblock;
    char nodelay;
    struct sockaddr_in saddr;
    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock == INVALID_SOCKET)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Socket Stream Error(%d)\n",ret);
        goto fail;
    }
    nonblock = 1;
    ret = ioctlsocket(sock,FIONBIO,&nonblock);
    if(ret != NO_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("ioctl nonblock Error(%d)\n",ret);
        goto fail;
    }

    nodelay = 1;
    ret = setsockopt(sock,IPPROTO_TCP,   TCP_NODELAY,  &nodelay,sizeof(nodelay));
    if(ret == SOCKET_ERROR)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("set no delay Error(%d)\n",ret);
        goto fail;
    }
    DEBUG_INFO("Set Nodelay\n");

    ZeroMemory(&saddr,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(pIp);
    saddr.sin_port = htons(port);

    ret = connect(sock,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret == SOCKET_ERROR)
    {
        ret=WSAGetLastError() ? WSAGetLastError() : 1;
        if(ret != WSAEWOULDBLOCK  &&
                ret != WSAEINPROGRESS)
        {
            ERROR_INFO("connect (%s:%d) Error(%d)\n",pIp,port,ret);
            goto fail;
        }
        *pConnected = 0;
    }
    else
    {
        *pConnected = 1;
    }

    return sock;

fail:
    assert(ret > 0);
    if(sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
    sock = INVALID_SOCKET;
    SetLastError(ret);
    return INVALID_SOCKET;
}

int InsertDevEvent(DEVICEEVENT *pDevEvent,int back)
{
    int ret =0;
    EnterCriticalSection(&st_DevEventCS);
    if(st_DevEvent.size() > 20)
    {
        ret = -ERROR_SHARING_BUFFER_EXCEEDED;
		ERROR_INFO("Event not send more than 20\n");
        goto unlock;
    }
    if(back)
    {
        st_DevEvent.push_back(*pDevEvent);
    }
    else
    {
        st_DevEvent.insert(st_DevEvent.begin(),*pDevEvent);
    }

unlock:
    LeaveCriticalSection(&st_DevEventCS);
    return ret;
}

int GetDevEvent(DEVICEEVENT& devevent)
{
    int ret = 0;
    EnterCriticalSection(&st_DevEventCS);
    if(st_DevEvent.size() > 0)
    {
        devevent = st_DevEvent[0];
        st_DevEvent.erase(st_DevEvent.begin());
        ret = 1;
    }
    LeaveCriticalSection(&st_DevEventCS);
    return ret;
}

int HandleSendDevEvent(SOCKET sock,int& haswrite,int& blocked)
{
    DEVICEEVENT devevent;
    int curwrite=haswrite;
    int ret;
    int cnt =0;
    char* pCurPtr;

    pCurPtr = (char*)&devevent;
    while(1)
    {
        ret = GetDevEvent(devevent);
        if(ret == 0)
        {
            break;
        }

        ret = send(sock,(pCurPtr+curwrite),sizeof(devevent) - curwrite,0);
        if(ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError() ? WSAGetLastError() : 1;
            if(ret != WSAEWOULDBLOCK)
            {
                ERROR_INFO("Send Socket Error(%d)\n",ret);
                goto fail;
            }
            ret = InsertDevEvent(&devevent,0);
            if(ret < 0)
            {
                ret = ERROR_SHARING_BUFFER_EXCEEDED;
                goto fail;
            }
            blocked = 1;
            haswrite = curwrite;
            return cnt;
        }

        if((curwrite + ret) >= sizeof(devevent))
        {
            cnt ++;
            curwrite = 0;
            if(devevent.devtype == DEVICE_TYPE_KEYBOARD)
            {
                DEBUG_INFO("HandleEvent(0x%08x) keyevent(0x%08x:%d) keycode (0x%08x:%d)\n",GetTickCount(),devevent.event.keyboard.event,devevent.event.keyboard.event,devevent.event.keyboard.code,devevent.event.keyboard.code);
            }
        }
        else
        {
            ret = InsertDevEvent(&devevent,0);
            if(ret < 0)
            {
                ret = ERROR_SHARING_BUFFER_EXCEEDED;
                goto fail;
            }
            blocked = 1;
            curwrite += ret;
            haswrite = curwrite;
            return cnt;
        }
    }

    return cnt;
fail:
    SetLastError(ret);
    return -ret;
}

#define  MAX_ERROR_SIZE   256

DWORD WINAPI SocketThreadImpl(LPVOID lparam)
{
    thread_control_t *pThreadControl= (thread_control_t*)lparam;
    SOCKET sock=INVALID_SOCKET;
    int ret;
    int connected=0;
#ifdef _UNICODE
    std::auto_ptr<wchar_t> pChar2(new wchar_t[MAX_ERROR_SIZE]);
    wchar_t* pChar = pChar2.get();
#else
    std::auto_ptr<char> pChar2(new char[MAX_ERROR_SIZE]);
    char* pChar = pChar2.get();
#endif
    HANDLE* pWaitHandle=NULL;
    int waitnum = 2;
    int haswrite=0,blocked=0;
    DWORD dret;
    BOOL bret;
    DWORD stick,ctick,etick;


    sock =ConnectSocket(g_Host,g_Port,&connected);
    if(sock == INVALID_SOCKET)
    {
        ret = LAST_ERROR_CODE();
        goto out;
    }
    g_Socket = sock;

    /*now check if we have the connected*/
    pWaitHandle = (HANDLE*)calloc(2,sizeof(*pWaitHandle));
    if(pWaitHandle == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto out;
    }
    pWaitHandle[0] = NULL;
    pWaitHandle[1] = WSA_INVALID_EVENT;

    pWaitHandle[0] = pThreadControl->exitevt;
    pWaitHandle[1] = WSACreateEvent();
    if(pWaitHandle[1] == WSA_INVALID_EVENT)
    {
        ret = WSAGetLastError() ? WSAGetLastError() : 1;
        ERROR_INFO("Create Accept Event Error(%d)\n",ret);
        SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Create Accept Event Error(%d)"),ret);
        ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
        goto out;
    }

    if(connected)
    {
        ret = WSAEventSelect(sock,pWaitHandle[1],FD_CLOSE);
        if(ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError() ? WSAGetLastError() : 1;
            ERROR_INFO("Select Accept Event Error(%d)\n",ret);
            SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Select Accept Event Error(%d)"),ret);
            ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
            goto out;
        }
    }
    else
    {
        ret = WSAEventSelect(sock,pWaitHandle[1],FD_CONNECT|FD_CLOSE);
        if(ret == SOCKET_ERROR)
        {
            ret = WSAGetLastError() ? WSAGetLastError() : 1;
            ERROR_INFO("Select Connect Event Error(%d)\n",ret);
            SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Select Connect Event Error(%d)"),ret);
            ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
            goto out;
        }
        stick = GetTickCount();
        etick = stick + 3000;
        ctick = stick;
    }
    waitnum = 2;

    while(pThreadControl->running)
    {
        /*we wait for the time*/
        dret = WaitForMultipleObjects(waitnum,pWaitHandle,FALSE,5);
        if(dret == WAIT_OBJECT_0)
        {
            ERROR_INFO("exit notify\n");
        }
        else if(dret == (WAIT_OBJECT_0 + 1))
        {

            bret = WSAResetEvent(pWaitHandle[1]);
            if(!bret)
            {
                ret = WSAGetLastError() ? WSAGetLastError() : 1;
                ERROR_INFO("Reset Event Error(%d)\n",ret);
                SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Reset Event Error(%d)"),ret);
                ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
                goto out;
            }
            if(connected == 0)
            {
                ret = WSAEventSelect(sock,pWaitHandle[1],FD_CLOSE);
                if(ret == SOCKET_ERROR)
                {
                    ret = WSAGetLastError() ? WSAGetLastError() : 1;
                    ERROR_INFO("Select Close Event Error(%d)\n",ret);
                    SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Select Close Event Error(%d)"),ret);
                    ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
                    goto out;
                }
                connected = 1;
            }
            else
            {
                ERROR_INFO("Closed\n");
                SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Remote Closed"));
                ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
                ret = 0;
                goto out;
            }
        }
        else if(dret == WAIT_TIMEOUT)
        {
            if(connected)
            {
                ret = HandleSendDevEvent(sock,haswrite,blocked);
                if(ret < 0)
                {
                    ret = LAST_ERROR_CODE();
                    SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Send Event Error(%d)"),ret);
                    ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
                    goto out;
                }

            }
            else
            {
                ctick = GetTickCount();
                if(ctick > etick)
                {
                    ret = ERROR_CONNECTION_UNAVAIL;
                    ERROR_INFO("Connect(%s:%d) Failed\n",g_Host,g_Port);
                    SprintfString(pChar,MAX_ERROR_SIZE,TEXT("Connect(%s:%d) Failed"),g_Host,g_Port);
                    ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
                    goto out;
                }
            }
        }
        else if(dret == WAIT_FAILED)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Wait Error(%d)\n",ret);
            SprintfString(pChar,MAX_ERROR_SIZE,TEXT("wait failed (%d)"),ret);
            ::MessageBox(g_hWnd,pChar,TEXT("Error"),MB_OK);
            goto out;
        }
    }

    ret = 0;

out:
    if(pWaitHandle)
    {
        if(pWaitHandle[1] != WSA_INVALID_EVENT)
        {
            WSACloseEvent(pWaitHandle[1]);
        }
        pWaitHandle[1] = WSA_INVALID_EVENT;
        free(pWaitHandle);
    }
    pWaitHandle = NULL;
    if(sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
    sock = INVALID_SOCKET;
    g_Socket = sock;
    SetLastError(ret);
    pThreadControl->exited = 1;
    return (DWORD) -ret;
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

    InitializeCriticalSection(&st_DevEventCS);
    ZeroMemory(&st_SockThreadCtrl,sizeof(st_SockThreadCtrl));
    st_SockThreadCtrl.exited = 1;
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
        Sleep(30);
        UpdateCodeMessage();
    }
    ret = 0;
out:
    DirectInput_Fini();
    WSACleanup();

    DeleteCriticalSection(&st_DevEventCS);
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

LONG StrToUL(wchar_t* pString,wchar_t**ppEnd,int base)
{

    return  wcstoul(pString,ppEnd,base);
}

int TStrNCmp(wchar_t* pString ,wchar_t* pCmp,int num)
{
    return wcsncmp(pString,pCmp,num);
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




int SprintfString(wchar_t* pString ,int count,const wchar_t* pfmt,...)
{
    int ret;
    va_list ap;

    va_start(ap,fmt);

    ret = _vsnprintf_s(pString,count,_TRUNCATE,pfmt,ap);

    DEBUG_INFO("%s\n",pString);
    return ret;
}

LONG StrToUL(char* pString,char**ppEnd,int base)
{

    return  strtoul(pString,ppEnd,base);
}

int TStrNCmp(wchar_t* pString ,wchar_t* pCmp,int num)
{
    return strncmp(pString,pCmp,num);
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

int GetCheckSel(HWND hwndDlg,int nIDDlgItem)
{
    HWND hCtrlItem=NULL;
    int ret=0;
    UINT checked;

    checked =  IsDlgButtonChecked(hwndDlg,nIDDlgItem);
    if(checked == BST_CHECKED)
    {
        ret = 1;
    }
    return ret;
}

int SetCheckSel(HWND hwndDlg,int nIDDlgItem,int checked)
{
    BOOL bret;
    UINT setchecked=BST_UNCHECKED;
    int ret=0;

    if(checked)
    {
        setchecked = BST_CHECKED;
    }

    bret = CheckDlgButton(hwndDlg,nIDDlgItem,setchecked);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("<0x%08x>:%d Set Check(%d) Error(%d)\n",hwndDlg,nIDDlgItem,checked,ret);
        SetLastError(ret);
        return -1;
    }
    return 0;
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

    SprintfString(pChar,256,TEXT("127.0.0.1"));
    bret = SetDialogItemString(hwnd,IDC_EDT_HOST,pChar);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Set IDC_EDT_HOST Error(%d)\n",ret);
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
    SetCheckSel(hwnd,IDC_CHK_RESETMOUSE,1);

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
    int resetmouse=0;
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
    DEBUG_INFO("pHostAnsi(%s) g_Host(%s)\n",pHostAnsi,g_Host);
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

	g_ResetMouse = GetCheckSel(hwnd,IDC_CHK_RESETMOUSE);



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


BOOL InitialializeAbsPosDialog(HWND hwnd)
{
    TCHAR str[32];
    int ret;
    BOOL bret;
    SetCheckSel(hwnd,IDC_CHK_XPOS,g_AbsPosXHex);
    SetCheckSel(hwnd,IDC_CHK_YPOS,g_AbsPosYHex);
    if(g_AbsPosXHex == 0)
    {
        ret = SprintfString(str,32,TEXT("%d"),g_AbsPosX);
    }
    else
    {
        ret = SprintfString(str,32,TEXT("0x%x"),g_AbsPosX);
    }

    if(ret < 0)
    {
        return FALSE;
    }

    bret = SetDialogItemString(hwnd,IDC_EDT_XPOS,str);
    if(!bret)
    {
        return FALSE;
    }

    if(g_AbsPosYHex == 0)
    {
        ret = SprintfString(str,32,TEXT("%d"),g_AbsPosY);
    }
    else
    {
        ret = SprintfString(str,32,TEXT("0x%x"),g_AbsPosY);
    }

    if(ret < 0)
    {
        return FALSE;
    }

    bret = SetDialogItemString(hwnd,IDC_EDT_YPOS,str);
    if(!bret)
    {
        return FALSE;
    }


    return TRUE;
}

BOOL GetAbsPosParam(HWND hwnd)
{
    int ret;
    TCHAR xstr[32],ystr[32],*endstr=NULL;
    long xpos,ypos;
    int xhex,yhex;

    ret = GetDialogItemString(hwnd,IDC_EDT_XPOS,xstr,32);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return FALSE;
    }

    xhex = GetCheckSel(hwnd,IDC_CHK_XPOS);
    if(xhex)
    {
        if(TStrNCmp(xstr,TEXT("0x"),2)==0)
        {
            xpos = StrToUL(xstr+2,&endstr,16);
        }
        else
        {
            xpos = StrToUL(xstr,&endstr,16);
        }
    }
    else
    {
        xpos = StrToUL(xstr,&endstr,10);
    }

    ret = GetDialogItemString(hwnd,IDC_EDT_YPOS,ystr,32);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return FALSE;
    }

    yhex = GetCheckSel(hwnd,IDC_CHK_YPOS);
    if(yhex)
    {
        if(TStrNCmp(ystr,TEXT("0x"),2)==0)
        {
            ypos = StrToUL(ystr+2,&endstr,16);
        }
        else
        {
            ypos = StrToUL(ystr,&endstr,16);
        }
    }
    else
    {
        ypos = StrToUL(ystr,&endstr,10);
    }

    g_AbsPosX = xpos;
    g_AbsPosY = ypos;
    g_AbsPosXHex = xhex;
    g_AbsPosYHex = yhex;
    return TRUE;
}

BOOL CALLBACK SetAbsolutePosDlgProc(HWND hwndDlg,
                                    UINT message,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    BOOL bret=FALSE;
    switch(message)
    {
    case WM_INITDIALOG:
        bret = InitialializeAbsPosDialog(hwndDlg);
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
            bret = GetAbsPosParam(hwndDlg);
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

BOOL InsertAbsPos(HWND hwnd)
{
    DEVICEEVENT evt;

    evt.devtype = DEVICE_TYPE_MOUSE;
    evt.devid = 0;
    evt.event.mouse.code = MOUSE_CODE_MOUSE;
    evt.event.mouse.event = MOUSE_EVENT_ABS_MOVING;
    evt.event.mouse.x = g_AbsPosX;
    evt.event.mouse.y = g_AbsPosY;
    InsertDevEvent(&evt,1);
    return TRUE;
}


BOOL SetAbsPos(HWND hwnd)
{
    UINT nRet;
    nRet = DialogBox(hInst,MAKEINTRESOURCE(IDD_DLG_ABSPOS),hwnd,SetAbsolutePosDlgProc);
    if(nRet == IDOK)
    {
        return InsertAbsPos(hwnd);
    }

    return FALSE;
}



void StopConnect(HWND hwnd)
{
    StopThreadControl(&st_SockThreadCtrl);
    st_DevEvent.clear();
    ZeroMemory(g_KeyStateBuffer,sizeof(g_KeyStateBuffer));
    ZeroMemory(g_LastpKeyStateBuffer,sizeof(g_LastpKeyStateBuffer));
    ZeroMemory(&g_diMouseState,sizeof(g_diMouseState));
    ZeroMemory(&g_LastdiMouseState,sizeof(g_LastdiMouseState));
    g_LastPressKey = 0;
    g_LastPressedTimes = 0;
    return ;
}


BOOL StartConnect(HWND hwnd)
{
    int ret;
    int resetmouse =0;
    DEVICEEVENT evt;
    StopConnect(hwnd);
    ret = StartThreadControl(&st_SockThreadCtrl,SocketThreadImpl,&st_SockThreadCtrl,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Start Connect Thread Error(%d)\n",ret);
        goto fail;
    }

	/*to reset mouse ,just insert dev event*/
    if(g_ResetMouse)
    {
        ZeroMemory(&evt,sizeof(evt));
        evt.devtype = DEVICE_TYPE_MOUSE;
        evt.devid = 0;
        evt.event.mouse.code = MOUSE_CODE_MOUSE;
        evt.event.mouse.event = MOUSE_EVENT_ABS_MOVING;
        evt.event.mouse.x = IO_MOUSE_RESET_X;
        evt.event.mouse.y = IO_MOUSE_RESET_Y;
        InsertDevEvent(&evt,1);
    }

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
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

