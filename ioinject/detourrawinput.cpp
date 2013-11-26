
#include "detourrawinput.h"
#include <output_debug.h>
#include <detours/detours.h>

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


#include "detourrawinput_debug.cpp"



BOOL DetourRawInputInit(void)
{
    __RawInputDetour();
    return TRUE;
}


void DetourRawInputFini(void)
{
    return ;
}
