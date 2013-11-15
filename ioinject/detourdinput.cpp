
/*****************************************
*  this file is to detour the functions of directinput8
*  this is the most common input for directx
*****************************************/
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>
#include <vector>
#include <assert.h>
#include <output_debug.h>
#include <detours/detours.h>
#include "detourdinput.h"
#include <injectbase.h>

#define  DEBUG_MODE       1
#undef   DEBUG_MODE

#define  EMULATION_MODE   1
//#undef   EMULATION_MODE

#if defined(DEBUG_MODE) && defined(EMULATION_MODE)
#error "could not define DEBUG_MODE and EMULATION_MODE both"
#endif

#if defined(DEBUG_MODE)
#elif defined(EMULATION_MODE)
#else
#error "must specify EMULATION_MODE or DEBUG_MODE"
#endif


#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))
#define COM_METHOD(TYPE, METHOD) TYPE STDMETHODCALLTYPE METHOD

#define DINPUT_DEBUG_INFO  DEBUG_INFO
#define DINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT


/****************************************************
these are the mouse defines
****************************************************/
#define SYS_MOUSE_STATE_SIZE   0x10
#define SYS_MOUSE_MOVE_UP      (-1)
#define SYS_MOUSE_MOVE_LEFT    (-1)
#define SYS_MOUSE_MOVE_DOWN    (1)
#define SYS_MOUSE_MOVE_RIGHT   (1)
#define SYS_MOUSE_LEFT_BTN     (0)
#define SYS_MOUSE_RIGHT_BTN    (1)
#define SYS_MOUSE_WHEEL_BTN_FORWARD  (1)
#define SYS_MOUSE_WHEEL_BTN_BACKWARD (-1)

/****************************************************
these are the keyboard defines
****************************************************/
#define SYS_KEYBOARD_STATE_SIZE  256
#define SYS_KEYBOARD_ESC         0x01      /*escape*/
#define SYS_KEYBOARD_A           0x1e
#define SYS_KEYBOARD_B           0x30
#define SYS_KEYBOARD_C           0x2e
#define SYS_KEYBOARD_D           0x20
#define SYS_KEYBOARD_E           0x12
#define SYS_KEYBOARD_F           0x21
#define SYS_KEYBOARD_G           0x22
#define SYS_KEYBOARD_H           0x23
#define SYS_KEYBOARD_I           0x17
#define SYS_KEYBOARD_J           0x24
#define SYS_KEYBOARD_K           0x25
#define SYS_KEYBOARD_L           0x26
#define SYS_KEYBOARD_M           0x32
#define SYS_KEYBOARD_N           0x31
#define SYS_KEYBOARD_O           0x18
#define SYS_KEYBOARD_P           0x19
#define SYS_KEYBOARD_Q           0x10
#define SYS_KEYBOARD_R           0x13
#define SYS_KEYBOARD_S           0x1f
#define SYS_KEYBOARD_T           0x14
#define SYS_KEYBOARD_U           0x16
#define SYS_KEYBOARD_V           0x2f
#define SYS_KEYBOARD_W           0x11
#define SYS_KEYBOARD_X           0x2d
#define SYS_KEYBOARD_Y           0x15
#define SYS_KEYBOARD_Z           0x2c
#define SYS_KEYBOARD_F1          0x3b
#define SYS_KEYBOARD_F2          0x3c
#define SYS_KEYBOARD_F3          0x3d
#define SYS_KEYBOARD_F4          0x3e
#define SYS_KEYBOARD_F5          0x3f
#define SYS_KEYBOARD_F6          0x40
#define SYS_KEYBOARD_F7          0x41
#define SYS_KEYBOARD_F8          0x42
#define SYS_KEYBOARD_F9          0x43
#define SYS_KEYBOARD_F10         0x44
#define SYS_KEYBOARD_F11         0x57
#define SYS_KEYBOARD_F12         0x58
#define SYS_KEYBOARD_1           0x02
#define SYS_KEYBOARD_2           0x03
#define SYS_KEYBOARD_3           0x04
#define SYS_KEYBOARD_4           0x05
#define SYS_KEYBOARD_5           0x06
#define SYS_KEYBOARD_6           0x07
#define SYS_KEYBOARD_7           0x08
#define SYS_KEYBOARD_8           0x09
#define SYS_KEYBOARD_9           0x0a
#define SYS_KEYBOARD_0           0x0b
#define SYS_KEYBOARD_APOSTTRO    0x29 /*`*/
#define SYS_KEYBOARD_HIFEN       0x0c /*-*/
#define SYS_KEYBOARD_EQUAL       0x0d /*=*/
#define SYS_KEYBOARD_BACKSPACE   0x0e /*backspace*/
#define SYS_KEYBOARD_TAB         0x0f /*tab*/
#define SYS_KEYBOARD_MID_LQUOTE  0x1a /*[*/
#define SYS_KEYBOARD_MID_RQUOTE  0x1b /*]*/
#define SYS_KEYBOARD_BACK_SLASH  0x2b /*\\*/
#define SYS_KEYBOARD_SEMICOLON   0x27 /*;*/
#define SYS_KEYBOARD_QUOTE       0x33 /*'*/
#define SYS_KEYBOARD_LSHIFT      0x2a /* left shift*/
#define SYS_KEYBOARD_COMMA       0x33 /*,*/
#define SYS_KEYBOARD_DOT         0x34 /*.*/
#define SYS_KEYBOARD_RSHIFT      0x36 /* right shift*/
#define SYS_KEYBOARD_ENTER       0x1c /* enter */
#define SYS_KEYBOARD_LCTRL       0x1d /* left ctrl*/
#define SYS_KEYBOARD_LWIN        0xdb /*left win */
#define SYS_KEYBOARD_LALT        0x38 /*left alt*/
#define SYS_KEYBOARD_RALT        0xb8 /*right alt*/
#define SYS_KEYBOARD_RWIN        0xdc /*right win*/
#define SYS_KEYBOARD_RMOUSE      0xdd /*right mouse click emulation */
#define SYS_KEYBOARD_RCTRL       0x9d /*right ctrl*/
#define SYS_KEYBOARD_CAPS        0x3a /*caps lock*/
#define SYS_KEYBOARD_SPACE       0x39
#define SYS_KEYBOARD_INS         0x52 /*insert*/
#define SYS_KEYBOARD_HOME        0x47
#define SYS_KEYBOARD_PAGEUP      0x49
#define SYS_KEYBOARD_DEL         0x53
#define SYS_KEYBOARD_END         0x4f
#define SYS_KEYBOARD_PAGEDOWN    0x51
#define SYS_KEYBOARD_UP          0x48
#define SYS_KEYBOARD_DOWN        0x50
#define SYS_KEYBOARD_LEFT        0x4b
#define SYS_KEYBOARD_RIGHT       0x4d
#define SYS_KEYBOARD_NUM         0x45
#define SYS_KEYBOARD_PRNSCRN     0xb7 /*print screen*/
#define SYS_KEYBOARD_SCROLL      0x46
#define SYS_KEYBOARD_PAUSEBREAK  0xc5 /*pause break*/
#define SYS_KEYBOARD_SLASH       0x35 /*/*/
#define SYS_KEYBOARD_NUM_SLASH   0xb5 /* / on numpad*/
#define SYS_KEYBOARD_NUM_STAR    0x37 /* * on numpad*/
#define SYS_KEYBOARD_NUM_MINUS   0x4a /* - on numpad*/
#define SYS_KEYBOARD_NUM_7       0x47 /* 7 on numpad*/
#define SYS_KEYBOARD_NUM_8       0x48 /* 8 on numpad*/
#define SYS_KEYBOARD_NUM_9       0x49 /* 9 on numpad*/
#define SYS_KEYBOARD_NUM_4       0x4b /* 4 on numpad*/
#define SYS_KEYBOARD_NUM_5       0x4c /* 5 on numpad*/
#define SYS_KEYBOARD_NUM_6       0x4d /* 6 on numpad*/
#define SYS_KEYBOARD_NUM_1       0x4f /* 1 on numpad*/
#define SYS_KEYBOARD_NUM_2       0x50 /* 2 on numpad*/
#define SYS_KEYBOARD_NUM_3       0x51 /* 3 on numpad*/
#define SYS_KEYBOARD_NUM_0       0x52 /* 0 on numpad*/
#define SYS_KEYBOARD_NUM_DOT     0x53 /* . on numpad*/
#define SYS_KEYBOARD_NUM_PLUS    0x4e /* + on numpad*/
#define SYS_KEYBOARD_NUM_ENTER   0x9c /* enter on numpad*/


static int st_IOInjectInit=0;

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#ifdef _DEBUG
#pragma comment(lib,"injectbased.lib")
#else
#pragma comment(lib,"injectbase.lib")

#endif

/*****************************************
*  to make the IDirectInputDevice8A hook
*
*****************************************/
#ifdef DEBUG_MODE
#include "detourdinput_debug.cpp"
#endif

/*********************************************
* emulation mode for direct input
*********************************************/
#ifdef EMULATION_MODE
#include "detourdinput_emulation.cpp"
#endif


/*****************************************
*   to make the base directinput 8 root functions
*
*****************************************/
typedef HRESULT(WINAPI *DirectInput8Create_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
DirectInput8Create_t DirectInput8CreateNext = DirectInput8Create;

HRESULT WINAPI DirectInput8CreateCallBack(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    HRESULT hr;

    hr = DirectInput8CreateNext(hinst,dwVersion,riidltf,ppvOut,punkOuter);
    if(hr == DI_OK)
    {
        /*now successful ,so we should do this for the out*/
        DINPUT_DEBUG_INFO("riidltf %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                          riidltf.Data1,
                          riidltf.Data2,
                          riidltf.Data3,
                          riidltf.Data4[0],
                          riidltf.Data4[1],
                          riidltf.Data4[2],
                          riidltf.Data4[3],
                          riidltf.Data4[4],
                          riidltf.Data4[5],
                          riidltf.Data4[6],
                          riidltf.Data4[7]);
        DINPUT_DEBUG_INFO("dwVersion is 0x%08x\n",dwVersion);
        if(ppvOut)
        {
            DINPUT_DEBUG_INFO("*ppvOut 0x%p\n",*ppvOut);
            if(riidltf == IID_IDirectInput8A)
            {
                CDirectInput8AHook* pHookA = NULL;
                IDirectInput8A* pPtrA = (IDirectInput8A*)*ppvOut;
                pHookA = RegisterDirectInput8AHook(pPtrA);
                assert(pHookA);
                DINPUT_DEBUG_INFO("IDirectInput8A(0x%p) =>CDirectInput8AHook(0x%p)\n",pPtrA,pHookA);
                *ppvOut = pHookA;
            }
            else if(riidltf == IID_IDirectInput8W)
            {
                CDirectInput8WHook *pHookW = NULL;
                IDirectInput8W* pPtrW = (IDirectInput8W*) *ppvOut;
                pHookW = RegisterDirectInput8WHook(pPtrW);
                assert(pHookW);
                DINPUT_DEBUG_INFO("IDirectInput8W(0x%p) => CDirectInput8WHook(0x%p)\n",pPtrW,pHookW);
                *ppvOut = pHookW;
            }
        }
        if(punkOuter)
        {
            DINPUT_DEBUG_INFO("*punkOuter 0x%p\n",*punkOuter);
        }

    }
    return hr;
}

void DetourDirectInputFini(void)
{
    if(st_IOInjectInit)
    {
        /*nothing to done*/
    }
    st_IOInjectInit = 0;
    return ;
}

BOOL __DetourDirectInput8CallBack(void)
{
    DEBUG_BUFFER_FMT(DirectInput8CreateNext,10,"Before DirectInput8CreateNext(0x%p)",DirectInput8CreateNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&DirectInput8CreateNext,DirectInput8CreateCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(DirectInput8CreateNext,10,"After DirectInput8CreateNext(0x%p)",DirectInput8CreateNext);

    return TRUE;
}


BOOL DetourDirectInputInit(void)
{
    BOOL bret;
    int ret;
    DINPUT_DEBUG_INFO("st_IOInjectInit = %d\n",st_IOInjectInit);
    if(st_IOInjectInit > 0)
    {
        DINPUT_DEBUG_INFO("\n");
        return TRUE;
    }
    /*now first to init all the critical section*/
    InitializeCriticalSection(&st_DIDevice8ACS);
    InitializeCriticalSection(&st_DIDevice8WCS);
    InitializeCriticalSection(&st_DI8ACS);
    InitializeCriticalSection(&st_DI8WCS);

    /*now to detour */
    bret = __DetourDirectInput8CallBack();
    if(!bret)
    {
        /*we pretend true ,not let ok*/
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Detour Direct Input Callback error(%d)\n",ret);
        goto fail;
    }

    DINPUT_DEBUG_INFO("Initialize IoInject succ\n");
    st_IOInjectInit = 1;
    return TRUE;

fail:
    SetLastError(ret);
    return FALSE;

}

