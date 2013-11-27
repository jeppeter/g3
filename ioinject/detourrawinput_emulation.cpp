

#include <vector>

static CRITICAL_SECTION st_EmulationRawinputCS;
static std::vector<RAWINPUT*> st_KeyRawInputVecs;
static RID_DEVICE_INFO* st_KeyRawInputHandle=NULL;
static std::vector<RAWINPUT*> st_MouseRawInputVecs;
static RID_DEVICE_INFO* st_MouseRawInputHandle=NULL;


RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;
GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoANext=GetRawInputDeviceInfoA;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoWNext=GetRawInputDeviceInfoW;
GetRawInputDeviceListFunc_t GetRawInputDeviceListNext=GetRawInputDeviceList;
GetKeyStateFunc_t GetKeyStateNext=GetKeyState;
GetAsyncKeyStateFunc_t GetAsyncKeyStateNext=GetAsyncKeyState;

#define  DETOURRAWINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  DETOURRAWINPUT_DEBUG_INFO      DEBUG_INFO


HANDLE __RegisterKeyboardHandle()
{
    RID_DEVICE_INFO *pKeyboardInfo=NULL,*pAllocInfo=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pKeyboardInfo = st_KeyRawInputHandle;
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pKeyboardInfo == NULL)
    {
        pAllocInfo = calloc(sizeof(*pAllocInfo),1);
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pAllocInfo->cbSize = 8 + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEKEYBOARD;
        pAllocInfo->keyboard.dwType = 7;
        pAllocInfo->keyboard.dwSubType = 0;
        pAllocInfo->keyboard.dwKeyboardMode = 1;
        pAllocInfo->keyboard.dwNumberOfFunctionKeys = 12;
        pAllocInfo->keyboard.dwNumberOfIndicators = 3;
        pAllocInfo->keyboard.dwNumberOfKeysTotal = 101;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_KeyRawInputHandle == NULL)
        {
            inserted = 1;
            st_KeyRawInputHandle = pAllocInfo;
            pKeyboardInfo = pAllocInfo;
        }
        else
        {
            pKeyboardInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pKeyboardInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
        }
    }

    return pKeyboardInfo;
}

HANDLE __RegisterMouseHandle()
{

}

BOOL WINAPI RegisterRawInputDevicesCallBack(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
)
{
    BOOL bret;
    RAWINPUTDEVICE *pDevice=NULL;
    int ret;
    UINT i;

    /*now first check for device is supported*/
    if(cbSize != sizeof(*pDevice) || pRawInputDevices == NULL || uiNumDevices == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }

    for(i=0; i<uiNumDevices; i++)
    {
        pDevice = &(pRawInputDevices[i]);
        /*we only supported keyboard and mouse ,others are not supported*/
        if(pDevice->usUsagePage != 0x1 || (pDevice->usUsage != 0x2 && pDevice->usUsage != 0x6))
        {
            ret = ERROR_NOT_SUPPORTED;
            SetLastError(ret);
            return FALSE;
        }
    }

    bret = RegisterRawInputDevicesNext(pRawInputDevices,uiNumDevices,cbSize);
    if(bret)
    {
        if(pRawInputDevices)
        {
            DETOURRAWINPUT_DEBUG_BUFFER_FMT(pRawInputDevices,cbSize*uiNumDevices,"uiNumDevices (%d)",uiNumDevices);
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("uiNumDevices (%d)",uiNumDevices);
        }
    }

    return bret;
}


UINT WINAPI GetRawInputDeviceListCallBack(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize
)
{
    UINT uret;

    uret = GetRawInputDeviceListNext(pRawInputDeviceList,puiNumDevices,cbSize);
    if(puiNumDevices)
    {
        if(uret != (UINT)-1)
        {
            if(pRawInputDeviceList)
            {
                /*we should make */
            }
        }
    }

    return uret;
}

UINT WINAPI GetRawInputDeviceInfoACallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    UINT uret;

    uret = GetRawInputDeviceInfoANext(hDevice,uiCommand,pData,pcbSize);
    if(uret != (UINT)-1)
    {
        if(pData)
        {
            DETOURRAWINPUT_DEBUG_BUFFER_FMT(pData,*pcbSize,"GetRawInputDeviceInfoA hDevice(0x%08x) uiCommand 0x%08x(%d) uret(%d)",
                                            hDevice,uiCommand,uiCommand,uret);
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoA hDevice(0x%08x) uiCommand 0x%08x(%d) uret(%d)\n",hDevice,uiCommand,uiCommand,uret);
        }
    }
    else
    {
        if(pcbSize)
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoA hDevice(0x%08x) uiCommand 0x%08x(%d) *pcbSize(%d)\n",hDevice,uiCommand,uiCommand,*pcbSize);
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoA hDevice(0x%08x) uiCommand 0x%08x(%d) pcbSize==NULL\n",hDevice,uiCommand,uiCommand);
        }
    }

    return uret;
}


UINT WINAPI GetRawInputDeviceInfoWCallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    UINT uret;

    uret = GetRawInputDeviceInfoWNext(hDevice,uiCommand,pData,pcbSize);
    if(uret != (UINT)-1)
    {
        if(pData)
        {
            DETOURRAWINPUT_DEBUG_BUFFER_FMT(pData,*pcbSize,"GetRawInputDeviceInfoW hDevice(0x%08x) uiCommand 0x%08x(%d) uret(%d)",
                                            hDevice,uiCommand,uiCommand,uret);
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoW hDevice(0x%08x) uiCommand 0x%08x(%d) uret(%d)\n",hDevice,uiCommand,uiCommand,uret);
        }
    }
    else
    {
        if(pcbSize)
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoW hDevice(0x%08x) uiCommand 0x%08x(%d) *pcbSize(%d)\n",hDevice,uiCommand,uiCommand,*pcbSize);
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceInfoW hDevice(0x%08x) uiCommand 0x%08x(%d) pcbSize==NULL\n",hDevice,uiCommand,uiCommand);
        }
    }

    return uret;
}


UINT WINAPI GetRawInputDataCallBack(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader)
{
    UINT uret;
    RAWINPUT *pRaw=NULL;

    uret = GetRawInputDataNext(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);
    if(uret != (UINT)-1)
    {
        if(pData)
        {
            pRaw = (RAWINPUT*)pData;
            if(pRaw->header.dwType == RIM_TYPEKEYBOARD)
            {
                DETOURRAWINPUT_DEBUG_BUFFER_FMT(pData,*pcbSize,"KeyBoard rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                                hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
            else if(pRaw->header.dwType == RIM_TYPEMOUSE)
            {
                DETOURRAWINPUT_DEBUG_BUFFER_FMT(pData,*pcbSize,"Mouse rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                                hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
            else
            {
                DETOURRAWINPUT_DEBUG_BUFFER_FMT(pData,*pcbSize,"UnknownType 0x%08x(%d) rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                                pRaw->header.dwType,pRaw->header.dwType,hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("uiCommand rawinput(0x%08x) 0x%08x(%d) sizeheader(%d) uret(%d)",
                                      hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
        }
    }

    return uret;
}


SHORT WINAPI GetKeyStateCallBack(
    int nVirtKey
)
{
    SHORT sret;

    sret = GetKeyStateNext(nVirtKey);
    DETOURRAWINPUT_DEBUG_INFO("GetKeyState 0x%08x(%d) sret(0x%4x:%d)\n",
                              nVirtKey,nVirtKey,sret,sret);
    return sret;
}

SHORT WINAPI GetAsyncKeyStateCallBack(
    int vKey
)
{
    SHORT sret;

    sret = GetAsyncKeyStateNext(vKey);
    DETOURRAWINPUT_DEBUG_INFO("GetAsyncState 0x%08x(%d) sret(0x%4x:%d)\n",
                              vKey,vKey,sret,sret);
    return sret;
}






