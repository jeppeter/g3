

GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;


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
            DEBUG_BUFFER_FMT(pRawInputDevices,cbSize,"uiNumDevices (%d)",uiNumDevices);
        }
        else
        {
            DEBUG_INFO("uiNumDevices (%d)",uiNumDevices);
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
                DEBUG_BUFFER_FMT(pData,*pcbSize,"KeyBoard rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                 hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
            else if(pRaw->header.dwType == RIM_TYPEMOUSE)
            {
                DEBUG_BUFFER_FMT(pData,*pcbSize,"Mouse rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                 hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
            else
            {
                DEBUG_BUFFER_FMT(pData,*pcbSize,"UnknownType (%d) rawinput(0x%08x) uiCommand 0x%08x(%d) sizeheader(%d) uret(%d)",
                                 pRaw->header.dwType,hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
            }
        }
        else
        {
            DEBUG_INFO("uiCommand rawinput(0x%08x) 0x%08x(%d) sizeheader(%d) uret(%d)",
                       hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
        }
    }

    return uret;
}


int DetourRawInputControl(PIO_CAP_CONTROL_t pControl)
{
    int ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return -ret;
}

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


