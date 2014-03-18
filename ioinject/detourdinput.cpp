
/*****************************************
*  this file is to detour the functions of directinput8
*  this is the most common input for directx
*****************************************/
#define  DIRECTINPUT_VERSION   0x800
#include <dinput.h>
#include <dinputd.h>
#include <vector>
#include <assert.h>
#include <output_debug.h>
#include <detours/detours.h>
#include "detourdinput.h"
#include <injectbase.h>

#ifdef   IOCAP_DEBUG
#define  DETOUR_DINPUT_DEBUG       1
#endif


#ifdef   IOCAP_EMULATION
#define  DETOUR_DINPUT_EMULATION   1
#endif

#if defined(DETOUR_DINPUT_DEBUG) && defined(DETOUR_DINPUT_EMULATION)
#error "could not define DETOUR_DINPUT_DEBUG and DETOUR_DINPUT_EMULATION both"
#endif

#if defined(DETOUR_DINPUT_DEBUG)
#pragma message("@dinput debug@")
#elif defined(DETOUR_DINPUT_EMULATION)
#pragma message("@dinput emulation@")
#else
#error "must specify DETOUR_DINPUT_EMULATION or DETOUR_DINPUT_DEBUG"
#endif


#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))
#define COM_METHOD(TYPE, METHOD) TYPE STDMETHODCALLTYPE METHOD

#define DINPUT_DEBUG_INFO  DEBUG_INFO
#define DINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT





static int st_IOInjectInit=0;


/*****************************************
*  to make the IDirectInputDevice8A hook
*
*****************************************/
#ifdef DETOUR_DINPUT_DEBUG
#include "detourdinput_debug.cpp"
#endif

/*********************************************
* emulation mode for direct input
*********************************************/
#ifdef DETOUR_DINPUT_EMULATION
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

void DetourDirectInputFini(HMODULE hModule)
{
    if(st_IOInjectInit)
    {
#ifdef  DETOUR_DINPUT_DEBUG
        DetourDinputDebugFini(hModule);
#endif

#ifdef 	DETOUR_DINPUT_EMULATION
        DetourDinputEmulationFini(hModule);
#endif
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


BOOL DetourDirectInputInit(HMODULE hModule)
{
    BOOL bret;
    int ret;
    DINPUT_DEBUG_INFO("st_IOInjectInit = %d\n",st_IOInjectInit);
    if(st_IOInjectInit > 0)
    {
        DINPUT_DEBUG_INFO("\n");
        return TRUE;
    }
    InitializeCriticalSection(&st_DI8ACS);
    InitializeCriticalSection(&st_DI8WCS);

#ifdef DETOUR_DINPUT_DEBUG
    ret = DetourDinputDebugInit(hModule);
    if(ret < 0)
    {
        DINPUT_DEBUG_INFO("DebugInit Error\n");
        return TRUE;
    }
#endif

#ifdef 	DETOUR_DINPUT_EMULATION
    ret = DetourDinputEmulationInit(hModule);
    if(ret < 0)
    {
        DINPUT_DEBUG_INFO("EmulationInit Error\n");
        return TRUE;
    }
#endif

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
	assert(ret > 0);
#ifdef DETOUR_DINPUT_DEBUG
	DetourDinputDebugFini(hModule);
#endif 
#ifdef DETOUR_DINPUT_EMULATION
	DetourDinputEmulationFini(hModule);
#endif
	SetLastError(ret);
	return TRUE;
}

