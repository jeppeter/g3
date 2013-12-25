
#include "detourrawinput.h"
#include <output_debug.h>
#include <detours/detours.h>

#ifdef   IOCAP_DEBUG
#define  DETOUR_RAWINPUT_DEBUG      1
#endif

#ifdef   IOCAP_EMULATION
#define  DETOUR_RAWINPUT_EMULATION  1
#endif

typedef UINT(WINAPI *GetRawInputDataFunc_t)(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader);

typedef BOOL (WINAPI *RegisterRawInputDevicesFunc_t)(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
);

typedef UINT(WINAPI *GetRawInputDeviceInfoFunc_t)(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
);

typedef UINT(WINAPI *GetRawInputDeviceListFunc_t)(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize
);

typedef SHORT(WINAPI *GetKeyStateFunc_t)(
    int nVirtKey
);

typedef SHORT(WINAPI *GetAsyncKeyStateFunc_t)(
    int vKey
);

#if defined(DETOUR_RAWINPUT_DEBUG) && defined(DETOUR_RAWINPUT_EMULATION)
#error "only accept either DETOUR_RAWINPUT_DEBUG or DETOUR_RAWINPUT_EMULATION defined"
#endif

#if   defined(DETOUR_RAWINPUT_DEBUG)
#pragma message("@rawinput debug@")
#elif defined(DETOUR_RAWINPUT_EMULATION)
#pragma message("@rawinput emulation@")
#else
#error "must define one of DETOUR_RAWINPUT_DEBUG or DETOUR_RAWINPUT_EMULATION"
#endif


#ifdef DETOUR_RAWINPUT_DEBUG
#include "detourrawinput_debug.cpp"
#endif

#ifdef DETOUR_RAWINPUT_EMULATION
#include "detourrawinput_emulation.cpp"
#endif


BOOL DetourRawInputInit(HMODULE hModule)
{
    __RawInputDetour();
    return TRUE;
}


void DetourRawInputFini(HMODULE hModule)
{
    return ;
}
