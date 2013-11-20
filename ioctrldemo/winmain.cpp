
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
CIoController           *g_pIoController=NULL;


void DirectInput_Fini()
{
    ULONG uret;
    if(g_pMouseDevice)
    {
        if (g_MouseAcquire)
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
        if (g_KeyboardAcquire)
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
	if (hr != DI_OK)
	{
		ret = LAST_ERROR_CODE();
		ERROR_INFO("Could not SetCooperativeLevel error(%d)\n",ret);
		goto fail;
	}

    hr = g_pKeyboardDevice->Acquire();
    if (hr != DI_OK)
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
        if (hr == DI_OK)
        {
            break;
        }
        if (hr != DIERR_INPUTLOST && hr != DIERR_NOTACQUIRED)
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

void UpdateCodeMessage()
{
    if (g_pIoController == NULL)
    {
        return;
    }

    /*now we should check if the specified key is pressed*/
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd)
{
    HMENU hMenu=NULL;
    HWND hwnd = NULL;
    HRESULT hr;
    MSG msg={0};

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
            break;
        }
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

