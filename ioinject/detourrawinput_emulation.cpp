

#include <vector>

#define  DEVICE_GET_INFO   0x1
#define  DEVICE_GET_NAMEA  0x2
#define  DEVICE_GET_NAMEW  0x3

static CRITICAL_SECTION st_EmulationRawinputCS;
static std::vector<RAWINPUT*> st_KeyRawInputVecs;
static RID_DEVICE_INFO* st_KeyRawInputHandle=NULL;
static uint8_t *st_KeyRawInputName=NULL;
static wchar_t *st_KeyRawInputNameWide=NULL;
static int st_KeyLastInfo=DEVICE_GET_INFO;
static std::vector<RAWINPUT*> st_MouseRawInputVecs;
static RID_DEVICE_INFO* st_MouseRawInputHandle=NULL;
static uint8_t *st_MouseRawInputName=NULL;
static wchar_t *st_MouseRawInputNameWide=NULL;
static int st_MouseLastInfo=DEVICE_GET_INFO;


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
    uint8_t *pKeyName=NULL;
    wchar_t *pKeyUnicode=NULL;
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

        pKeyName = calloc(1,256);
        if(pKeyName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s(pKeyName,256,"\\\\?\\KeyBoard_Emulate",_TRUNCATE);
        pKeyUnicode = calloc(2,256);
        if(pKeyUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pKeyName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pKeyName,256,L"\\\\?\\KeyBoard_Emulate",_TRUNCATE);

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
            st_KeyRawInputName = pKeyName;
            st_KeyRawInputNameWide = pKeyUnicode;
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
            free(pKeyName);
            pKeyName = NULL
                       free(pKeyUnicode);
            pKeyUnicode = NULL;
        }
    }

    return pKeyboardInfo;
}

HANDLE __RegisterMouseHandle()
{
    RID_DEVICE_INFO *pMouseboardInfo=NULL,*pAllocInfo=NULL;
    uint8_t *pMouseName=NULL;
    wchar_t *pMouseUnicode=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pMouseboardInfo = st_MouseRawInputHandle;
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pMouseboardInfo == NULL)
    {
        pAllocInfo = calloc(sizeof(*pAllocInfo),1);
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pMouseName = calloc(1,256);
        if(pMouseName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s(pMouseName,256,"\\\\?\\Mouse_Emulate",_TRUNCATE);
        pMouseUnicode = calloc(2,256);
        if(pMouseUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pMouseName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pMouseName,256,L"\\\\?\\Mouse_Emulate",_TRUNCATE);

        pAllocInfo->cbSize = 8 + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEMOUSE;
        pAllocInfo->mouse.dwId = 256;
        pAllocInfo->mouse.dwNumberOfButtons = 3;
        pAllocInfo->mouse.dwSampleRate = 0;
        pAllocInfo->mouse.fHasHorizontalWheel = 0;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_MouseRawInputHandle == NULL)
        {
            inserted = 1;
            st_MouseRawInputHandle = pAllocInfo;
            pMouseboardInfo = pAllocInfo;
            st_MouseRawInputName = pMouseName;
            st_MouseRawInputNameWide = pMouseUnicode;
        }
        else
        {
            pMouseboardInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pMouseboardInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
            free(pMouseName);
            pMouseName = NULL;
            free(pMouseUnicode);
            pMouseUnicode = NULL;
        }
    }

    return pMouseboardInfo;
}

int __CopyKeyboardDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret;
    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_KeyRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_KeyRawInputHandle;
        pRawList->dwType = RIM_TYPEKEYBOARD;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;


}

int __CopyMouseDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret=0;

    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_MouseRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_MouseRawInputHandle;
        pRawList->dwType = RIM_TYPEMOUSE;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;
}


BOOL __GetDeviceInfoNoLock(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    RID_DEVICE_INFO *pGetInfo;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        ret = 0 ;
        bret = TRUE;
        st_KeyLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = 8 + sizeof(pInfo->keyboard);
        pInfo->dwType = RIM_TYPEKEYBOARD;
        CopyMemory(&(pInfo->keyboard),&(st_KeyRawInputHandle->keyboard),sizeof(pInfo->keyboard));
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        ret = 0;
        bret = TRUE;
        st_MouseLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = 8 + sizeof(pInfo->mouse);
        pInfo->dwType = RIM_TYPEMOUSE;
        CopyMemory(&(pInfo->mouse),&(st_MouseRawInputHandle->mouse),sizeof(pInfo->mouse));
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceInfo(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceInfoNoLock(hDevice,pInfo);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameANoLock(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen(st_KeyRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen(st_KeyRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen(st_KeyRawInputName) + 1;
            strncpy_s(pData,*pcbSize,st_KeyRawInputName,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen(st_MouseRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen(st_MouseRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen(st_MouseRawInputName) + 1;
            strncpy_s(pData,*pcbSize,st_MouseRawInputName,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceNameA(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameWNoLock(HANDLE hDevice,void * pData,UINT * pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_KeyRawInputName)*2 +1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_KeyRawInputName)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_KeyRawInputName)*2 + 2;
            wcsncpy_s(pData,(*pcbSize)/2,st_KeyRawInputName,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_MouseRawInputName)*2+1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_MouseRawInputName)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_MouseRawInputName)*2 + 2;
            wcsncpy_s(pData,(*pcbSize)/2,st_MouseRawInputName,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}




BOOL __GetDeviceNameW(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

int __GetDeviceInfoLast(HANDLE hDevice,void* pData,UINT* pcbSize)
{
    BOOL bret = FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    int copiedlen=-1;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(hDevice == (HANDLE) st_KeyRawInputHandle)
    {
        if(st_KeyLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = strlen(pData);
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = wcslen(pData)*2;
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                }
                else
                {
                    copiedlen = sizeof(RID_DEVICE_INFO);
                    ret = 0;
                }
            }
        }
        else
        {
        	/*we could not arrive here*/
            assert(0!=0);
        }
    }	
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        if(st_MouseLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = strlen(pData);
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
            }
            else
            {
                copiedlen = wcslen(pData)*2;
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                }
                else
                {
                    copiedlen = sizeof(RID_DEVICE_INFO);
                    ret = 0;
                }
            }
        }
        else
        {
        	/*we could not arrive here*/
            assert(0!=0);
        }
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(copiedlen < 0)
    {
        SetLastError(ret);
    }
    return copiedlen;
}

BOOL WINAPI RegisterRawInputDevicesCallBack(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
)
{
    RAWINPUTDEVICE *pDevice=NULL;
    int ret;
    UINT i;
    RID_DEVICE_INFO *pMouse=NULL,*pKeyBoard=NULL;
    pMouse = __RegisterMouseHandle();
    if(pMouse == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return FALSE;
    }

    pKeyBoard = __RegisterKeyboardHandle();
    if(pKeyBoard== NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return FALSE;
    }


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

    /*now all is ok  ,so we do not need any more*/
    return TRUE;
}


UINT WINAPI GetRawInputDeviceListCallBack(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize
)
{
    UINT uret;
    int ret;

    /*now check for the input */
    if(puiNumDevices == NULL || cbSize != sizeof(*pRawInputDeviceList))
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    if(*puiNumDevices < 2 || pRawInputDeviceList == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        *puiNumDevices = 2;
        SetLastError(ret);
        return (UINT) -1;
    }

    *puiNumDevices = 2;
    /*now we should copy the memory*/
    ret = __CopyKeyboardDeviceList(&(pRawInputDeviceList[0]));
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Copy Keyboard DeviceList Error(%d)\n",ret);
        SetLastError(ret);
        return (UINT) -1;
    }

    ret = __CopyMouseDeviceList(&(pRawInputDeviceList[1]));
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Copy Mouse DeviceList Error(%d)\n",ret);
        SetLastError(ret);
        return (UINT) -1;
    }

    return 2;
}

UINT WINAPI GetRawInputDeviceInfoACallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    UINT uret;
    BOOL bret;
    int ret;


    /*now first to copy the data*/
    if(uiCommand == RIDI_DEVICENAME)
    {
        if(pcbSize == NULL)
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }
        bret= __GetDeviceNameA(hDevice,pData,pcbSize);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return strlen(pData);
    }
    else if(uiCommand == RIDI_DEVICEINFO)
    {
        if(pcbSize == NULL)
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }
        if(*pcbSize != sizeof(RID_DEVICE_INFO))
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }

        bret= __GetDeviceInfo(hDevice,(RID_DEVICE_INFO*)pData);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return sizeof(RID_DEVICE_INFO);
    }
    else if(uiCommand == RIDI_PREPARSEDDATA)
    {
        if(pcbSize == NULL)
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }
    	ret = __GetDeviceInfoLast(hDevice,pData,pcbSize);
		return (UINT)ret;
    }

	/*now nothing find ,so we should return not supported*/
	ret = ERROR_NOT_SUPPORTED;
	SetLastError(ret);
	return (UINT) -1;
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






