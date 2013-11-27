

RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;
GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoANext=GetRawInputDeviceInfoA;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoWNext=GetRawInputDeviceInfoW;
GetRawInputDeviceListFunc_t GetRawInputDeviceListNext=GetRawInputDeviceList;
GetKeyStateFunc_t GetKeyStateNext=GetKeyState;
GetAsyncKeyStateFunc_t GetAsyncKeyStateNext=GetAsyncKeyState;

#define  DETOURRAWINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  DETOURRAWINPUT_DEBUG_INFO      DEBUG_INFO


BOOL WINAPI RegisterRawInputDevicesCallBack(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
)
{
    BOOL bret;

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
                RAWINPUTDEVICELIST *pRawInputDevice;
                UINT i;
                DETOURRAWINPUT_DEBUG_BUFFER_FMT(pRawInputDeviceList,uret*cbSize,"GetRawInputDeviceList *puiNumDevices(%d) cbSize(%d)",*puiNumDevices,cbSize);
                for(i=0; i<*puiNumDevices; i++)
                {
                    pRawInputDevice = &(pRawInputDeviceList[i]);
					DETOURRAWINPUT_DEBUG_INFO("[%d]hDevice 0x%08x dwType 0x%08x\n",pRawInputDevice->hDevice,pRawInputDevice->dwType);
                }
            }
            else
            {
                DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceList *puiNumDevices(%d) cbSize(%d)\n",*puiNumDevices,cbSize);
            }
        }
        else
        {
            DETOURRAWINPUT_DEBUG_INFO("*puiNumDevices(%d) cbSize(%d)\n",*puiNumDevices,cbSize);
        }
    }
    else
    {
        DETOURRAWINPUT_DEBUG_INFO("GetRawInputDeviceList puiNumDevices==NULL\n");
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



int DetourRawInputControl(PIO_CAP_CONTROL_t pControl)
{
    int ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return -ret;
}

int __RawInputDetour(void)
{
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"Before RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"Before GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"Before GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"Before GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"Before GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"Before GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"Before GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&RegisterRawInputDevicesNext,RegisterRawInputDevicesCallBack);
    DetourAttach((PVOID*)&GetRawInputDataNext,GetRawInputDataCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoANext,GetRawInputDeviceInfoACallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoWNext,GetRawInputDeviceInfoWCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceListNext,GetRawInputDeviceListCallBack);
    DetourAttach((PVOID*)&GetKeyStateNext,GetKeyStateCallBack);
    DetourAttach((PVOID*)&GetAsyncKeyStateNext,GetAsyncKeyStateCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"After RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"After GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"After GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"After GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"After GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"After GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"After GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    return 0;
}


