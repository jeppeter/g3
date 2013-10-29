
/*****************************************
*  this file is to detour the functions of directinput8
*  this is the most common input for directx
*****************************************/
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))
#define COM_METHOD(TYPE, METHOD) TYPE STDMETHODCALLTYPE METHOD



/*****************************************
*  to make the IDirectInputDevice8A hook
*
*****************************************/
class CDirectInputDevice8AHook;

static std::vector<IDirectInputDevice8A*> st_DIDevice8AVecs;
static std::vector<CDirectInputDevice8AHook*> st_CDIDevice8AHookVecs;
static CRITICAL_SECTION st_DIDevice8ACS;

#define EQUAL_DI_DEVICE_8A_VECS() \
do\
{\
	assert(st_DIDevice8AVecs.size() == st_CDIDevice8AHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInputDevice8AHook(IDirectInputDevice8A* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DIDevice8ACS);
    EQUAL_DI_DEVICE_8A_VECS();
    for(i=0; i<st_DIDevice8AVecs.size() ; i++)
    {
        if(st_DIDevice8AVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DIDevice8AVecs.erase(st_DIDevice8AVecs.begin()+findidx);
        st_CDIDevice8AHookVecs.erase(st_CDIDevice8AHookVecs.begin() + findidx);
    }
    LeaveCriticalSection(&st_DIDevice8ACS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}


#define  DIRECT_INPUT_DEVICE_8A_IN()
#define  DIRECT_INPUT_DEVICE_8A_OUT()


class CDirectInputDevice8AHook : public IDirectInputDevice8A
{
private:
    IDirectInputDevice8A* m_ptr;
public:
    CDirectInputDevice8AHook(IDirectInputDevice8A* ptr) : m_ptr(ptr) {};
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
            uret = UnRegisterDirectInput8AHook(this->m_ptr);
            if(uret == 0)
            {
                delete this;
            }
        }
        return uret;
    }


};


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
    LeaveCriticalSection(&st_DI8ACS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}

#define  DIRECT_INPUT_8A_IN()
#define  DIRECT_INPUT_8A_OUT()
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
                                     LPDIRECTINPUTDEVICE * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACK lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
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


    COM_METHOD(HRESULT,FindDevice)(THIS_ REFGUID rguidClass,LPCTSTR ptszName,LPGUID pguidInstance)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->FindDevice(rguidClass,ptszName,pguidInstance);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevicesBySemantics)(THIS_ LPCTSTR ptszUserName,LPDIACTIONFORMAT lpdiActionFormat,LPDIENUMDEVICESBYSEMANTICSCB lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8A_IN();
        hr = m_ptr->EnumDevicesBySemantics(ptszUserName,lpdiActionFormat,lpCallback,pvRef,dwFlags);
        DIRECT_INPUT_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMS lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
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
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
        st_DI8AVecs.push_back(ptr);
        st_CDI8AHookVecs.push_back(pHook);
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
    LeaveCriticalSection(&st_DI8WCS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}



#define  DIRECT_INPUT_8W_IN()
#define  DIRECT_INPUT_8W_OUT()

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
                                     LPDIRECTINPUTDEVICE * lplpDirectInputDevice,
                                     LPUNKNOWN pUnkOuter)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->CreateDevice(rguid,lplpDirectInputDevice,pUnkOuter);
        DIRECT_INPUT_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumDevices)(THIS_ DWORD dwDevType,LPDIENUMDEVICESCALLBACK lpCallback,LPVOID pvRef,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_8W_IN();
        hr = m_ptr->EnumDevices(dwDevType,lpCallback,pvRef,dwFlags);
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

    COM_METHOD(HRESULT,ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,LPDICONFIGUREDEVICESPARAMS lpdiCDParams,DWORD dwFlags,LPVOID pvRefData)
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
    CDirectInput8WHook* pHook=NULL;

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
        pHook =new CDirectInput8WHook(ptr);
        /*to add reference to control the delete procedure*/
        ptr->AddRef();
        st_DI8WVecs.push_back(ptr);
        st_CDI8WHookVecs.push_back(pHook);
    }
    else
    {
        pHook = st_CDI8WHookVecs[findidx];
    }
    LeaveCriticalSection(&st_DI8WCS);
    return pHook;
}



/*****************************************
*   to make the base directinput 8 root functions
*
*****************************************/
typedef HRESULT(WINAPI *DirectInput8Create_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
DirectInput8Create_t DirectInput8CreateNext = DirectInput8Create;

HRESULT WINAPI DirectInput8CreateCallBack(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    HRESULT hr;

    hr = DirectInput8CreateNext(hinst,dwVersion,riidltf,ppvOut,punkOuter);
    if(hr == DI_OK)
    {
        /*now successful ,so we should do this for the out*/
        DEBUG_INFO("riidltf %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                   riidltf.Data1,
                   riidltf.Data2,
                   riidltf.Data3,
                   riidltf.Data4[0],
                   riidltf.Data4[1],
                   riidltf.Data4[2],
                   riidltf.Data4[3],
                   riidltf.Data4[4],
                   riidltf.Data4[5],
                   riidltf.Data4[6],
                   riidltf.Data4[7]);
        DEBUG_INFO("dwVersion is 0x%08x\n",dwVersion);
        if(ppvOut)
        {
            DEBUG_INFO("*ppvOut 0x%p\n",*ppvOut);
        }
        if(punkOuter)
        {
            DEBUG_INFO("*punkOuter 0x%p\n",*punkOuter);
        }

        if(riidltf == IID_IDirectInput8A)
        {
            CDirectInput8AHook* pHookA = NULL;
            IDirectInput8A* pPtrA = (IDirectInput8A*)*ppvOut;
            pHookA = RegisterDirectInput8AHook(pPtrA);
            assert(pHookA);
            DEBUG_INFO("IDirectInput8A(0x%p) =>CDirectInput8AHook(0x%p)\n",pPtrA,pHookA);
            *ppvOut = pHookA;
        }
        else if(riidltf == IID_IDirectInput8W)
        {
            CDirectInput8WHook *pHookW = NULL;
            IDirectInput8W* pPtrW = (IDirectInput8W*) *ppvOut;
            pHookW = RegisterDirectInput8WHook(pPtrW);
            assert(pHookW);
            DEBUG_INFO("IDirectInput8W(0x%p) => CDirectInput8AHook(0x%p)\n",pPtrW,pHookW);
            *ppvOut = pHookW;
        }
    }
    return hr;
}


