/*************************************************************************

x509\routin.cpp

	-by Miles Chen (stainboyx@hotmail.com) 2009-1-29

*************************************************************************/
#include <Windows.h>
#include <vector>
#include <assert.h>
#include <TlHelp32.h>
#include <winbase.h>
#include <d3d9.h>
#include <output_debug.h>
#include <d3dx9tex.h>
#include <detours/detours.h>
#include <injectbase.h>
#include <imgcapcommon.h>
#include "routine.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavutil/pixfmt.h"
#ifdef __cplusplus
}
#endif

#define  ENDSCENE_HOOK_ENABLED  1

#define COM_METHOD(TYPE, METHOD) TYPE STDMETHODCALLTYPE METHOD
#define LAST_ERROR_RETURN()  (GetLastError() ? GetLastError() : 1)

#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"D3dx9.lib")

#ifdef _DEBUG
#pragma comment(lib,"injectbased.lib")
#else
#pragma comment(lib,"injectbase.lib")
#endif

extern "C" int RoutineDetourD11(void);
extern "C" void RotineClearD11(void);


#define  POINTER_STATE_GRAB        1
#define  POINTER_STATE_FREE        0
#define  POINTER_STATE_HOLD        -1
#define  POINTER_STATE_UNCERNTAIN  -2



static std::vector<IDirect3DDevice9*> st_DirectShowPointers;
static std::vector<int> st_DirectShowPointerState;
static CRITICAL_SECTION st_PointerCS;
static IDirect3D9* (WINAPI* Direct3DCreate9Next)(UINT SDKVersion)
    = Direct3DCreate9;

#ifdef ENDSCENE_HOOK_ENABLED
static CRITICAL_SECTION st_BufferCS;
static imgcap_buffer_t* st_pHandleCapBuffer=NULL;
static int st_CapBufferHandlein=0;
static HANDLE st_hCapBufferEvent=NULL;
static int st_CapBufferInited=0;
static int EndSceneCaptureBuffer(IDirect3DDevice9* pDevice);
static int RemoveImgBuffer(int force);
static imgcap_buffer_t* GetImgBuffer();
#endif /*ENDSCENE_HOOK_ENABLED*/

IDirect3DDevice9* GrapPointer(unsigned int& idx)
{
    IDirect3DDevice9* pPtr=NULL;
    int wait =0;
    BOOL bret;

    do
    {
        wait = 0;
        EnterCriticalSection(&st_PointerCS);
        assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
        if(st_DirectShowPointers.size()  > idx)
        {
            if(st_DirectShowPointerState[idx] == POINTER_STATE_FREE)
            {
                pPtr = st_DirectShowPointers[idx];
                st_DirectShowPointerState[idx] = POINTER_STATE_GRAB;
            }
            else
            {
                wait = 1;
            }
        }
        LeaveCriticalSection(&st_PointerCS);
        if(wait)
        {
            bret = SwitchToThread();
            if(!bret)
            {
                /*sleep for a while*/
                Sleep(10);
            }
        }
    }
    while(wait);

    return pPtr;
}

int FreePointer(IDirect3DDevice9* pPtr)
{
    int ret=0;
    int findidx;
    unsigned int i;
    int errorstate = POINTER_STATE_UNCERNTAIN;
    if(pPtr)
    {
        findidx = -1;
        EnterCriticalSection(&st_PointerCS);
        assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
        for(i=0; i<st_DirectShowPointers.size(); i++)
        {
            if(st_DirectShowPointers[i] == pPtr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            /*already for the free*/
            if(st_DirectShowPointerState[findidx] == POINTER_STATE_GRAB)
            {
                ret = 1;
                st_DirectShowPointerState[findidx] = POINTER_STATE_FREE;
            }
            else
            {
                errorstate = st_DirectShowPointerState[findidx];
            }
        }
        LeaveCriticalSection(&st_PointerCS);
    }

    if(errorstate != POINTER_STATE_UNCERNTAIN)
    {
        DEBUG_INFO("pointer 0x%p state %d\n",errorstate);
    }


    return ret;
}

BOOL HoldPointer(IDirect3DDevice9* pPtr)
{
    int wait;
    BOOL bret=FALSE;
    int findidx = -1;
    unsigned int i;
    do
    {
        wait = 0;
        findidx = -1;
        EnterCriticalSection(&st_PointerCS);
        assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
        for(i=0; i<st_DirectShowPointers.size() ; i++)
        {
            if(st_DirectShowPointers[i] == pPtr)
            {
                findidx = i;
                break;
            }
        }
        if(findidx >= 0)
        {
            if(st_DirectShowPointerState[findidx] == POINTER_STATE_FREE)
            {
                bret = TRUE;
                st_DirectShowPointerState[findidx] = POINTER_STATE_HOLD;
            }
            else
            {
                /*we should wait*/
                wait = 1;
            }
        }

        LeaveCriticalSection(&st_PointerCS);
        if(wait)
        {
            bret = SwitchToThread();
            if(!bret)
            {
                /*sleep for a while*/
                Sleep(10);
            }
        }
    }
    while(wait);

    return bret;
}

BOOL UnHoldPointer(IDirect3DDevice9* pPtr)
{
    BOOL bret=FALSE;
    int findidx = -1;
    unsigned int i;
    int errorstate=POINTER_STATE_UNCERNTAIN;
    EnterCriticalSection(&st_PointerCS);
    assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
    for(i=0; i<st_DirectShowPointers.size() ; i++)
    {
        if(st_DirectShowPointers[i] == pPtr)
        {
            findidx = i;
            break;
        }
    }
    if(findidx >= 0)
    {
        if(st_DirectShowPointerState[findidx] == POINTER_STATE_HOLD)
        {
            bret = TRUE;
            st_DirectShowPointerState[findidx] = POINTER_STATE_FREE;
        }
        else
        {
            errorstate = st_DirectShowPointerState[findidx];
        }
    }
    LeaveCriticalSection(&st_PointerCS);

    if(errorstate != POINTER_STATE_UNCERNTAIN)
    {
        DEBUG_INFO("pointer 0x%p not valid state %d\n",pPtr,errorstate);
    }

    return bret;
}

int UnRegisterPointer(IDirect3DDevice9* pPtr)
{
    int wait,releaseone;
    int ret;
    BOOL bret;
    int findidx;
    unsigned int i;
    do
    {
        wait = 0;
        releaseone = 0;
        findidx = -1;
        EnterCriticalSection(&st_PointerCS);
        assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
        for(i=0; i<st_DirectShowPointers.size(); i++)
        {
            if(st_DirectShowPointers[i] == pPtr)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            /*already for the free*/
            if(st_DirectShowPointerState[findidx] == POINTER_STATE_GRAB)
            {
                wait = 1;
            }
            else
            {
                /*we enter the vtbl function on hold pointer*/
                assert(st_DirectShowPointerState[findidx] == POINTER_STATE_HOLD);
                st_DirectShowPointers.erase(st_DirectShowPointers.begin() + findidx);
                st_DirectShowPointerState.erase(st_DirectShowPointerState.begin() + findidx);
                releaseone = 1;
            }

        }
        LeaveCriticalSection(&st_PointerCS);
        if(wait)
        {
            bret = SwitchToThread();
            if(!bret)
            {
                /*sleep for a while*/
                Sleep(10);
            }
        }
    }
    while(wait);

    /*release one we assume original is 1*/
    ret = 1;
    if(releaseone)
    {
        ret = pPtr->Release();
    }

    DEBUG_INFO("Unregister [0x%p] (release[%d]ret[%d])\n",pPtr,releaseone,ret);

    return ret ;
}

int RegisterPointer(IDirect3DDevice9* pPtr)
{
    int registered=0;
    unsigned int i;
    int findidx;
    int state;

    findidx = -1;

    EnterCriticalSection(&st_PointerCS);
    assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
    for(i=0; i<st_DirectShowPointers.size(); i++)
    {
        if(pPtr == st_DirectShowPointers[i])
        {
            findidx = i;
            break;
        }
    }

    if(findidx < 0)
    {
        registered = 1;
        st_DirectShowPointers.push_back(pPtr);
        /*for it is the free state as initialized*/
        state = POINTER_STATE_FREE;
        st_DirectShowPointerState.push_back(state);
    }
    LeaveCriticalSection(&st_PointerCS);

    if(registered)
    {
        pPtr->AddRef();
    }
    DEBUG_INFO("register [0x%p](%d)\n",pPtr);

    return registered ? 1: 0;
}


void FreeAllPointers()
{
    int wait,tryagain;
    IDirect3DDevice9* pPtr=NULL;
    unsigned int i;
    int findidx;
    BOOL bret;

    do
    {
        wait = 0;
        tryagain=0;
        findidx = -1;
        assert(pPtr == NULL);
        EnterCriticalSection(&st_PointerCS);
        assert(st_DirectShowPointers.size() == st_DirectShowPointerState.size());
        for(i=0; i<st_DirectShowPointers.size(); i++)
        {
            if(st_DirectShowPointerState[i] == POINTER_STATE_FREE)
            {
                findidx = i;
                break;
            }
        }

        if(findidx >= 0)
        {
            pPtr = st_DirectShowPointers[findidx];
            st_DirectShowPointers.erase(st_DirectShowPointers.begin()+findidx);
            st_DirectShowPointerState.erase(st_DirectShowPointerState.begin() + findidx);
            if(st_DirectShowPointers.size()> 0)
            {
                tryagain = 1;
            }
        }
        else
        {
            if(st_DirectShowPointers.size() > 0)
            {
                wait = 1;
            }
        }
        LeaveCriticalSection(&st_PointerCS);

        if(pPtr)
        {
            /*to free the pointer and we will not use this*/
            pPtr->Release();
        }
        pPtr = NULL;
        if(tryagain)
        {
            wait = 1;
            continue;
        }

        if(wait)
        {
            bret = SwitchToThread();
            if(!bret)
            {
                /*sleep for a while*/
                Sleep(10);
            }
        }
    }
    while(wait);

    return;
}




int InitializeEnviron()
{
    InitializeCriticalSection(&st_PointerCS);
    assert(st_DirectShowPointers.size()==0);
    assert(st_DirectShowPointerState.size() == 0);
#ifdef ENDSCENE_HOOK_ENABLED
    InitializeCriticalSection(&st_BufferCS);
    st_hCapBufferEvent = GetEvent(NULL,1);
    if(st_hCapBufferEvent)
    {
        st_CapBufferInited = 1;
    }
#endif /*ENDSCENE_HOOK_ENABLED*/
    return 0;
}

void FinializeEnviron()
{
#ifdef  ENDSCENE_HOOK_ENABLED
    int res;
    int initok=0;
    if(st_CapBufferInited)
    {
        st_CapBufferInited = 0;
        initok = 1;
    }

    if(initok)
    {
        /*now to get for the job*/
        while(1)
        {
            /*if we remove the buffer ,so it is ok*/
            res = RemoveImgBuffer(0);
            if(res > 0)
            {
                break;
            }
            SchedOut();
        }
        CloseHandle(st_hCapBufferEvent);
        st_hCapBufferEvent = NULL;
    }
#endif /*ENDSCENE_HOOK_ENABLED*/
    FreeAllPointers();
    /*we do not delete critical section ,so we should not give the critical section ok*/
}



//#define DX_DEBUG_FUNC_IN() do{HoldPointer(m_ptr);DEBUG_INFO("%s in\n",__FUNCTION__);}while(0)
#define DX_DEBUG_FUNC_IN() do{HoldPointer(m_ptr);}while(0)


//#define DX_DEBUG_FUNC_OUT() do{DEBUG_INFO("%s out\n",__FUNCTION__);UnHoldPointer(m_ptr);}while(0)
#define DX_DEBUG_FUNC_OUT() do{UnHoldPointer(m_ptr);}while(0)


/*************************************************************************
*
*          to make the
*************************************************************************/
//------------------------------------------------------------------------
// hook interface IDirect3DDevice9
class CDirect3DDevice9Hook : public IDirect3DDevice9
{
private:
    IDirect3DDevice9* m_ptr;
public:
    CDirect3DDevice9Hook(IDirect3DDevice9* ptr) : m_ptr(ptr)
    {
        ;
    }
public:
    /*** IUnknown methods ***/
    COM_METHOD(HRESULT, QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr =  m_ptr->QueryInterface(riid, ppvObj);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(ULONG, AddRef)(THIS)
    {
        ULONG ret;
        DX_DEBUG_FUNC_IN();
        ret=  m_ptr->AddRef();
        DX_DEBUG_FUNC_OUT();
        return ret;
    }
    COM_METHOD(ULONG, Release)(THIS)
    {
        ULONG uret,realret;
        int ret;
        IDirect3DDevice9 *pCurPtr=NULL;
        CDirect3DDevice9Hook *pThis=NULL;

        DX_DEBUG_FUNC_IN();
        pCurPtr = m_ptr;
        pThis = this;
        uret = m_ptr->Release();
        realret = uret;
        /*it means that is the just one ,we should return for the job*/
        if(uret == 1)
        {
            ret = UnRegisterPointer(m_ptr);
            /*if 1 it means not release one*/
            realret = ret;
        }
        DEBUG_INFO("[0x%p->0x%p] count %d\n",pThis,pCurPtr,realret);
        DX_DEBUG_FUNC_OUT();
        if(realret == 0)
        {
            delete this;
        }

        return realret;
    }

    /*** IDirect3DDevice9 methods ***/
    COM_METHOD(HRESULT, TestCooperativeLevel)(THIS)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->TestCooperativeLevel();
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(UINT, GetAvailableTextureMem)(THIS)
    {
        UINT ui;
        DX_DEBUG_FUNC_IN();
        ui = m_ptr->GetAvailableTextureMem();
        DX_DEBUG_FUNC_OUT();
        return ui;
    }
    COM_METHOD(HRESULT, EvictManagedResources)(THIS)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->EvictManagedResources();
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetDirect3D)(THIS_ IDirect3D9** ppD3D9)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetDirect3D(ppD3D9);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetDeviceCaps)(THIS_ D3DCAPS9* pCaps)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr =  m_ptr->GetDeviceCaps(pCaps);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetDisplayMode(iSwapChain, pMode);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetCreationParameters(pParameters);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags)
    {
        DX_DEBUG_FUNC_IN();
        m_ptr->SetCursorPosition(X, Y, Flags);
        DX_DEBUG_FUNC_OUT();
        return ;
    }
    COM_METHOD(BOOL, ShowCursor)(THIS_ BOOL bShow)
    {
        BOOL bret;
        DX_DEBUG_FUNC_IN();
        bret = m_ptr->ShowCursor(bShow);
        DX_DEBUG_FUNC_OUT();
        return bret;
    }
    COM_METHOD(HRESULT, CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetSwapChain(iSwapChain, pSwapChain);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(UINT, GetNumberOfSwapChains)(THIS)
    {
        UINT ui;
        DX_DEBUG_FUNC_IN();
        ui = m_ptr->GetNumberOfSwapChains();
        DX_DEBUG_FUNC_OUT();
        return ui;
    }
    COM_METHOD(HRESULT, Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->Reset(pPresentationParameters);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetRasterStatus(iSwapChain, pRasterStatus);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetDialogBoxMode)(THIS_ BOOL bEnableDialogs)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetDialogBoxMode(bEnableDialogs);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
    {
        DX_DEBUG_FUNC_IN();
        m_ptr->SetGammaRamp(iSwapChain, Flags, pRamp);
        DX_DEBUG_FUNC_OUT();
        return;
    }
    COM_METHOD(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp)
    {
        DX_DEBUG_FUNC_IN();
        m_ptr->GetGammaRamp(iSwapChain, pRamp);
        DX_DEBUG_FUNC_OUT();
        return ;
    }
    COM_METHOD(HRESULT, CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->UpdateTexture(pSourceTexture, pDestinationTexture);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetRenderTargetData(pRenderTarget, pDestSurface);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetFrontBufferData(iSwapChain, pDestSurface);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->ColorFill(pSurface, pRect, color);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetRenderTarget(RenderTargetIndex, pRenderTarget);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetDepthStencilSurface(pNewZStencil);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetDepthStencilSurface(ppZStencilSurface);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, BeginScene)(THIS)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->BeginScene();
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, EndScene)(THIS)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->EndScene();
#ifdef 	ENDSCENE_HOOK_ENABLED
        if(SUCCEEDED(hr))
        {
            EndSceneCaptureBuffer(m_ptr);
        }
#endif /*ENDSCENE_HOOK_ENABLED*/
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->Clear(Count, pRects, Flags, Color, Z, Stencil);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetTransform(State, pMatrix);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetTransform(State, pMatrix);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE Type,CONST D3DMATRIX* pD3DMatrix)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->MultiplyTransform(Type, pD3DMatrix);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetViewport(pViewport);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetViewport)(THIS_ D3DVIEWPORT9* pViewport)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetViewport(pViewport);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetMaterial(pMaterial);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetMaterial)(THIS_ D3DMATERIAL9* pMaterial)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetMaterial(pMaterial);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9* pD3DLight9)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetLight(Index, pD3DLight9);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetLight)(THIS_ DWORD Index,D3DLIGHT9* pD3DLight9)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetLight(Index, pD3DLight9);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, LightEnable)(THIS_ DWORD Index,BOOL Enable)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->LightEnable(Index, Enable);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetLightEnable(Index, pEnable);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetClipPlane(Index, pPlane);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetClipPlane)(THIS_ DWORD Index,float* pPlane)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetClipPlane(Index, pPlane);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetRenderState(State, Value);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetRenderState(State, pValue);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateStateBlock(Type, ppSB);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, BeginStateBlock)(THIS)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->BeginStateBlock();
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->EndStateBlock(ppSB);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetClipStatus(pClipStatus);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetClipStatus(pClipStatus);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetTexture(Stage, ppTexture);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetTexture(Stage, pTexture);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetTextureStageState(Stage, Type, pValue);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetTextureStageState(Stage, Type, Value);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetSamplerState(Sampler, Type, pValue);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetSamplerState(Sampler, Type, Value);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, ValidateDevice)(THIS_ DWORD* pNumPasses)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->ValidateDevice(pNumPasses);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetPaletteEntries(PaletteNumber, pEntries);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetPaletteEntries(PaletteNumber, pEntries);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetCurrentTexturePalette)(THIS_ UINT PaletteNumber)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetCurrentTexturePalette(PaletteNumber);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetCurrentTexturePalette(PaletteNumber);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetScissorRect)(THIS_ CONST RECT* pRect)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetScissorRect(pRect);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetScissorRect)(THIS_ RECT* pRect)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetScissorRect(pRect);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetSoftwareVertexProcessing(bSoftware);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(BOOL, GetSoftwareVertexProcessing)(THIS)
    {
        BOOL bret;
        DX_DEBUG_FUNC_IN();
        bret = m_ptr->GetSoftwareVertexProcessing();
        DX_DEBUG_FUNC_OUT();
        return bret;
    }
    COM_METHOD(HRESULT, SetNPatchMode)(THIS_ float nSegments)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetNPatchMode(nSegments);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(float, GetNPatchMode)(THIS)
    {
        float flt;
        DX_DEBUG_FUNC_IN();
        flt = m_ptr->GetNPatchMode();
        DX_DEBUG_FUNC_OUT();
        return flt;
    }
    COM_METHOD(HRESULT, DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateVertexDeclaration(pVertexElements, ppDecl);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetVertexDeclaration(pDecl);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetVertexDeclaration(ppDecl);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetFVF)(THIS_ DWORD FVF)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetFVF(FVF);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetFVF)(THIS_ DWORD* pFVF)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetFVF(pFVF);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateVertexShader(pFunction, ppShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetVertexShader(pShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetVertexShader(ppShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetStreamSourceFreq(StreamNumber, Setting);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetStreamSourceFreq(StreamNumber, pSetting);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetIndices(pIndexData);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetIndices(ppIndexData);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreatePixelShader(pFunction, ppShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetPixelShader(pShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetPixelShader(ppShader);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, DeletePatch)(THIS_ UINT Handle)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->DeletePatch(Handle);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }
    COM_METHOD(HRESULT, CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
    {
        HRESULT hr;
        DX_DEBUG_FUNC_IN();
        hr = m_ptr->CreateQuery(Type, ppQuery);
        DX_DEBUG_FUNC_OUT();
        return hr;
    }


};
//------------------------------------------------------------------------



//------------------------------------------------------------------------
// hook interface IDirect3D9
class CDirect3D9Hook : public IDirect3D9
{
private:
    IDirect3D9* m_ptr;

public:
    CDirect3D9Hook(IDirect3D9* ptr) : m_ptr(ptr) {}

public:
    /*** IUnknown methods ***/
    COM_METHOD(HRESULT, QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {
        /*TODO*/ return m_ptr->QueryInterface(riid, ppvObj);
    }
    COM_METHOD(ULONG, AddRef)(THIS)
    {
        /*TODO*/ return m_ptr->AddRef();
    }
    COM_METHOD(ULONG, Release)(THIS)
    {
        return  m_ptr->Release();
    }

    /*** IDirect3D9 methods ***/
    COM_METHOD(HRESULT, RegisterSoftwareDevice)(THIS_ void* pInitializeFunction)
    {
        /*TODO*/ return m_ptr->RegisterSoftwareDevice(pInitializeFunction);
    }
    COM_METHOD(UINT, GetAdapterCount)(THIS)
    {
        /*TODO*/ return m_ptr->GetAdapterCount();
    }
    COM_METHOD(HRESULT, GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
    {
        /*TODO*/ return m_ptr->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
    }
    COM_METHOD(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format)
    {
        /*TODO*/ return m_ptr->GetAdapterModeCount(Adapter, Format);
    }
    COM_METHOD(HRESULT, EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
    {
        /*TODO*/ return m_ptr->EnumAdapterModes(Adapter, Format, Mode, pMode);
    }
    COM_METHOD(HRESULT, GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode)
    {
        /*TODO*/ return m_ptr->GetAdapterDisplayMode(Adapter, pMode);
    }
    COM_METHOD(HRESULT, CheckDeviceType)(THIS_ UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
    {
        /*TODO*/ return m_ptr->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
    }
    COM_METHOD(HRESULT, CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
    {
        /*TODO*/ return m_ptr->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
    }
    COM_METHOD(HRESULT, CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
    {
        /*TODO*/ return m_ptr->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
    }
    COM_METHOD(HRESULT, CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
    {
        /*TODO*/ return m_ptr->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
    }
    COM_METHOD(HRESULT, CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
    {
        /*TODO*/ return m_ptr->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
    }
    COM_METHOD(HRESULT, GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
    {
        /*TODO*/ return m_ptr->GetDeviceCaps(Adapter, DeviceType, pCaps);
    }
    COM_METHOD(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter)
    {
        /*TODO*/ return m_ptr->GetAdapterMonitor(Adapter);
    }
    COM_METHOD(HRESULT, CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
    {
        int ret;
        HRESULT hr = m_ptr->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

        if(SUCCEEDED(hr))
        {

            ret = RegisterPointer(*ppReturnedDeviceInterface);
            if(ret == 0)
            {
                ULONG uret;
                /*not register success ,so we return not ok*/
                uret = (*ppReturnedDeviceInterface)->Release();
                assert(uret == 0);
                *ppReturnedDeviceInterface=NULL;
                hr = E_ABORT;
            }
            else
            {
                *ppReturnedDeviceInterface = static_cast<IDirect3DDevice9*>(new CDirect3DDevice9Hook(*ppReturnedDeviceInterface));
            }

        }

        return hr;
    }

};
//------------------------------------------------------------------------



//------------------------------------------------------------------------
// hook Direct3DCreate9@d3d9.dll

IDirect3D9*
WINAPI
Direct3DCreate9Callback(UINT SDKVersion)
{
    IDirect3D9* pv = NULL;
    DEBUG_INFO("Before Call Direct3DCreate9Next\n");
    pv= Direct3DCreate9Next(SDKVersion);
    DEBUG_INFO("After Call Direct3DCreate9Next pv(0x%p)\n",pv);

    if(pv)
    {
        DEBUG_INFO("pv get 0x%p\n",pv);
        pv = static_cast<IDirect3D9*>(new CDirect3D9Hook(pv));
    }

    return pv;
}
//------------------------------------------------------------------------


int InitializeHook(void)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DEBUG_BUFFER_FMT(Direct3DCreate9Next,10,"Before Detour Direct3DCreate9Next(0x%p)",Direct3DCreate9Next);
    DetourAttach((PVOID*)&Direct3DCreate9Next, Direct3DCreate9Callback);
    DEBUG_BUFFER_FMT(Direct3DCreate9Next,10,"After Detour Direct3DCreate9Next(0x%p)",Direct3DCreate9Next);
    DetourTransactionCommit();


    return 0;
}

int Routine(HMODULE hModule)
{
    DEBUG_INFO("Initialize Routine");
    InsertModuleFileName(hModule);
    InitializeEnviron();
    InitializeHook();
    RoutineDetourD11();
    DEBUG_INFO("Initialize routine succ\n");
    return 0;
}

int Cleanup()
{
    RotineClearD11();
    FinializeEnviron();
    return 0;
}

AVPixelFormat __TransD3DFORMAT(D3DFORMAT format)
{
    AVPixelFormat avformat;
    switch(format)
    {
    case D3DFMT_UNKNOWN:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_R8G8B8:
        avformat = AV_PIX_FMT_RGB24;
        break;
    case D3DFMT_A8R8G8B8:
        avformat = AV_PIX_FMT_BGRA;
        break;
    case D3DFMT_X8R8G8B8:
        avformat = PIX_FMT_0RGB;
        break;
    case D3DFMT_R5G6B5:
        avformat = AV_PIX_FMT_RGB565LE;
        break;
    case D3DFMT_X1R5G5B5:
        avformat = AV_PIX_FMT_RGB555LE;
        break;
    case D3DFMT_A1R5G5B5:
        avformat = AV_PIX_FMT_BGR555LE;
        break;
    case D3DFMT_A4R4G4B4:
        avformat = AV_PIX_FMT_RGB444LE;
        break;
    case D3DFMT_R3G3B2:
        avformat = PIX_FMT_BGR8;
        break;
    case D3DFMT_A8:
        avformat = AV_PIX_FMT_GRAY8A;
        break;
    case D3DFMT_A8R3G3B2:
        /*can not find the correct one*/
        avformat =  AV_PIX_FMT_BGR8;
        break;
    case D3DFMT_X4R4G4B4:
        avformat = AV_PIX_FMT_RGB444LE;
        break;
    case D3DFMT_A2B10G10R10:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A8B8G8R8:
        avformat = AV_PIX_FMT_ABGR;
        break;
    case D3DFMT_X8B8G8R8:
        avformat =AV_PIX_FMT_0BGR ;
        break;
    case D3DFMT_G16R16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A2R10G10B10 :
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A16B16G16R16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A8P8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_P8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_L8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A8L8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A4L4:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_V8U8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_L6V5U5:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_X8L8V8U8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_Q8W8V8U8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_V16U16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A2W10V10U10:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_UYVY:
        avformat = AV_PIX_FMT_UYVY422;
        break;
    case D3DFMT_R8G8_B8G8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_YUY2:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_G8R8_G8B8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_DXT1:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_DXT2:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_DXT3:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_DXT4:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_DXT5:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D16_LOCKABLE:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D32:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D15S1:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D24S8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D24X8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D24X4S4:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D32F_LOCKABLE:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D24FS8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_D32_LOCKABLE:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_S8_LOCKABLE:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_L16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_VERTEXDATA:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_INDEX16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_INDEX32:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_Q16W16V16U16:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_MULTI2_ARGB8:
        avformat = AV_PIX_FMT_ARGB;
        break;
    case D3DFMT_R16F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_G16R16F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A16B16G16R16F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_R32F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_G32R32F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A32B32G32R32F:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_CxV8U8:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A1:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_A2B10G10R10_XR_BIAS:
        avformat = AV_PIX_FMT_NONE;
        break;
    case D3DFMT_BINARYBUFFER:
        avformat = AV_PIX_FMT_NONE;
        break;
    default:
        avformat = AV_PIX_FMT_NONE;
        break;
    }

    return avformat;
}



int __CaptureBufferDX9(IDirect3DDevice9* pDevice,HANDLE hRemoteHandle,void* pRemoteAddr,int RemoteSize,unsigned int* pFormat,unsigned int *pWidth,unsigned int* pHeight)
{
    int ret=1;
    HRESULT hr;
    LPDIRECT3DSURFACE9 pSurface = NULL,pBackBuffer=NULL;
    BOOL bret;
    D3DSURFACE_DESC desc;
    D3DLOCKED_RECT LockRect;
    DWORD curret;
    int lockedrect=0;
    int totalbytes=0;
    int writelen=0;
    //DEBUG_INFO("\n");
    __try
    {
        //DEBUG_INFO("\n");

        hr = pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("could not get backbuffer(0x%08x) (%d)\n",hr,ret);
            goto fail;
        }
        //DEBUG_INFO("\n");
        hr = pBackBuffer->GetDesc(&desc);
        if(!FAILED(hr))
        {
            DEBUG_INFO("pBackBuffer Format 0x%08x Type 0x%08x Usage 0x%08x Pool 0x%08x SampleType 0x%08x SampleQuality 0x%08x Width %d Height %d\n",
                       desc.Format,desc.Type,desc.Usage,desc.Pool,desc.MultiSampleType,desc.MultiSampleQuality,desc.Width,desc.Height);
        }
        else
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("pBackBuffer->GetDesc Failed(0x%08x)(%d)\n",hr,ret);
            goto fail;
        }

        hr = pDevice->CreateOffscreenPlainSurface(desc.Width,
                desc.Height,
                desc.Format,
                D3DPOOL_SYSTEMMEM, &pSurface, NULL);
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("could not create surface (0x%08x) (%d)\n",hr,ret);
            goto fail;
        }
        //DEBUG_INFO("\n");

        hr = pSurface->GetDesc(&desc);
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("pSurface->GetDesc Failed(0x%08x)(%d)\n",hr,ret);
            goto fail;
        }

        DEBUG_INFO("pSurface	Format 0x%08x Type 0x%08x Usage 0x%08x Pool 0x%08x SampleType 0x%08x SampleQuality 0x%08x Width %d Height %d\n",
                   desc.Format,desc.Type,desc.Usage,desc.Pool,desc.MultiSampleType,desc.MultiSampleQuality,desc.Width,desc.Height);

        SetLastError(0);
        hr = pDevice->GetRenderTargetData(pBackBuffer,pSurface);
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("could not get render target data (0x%08x)  (%d)\n",hr,ret);
            goto fail;
        }
        //DEBUG_INFO("\n");

        /*now to lock rect for it will give the for copy memory*/
        hr = pSurface->LockRect(&LockRect,NULL, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY);
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            DEBUG_INFO("could not lock rect (0x%08x) (%d)\n",hr,ret);
            goto fail;
        }

        lockedrect= 1;

        /*now to copy memory*/
        totalbytes = desc.Width * desc.Height * 32 / 8;
        if(RemoteSize < totalbytes)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            DEBUG_INFO("buffer size %d < needsize  (%d)\n",RemoteSize,
                       totalbytes);
            goto fail;
        }


        writelen = 0;
        while(writelen < totalbytes)
        {
            bret = WriteProcessMemory(hRemoteHandle,(LPVOID)((ptr_t)pRemoteAddr + writelen),(LPCVOID)(((ptr_t)LockRect.pBits)+writelen),totalbytes-writelen,&curret);
            if(!bret)
            {
                ret = LAST_ERROR_RETURN();
                ERROR_INFO("could not write (%d process address 0x%p with size %d) error(%d)\n",hRemoteHandle,
                           pRemoteAddr,totalbytes,ret);
                goto fail;
            }
            writelen += curret;
        }

        hr = pSurface->UnlockRect();
        if(FAILED(hr))
        {
            ret = LAST_ERROR_RETURN();
            ERROR_INFO("could not unlockrect (0x%08lx)(%d)\n",hr,ret);
            goto fail;
        }
        /*now to unlock*/
        lockedrect = 0;

        /*now to set format we will */
        *pFormat =(unsigned int)__TransD3DFORMAT(desc.Format);
        *pWidth = desc.Width;
        *pHeight = desc.Height;

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ret = GetLastError() ? GetLastError() : 1;
        DEBUG_INFO("catch exception %d lockedrect %d\n",GetExceptionCode(),lockedrect);
        goto fail;
    }
    //DEBUG_INFO("\n");
    if(lockedrect)
    {
        assert(pSurface);
        pSurface->UnlockRect();
    }
    lockedrect = 0;
    if(pSurface)
    {
        pSurface->Release();
    }
    pSurface = NULL;

    if(pBackBuffer)
    {
        pBackBuffer->Release();
    }
    pBackBuffer = NULL;
    return totalbytes;
fail:
    assert(ret > 0);

    if(lockedrect)
    {
        assert(pSurface);
        pSurface->UnlockRect();
    }
    lockedrect = 0;
    if(pSurface)
    {
        pSurface->Release();
    }
    pSurface = NULL;
    if(pBackBuffer)
    {
        pBackBuffer->Release();
    }
    pBackBuffer = NULL;
    SetLastError(ret);
    return -ret;
}



EXTERN_C __declspec(dllexport) void* CaptureBuffer(imgcap_buffer_t *pCapture);

extern "C" int CaptureBufferDX11(imgcap_buffer_t* pCapture);
int CaptureBufferDX9(imgcap_buffer_t* pCapture);
void* CaptureBuffer(imgcap_buffer_t *pCapture)
{
    int ret;

    ret = CaptureBufferDX9(pCapture);
    if(ret >= 0)
    {
        DEBUG_INFO("\n");
        return (void*) ret;
    }

    DEBUG_INFO("\n");
    ret = CaptureBufferDX11(pCapture);
    return (void*)ret;
}

void AceCrash() {}

#ifdef ENDSCENE_HOOK_ENABLED
/************************************************************
*  this is the handle for endscene hook and done
*
************************************************************/

static int EndSceneCaptureBuffer(IDirect3DDevice9* pDevice)
{
    int ret,olderr;
    HANDLE hRemoteProc=NULL;
    imgcap_buffer_t* pCapture=NULL;

    /*to pretend we do not modify the last error*/
    olderr = GetLastError();

    pCapture = GetImgBuffer();
    if(pCapture == NULL)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        goto fail;
    }
	/*we get event ,so it is ok */
	assert(st_hCapBufferEvent);

    hRemoteProc = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION ,FALSE,pCapture->processid);
    if(hRemoteProc == NULL)
    {
        ret = LAST_ERROR_RETURN();
        ERROR_INFO("Open Process[%d] error(%d)\n",pCapture->processid,ret);
        goto fail;
    }

    ret = __CaptureBufferDX9(pDevice,hRemoteProc,pCapture->data,pCapture->datalen,&(pCapture->format),&(pCapture->width),&(pCapture->height));
    if(ret < 0)
    {
        ret = LAST_ERROR_RETURN();
        ERROR_INFO("Capture [0x%p] Error(%d)\n",pDevice,ret);
        goto fail;
    }

    pCapture->filledlen = ret;

    if(hRemoteProc)
    {
        CloseHandle(hRemoteProc);
    }
    hRemoteProc = NULL;

    pCapture->getresult = 0;
    SetEvent(st_hCapBufferEvent);
    SetLastError(olderr);
    return 0;
fail:
    assert(ret > 0);
    if(hRemoteProc)
    {
        CloseHandle(hRemoteProc);
    }
    hRemoteProc = NULL;
    if(pCapture)
    {
        pCapture->getresult = ret;
        SetEvent(st_hCapBufferEvent);
    }
    pCapture = NULL;
    SetLastError(olderr);
    return -ret;

}

static int InsertImgBuffer(imgcap_buffer_t* pCapBuffer)
{
    int ret = -1;

    EnterCriticalSection(&st_BufferCS);
    if(st_pHandleCapBuffer == NULL)
    {
    	assert(st_CapBufferHandlein == 0);
        ret = 1;
        st_pHandleCapBuffer = pCapBuffer;
    }
    LeaveCriticalSection(&st_BufferCS);

    return ret;
}

static imgcap_buffer_t* GetImgBuffer()
{
    imgcap_buffer_t* pCapBuffer =NULL;

    EnterCriticalSection(&st_BufferCS);
    if(st_CapBufferHandlein == 0 && st_pHandleCapBuffer)
    {
        pCapBuffer = st_pHandleCapBuffer;
        st_CapBufferHandlein = 1;
    }
    LeaveCriticalSection(&st_BufferCS);
    return pCapBuffer;
}

static int RemoveImgBuffer(int force)
{
    int ret = -1;

    EnterCriticalSection(&st_BufferCS);
    if(force)
    {
        st_pHandleCapBuffer = NULL;
        st_CapBufferHandlein = 0;
        ret = 1;
    }
    else if(st_CapBufferHandlein == 0)
    {
        st_pHandleCapBuffer = NULL;
        st_CapBufferHandlein = 0;
        ret = 1;
    }
    LeaveCriticalSection(&st_BufferCS);
    return ret;
}

int CaptureBufferDX9(imgcap_buffer_t* pCapture)
{
    int ret;
    DWORD dret;
    int res;

    if(st_hCapBufferEvent == NULL || st_CapBufferInited == 0)
    {
        ret = ERROR_BAD_ENVIRONMENT;
        return -ret;
    }

    /*now to set img buffer */
    ret = InsertImgBuffer(pCapture);
    if(ret < 0)
    {
        ret = ERROR_ALREADY_ASSIGNED;
        return -ret;
    }

    /*now we should wait at least 100 millisecond ,if we wait timeout ,so we should remove the img buffer and not let it do,
       if we are not */

    dret = WaitForSingleObjectEx(st_hCapBufferEvent,100,TRUE);
    if(dret != WAIT_OBJECT_0)
    {
        ret = LAST_ERROR_RETURN();
        goto fail;
    }

    /*now we should remove it*/
    res = RemoveImgBuffer(1);
    assert(res > 0);

    /*now we should test whether it is ok in the job*/
    if(pCapture->getresult != 0)
    {
        ret = pCapture->getresult;
        ERROR_INFO("capture error %d\n",ret);
        SetLastError(ret);
        return -ret;
    }


    return pCapture->filledlen;

fail:
    assert(ret > 0);

    while(1)
    {
        res = RemoveImgBuffer(0);
        if(res > 0)
        {
            break;
        }

        /*now we can not remove img buffer ,so we should wait for it handled*/
        dret = WaitForSingleObjectEx(st_hCapBufferEvent,100,TRUE);
        if(dret == WAIT_OBJECT_0)
        {
            break;
        }
    }

    res = RemoveImgBuffer(1);
    assert(res > 0);
    SetLastError(ret);
    return -ret;
}


#else   /*ENDSCENE_HOOK_ENABLED*/
int CaptureBufferDX9(imgcap_buffer_t* pCapture)
{
    IDirect3DDevice9* pDevice=NULL;
    unsigned int idx=0;
    int ret;
    HANDLE hRemoteProc=NULL;
    int getlen;

    hRemoteProc = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION ,FALSE,pCapture->processid);
    if(hRemoteProc == NULL)
    {
        ret = LAST_ERROR_RETURN();
        ERROR_INFO("\n");
        goto fail;
    }

    while(1)
    {
        pDevice = GrapPointer(idx);
        if(pDevice == NULL)
        {
            ret = ERROR_DEVICE_ENUMERATION_ERROR;
            ERROR_INFO("\n");
            goto fail;
        }

        ret = __CaptureBufferDX9(pDevice,hRemoteProc,pCapture->data,pCapture->datalen,&(pCapture->format),&(pCapture->width),&(pCapture->height));
        if(ret >= 0)
        {
            break;
        }

        FreePointer(pDevice);
        pDevice = NULL;
        idx ++;

    }

    getlen = ret;
    if(pDevice)
    {
        FreePointer(pDevice);
    }
    pDevice=NULL;
    if(hRemoteProc)
    {
        CloseHandle(hRemoteProc);
    }
    hRemoteProc = NULL;
    return getlen;
fail:
    assert(ret > 0);
    if(pDevice)
    {
        FreePointer(pDevice);
    }
    pDevice=NULL;
    if(hRemoteProc)
    {
        CloseHandle(hRemoteProc);
    }
    hRemoteProc = NULL;
    SetLastError(ret);
    return -ret;
}
#endif  /*ENDSCENE_HOOK_ENABLED*/

