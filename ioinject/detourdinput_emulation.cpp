
#include <iocapcommon.h>
#include <vector>
#include "ioinject_thread.h"
#define  DIRECTINPUT_VERSION 0x800
#include <dinput.h>


#define MAX_HWND_SIZE   20

#define MOUSE_NOT_SET_STATE             -1
#define MOUSE_NORMAL_STATE              0
#define MOUSE_RESET_MOST_LEFTTOP        1
#define MOUSE_RESET_WNDCLIENT_LEFTTOP   2


static CRITICAL_SECTION st_Dinput8KeyMouseStateCS;
static POINT st_LastDiMousePoint= {1,1};
static POINT st_PrevDiMousePoint= {1,1};
static UINT st_LastDiMouseZ;
static int st_MouseGetState=MOUSE_NORMAL_STATE;
static std::vector<LPDIDEVICEOBJECTDATA> st_pMouseData;
static std::vector<int> st_MouseDataNums;
static std::vector<int> st_MouseDataIdx;
static std::vector<LPDIDEVICEOBJECTDATA> st_pKeyboardData;

#define  MOUSE_DATA_EQUAL()  \
do\
{\
	assert(st_pMouseData.size() == st_MouseDataIdx.size());\
	assert(st_MouseDataIdx.size() == st_MouseDataNums.size());\
}while(0)

LPDIDEVICEOBJECTDATA __GetKeyboardData();
int __InsertKeyboardDinputData(DIDEVICEOBJECTDATA* pData,int back);
LPDIDEVICEOBJECTDATA __GetMouseData(int *pNum,int *pIdx);
int __InsertMouseDinputData(DIDEVICEOBJECTDATA *pData,int num,int idx,int back);


int __DetourDinput8Init(void)
{
    st_LastDiMousePoint = {0,0};
    st_LastDiMouseZ = 0;
    st_MouseGetState = MOUSE_NORMAL_STATE;
    return 0;
}

int DetourDinput8Init(LPVOID pParam,LPVOID pInput)
{
    int ret=0;

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    ret = InitBaseKeyState();
    if(ret >= 0)
    {
        ret = InitBaseMouseState();
        if(ret >= 0)
        {
            ret = __DetourDinput8Init();
        }
    }
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    return ret;
}

int __CopyDiMouseState(PVOID pData, UINT cbSize)
{
    int ret=0;
    int i;
    UINT mousekeybtns[3];
    UINT mousez=0;
    POINT mousepoint;
    DIMOUSESTATE *pMouseState=NULL;
    int copied=0;


    if(cbSize < sizeof(*pMouseState))
    {
        ret=  ERROR_INSUFFICIENT_BUFFER;
        SetLastError(ret);
        return -ret;
    }

    pMouseState = (DIMOUSESTATE*)pData;



    /*we do not call GetBaseMouseState in the critical section ,because if this is disorder ,the final state will not disturb*/
    ret = GetBaseMouseState(mousekeybtns,3,&mousepoint,&mousez);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return -1;
    }
    ZeroMemory(pMouseState,cbSize);

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    if(st_MouseGetState == MOUSE_NORMAL_STATE)
    {

        /*now to compare the state*/
        pMouseState->lX = (mousepoint.x - st_LastDiMousePoint.x);
        pMouseState->lY = (mousepoint.y - st_LastDiMousePoint.y);
        pMouseState->lZ = (mousez - st_LastDiMouseZ);
        st_LastDiMouseZ = mousez;
    }
    else if(st_MouseGetState == MOUSE_RESET_MOST_LEFTTOP)
    {
        /*to move the mouse pointer to the most left-top pointer*/
        pMouseState->lX = IO_MOUSE_RESET_X_MOV;
        pMouseState->lY = IO_MOUSE_RESET_Y_MOV;
        /*do not make any lz moving ,this will be do when the last one here*/
        pMouseState->lZ = 0;
        st_MouseGetState = MOUSE_RESET_WNDCLIENT_LEFTTOP;
    }
    else if(st_MouseGetState == MOUSE_RESET_WNDCLIENT_LEFTTOP)
    {
        /*to move to the top pointer*/
        pMouseState->lX = mousepoint.x ;
        pMouseState->lY = mousepoint.y ;
        /*do not make any lz moving ,this will be do when the last one here*/
        pMouseState->lZ = 0;
        st_MouseGetState = MOUSE_NORMAL_STATE;
    }

    for(i=0; i<3; i++)
    {
        if(mousekeybtns[i])
        {
            pMouseState->rgbButtons[i] = 0x80;
        }
    }
    st_LastDiMousePoint = mousepoint;
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    return sizeof(*pMouseState);
}

int __CopyDiKeyState(PVOID pData,UINT cbSize)
{
    int ret=0;
    BYTE keystate[256];
    if(cbSize < sizeof(256))
    {
        ret=  ERROR_INSUFFICIENT_BUFFER;
        SetLastError(ret);
        return -ret;
    }

    ret = GetBaseKeyState(keystate,256);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return -1;
    }

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    CopyMemory(pData,keystate,256);
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    return 256;
}


/***************************************************
*  COM api for DirectInput8
*
***************************************************/


class CDirectInputDevice8AHook;
class CDirectInputDevice8WHook;
class CDirectInputJoyConfig8Hook;
class CDirectInputJoyConfigHook;


static std::vector<IDirectInputDevice8A*> st_Key8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_Key8AHookVecs;
static std::vector<IDirectInputDevice8A*> st_Mouse8AVes;
static std::vector<CDirectInputDevice8AHook*> st_Mouse8AHookVecs;
static std::vector<IDirectInputDevice8A*> st_NotSet8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_NotSet8AHookVecs;


static std::vector<IDirectInputDevice8W*> st_Key8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_Key8WHookVecs;
static std::vector<IDirectInputDevice8W*> st_Mouse8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_Mouse8WHookVecs;
static std::vector<IDirectInputDevice8W*> st_NotSet8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_NotSet8WHookVecs;


static std::vector<IDirectInputJoyConfig8*> st_DIJoyConfig8Vecs;
static std::vector<CDirectInputJoyConfig8Hook*> st_CDIJoyConfig8HookVecs;

static CRITICAL_SECTION st_Dinput8DeviceCS;

#define  JOY_CONFIG8_ASSERT()  \
do\
{\
	assert(st_CDIJoyConfig8HookVecs.size() == st_DIJoyConfig8Vecs.size());\
}while(0)

ULONG UnregisterDirectInputJoyConfig8(CDirectInputJoyConfig8Hook* pHookConfig8)
{
    ULONG uret=1;
    int findidx = -1;
    IDirectInputJoyConfig8* pConfig8=NULL;
    UINT i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    JOY_CONFIG8_ASSERT();
    for(i=0; i<st_CDIJoyConfig8HookVecs.size(); i++)
    {
        if(st_CDIJoyConfig8HookVecs[i] == pHookConfig8)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        pConfig8 = st_DIJoyConfig8Vecs[findidx];
        st_CDIJoyConfig8HookVecs.erase(st_CDIJoyConfig8HookVecs.begin() + findidx);
        st_DIJoyConfig8Vecs.erase(st_DIJoyConfig8Vecs.begin() + findidx);
        uret = pConfig8->Release();
        if(uret != 0)
        {
            ERROR_INFO("Hook<0x%p> (Config8:0x%p) uret(%d)\n",pHookConfig8,pConfig8,uret);
        }
    }
    else
    {
        ERROR_INFO("HookConfig8<0x%p> not found\n",pHookConfig8);
    }
    LeaveCriticalSection(&st_Dinput8DeviceCS);
    return uret;
}


#define

class CDirectInputJoyConfig8Hook : public IDirectInputJoyConfig8
{
private:
    IDirectInputJoyConfig8 *m_ptr;
    ULONG m_uret;
    HWND m_CopHwnd;
public:
    CDirectInputJoyConfig8Hook(IDirectInputJoyConfig8* ptr) : m_ptr(ptr) , m_uret(1)
    {
        m_CopHwnd = NULL;
    }
    ~CDirectInputJoyConfig8Hook()
    {
        this->m_ptr = NULL;
    }

public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj)
    {
        HRESULT hr=S_FAILED;
        DINPUT_JOYCONFIG_IN();
        DEBUG_INFO("riid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                   riid.Data1,
                   riid.Data2,
                   riid.Data3,
                   riid.Data4[0],
                   riid.Data4[1],
                   riid.Data4[2],
                   riid.Data4[3],
                   riid.Data4[4],
                   riid.Data4[5],
                   riid.Data4[6],
                   riid.Data4[7]);
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }

    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DINPUT_JOYCONFIG_IN();
        this->m_uret ++;
        uret = this->m_uret;
        DINPUT_JOYCONFIG_OUT();
        return uret;
    }

    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DINPUT_JOYCONFIG_IN();
        this->m_uret --;
        uret = this->m_uret;
        if(uret == 0)
        {
            UnregisterDirectInputJoyConfig8(this);
            delete this;
        }
        return uret;
    }


    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr = S_OK;
        DINPUT_JOYCONFIG_IN();
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr=S_OK;
        DINPUT_JOYCONFIG_IN();
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }
    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD level)
    {
        HRESULT hr=S_OK;
        DINPUT_JOYCONFIG_IN();
        this->m_CopHwnd = hwnd;
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendNotify)(THIS)
    {
        HRESULT hr=S_OK;
        DINPUT_JOYCONFIG_IN();
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumTypes)(THIS_ LPDIJOYTYPECALLBACK lpCallback,LPVOID pvRef)
    {
        HRESULT hr;

        if(lpCallback == NULL)
        {
            return S_FAIL;
        }
        DINPUT_JOYCONFIG_IN();
		
        DINPUT_JOYCONFIG_OUT();
        return hr;
    }


};

#define IS_IID_MOUSE(riid)  ( (riid)	== GUID_SysMouse ||(riid) == GUID_SysMouseEm ||(riid) == GUID_SysMouseEm2 )
#define IS_IID_KEYBOARD(riid) ((riid) == GUID_SysKeyboard ||(riid) == GUID_SysKeyboardEm ||(riid) == GUID_SysKeyboardEm2)

#define EQUAL_DEVICE_8A_VECS() \
do\
{\
	assert(st_Key8AVecs.size() == st_Key8AHookVecs.size());\
	assert(st_Mouse8AVes.size() == st_Mouse8AHookVecs.size());\
	assert(st_NotSet8AVecs.size() == st_NotSet8AHookVecs.size());\
}while(0)


#define  EQUAL_DEVICE_8W_VECS()  \
do\
{\
	assert(st_Key8WVecs.size() == st_Key8WHookVecs.size());\
	assert(st_Mouse8WVecs.size() == st_Mouse8WHookVecs.size());\
	assert(st_NotSet8WVecs.size() == st_NotSet8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInputDevice8AHook(IDirectInputDevice8A* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8A_VECS();

    findidx = -1;
    for(i=0; i<st_Key8AVecs.size() ; i++)
    {
        if(st_Key8AVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_Key8AHookVecs.erase(st_Key8AHookVecs.begin() + findidx);
        st_Key8AVecs.erase(st_Key8AVecs.begin() + findidx);
    }
    else
    {
        findidx = -1;
        for(i=0; i<st_Mouse8AVes.size() ; i++)
        {
            if(st_Mouse8AVes[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            st_Mouse8AVes.erase(st_Mouse8AVes.begin() + findidx);
            st_Mouse8AHookVecs.erase(st_Mouse8AHookVecs.begin() + findidx);
        }
        else
        {
            findidx = -1;
            for(i=0; i<st_NotSet8AVecs.size(); i++)
            {
                if(st_NotSet8AVecs[i] == ptr)
                {
                    findidx = i;
                    break;
                }
            }

            if(findidx >= 0)
            {
                st_NotSet8AVecs.erase(st_NotSet8AVecs.begin() + findidx);
                st_NotSet8AHookVecs.erase(st_NotSet8AHookVecs.begin() + findidx);
            }
            else
            {
                ERROR_INFO("<0x%p> not found for dinput device\n",ptr);
            }
        }
    }
    LeaveCriticalSection(&st_Dinput8DeviceCS);


    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}






//#define  DIRECT_INPUT_DEVICE_8A_IN()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
//#define  DIRECT_INPUT_DEVICE_8A_OUT()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8A_IN()
#define  DIRECT_INPUT_DEVICE_8A_OUT()


#define  SET_BIT(c)  ((c)=0x80)
#define  CLEAR_BIT(c)  ((c)=0x00)


class CDirectInputDevice8AHook : public IDirectInputDevice8A
{
private:
    IDirectInputDevice8A* m_ptr;
    IID m_iid;
    int m_BufSize;
    int m_SeqId;
    CRITICAL_SECTION m_CS;
private:
    int __IsMouseDevice()
    {
        int ret = 0;
        if(IS_IID_MOUSE((this->m_iid)))
        {
            ret = 1;
        }

        return ret;
    }

    int __IsKeyboardDevice()
    {
        int ret = 0;
        if(IS_IID_KEYBOARD(this->m_iid))
        {
            ret = 1;
        }
        return ret;
    }


public:
    CDirectInputDevice8AHook(IDirectInputDevice8A* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
        m_BufSize = 0;
        m_SeqId = 0;
        InitializeCriticalSection(&(m_CS));
    };

    ~CDirectInputDevice8AHook()
    {
        m_iid = IID_NULL;
        m_SeqId = 0;
        m_BufSize = 0;
        DeleteCriticalSection(&(m_CS));
    }




public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8A_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8A_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_DEVICE_8A_OUT();
        if(uret == 1)
        {
            uret = UnRegisterDirectInputDevice8AHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,GetCapabilities)(THIS_ LPDIDEVCAPS lpDIDevCaps)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetCapabilities(lpDIDevCaps);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumObjects(lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetProperty)(THIS_ REFGUID rguidProp,LPDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetProperty)(THIS_ REFGUID rguidProp,LPCDIPROPHEADER pdiph)
    {
        HRESULT hr;
        LPDIPROPDWORD pWord=NULL;
        GUID* pGuid = (GUID*)&rguidProp;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
        if(SUCCEEDED(hr) && pGuid == (GUID*)1)
        {
            EnterCriticalSection(&(this->m_CS));
            pWord = (LPDIPROPDWORD)pdiph;
            this->m_BufSize = pWord->dwData;
            LeaveCriticalSection(&(this->m_CS));
        }
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Acquire();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Unacquire();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceState)(THIS_ DWORD cbData,LPVOID lpvData)
    {
        HRESULT hr;
        int ret;
        DIRECT_INPUT_DEVICE_8A_IN();
        if(this->__IsMouseDevice())
        {
            if(lpvData == NULL)
            {
                ret = ERROR_INVALID_PARAMETER;
                hr = DIERR_INVALIDPARAM;
                SetLastError(ret);
            }
            else
            {
                ret=  __CopyDiMouseState(lpvData,cbData);
                if(ret < 0)
                {
                    hr = DIERR_INVALIDPARAM;
                }
                else
                {
                    hr = DI_OK;
                }
            }
        }
        else if(this->__IsKeyboardDevice())
        {
            if(lpvData == NULL)
            {
                ret = ERROR_INVALID_PARAMETER;
                hr = DIERR_INVALIDPARAM;
                SetLastError(ret);
            }
            else
            {
                ret=  __CopyDiKeyState(lpvData,cbData);
                if(ret < 0)
                {
                    hr = DIERR_INVALIDPARAM;
                }
                else
                {
                    hr = DI_OK;
                }
            }
        }
        else
        {
            hr = this->m_ptr->GetDeviceState(cbData,lpvData);
        }
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr = DI_OK;
        LPDIDEVICEOBJECTDATA pData=NULL,pCopyData;
        int idx=0,num=0,origidx=0;
        int i;
        int ret;
        DIRECT_INPUT_DEVICE_8A_IN();
        if(cbObjectData != sizeof(*pData) || pdwInOut == NULL || *pdwInOut == 0)
        {
            hr = DIERR_INVALIDPARAM;
        }
        else if(this->__IsKeyboardDevice() || this->__IsMouseDevice())
        {

            hr = DI_OK;
            EnterCriticalSection(&(this->m_CS));
            if(this->m_BufSize == 0)
            {
                hr = DIERR_NOTBUFFERED;
            }
            LeaveCriticalSection(&(this->m_CS));
            if(hr != DI_OK)
            {
                ERROR_INFO("not initialized for buffer\n");
                goto fail;
            }

            if(this->__IsKeyboardDevice())
            {
                pData = __GetKeyboardData();
                if(pData == NULL)
                {
                    *pdwInOut = 0;
                }
                else
                {
                    /*now if the data is ok ,so copy it*/
                    EnterCriticalSection(&(this->m_CS));
                    if(rgdod)
                    {
                        *pdwInOut = 1;
                        pData->dwSequence = this->m_SeqId;
                        pData->uAppData = 0xffffffff;
                        CopyMemory(rgdod,pData,sizeof(*pData));
                    }

                    if(dwFlags != DIGDD_PEEK && pData)
                    {
                        this->m_SeqId ++;
                    }

                    if(dwFlags == DIGDD_PEEK && pData)
                    {
                        ret = __InsertKeyboardDinputData(pData,0);
                        if(ret < 0)
                        {
                            LeaveCriticalSection(&(this->m_CS));
                            assert(0!=0);
                            hr = DIERR_OUTOFMEMORY;
                            goto fail;
                        }
                    }
                    LeaveCriticalSection(&(this->m_CS));

                    /*to free data*/
                    if(pData)
                    {
                        free(pData);
                    }
                    pData = NULL;
                }
            }
            else if(this->__IsMouseDevice())
            {
                pData = __GetMouseData(&num,&idx);
                if(pData == NULL)
                {
                    *pdwInOut = 0;
                }
                else
                {
                    EnterCriticalSection(&(this->m_CS));
                    origidx=  idx;
                    if((int)(*pdwInOut) >= (num - idx))
                    {
                        (*pdwInOut) = (num - idx);
                        idx = num;
                    }
                    else
                    {
                        *pdwInOut = *pdwInOut;
                        idx += (*pdwInOut);
                    }

                    if(rgdod)
                    {
                        for(i=0; i<num; i++)
                        {
                            pData[i].dwSequence = this->m_SeqId;
                            pData[i].uAppData = 0xffffffff;
                        }
                        pCopyData = pData + idx;
                        CopyMemory(rgdod,pCopyData,(*pdwInOut)* sizeof(*pData));
                    }

                    if(dwFlags != DIGDD_PEEK && pData && num == idx)
                    {
                        this->m_SeqId ++;
                    }

                    if(dwFlags == DIGDD_PEEK && pData)
                    {
                        ret = __InsertMouseDinputData(pData,num,origidx,0);
                        if(ret < 0)
                        {
                            LeaveCriticalSection(&(this->m_CS));
                            assert(0!=0);
                            hr = DIERR_OUTOFMEMORY;
                            goto fail;
                        }
                    }
                    else if(num != idx && pData)
                    {
                        ret = __InsertMouseDinputData(pData,num,idx,0);
                        if(ret < 0)
                        {
                            LeaveCriticalSection(&(this->m_CS));
                            assert(0!=0);
                            hr = DIERR_OUTOFMEMORY;
                            goto fail;
                        }
                    }
                    LeaveCriticalSection(&(this->m_CS));

                    if(pData)
                    {
                        DEBUG_BUFFER_FMT(rgdod,sizeof(*rgdod)*(*pdwInOut),"Mouse Data (%d)",(*pdwInOut));
                        free(pData);
                    }
                    pData = NULL;
                }
            }
        }
        else
        {
            hr = this->m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        }
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
fail:
        if(pData)
        {
            free(pData);
        }
        pData = NULL;
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetDataFormat)(THIS_ LPCDIDATAFORMAT lpdf)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetDataFormat(lpdf);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetEventNotification)(THIS_ HANDLE hEvent)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetEventNotification(hEvent);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetCooperativeLevel(hwnd,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA pdidoi,DWORD dwObj,DWORD dwHow)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetObjectInfo(pdidoi,dwObj,dwHow);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA pdidi)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetDeviceInfo(pdidi);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_ HINSTANCE hinst,DWORD dwVersion,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Initialize(hinst,dwVersion,rguid);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,CreateEffect)(THIS_ REFGUID rguid,LPCDIEFFECT lpeff,LPDIRECTINPUTEFFECT * ppdeff,LPUNKNOWN punkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->CreateEffect(rguid,lpeff,ppdeff,punkOuter);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA lpCallback,LPVOID pvRef,DWORD dwEffType)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumEffects(lpCallback,pvRef,dwEffType);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,GetEffectInfo)(THIS_ LPDIEFFECTINFOA pdei,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetEffectInfo(pdei,rguid);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetForceFeedbackState)(THIS_  LPDWORD pdwOut)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetForceFeedbackState(pdwOut);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendForceFeedbackCommand)(THIS_ DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SendForceFeedbackCommand(dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumCreatedEffectObjects)(THIS_  LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,LPVOID pvRef,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumCreatedEffectObjects(lpCallback,pvRef,fl);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Escape)(THIS_ LPDIEFFESCAPE pesc)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Escape(pesc);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Poll)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->Poll();
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendDeviceData)(THIS_ DWORD cbObjectData,LPCDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SendDeviceData(cbObjectData,rgdod,pdwInOut,fl);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffectsInFile)(THIS_ LPCSTR lpszFileName,LPDIENUMEFFECTSINFILECALLBACK pec,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->EnumEffectsInFile(lpszFileName,pec,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,WriteEffectToFile)(THIS_ LPCSTR lpszFileName,DWORD dwEntries,LPDIFILEEFFECT rgDiFileEft,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->WriteEffectToFile(lpszFileName,dwEntries,rgDiFileEft,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,BuildActionMap)(THIS_ LPDIACTIONFORMATA lpdiaf,LPCSTR lpszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->BuildActionMap(lpdiaf,lpszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetActionMap)(THIS_  LPDIACTIONFORMATA lpdiActionFormat,LPCSTR lptszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetActionMap(lpdiActionFormat,lptszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetImageInfo)(THIS_  LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetImageInfo(lpdiDevImageInfoHeader);
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

};

CDirectInputDevice8AHook* RegisterDirectInputDevice8AHook(IDirectInputDevice8A* ptr,REFIID riid)
{
    CDirectInputDevice8AHook* pHookA=NULL;
    int findidx = -1;
    unsigned int i;


    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8A_VECS();
    if(IS_IID_MOUSE(riid))
    {
        for(i=0; i<st_Mouse8AVes.size(); i++)
        {
            if(st_Mouse8AVes[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_Mouse8AHookVecs[findidx];
        }
        else
        {
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
            st_Mouse8AVes.push_back(ptr);
            st_Mouse8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
        if(st_Mouse8AVes.size() > 1)
        {
            DEBUG_INFO("Mouse8AVes size (%d)\n",st_Mouse8AVes.size());
        }

    }
    else if(IS_IID_KEYBOARD(riid))
    {
        for(i=0; i<st_Key8AVecs.size(); i++)
        {
            if(st_Key8AVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_Key8AHookVecs[findidx];
        }
        else
        {
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
            st_Key8AVecs.push_back(ptr);
            st_Key8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
        if(st_Key8AVecs.size() > 1)
        {
            DEBUG_INFO("Key8AVecs size (%d)\n",st_Key8AVecs.size());
        }
    }
    else
    {
        for(i=0; i<st_NotSet8AVecs.size(); i++)
        {
            if(st_NotSet8AVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookA = st_NotSet8AHookVecs[findidx];
        }
        else
        {
            pHookA =new CDirectInputDevice8AHook(ptr,riid);
            st_NotSet8AVecs.push_back(ptr);
            st_NotSet8AHookVecs.push_back(pHookA);
            ptr->AddRef();
        }
    }

    LeaveCriticalSection(&st_Dinput8DeviceCS);

    return pHookA;
}



/*****************************************
*  to make the IDirectInputDevice8W hook
*
*****************************************/

ULONG UnRegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8W_VECS();
    findidx = -1;
    for(i=0; i<st_Key8WVecs.size() ; i++)
    {
        if(st_Key8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_Key8WVecs.erase(st_Key8WVecs.begin() + findidx);
        st_Key8WHookVecs.erase(st_Key8WHookVecs.begin() + findidx);
    }
    else
    {
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(st_Mouse8WVecs[i] == ptr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            st_Mouse8WVecs.erase(st_Mouse8WVecs.begin() + findidx);
            st_Mouse8WHookVecs.erase(st_Mouse8WHookVecs.begin() + findidx);
        }
        else
        {
            for(i=0; i<st_NotSet8WVecs.size() ; i++)
            {
                if(st_NotSet8WVecs[i] == ptr)
                {
                    findidx = i;
                    break;
                }
            }

            if(findidx >= 0)
            {
                st_NotSet8WVecs.erase(st_NotSet8WVecs.begin() + findidx);
                st_NotSet8WHookVecs.erase(st_NotSet8WHookVecs.begin() + findidx);
            }
            else
            {
                ERROR_INFO("<0x%p> not in the vectors\n",ptr);
            }
        }
    }

    LeaveCriticalSection(&st_Dinput8DeviceCS);

    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}


//#define  DIRECT_INPUT_DEVICE_8W_IN()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p->0x%p in\n",__FUNCTION__,this,this->m_ptr);}while(0)
//#define  DIRECT_INPUT_DEVICE_8W_OUT()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p->0x%p out\n",__FUNCTION__,this,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8W_IN()
#define  DIRECT_INPUT_DEVICE_8W_OUT()


class CDirectInputDevice8WHook : public IDirectInputDevice8W
{
private:
    IDirectInputDevice8W* m_ptr;
    int m_BufSize;
    int m_SeqId;
    IID m_iid;
    CRITICAL_SECTION m_CS;
private:
    int __IsMouseDevice()
    {
        int ret = 0;
        if(IS_IID_MOUSE((this->m_iid)))
        {
            ret = 1;
        }

        return ret;
    }

    int __IsKeyboardDevice()
    {
        int ret = 0;
        if(IS_IID_KEYBOARD(this->m_iid))
        {
            ret = 1;
        }
        return ret;
    }
public:
    CDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
        m_BufSize = 0;
        m_SeqId = 0;
        DEBUG_INFO("\n");
        InitializeCriticalSection(&(m_CS));
        DEBUG_INFO("\n");
    };
    ~CDirectInputDevice8WHook()
    {
        m_iid = IID_NULL;
        m_BufSize = 0;
        m_SeqId = 0;
        DeleteCriticalSection(&(m_CS));
    }
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8W_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_DEVICE_8W_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_DEVICE_8W_OUT();
        DINPUT_DEBUG_INFO("<0x%p->0x%p>uret = %d\n",this,this->m_ptr,uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInputDevice8WHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,GetCapabilities)(THIS_ LPDIDEVCAPS lpDIDevCaps)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetCapabilities(lpDIDevCaps);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumObjects(lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetProperty)(THIS_ REFGUID rguidProp,LPDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetProperty(rguidProp,pdiph);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetProperty)(THIS_ REFGUID rguidProp,LPCDIPROPHEADER pdiph)
    {
        HRESULT hr;
        DIPROPDWORD* pWord;
        GUID* pGuid = (GUID*)(&rguidProp);
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
        if(SUCCEEDED(hr) && pGuid == (GUID*)1)
        {
            DEBUG_INFO("\n");
            EnterCriticalSection(&(this->m_CS));
            DEBUG_INFO("\n");
            pWord = (LPDIPROPDWORD)pdiph;
            DEBUG_INFO("\n");
            this->m_BufSize = pWord->dwData;
            DEBUG_INFO("\n");
            LeaveCriticalSection(&(this->m_CS));
            DEBUG_INFO("\n");
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Acquire();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Unacquire();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceState)(THIS_ DWORD cbData,LPVOID lpvData)
    {
        HRESULT hr;
        int ret;
        DIRECT_INPUT_DEVICE_8W_IN();
        if(this->__IsMouseDevice())
        {
            if(lpvData == NULL)
            {
                ret = ERROR_INVALID_PARAMETER;
                hr = DIERR_INVALIDPARAM;
                SetLastError(ret);
            }
            else
            {
                ret=  __CopyDiMouseState(lpvData,cbData);
                if(ret < 0)
                {
                    hr = DIERR_INVALIDPARAM;
                }
                else
                {
                    hr = DI_OK;
                }
            }
        }
        else if(this->__IsKeyboardDevice())
        {
            if(lpvData == NULL)
            {
                ret = ERROR_INVALID_PARAMETER;
                hr = DIERR_INVALIDPARAM;
                SetLastError(ret);
            }
            else
            {
                ret=  __CopyDiKeyState(lpvData,cbData);
                if(ret < 0)
                {
                    hr = DIERR_INVALIDPARAM;
                }
                else
                {
                    hr = DI_OK;
                }
            }
        }
        else
        {
            hr = this->m_ptr->GetDeviceState(cbData,lpvData);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr=DI_OK;
        LPDIDEVICEOBJECTDATA pData=NULL,pCopyData;
        int ret,num=0,idx=0,origidx=0;
        int i;
        DIRECT_INPUT_DEVICE_8W_IN();
        if(this->__IsMouseDevice() || this->__IsKeyboardDevice())
        {
            if(cbObjectData != sizeof(*rgdod) || (dwFlags != 0 && dwFlags != DIGDD_PEEK) ||
                    pdwInOut == NULL)
            {
                hr = DIERR_INVALIDPARAM;
            }
            else
            {
                hr = DI_OK;
                EnterCriticalSection(&(this->m_CS));
                if(this->m_BufSize == 0)
                {
                    hr = DIERR_NOTBUFFERED;
                }
                LeaveCriticalSection(&(this->m_CS));
                if(hr != DI_OK)
                {
                    ERROR_INFO("not initialized for buffer\n");
                    goto fail;
                }
                if(this->__IsKeyboardDevice())
                {
                    pData = __GetKeyboardData();
                    EnterCriticalSection(&(this->m_CS));
                    if(pData)
                    {
                        if(rgdod)
                        {
                            pData->dwSequence = this->m_SeqId;
                            pData->uAppData = 0xffffffff;
                            CopyMemory(rgdod,pData,sizeof(*pData));
                        }
                        *pdwInOut = 1;
                    }
                    else
                    {
                        *pdwInOut = 0;
                    }

                    if(dwFlags != DIGDD_PEEK && pData)
                    {
                        this->m_SeqId ++;
                    }
                    if(dwFlags == DIGDD_PEEK && pData)
                    {
                        ret = __InsertKeyboardDinputData(pData,0);
                        if(ret < 0)
                        {
                            LeaveCriticalSection(&(this->m_CS));
                            assert(0!=0);
                            hr = DIERR_OUTOFMEMORY;
                            goto fail;
                        }
                    }
                    LeaveCriticalSection(&(this->m_CS));


                    if(pData)
                    {
                        free(pData);
                    }
                    pData = NULL;

                }
                else if(this->__IsMouseDevice())
                {
                    pData = __GetMouseData(&num,&idx);
                    EnterCriticalSection(&(this->m_CS));
                    if(pData == NULL)
                    {
                        *pdwInOut = 0;
                    }
                    else
                    {
                        origidx = idx;
                        if((int)(*pdwInOut) < (num - idx))
                        {
                            *pdwInOut= *pdwInOut;
                            idx += (*pdwInOut);
                        }
                        else
                        {
                            *pdwInOut = num - idx;
                            idx = num;
                        }

                        if(rgdod)
                        {
                            for(i=0; i<num; i++)
                            {
                                pData[i].dwSequence = this->m_SeqId;
                                pData[i].uAppData = 0xffffffff;
                            }
                            pCopyData = pData + origidx;
                            CopyMemory(rgdod,pCopyData,(*pdwInOut)*sizeof(*pData));
                        }
                        if(dwFlags != DIGDD_PEEK && pData && num == idx)
                        {
                            this->m_SeqId ++;
                        }
                        if(dwFlags == DIGDD_PEEK && pData)
                        {
                            ret = __InsertMouseDinputData(pData,num,origidx,0);
                            if(ret < 0)
                            {
                                LeaveCriticalSection(&(this->m_CS));
                                assert(0!=0);
                                hr = DIERR_OUTOFMEMORY;
                                goto fail;
                            }
                        }
                        else if(num != idx && pData)
                        {
                            ret = __InsertMouseDinputData(pData,num,idx,0);
                            if(ret < 0)
                            {
                                LeaveCriticalSection(&(this->m_CS));
                                assert(0!=0);
                                hr = DIERR_OUTOFMEMORY;
                                goto fail;
                            }
                        }
                    }
                    LeaveCriticalSection(&(this->m_CS));
                    if(pData)
                    {
                        free(pData);
                    }
                    pData = NULL;
                }
            }
        }
        else
        {
            hr = m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
fail:
        if(pData)
        {
            free(pData);
        }
        pData = NULL;
        return hr;
    }

    COM_METHOD(HRESULT,SetDataFormat)(THIS_ LPCDIDATAFORMAT lpdf)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetDataFormat(lpdf);
        if(hr == DI_OK)
        {
            if(this->m_iid == GUID_SysMouse)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouse Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboard)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboard Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_Joystick)
            {
                DINPUT_DEBUG_INFO("<0x%p> Joystick Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm2 Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm Format\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm2 Format\n",this->m_ptr);
            }

            if(lpdf == &c_dfDIMouse)
            {
                DINPUT_DEBUG_INFO("c_dfDIMouse\n");
            }
            else if(lpdf == &c_dfDIMouse2)
            {
                DINPUT_DEBUG_INFO("c_dfDIMouse2\n");
            }
            else if(lpdf == &c_dfDIKeyboard)
            {
                DINPUT_DEBUG_INFO("c_dfDIKeyboard\n");
            }
            else if(lpdf == &c_dfDIJoystick)
            {
                DINPUT_DEBUG_INFO(" c_dfDIJoystick\n");
            }
            else if(lpdf == &c_dfDIJoystick2)
            {
                DINPUT_DEBUG_INFO(" c_dfDIJoystick2\n");
            }
            if(lpdf->dwSize)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpdf,lpdf->dwSize,"<0x%p> format",this->m_ptr);
            }

            if(lpdf->rgodf && lpdf->dwObjSize)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpdf->rgodf,lpdf->dwObjSize,"<0x%p> flag 0x%08x datasize 0x%08x numobjs 0x%08x",this->m_ptr,lpdf->dwFlags,lpdf->dwDataSize,lpdf->dwNumObjs);
            }


        }
        else
        {
            ERROR_INFO("<0x%p> SetDataFormat Error\n",this->m_ptr);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetEventNotification)(THIS_ HANDLE hEvent)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetEventNotification(hEvent);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetCooperativeLevel(hwnd,dwFlags);
        if(hr == DI_OK)
        {
            DINPUT_DEBUG_INFO("<0x%p> hwnd 0x%08x dwLevel 0x%08x\n",this->m_ptr,
                              hwnd,dwFlags);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEW pdidoi,DWORD dwObj,DWORD dwHow)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetObjectInfo(pdidoi,dwObj,dwHow);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEW pdidi)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceInfo(pdidi);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_ HINSTANCE hinst,DWORD dwVersion,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Initialize(hinst,dwVersion,rguid);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,CreateEffect)(THIS_ REFGUID rguid,LPCDIEFFECT lpeff,LPDIRECTINPUTEFFECT * ppdeff,LPUNKNOWN punkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->CreateEffect(rguid,lpeff,ppdeff,punkOuter);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKW lpCallback,LPVOID pvRef,DWORD dwEffType)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumEffects(lpCallback,pvRef,dwEffType);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,GetEffectInfo)(THIS_ LPDIEFFECTINFOW pdei,REFGUID rguid)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetEffectInfo(pdei,rguid);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetForceFeedbackState)(THIS_  LPDWORD pdwOut)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetForceFeedbackState(pdwOut);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendForceFeedbackCommand)(THIS_ DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SendForceFeedbackCommand(dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumCreatedEffectObjects)(THIS_  LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,LPVOID pvRef,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumCreatedEffectObjects(lpCallback,pvRef,fl);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Escape)(THIS_ LPDIEFFESCAPE pesc)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Escape(pesc);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Poll)(THIS)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->Poll();
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendDeviceData)(THIS_ DWORD cbObjectData,LPCDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD fl)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SendDeviceData(cbObjectData,rgdod,pdwInOut,fl);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumEffectsInFile)(THIS_ LPCWSTR lpszFileName,LPDIENUMEFFECTSINFILECALLBACK pec,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->EnumEffectsInFile(lpszFileName,pec,pvRef,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,WriteEffectToFile)(THIS_ LPCWSTR lpszFileName,DWORD dwEntries,LPDIFILEEFFECT rgDiFileEft,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->WriteEffectToFile(lpszFileName,dwEntries,rgDiFileEft,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,BuildActionMap)(THIS_ LPDIACTIONFORMATW lpdiaf,LPCTSTR lpszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->BuildActionMap(lpdiaf,lpszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetActionMap)(THIS_	LPDIACTIONFORMATW lpdiActionFormat,LPCTSTR lptszUserName,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetActionMap(lpdiActionFormat,lptszUserName,dwFlags);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetImageInfo)(THIS_	LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetImageInfo(lpdiDevImageInfoHeader);
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

};

CDirectInputDevice8WHook* RegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid)
{
    CDirectInputDevice8WHook* pHookW=NULL;
    int findidx = -1;
    unsigned int i;

    EnterCriticalSection(&st_Dinput8DeviceCS);
    EQUAL_DEVICE_8W_VECS();
    if(IS_IID_KEYBOARD(riid))
    {
        for(i=0; i<st_Key8WVecs.size() ; i++)
        {
            if(ptr == st_Key8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_Key8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_Key8WVecs.push_back(ptr);
            st_Key8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
        if(st_Key8WVecs.size() > 1)
        {
            DEBUG_INFO("Key8WVecs size (%d)\n",st_Key8WVecs.size());
        }
    }
    else if(IS_IID_MOUSE(riid))
    {
        for(i=0; i<st_Mouse8WVecs.size() ; i++)
        {
            if(ptr == st_Mouse8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_Mouse8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_Mouse8WVecs.push_back(ptr);
            st_Mouse8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
        if(st_Mouse8WVecs.size() > 1)
        {
            DEBUG_INFO("Mouse8WVecs size (%d)\n",st_Mouse8WVecs.size());
        }
    }
    else
    {
        for(i=0; i<st_NotSet8WVecs.size() ; i++)
        {
            if(ptr == st_NotSet8WVecs[i])
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pHookW = st_NotSet8WHookVecs[findidx];
        }
        else
        {
            pHookW = new CDirectInputDevice8WHook(ptr,riid);
            assert(pHookW);
            st_NotSet8WVecs.push_back(ptr);
            st_NotSet8WHookVecs.push_back(pHookW);
            /*to make it not freed by accident*/
            ptr->AddRef();
        }
    }

    LeaveCriticalSection(&st_Dinput8DeviceCS);

    return pHookW;
}



/*****************************************
*   to make the DirectInput8A hook
*
*****************************************/
class CDirectInput8AHook;

static std::vector<IDirectInput8A*> st_DI8AVecs;
static std::vector<CDirectInput8AHook*> st_CDI8AHookVecs;
static CRITICAL_SECTION st_DI8ACS;

#define EQUAL_DI8A_VECS() \
do\
{\
	assert(st_DI8AVecs.size() == st_CDI8AHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInput8AHook(IDirectInput8A* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DI8ACS);
    EQUAL_DI8A_VECS();
    for(i=0; i<st_DI8AVecs.size() ; i++)
    {
        if(st_DI8AVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DI8AVecs.erase(st_DI8AVecs.begin()+findidx);
        st_CDI8AHookVecs.erase(st_CDI8AHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("could not find 0x%p DirectInput8A\n",ptr);
    }
    LeaveCriticalSection(&st_DI8ACS);

    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}

#define  DIRECT_INPUT_8A_IN() do{DINPUT_DEBUG_INFO("Input8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_8A_OUT() do{DINPUT_DEBUG_INFO("Input8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)
class CDirectInput8AHook :public IDirectInput8A
{
private:
    IDirectInput8A* m_ptr;

public:
    CDirectInput8AHook(IDirectInput8A* ptr):m_ptr(ptr) {};
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8A_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_8A_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8A_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_8A_OUT();
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
        if(uret == 1)
        {
            uret = UnRegisterDirectInput8AHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,CreateDevice)(THIS_ REFGUID rguid,
                                     LPDIRECTINPUTDEVICE8A * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        if(SUCCEEDED(hr))
        {
            CDirectInputDevice8AHook* pHookA=NULL;
            if(rguid == GUID_SysMouse)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouse 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboard)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboard 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_Joystick)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("joystick 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysMouseEm)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysMouseEm2)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboardEm)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }
            else if(rguid == GUID_SysKeyboardEm2)
            {
                pHookA = RegisterDirectInputDevice8AHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookA);
            }


            DINPUT_DEBUG_INFO("rguid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              rguid.Data1,
                              rguid.Data2,
                              rguid.Data3,
                              rguid.Data4[0],
                              rguid.Data4[1],
                              rguid.Data4[2],
                              rguid.Data4[3],
                              rguid.Data4[4],
                              rguid.Data4[5],
                              rguid.Data4[6],
                              rguid.Data4[7]);

            if(pHookA)
            {
                /*if we make the hook object ,just replace it to the upper caller*/
                *lplpDirectInputDevice = pHookA;
            }
        }
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACKA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        if(dwDevType == DI8DEVCLASS_GAMECTRL)
        {
            /*we do not get any gamectrl ,so just return ok*/
            DEBUG_INFO("\n");
            hr = DI_OK;
        }
        else
        {
            hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
        }
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceStatus)(THIS_  REFGUID rguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->GetDeviceStatus(rguidInstance);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_  HINSTANCE hinst,DWORD dwVersion)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->Initialize(hinst,dwVersion);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,FindDevice)(THIS_ REFGUID rguidClass,LPCSTR ptszName,LPGUID pguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->FindDevice(rguidClass,ptszName,pguidInstance);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevicesBySemantics)(THIS_ LPCSTR ptszUserName,LPDIACTIONFORMATA lpdiActionFormat,LPDIENUMDEVICESBYSEMANTICSCBA lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->EnumDevicesBySemantics(ptszUserName,lpdiActionFormat,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMSA lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->ConfigureDevices(lpdiCallback,lpdiCDParams,dwFlags,pvRefData);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

};

CDirectInput8AHook* RegisterDirectInput8AHook(IDirectInput8A* ptr)
{
    int findidx=-1;
    unsigned int i;
    CDirectInput8AHook* pHook=NULL;

    EnterCriticalSection(&st_DI8ACS);
    EQUAL_DI8A_VECS();
    for(i=0; i<st_DI8AVecs.size() ; i++)
    {
        if(st_DI8AVecs[i]==ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        pHook =new CDirectInput8AHook(ptr);
        st_DI8AVecs.push_back(ptr);
        st_CDI8AHookVecs.push_back(pHook);
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
    }
    else
    {
        pHook = st_CDI8AHookVecs[findidx];
    }
    LeaveCriticalSection(&st_DI8ACS);
    return pHook;
}


/*****************************************
*   to make the DirectInput8W hook
*
*****************************************/


class CDirectInput8WHook;

static std::vector<IDirectInput8W*> st_DI8WVecs;
static std::vector<CDirectInput8WHook*> st_CDI8WHookVecs;
static CRITICAL_SECTION st_DI8WCS;

#define EQUAL_DI8W_VECS() \
do\
{\
	assert(st_DI8WVecs.size() == st_CDI8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInput8WHook(IDirectInput8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DI8WCS);
    EQUAL_DI8W_VECS();
    for(i=0; i<st_DI8WVecs.size() ; i++)
    {
        if(st_DI8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DI8WVecs.erase(st_DI8WVecs.begin()+findidx);
        st_CDI8WHookVecs.erase(st_CDI8WHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("Not Register 0x%p\n",ptr);
    }
    LeaveCriticalSection(&st_DI8WCS);

    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}



#define  DIRECT_INPUT_8W_IN()  do{DINPUT_DEBUG_INFO("Input8W::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_8W_OUT()  do{DINPUT_DEBUG_INFO("Input8W::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)

class CDirectInput8WHook : IDirectInput8W
{
private:
    IDirectInput8W* m_ptr;
public:
    CDirectInput8WHook(IDirectInput8W* ptr) : m_ptr(ptr) {};
public:
    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid,void **ppvObject)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->QueryInterface(riid,ppvObject);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }
    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8W_IN();
        uret = m_ptr->AddRef();
        DIRECT_INPUT_8W_OUT();
        return uret;
    }
    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DIRECT_INPUT_8W_IN();
        uret = m_ptr->Release();
        DIRECT_INPUT_8W_OUT();
        if(uret == 1)
        {
            uret = UnRegisterDirectInput8WHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }

    COM_METHOD(HRESULT,CreateDevice)(THIS_ REFGUID rguid,
                                     LPDIRECTINPUTDEVICE8W * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        SetUnHandlerExceptionDetour();
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        if(SUCCEEDED(hr))
        {
            CDirectInputDevice8WHook* pHookW=NULL;
            if(rguid == GUID_SysMouse)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouse 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboard)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboard 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_Joystick)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("joystick 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysMouseEm)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysMouseEm2)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("sysmouseem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboardEm)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }
            else if(rguid == GUID_SysKeyboardEm2)
            {
                pHookW = RegisterDirectInputDevice8WHook(*lplpDirectInputDevice,rguid);
                DINPUT_DEBUG_INFO("syskeyboardem2 0x%p hook 0x%p\n",*lplpDirectInputDevice,pHookW);
            }


            DINPUT_DEBUG_INFO("rguid %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                              rguid.Data1,
                              rguid.Data2,
                              rguid.Data3,
                              rguid.Data4[0],
                              rguid.Data4[1],
                              rguid.Data4[2],
                              rguid.Data4[3],
                              rguid.Data4[4],
                              rguid.Data4[5],
                              rguid.Data4[6],
                              rguid.Data4[7]);

            if(pHookW)
            {
                /*if we make the hook object ,just replace it to the upper caller*/
                *lplpDirectInputDevice = pHookW;
            }
        }
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACKW lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        if(dwDevType == DI8DEVCLASS_GAMECTRL)
        {
            DEBUG_INFO("\n");
            hr = DI_OK;
        }
        else
        {
            hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
        }
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceStatus)(THIS_  REFGUID rguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->GetDeviceStatus(rguidInstance);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,RunControlPanel)(THIS_ HWND hwndOwner,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->RunControlPanel(hwndOwner,dwFlags);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,Initialize)(THIS_  HINSTANCE hinst,DWORD dwVersion)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->Initialize(hinst,dwVersion);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,FindDevice)(THIS_ REFGUID rguidClass,LPCTSTR ptszName,LPGUID pguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->FindDevice(rguidClass,ptszName,pguidInstance);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevicesBySemantics)(THIS_ LPCTSTR ptszUserName,LPDIACTIONFORMAT lpdiActionFormat,LPDIENUMDEVICESBYSEMANTICSCB lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->EnumDevicesBySemantics(ptszUserName,lpdiActionFormat,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMSW lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->ConfigureDevices(lpdiCallback,lpdiCDParams,dwFlags,pvRefData);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

};

CDirectInput8WHook* RegisterDirectInput8WHook(IDirectInput8W* ptr)
{
    int findidx=-1;
    unsigned int i;
    CDirectInput8WHook* pHookW=NULL;

    EnterCriticalSection(&st_DI8WCS);
    EQUAL_DI8W_VECS();
    for(i=0; i<st_DI8WVecs.size() ; i++)
    {
        if(st_DI8WVecs[i]==ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        pHookW =new CDirectInput8WHook(ptr);
        st_DI8WVecs.push_back(ptr);
        st_CDI8WHookVecs.push_back(pHookW);
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
    }
    else
    {
        pHookW = st_CDI8WHookVecs[findidx];
    }
    LeaveCriticalSection(&st_DI8WCS);
    return pHookW;
}

#define DIK_NULL  0xff

static int st_CodeMapDik[256] =
{
    DIK_A             ,DIK_B              ,DIK_C              ,DIK_D              ,DIK_E              ,  /*5*/
    DIK_F             ,DIK_G              ,DIK_H              ,DIK_I              ,DIK_J              ,  /*10*/
    DIK_K             ,DIK_L              ,DIK_M              ,DIK_N              ,DIK_O              ,  /*15*/
    DIK_P             ,DIK_Q              ,DIK_R              ,DIK_S              ,DIK_T              ,  /*20*/
    DIK_U             ,DIK_V              ,DIK_W              ,DIK_X              ,DIK_Y              ,  /*25*/
    DIK_Z             ,DIK_0              ,DIK_1              ,DIK_2              ,DIK_3              ,  /*30*/
    DIK_4             ,DIK_5              ,DIK_6              ,DIK_7              ,DIK_8              ,  /*35*/
    DIK_9             ,DIK_ESCAPE         ,DIK_MINUS          ,DIK_EQUALS         ,DIK_BACK           ,  /*40*/
    DIK_TAB           ,DIK_LBRACKET       ,DIK_RBRACKET       ,DIK_RETURN         ,DIK_LCONTROL       ,  /*45*/
    DIK_SEMICOLON     ,DIK_APOSTROPHE     ,DIK_GRAVE          ,DIK_LSHIFT         ,DIK_BACKSLASH      ,  /*50*/
    DIK_COMMA         ,DIK_PERIOD         ,DIK_SLASH          ,DIK_RSHIFT         ,DIK_MULTIPLY       ,  /*55*/
    DIK_LMENU         ,DIK_SPACE          ,DIK_CAPITAL        ,DIK_F1             ,DIK_F2             ,  /*60*/
    DIK_F3            ,DIK_F4             ,DIK_F5             ,DIK_F6             ,DIK_F7             ,  /*65*/
    DIK_F8            ,DIK_F9             ,DIK_F10            ,DIK_F11            ,DIK_F12            ,  /*70*/
    DIK_F13           ,DIK_F14            ,DIK_F15            ,DIK_NUMLOCK        ,DIK_SCROLL         ,  /*75*/
    DIK_SUBTRACT      ,DIK_NUMPAD0        ,DIK_NUMPAD1        ,DIK_NUMPAD2        ,DIK_NUMPAD3        ,  /*80*/
    DIK_NUMPAD4       ,DIK_NUMPAD5        ,DIK_NUMPAD6        ,DIK_NUMPAD7        ,DIK_NUMPAD8        ,  /*85*/
    DIK_NUMPAD9       ,DIK_ADD            ,DIK_DECIMAL        ,DIK_OEM_102        ,DIK_KANA           ,  /*90*/
    DIK_ABNT_C1       ,DIK_CONVERT        ,DIK_NOCONVERT      ,DIK_YEN            ,DIK_ABNT_C2        ,  /*95*/
    DIK_NUMPADEQUALS  ,DIK_PREVTRACK      ,DIK_AT             ,DIK_COLON          ,DIK_UNDERLINE      ,  /*100*/
    DIK_KANJI         ,DIK_STOP           ,DIK_AX             ,DIK_UNLABELED      ,DIK_NEXTTRACK      ,  /*105*/
    DIK_NUMPADENTER   ,DIK_RCONTROL       ,DIK_MUTE           ,DIK_CALCULATOR     ,DIK_PLAYPAUSE      ,  /*110*/
    DIK_MEDIASTOP     ,DIK_VOLUMEDOWN     ,DIK_VOLUMEUP       ,DIK_WEBHOME        ,DIK_NUMPADCOMMA    ,  /*115*/
    DIK_DIVIDE        ,DIK_SYSRQ          ,DIK_RMENU          ,DIK_PAUSE          ,DIK_HOME           ,  /*120*/
    DIK_UP            ,DIK_PRIOR          ,DIK_LEFT           ,DIK_RIGHT          ,DIK_END            ,  /*125*/
    DIK_DOWN          ,DIK_NEXT           ,DIK_INSERT         ,DIK_DELETE         ,DIK_LWIN           ,  /*130*/
    DIK_RWIN          ,DIK_APPS           ,DIK_POWER          ,DIK_SLEEP          ,DIK_WAKE           ,  /*135*/
    DIK_WEBSEARCH     ,DIK_WEBFAVORITES   ,DIK_WEBREFRESH     ,DIK_WEBSTOP        ,DIK_WEBFORWARD     ,  /*140*/
    DIK_WEBBACK       ,DIK_MYCOMPUTER     ,DIK_MAIL           ,DIK_MEDIASELECT    ,DIK_NULL           ,  /*144*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*150*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*155*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*160*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*165*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*170*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*175*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*180*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*185*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*190*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*195*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*200*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*205*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*210*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*215*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*220*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*225*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*230*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*235*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*240*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*245*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*250*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*255*/
    DIK_NULL
};


int __InsertKeyboardDinputData(DIDEVICEOBJECTDATA* pData,int back)
{
    DIDEVICEOBJECTDATA* pInsert=NULL,*pRemove=NULL;
    int ret=1;

    pInsert = (LPDIDEVICEOBJECTDATA)calloc(1,sizeof(*pData));
    if(pInsert == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return -ret;
    }

    CopyMemory(pInsert,pData,sizeof(*pData));
    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    if(back)
    {
        if(st_pKeyboardData.size() > 20)
        {
            pRemove = st_pKeyboardData[0];
            st_pKeyboardData.erase(st_pKeyboardData.begin());
            ret = 0;
        }
        st_pKeyboardData.push_back(pInsert);
    }
    else
    {
        st_pKeyboardData.insert(st_pKeyboardData.begin(),pInsert);
    }

    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    if(pRemove)
    {
        free(pRemove);
    }
    pRemove = NULL;
    return ret;
}


LPDIDEVICEOBJECTDATA __GetKeyboardData()
{
    LPDIDEVICEOBJECTDATA pGetData=NULL;

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    if(st_pKeyboardData.size() > 0)
    {
        pGetData = st_pKeyboardData[0];
        st_pKeyboardData.erase(st_pKeyboardData.begin());
    }
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    return pGetData;
}

LPDIDEVICEOBJECTDATA __GetMouseData(int *pNum,int *pIdx)
{
    LPDIDEVICEOBJECTDATA pData=NULL;
    int num,idx;

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    MOUSE_DATA_EQUAL();
    if(st_pMouseData.size() > 0)
    {
        pData = st_pMouseData[0];
        num = st_MouseDataNums[0];
        idx = st_MouseDataIdx[0];
        st_pMouseData.erase(st_pMouseData.begin());
        st_MouseDataNums.erase(st_MouseDataNums.begin());
        st_MouseDataIdx.erase(st_MouseDataIdx.begin());
    }
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);

    if(pData)
    {
        *pNum = num;
        *pIdx = idx;
    }
    return pData;
}


int __InsertMouseDinputData(DIDEVICEOBJECTDATA *pData,int num,int idx,int back)
{
    DIDEVICEOBJECTDATA* pInsert=NULL,*pRemove=NULL;
    int ret=1;

    pInsert = (LPDIDEVICEOBJECTDATA)calloc(num,sizeof(*pData));
    if(pInsert == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return -ret;
    }
    CopyMemory(pInsert,pData,sizeof(*pData)*num);

    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    MOUSE_DATA_EQUAL();
    if(back)
    {
        if(st_pMouseData.size() > 20)
        {
            pRemove = st_pMouseData[0];
            st_pMouseData.erase(st_pMouseData.begin());
            st_MouseDataNums.erase(st_MouseDataNums.begin());
            st_MouseDataIdx.erase(st_MouseDataIdx.begin());
            ret = 0;
        }

        st_pMouseData.push_back(pInsert);
        st_MouseDataNums.push_back(num);
        st_MouseDataIdx.push_back(idx);
    }
    else
    {
        st_pMouseData.insert(st_pMouseData.begin(),pInsert);
        st_MouseDataNums.insert(st_MouseDataNums.begin(),num);
        st_MouseDataIdx.insert(st_MouseDataIdx.begin(),idx);
    }
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);

    if(pRemove)
    {
        free(pRemove);
    }
    pRemove = NULL;
    return ret;
}


int __Dinput8InsertKeyboardEvent(LPDEVICEEVENT pDevEvent)
{
    int ret;
    DIDEVICEOBJECTDATA data;
    int scancode;

    /*now first to check for the value*/
    if(pDevEvent->event.keyboard.event != KEYBOARD_EVENT_DOWN &&
            pDevEvent->event.keyboard.event != KEYBOARD_EVENT_UP)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> Keyboard Event(%d) not valid\n",
                   pDevEvent,pDevEvent->event.keyboard.event);
        goto fail;
    }

    if(pDevEvent->event.keyboard.code >= 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> Keyboard Code(%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.code);
        goto fail;
    }



    scancode = st_CodeMapDik[pDevEvent->event.keyboard.code];
    if(scancode == DIK_NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> Keyboard Code(%d) => DIK_NULL code\n",pDevEvent,pDevEvent->event.keyboard.code);
        goto fail;
    }

    /*now format data*/
    ZeroMemory(&data,sizeof(data));
    data.dwOfs = scancode;
    if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN)
    {
        data.dwData = 0x80;
    }
    else if(pDevEvent->event.keyboard.event == KEYBOARD_EVENT_UP)
    {
        data.dwData = 0x0;
    }

    data.dwTimeStamp = GetTickCount();
    /*sequence we do not need any more uAppData we do not any more*/
    ret = __InsertKeyboardDinputData(&data,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    SetLastError(0);
    return 0;
fail:
    SetLastError(ret);
    return -ret;
}

int __Dinput8InsertMouseEvent(LPDEVICEEVENT pDevEvent)
{
    int ret;
    DIDEVICEOBJECTDATA data[4];
    int num=0,idx=0;
    POINT pt;
    int origstate=MOUSE_NOT_SET_STATE;

    if(pDevEvent->event.mouse.event >= MOUSE_EVENT_MAX)
    {
        ret = ERROR_INVALID_PARAMETER;
        goto fail;
    }

    ZeroMemory(&data,sizeof(data));
    /*we do not used this now get it and make it ok*/
    ret = BaseGetMousePointAbsolution(&pt);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Get Mouse Point Absolution Error(%d)\n",ret);
        goto fail;
    }

    switch(pDevEvent->event.mouse.event)
    {
    case MOUSE_EVENT_KEYDOWN:
        if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON0;
        }
        else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON1;
        }
        else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON2;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("Mouse Event KEYDOWN code(%d) Error\n",pDevEvent->event.mouse.code);
            goto fail;
        }
        data[num].dwData = 0x80;
        data[num].dwTimeStamp = GetTickCount();
        num ++;
        break;
    case MOUSE_EVENT_KEYUP:
        if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON0;
        }
        else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON1;
        }
        else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
        {
            data[num].dwOfs = DIMOFS_BUTTON2;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("Mouse Event KEYUP code(%d) Error\n",pDevEvent->event.mouse.code);
            goto fail;
        }
        data[num].dwData = 0x00;
        data[num].dwTimeStamp = GetTickCount();
        num ++;
        break;
    case MOUSE_EVNET_MOVING:
        EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
        if(pDevEvent->event.mouse.x != 0)
        {
            if((st_PrevDiMousePoint.x + pDevEvent->event.mouse.x) != pt.x)
            {
                ERROR_INFO("Mouse x diff(%d + %d) != %d\n",
                           st_PrevDiMousePoint.x ,pDevEvent->event.mouse.x,pt.x);
            }
            data[num].dwOfs = DIMOFS_X;
            data[num].dwData = (pt.x - st_PrevDiMousePoint.x);
            data[num].dwTimeStamp = GetTickCount();
            num ++;
        }
        if(pDevEvent->event.mouse.y != 0)
        {
            if((st_PrevDiMousePoint.y + pDevEvent->event.mouse.y) != pt.y)
            {
                ERROR_INFO("Mouse y diff(%d + %d) != %d\n",
                           st_PrevDiMousePoint.y ,pDevEvent->event.mouse.y,pt.y);
            }
            data[num].dwOfs = DIMOFS_Y;
            data[num].dwData = (pt.y - st_PrevDiMousePoint.y);
            data[num].dwTimeStamp = GetTickCount();
            num ++;
        }
        st_PrevDiMousePoint.x = pt.x;
        st_PrevDiMousePoint.y = pt.y;
        LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
        break;
    case MOUSE_EVENT_SLIDE:
        data[num].dwOfs = DIMOFS_Z;
        data[num].dwData = pDevEvent->event.mouse.x;
        data[num].dwTimeStamp = GetTickCount();
        num ++;
        break;
    case MOUSE_EVENT_ABS_MOVING:
        EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
        if(pDevEvent->event.mouse.x == IO_MOUSE_RESET_X && pDevEvent->event.mouse.y == IO_MOUSE_RESET_Y)
        {
            /*it is reset mouse ,so we should do this moving move to the most top-left point*/
            data[num].dwOfs = DIMOFS_X;
            data[num].dwData = IO_MOUSE_RESET_X_MOV;
            data[num].dwTimeStamp = GetTickCount();
            num ++;

            data[num].dwOfs = DIMOFS_Y;
            data[num].dwData = IO_MOUSE_RESET_Y_MOV;
            data[num].dwTimeStamp = GetTickCount();
            num ++;

            /*now move back for it points*/
            data[num].dwOfs = DIMOFS_X;
            data[num].dwData = pt.x;
            data[num].dwTimeStamp = GetTickCount() +1;
            num ++;

            data[num].dwOfs = DIMOFS_Y;
            data[num].dwData = pt.y;
            data[num].dwTimeStamp = GetTickCount() + 1;
            num ++;
            origstate = st_MouseGetState;
            st_MouseGetState = MOUSE_RESET_MOST_LEFTTOP;
        }
        else
        {
            if(st_PrevDiMousePoint.x != pt.x)
            {
                data[num].dwOfs = DIMOFS_X;
                data[num].dwData = (st_PrevDiMousePoint.x - pt.x);
                data[num].dwTimeStamp = GetTickCount();
                num ++;
            }
            if(st_PrevDiMousePoint.y != pt.y)
            {
                data[num].dwOfs = DIMOFS_Y;
                data[num].dwData = (st_PrevDiMousePoint.y - pt.y);
                data[num].dwTimeStamp = GetTickCount();
                num ++;
            }
        }
        st_PrevDiMousePoint.x = pt.x;
        st_PrevDiMousePoint.y = pt.y;
        LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
        break;
    default:
        assert(0!=0);
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("event(%d) really error\n",pDevEvent->event.mouse.event);
        goto fail;
    }

    if(num == 0)
    {
        /*yes nothing to handle ,so just return*/
        goto succ;
    }

    /*num equals 0*/
    if(num <= 2)
    {
        ret = __InsertMouseDinputData(data,num,idx,1);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            goto fail;
        }
    }
    else
    {
        /*this is reset use*/
        assert(num == 4);
        assert(pDevEvent->event.mouse.event == MOUSE_EVENT_ABS_MOVING);
        assert(pDevEvent->event.mouse.x == IO_MOUSE_RESET_X);
        assert(pDevEvent->event.mouse.y == IO_MOUSE_RESET_Y);

        /*now first to insert mouse data for the data of input*/
        ret = __InsertMouseDinputData(data,2,0,1);
        if(ret < 0)
        {
            ret = GETERRNO();
            goto fail;
        }

        ret = __InsertMouseDinputData(&(data[2]),2,0,1);
        assert(ret >= 0);
    }

succ:
    SetLastError(0);
    return 0;
fail:
    EnterCriticalSection(&st_Dinput8KeyMouseStateCS);
    if(origstate != MOUSE_NOT_SET_STATE)
    {
        st_MouseGetState = origstate;
    }
    LeaveCriticalSection(&st_Dinput8KeyMouseStateCS);
    SetLastError(ret);
    return -ret;
}


int Dinput8EventHandler(LPVOID pParam,LPVOID pInput)
{
    LPDEVICEEVENT pDevEvent = (LPDEVICEEVENT)pInput;
    int ret;

    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        return __Dinput8InsertKeyboardEvent(pDevEvent);
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        return __Dinput8InsertMouseEvent(pDevEvent);
    }

    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("<0x%p> devtype(%d) not supported\n",pDevEvent,pDevEvent->devtype);
    SetLastError(ret);
    return -ret;
}


void DetourDinputEmulationFini(HMODULE hModule)
{
    UnRegisterEventListHandler(Dinput8EventHandler);
    UnRegisterEventListInit(DetourDinput8Init);
    return ;
}

int DetourDinputEmulationInit(HMODULE hModule)
{
    int ret;
    InitializeCriticalSection(&st_Dinput8DeviceCS);
    InitializeCriticalSection(&st_Dinput8KeyMouseStateCS);

    ret = RegisterEventListInit(DetourDinput8Init,NULL,DINPUT_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not register Eventlist Init Error(%d)\n",ret);
        goto fail;
    }

    ret = RegisterEventListHandler(Dinput8EventHandler,NULL,DINPUT_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not register EventList Handler Error(%d)\n",ret);
        goto fail;
    }


    SetLastError(0);
    return 0;
fail:
    assert(ret > 0);
    UnRegisterEventListHandler(Dinput8EventHandler);
    UnRegisterEventListInit(DetourDinput8Init);
    SetLastError(ret);
    return FALSE;
}


