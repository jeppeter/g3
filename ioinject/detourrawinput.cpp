
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
