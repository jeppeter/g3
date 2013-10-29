
/*****************************************
*  this file is to detour the functions of directinput8
*  this is the most common input for directx
*****************************************/
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))
#define COM_METHOD(TYPE, METHOD) TYPE STDMETHODCALLTYPE METHOD



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


};


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
    }
    return hr;
}


