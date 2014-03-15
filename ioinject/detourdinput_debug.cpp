
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
    else
    {
        ERROR_INFO("could not find 0x%p DirectInputDevice8A\n",ptr);
    }
    LeaveCriticalSection(&st_DIDevice8ACS);


    uret = 1;
    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}

ULONG UnRegisterJoyConfig(CDirectInputJoyConfig8Hook* pHook)
{
    ULONG uret=1;

    return uret;
}


#define  DINPUT_JOYCONFIG8_IN()
#define  DINPUT_JOYCONFIG8_OUT()

class CDirectInputJoyConfig8Hook : public IDirectInputJoyConfig8
{
private:
    IDirectInputJoyConfig8 *m_ptr;
    GUID m_guid;
    LPDIJOYTYPECALLBACK m_pEnumFunc;
    LPVOID m_pEnumVoid;
private:

    BOOL __DIEnumJoyTypeProcImpl(LPCWSTR pwszTypeName)
    {
        BOOL bret;
        if(this->m_pEnumFunc)
        {
            bret = this->m_pEnumFunc(pwszTypeName,this->m_pEnumVoid);
            return bret;
        }
        return DIENUM_CONTINUE,;
    }

    static BOOL DIEnumJoyTypeProc(LPCWSTR pwszTypeName, LPVOID pvRef)
    {
        CDirectInputJoyConfig8Hook *pThis=(CDirectInputJoyConfig8Hook*)pvRef;
        return pThis->__DIEnumJoyTypeProcImpl(pwszTypeName);
    }

public:
    CDirectInputJoyConfig8Hook(IDirectInputJoyConfig8 *ptr,REFGUID guid)
    {
        m_ptr = ptr;
        m_guid = guid;
    }

    ~CDirectInputJoyConfig8Hook()
    {
        m_ptr = NULL;
        m_guid = GUID_NULL;
    }

    COM_METHOD(HRESULT,QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->QueryInterface(riid,ppvObj);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(ULONG,AddRef)(THIS)
    {
        ULONG uret;
        DINPUT_JOYCONFIG8_IN();
        uret = this->m_ptr->AddRef();
        DINPUT_JOYCONFIG8_OUT();
        return uret;
    }

    COM_METHOD(ULONG,Release)(THIS)
    {
        ULONG uret;
        DINPUT_JOYCONFIG8_IN();
        uret = this->m_ptr->Release();
        if(uret == 1)
        {
            uret = UnRegisterJoyConfig(this);
            if(uret == 0)
            {
                delete this;
            }
        }

        DINPUT_JOYCONFIG8_OUT();
        return uret;
    }

    COM_METHOD(HRESULT,Acquire)(THIS)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->Acquire();
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }


    COM_METHOD(HRESULT,Unacquire)(THIS)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->Unacquire();
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetCooperativeLevel)(THIS_ HWND hwnd,DWORD level)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->SetCooperativeLevel(hwnd,level);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SendNotify)(THIS)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->SendNotify();
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,EnumTypes)(THIS_ LPDIJOYTYPECALLBACK lpCallback,LPVOID pvRef)
    {
        HRESULT hr;

        DINPUT_JOYCONFIG8_IN();
        this->m_pEnumFunc = lpCallback;
        this->m_pEnumVoid = pvRef;
        hr = this->m_ptr->EnumTypes(CDirectInputJoyConfig8Hook::DIEnumJoyTypeProc,this);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetTypeInfo)(LPCWSTR pwszTypeName,LPDIJOYTYPEINFO pjti,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->GetTypeInfo(pwszTypeName,pjti,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetTypeInfo)(LPCWSTR pwszTypeName,LPDIJOYTYPEINFO pjti,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->SetTypeInfo(pwszTypeName,pjti,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,DeleteType)(THIS_ LPCWSTR pwszTypeName)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->DeleteType(pwszTypeName);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetConfig)(THIS_ UINT uiJoy,LPDIJOYCONFIG pjc,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->GetConfig(uiJoy,pjc,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetConfig)(THIS_ UINT uiJoy,LPDIJOYCONFIG pjc,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->SetConfig(uiJoy,pjc,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,DeleteConfig)(THIS_ UINT uiJoy)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->DeleteConfig(uiJoy);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }
    COM_METHOD(HRESULT,GetUserValues)(THIS_ LPDIJOYUSERVALUES pjuv,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->GetUserValues(pjuv,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,SetUserValues)(THIS_ LPCDIJOYUSERVALUES pjuv,DWORD dwFlags)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->SetUserValues(pjuv,dwFlags);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,AddNewHardware)(THIS_ HWND hwndOwner,REFGUID rguidClass)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->AddNewHardware(hwndOwner,rguidClass);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,OpenTypeKey)(THIS_ LPCWSTR pwszType,REGSAM regsam,PHKEY phk)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->OpenTypeKey(pwszType,regsam,phk);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,OpenAppStatusKey(THIS_ PHKEY phKey)
    {
        HRESULT hr;
        DINPUT_JOYCONFIG8_IN();
        hr = this->m_ptr->OpenAppStatusKey(phKey);
        DINPUT_JOYCONFIG8_OUT();
        return hr;
    }


}

#define  DIRECT_INPUT_DEVICE_8A_IN()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p in\n",__FUNCTION__,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8A_OUT()  do{DINPUT_DEBUG_INFO("Device8A::%s 0x%p out\n",__FUNCTION__,this->m_ptr);}while(0)


class CDirectInputDevice8AHook : public IDirectInputDevice8A
{
private:
    IDirectInputDevice8A* m_ptr;
    IID m_iid;
public:
    CDirectInputDevice8AHook(IDirectInputDevice8A* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
    };
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
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
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
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
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
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetDeviceState(cbData,lpvData);
        if(SUCCEEDED(hr))
        {
            DEBUG_BUFFER_FMT(lpvData,cbData,"<0x%p>(0x%p) GetData:",this,this->m_ptr);
        }
        DIRECT_INPUT_DEVICE_8A_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8A_IN();
        hr = m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        if(SUCCEEDED(hr) && rgdod)
        {
            DEBUG_BUFFER_FMT(rgdod,cbObjectData*(*pdwInOut),"<0x%p>(0x%p) dwFlags 0x%08x objdata 0x%08x inout (%d):",
                             this,this->m_ptr,dwFlags,cbObjectData,*pdwInOut);
        }
        else if(SUCCEEDED(hr))
        {
            DEBUG_INFO("<0x%p>(0x%p) rgdod NULL dwFlags 0x%08x inout %d\n",this,this->m_ptr,dwFlags,*pdwInOut);
        }
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
        DEBUG_INFO("<0x%p>(0x%p) Poll result(0x%08x)\n",this,this->m_ptr,hr);
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
        pHookA = st_CDIDevice8AHookVecs[findidx];
    }
    else
    {
        pHookA = new CDirectInputDevice8AHook(ptr,riid);
        st_CDIDevice8AHookVecs.push_back(pHookA);
        st_DIDevice8AVecs.push_back(ptr);

        /*to add reference ,it will give release ok*/
        ptr->AddRef();
    }

    LeaveCriticalSection(&st_DIDevice8ACS);

    return pHookA;
}



/*****************************************
*  to make the IDirectInputDevice8W hook
*
*****************************************/
class CDirectInputDevice8WHook;

static std::vector<IDirectInputDevice8W*> st_DIDevice8WVecs;
static std::vector<CDirectInputDevice8WHook*> st_CDIDevice8WHookVecs;
static CRITICAL_SECTION st_DIDevice8WCS;

#define EQUAL_DI_DEVICE_8W_VECS() \
do\
{\
	assert(st_DIDevice8WVecs.size() == st_CDIDevice8WHookVecs.size());\
}while(0)

ULONG UnRegisterDirectInputDevice8WHook(IDirectInputDevice8W* ptr)
{
    int findidx = -1;
    ULONG uret=1;
    unsigned int i;

    EnterCriticalSection(&st_DIDevice8WCS);
    EQUAL_DI_DEVICE_8W_VECS();
    for(i=0; i<st_DIDevice8WVecs.size() ; i++)
    {
        if(st_DIDevice8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        st_DIDevice8WVecs.erase(st_DIDevice8WVecs.begin()+findidx);
        st_CDIDevice8WHookVecs.erase(st_CDIDevice8WHookVecs.begin() + findidx);
    }
    else
    {
        ERROR_INFO("could not find 0x%p DirectInputDevice8W\n",ptr);
    }
    LeaveCriticalSection(&st_DIDevice8WCS);

    if(findidx >= 0)
    {
        uret = ptr->Release();
    }
    return uret;
}


#define  DIRECT_INPUT_DEVICE_8W_IN()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p->0x%p in\n",__FUNCTION__,this,this->m_ptr);}while(0)
#define  DIRECT_INPUT_DEVICE_8W_OUT()  do{DINPUT_DEBUG_INFO("Device8W::%s 0x%p->0x%p out\n",__FUNCTION__,this,this->m_ptr);}while(0)


class CDirectInputDevice8WHook : public IDirectInputDevice8W
{
private:
    IDirectInputDevice8W* m_ptr;
    IID m_iid;
public:
    CDirectInputDevice8WHook(IDirectInputDevice8W* ptr,REFIID riid) : m_ptr(ptr)
    {
        m_iid = riid;
    };
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
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
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
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->SetProperty(rguidProp,pdiph);
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
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceState(cbData,lpvData);
        if(SUCCEEDED(hr))
        {
            DEBUG_BUFFER_FMT(lpvData,cbData,"<0x%p>(0x%p) GetData:",this,this->m_ptr);
        }
        if(hr == DI_OK)
        {
            if(this->m_iid == GUID_SysMouse)
            {
#if 0
                DIMOUSESTATE* pMouseState = (DIMOUSESTATE*)lpvData;
                DINPUT_DEBUG_INFO("<0x%p> SysMouse data\n",this->m_ptr);
                if(cbData >= sizeof(*pMouseState))
                {
                    DINPUT_DEBUG_INFO("lx %ld ly %ld lz %ld rgbbutton[0] 0x%02x rgbbutton[1] 0x%02x rgbbutton[2] 0x%02x rgbbutton[3] 0x%02x\n",
                                      pMouseState->lX,
                                      pMouseState->lY,
                                      pMouseState->lZ,
                                      pMouseState->rgbButtons[0],
                                      pMouseState->rgbButtons[1],
                                      pMouseState->rgbButtons[2],
                                      pMouseState->rgbButtons[3]);
                }
                DINPUT_DEBUG_BUFFER_FMT(lpvData,cbData,NULL);
#endif
            }
            else if(this->m_iid == GUID_SysKeyboard)
            {
                DINPUT_DEBUG_BUFFER_FMT(lpvData,cbData,"<0x%p> SysKeyboard data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_Joystick)
            {
                DINPUT_DEBUG_INFO("<0x%p> Joystick data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysMouseEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysMouseEm2 data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm data\n",this->m_ptr);
            }
            else if(this->m_iid == GUID_SysKeyboardEm2)
            {
                DINPUT_DEBUG_INFO("<0x%p> SysKeyboardEm2 data\n",this->m_ptr);
            }
        }
        else
        {
            DINPUT_DEBUG_INFO("<0x%p> size(0x%08x) return 0x%08x\n",this->m_ptr,cbData,hr);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
        return hr;
    }

    COM_METHOD(HRESULT,GetDeviceData)(THIS_ DWORD cbObjectData,LPDIDEVICEOBJECTDATA rgdod,LPDWORD pdwInOut,DWORD dwFlags)
    {
        HRESULT hr;
        DIRECT_INPUT_DEVICE_8W_IN();
        hr = m_ptr->GetDeviceData(cbObjectData,rgdod,pdwInOut,dwFlags);
        if(SUCCEEDED(hr) && rgdod)
        {
            DEBUG_BUFFER_FMT(rgdod,cbObjectData*(*pdwInOut),"<0x%p>(0x%p) dwFlags 0x%08x objdata 0x%08x inout (%d):",
                             this,this->m_ptr,dwFlags,cbObjectData,*pdwInOut);
        }
        else if(SUCCEEDED(hr))
        {
            DEBUG_INFO("<0x%p>(0x%p) rgdod NULL dwFlags 0x%08x inout %d\n",this,this->m_ptr,dwFlags,*pdwInOut);
        }
        DIRECT_INPUT_DEVICE_8W_OUT();
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
        DEBUG_INFO("<0x%p>(0x%p) Poll result(0x%08x)\n",this,this->m_ptr,hr);
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

    EnterCriticalSection(&st_DIDevice8WCS);
    EQUAL_DI_DEVICE_8W_VECS();
    for(i=0; i<st_DIDevice8WVecs.size() ; i++)
    {
        if(st_DIDevice8WVecs[i] == ptr)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        pHookW = st_CDIDevice8WHookVecs[findidx];
    }
    else
    {
        pHookW = new CDirectInputDevice8WHook(ptr,riid);
        st_CDIDevice8WHookVecs.push_back(pHookW);
        st_DIDevice8WVecs.push_back(ptr);
        /*to add reference ,it will give release ok*/
        ptr->AddRef();
    }

    LeaveCriticalSection(&st_DIDevice8WCS);

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
    void* m_pCallParam;
    LPDIENUMDEVICESCALLBACKA m_pEnumCallBack;
public:
    CDirectInput8AHook(IDirectInput8A* ptr):m_ptr(ptr)
    {
        m_pCallParam = NULL;
        m_pEnumCallBack=NULL;
    };
private:
    static BOOL FAR PASCAL HookDIEnumDevicesCallback(LPCDIDEVICEINSTANCEA lpddi,LPVOID pvRef)
    {
        CDirectInput8AHook* pThis = (CDirectInput8AHook*)pvRef;
        BOOL bret=DIENUM_CONTINUE;
        if(pThis->m_pEnumCallBack)
        {
            bret = pThis->m_pEnumCallBack(lpddi,pThis->m_pCallParam);
            DEBUG_BUFFER_FMT(lpddi,sizeof(*lpddi),"dwSize %d bret %d",lpddi->dwSize,bret);
            DEBUG_INFO("guidInstance 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidInstance.Data1,
                       lpddi->guidInstance.Data2,
                       lpddi->guidInstance.Data3,
                       lpddi->guidInstance.Data4[0],
                       lpddi->guidInstance.Data4[1],
                       lpddi->guidInstance.Data4[2],
                       lpddi->guidInstance.Data4[3],
                       lpddi->guidInstance.Data4[4],
                       lpddi->guidInstance.Data4[5],
                       lpddi->guidInstance.Data4[6],
                       lpddi->guidInstance.Data4[7]);
            DEBUG_INFO("guidProduct 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidProduct.Data1,
                       lpddi->guidProduct.Data2,
                       lpddi->guidProduct.Data3,
                       lpddi->guidProduct.Data4[0],
                       lpddi->guidProduct.Data4[1],
                       lpddi->guidProduct.Data4[2],
                       lpddi->guidProduct.Data4[3],
                       lpddi->guidProduct.Data4[4],
                       lpddi->guidProduct.Data4[5],
                       lpddi->guidProduct.Data4[6],
                       lpddi->guidProduct.Data4[7]);
            DEBUG_INFO("dwDevType 0x%08x\n",lpddi->dwDevType);
            DEBUG_INFO("tszInstanceName %s tszProductName %s\n",lpddi->tszInstanceName,lpddi->tszProductName);
            DEBUG_INFO("guidFFDriver 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidFFDriver.Data1,
                       lpddi->guidFFDriver.Data2,
                       lpddi->guidFFDriver.Data3,
                       lpddi->guidFFDriver.Data4[0],
                       lpddi->guidFFDriver.Data4[1],
                       lpddi->guidFFDriver.Data4[2],
                       lpddi->guidFFDriver.Data4[3],
                       lpddi->guidFFDriver.Data4[4],
                       lpddi->guidFFDriver.Data4[5],
                       lpddi->guidFFDriver.Data4[6],
                       lpddi->guidFFDriver.Data4[7]);
            DEBUG_INFO("wUsagePage 0x%04x wUsage 0x%04x\n",lpddi->wUsagePage,lpddi->wUsage);
        }
        return bret;
    };
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
        this->m_pCallParam = pvRef;
        this->m_pEnumCallBack = lpCallback;
        if(this->m_pEnumCallBack)
        {
            hr = m_ptr->EnumDevices(dwDevType,CDirectInput8AHook::HookDIEnumDevicesCallback,this,dwFlags);
        }
        else
        {
            hr = m_ptr->EnumDevices(dwDevType,NULL,pvRef,dwFlags);
        }
        DEBUG_INFO("EnumDevices dwDevType 0x%08x dwFlags 0x%08x this->m_pEnumCallBack 0x%p hr(0x%08x)\n",dwDevType,dwFlags,this->m_pEnumCallBack,hr);
        this->m_pCallParam = NULL;
        this->m_pEnumCallBack = NULL;
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
    LPVOID m_pCallParam;
    LPDIENUMDEVICESCALLBACKW m_pEnumCallBack;
public:
    CDirectInput8WHook(IDirectInput8W* ptr) : m_ptr(ptr)
    {
        m_pCallParam= NULL;
        m_pEnumCallBack = NULL;
    };
    static BOOL FAR PASCAL HookDIEnumDevicesCallback(LPCDIDEVICEINSTANCEW lpddi,LPVOID pvRef)
    {
        CDirectInput8WHook* pThis = (CDirectInput8WHook*)pvRef;
        BOOL bret=DIENUM_CONTINUE;
        if(pThis->m_pEnumCallBack)
        {
            bret = pThis->m_pEnumCallBack(lpddi,pThis->m_pCallParam);
            DEBUG_BUFFER_FMT(lpddi,sizeof(*lpddi),"dwSize %d bret %d",lpddi->dwSize,bret);
            DEBUG_INFO("guidInstance 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidInstance.Data1,
                       lpddi->guidInstance.Data2,
                       lpddi->guidInstance.Data3,
                       lpddi->guidInstance.Data4[0],
                       lpddi->guidInstance.Data4[1],
                       lpddi->guidInstance.Data4[2],
                       lpddi->guidInstance.Data4[3],
                       lpddi->guidInstance.Data4[4],
                       lpddi->guidInstance.Data4[5],
                       lpddi->guidInstance.Data4[6],
                       lpddi->guidInstance.Data4[7]);
            DEBUG_INFO("guidProduct 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidProduct.Data1,
                       lpddi->guidProduct.Data2,
                       lpddi->guidProduct.Data3,
                       lpddi->guidProduct.Data4[0],
                       lpddi->guidProduct.Data4[1],
                       lpddi->guidProduct.Data4[2],
                       lpddi->guidProduct.Data4[3],
                       lpddi->guidProduct.Data4[4],
                       lpddi->guidProduct.Data4[5],
                       lpddi->guidProduct.Data4[6],
                       lpddi->guidProduct.Data4[7]);
            DEBUG_INFO("dwDevType 0x%08x\n",lpddi->dwDevType);
            DEBUG_INFO("tszInstanceName %S tszProductName %S\n",lpddi->tszInstanceName,lpddi->tszProductName);
            DEBUG_INFO("guidFFDriver 0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                       lpddi->guidFFDriver.Data1,
                       lpddi->guidFFDriver.Data2,
                       lpddi->guidFFDriver.Data3,
                       lpddi->guidFFDriver.Data4[0],
                       lpddi->guidFFDriver.Data4[1],
                       lpddi->guidFFDriver.Data4[2],
                       lpddi->guidFFDriver.Data4[3],
                       lpddi->guidFFDriver.Data4[4],
                       lpddi->guidFFDriver.Data4[5],
                       lpddi->guidFFDriver.Data4[6],
                       lpddi->guidFFDriver.Data4[7]);
            DEBUG_INFO("wUsagePage 0x%04x wUsage 0x%04x\n",lpddi->wUsagePage,lpddi->wUsage);
        }
        return bret;
    };

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
        DINPUT_DEBUG_INFO("uret = %d\n",uret);
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
        this->m_pCallParam = pvRef;
        this->m_pEnumCallBack = lpCallback;
        if(this->m_pEnumCallBack)
        {
            hr = m_ptr->EnumDevices(dwDevType,CDirectInput8WHook::HookDIEnumDevicesCallback,this,dwFlags);
        }
        else
        {
            hr = m_ptr->EnumDevices(dwDevType,NULL,pvRef,dwFlags);
        }
        DEBUG_INFO("EnumDevices dwDevType 0x%08x dwFlags 0x%08x this->m_pEnumCallBack 0x%p hr(0x%08x)\n",dwDevType,dwFlags,this->m_pEnumCallBack,hr);
        this->m_pCallParam = NULL;
        this->m_pEnumCallBack = NULL;
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

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return -ERROR_NOT_SUPPORTED;
}

int DetourDinputPressKeyDown(UINT scancode)
{
    return 0;
}

int DetourDinputClientMousePoint(POINT* pPoint)
{
    pPoint->x = 0;
    pPoint->y = 0;
    return 0;
}

int DetourDinputMouseBtnDown(UINT btn)
{
    return 0;
}

int DetourDinputSetWindowsRect(HWND hWnd,RECT *pRect)
{
    return 0;
}

int DetourDinput8GetMousePointAbsolution(POINT *pPoint)
{
    pPoint->x = 0;
    pPoint->y = 0;
    return 0;
}


int DetourDinputPressKeyDownTimes(UINT scancode)
{
    return 0;
}

int DetourDinputScreenMousePoint(HWND hwnd,POINT* pPoint)
{
    pPoint->x = 1;
    pPoint->y = 1;
    return 0;
}

void DetourDinputDebugFini(HMODULE hModule)
{
    return;
}

int DetourDinputDebugInit(HMODULE hModule)
{
    /*now first to init all the critical section*/
    InitializeCriticalSection(&st_DIDevice8ACS);
    InitializeCriticalSection(&st_DIDevice8WCS);
    return 0;
}

int DetourDinputSetWindowsRect(HWND hWnd)
{
    return 0;
}



