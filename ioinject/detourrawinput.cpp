
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

GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;


#include "detourrawinput_debug.cpp"

int __RawInputDetour(void)
{
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"Before GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"Before RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&RegisterRawInputDevicesNext,RegisterRawInputDevicesCallBack);
    DetourAttach((PVOID*)&GetRawInputDataNext,GetRawInputDataCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"After GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"After RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    return 0;
}


BOOL DetourRawInputInit(void)
{
    __RawInputDetour();
    return TRUE;
}


void DetourRawInputFini(void)
{
    return ;
}
