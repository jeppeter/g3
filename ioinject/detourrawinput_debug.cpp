



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

    uret = GetRawInputDataNext(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);
    if(uret != (UINT)-1)
    {
        if(pData)
        {
            DEBUG_BUFFER_FMT(pData,*pcbSize,"uiCommand rawinput(0x%08x) 0x%08x(%d) sizeheader(%d) uret(%d)",
                             hRawInput,uiCommand,uiCommand,cbSizeHeader,uret);
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

