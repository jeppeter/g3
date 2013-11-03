
#include <hogvideo.h>
#include <uniansi.h>

#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))

static IGraphBuilder *st_pGraph=NULL;
static IMediaEvent *st_pEvent=NULL;
static IVideoWindow *st_pVideoWindow=NULL;
static IMediaControl *st_pMediaControl=NULL;
static int st_CoInitialized=0;


static void __UnHog()
{
    if(st_pMediaControl == NULL)
    {
        return ;
    }
    st_pMediaControl->Stop();
    return ;
}

static void __FiniHogVideo()
{
    ULONG uret;
    __UnHog();
    if(st_pEvent)
    {
        long evCode;
        st_pEvent->WaitForCompletion(INFINITE, &evCode);
    }

    if(st_pEvent)
    {
        uret = st_pEvent->Release();
        assert(uret == 0);
    }
    st_pEvent = NULL;

    if(st_pVideoWindow)
    {
        uret = st_pVideoWindow->Release();
        assert(uret == 0);
    }
    st_pVideoWindow = NULL;

    if(st_pMediaControl)
    {
        uret = st_pMediaControl->Release();
        assert(uret == 0);
    }
    st_pMediaControl = NULL;

    if(st_pGraph)
    {
        uret = st_pGraph->Release();
        assert(uret == 0);
    }
    st_pGraph = NULL;

    if(st_CoInitialized)
    {
        CoUninitialize();
    }
    st_CoInitialized = 0;
    return ;
}

static int __InitHogVideo(const char* hogfile)
{
    HRESULT hr;
    int ret;
#ifdef _UNICODE
    wchar_t *pHogWide=NULL;
    int hogwidesize=0;
#else
    char *pHogAnsi=NULL;
#endif
    assert(st_CoInitialized == 0);
    hr = CoInitialize(NULL);
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("could not CoInitialize HRESULT(0x%08x) error(%d)\n",hr,ret);
        goto fail;
    }

    st_CoInitialized = 1;

    assert(st_pGraph);
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder, (void **)&st_pGraph);
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("CoCreateInstance HRESULT(0x%08x) error(%d)\n",
                   hr,ret);
        goto fail;
    }

    hr = st_pGraph->QueryInterface(IID_IMediaControl, (void **)&st_pMediaControl);
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Query MediaControl HRESULT(0x%08x) error(%d)\n",hr,ret);
        goto fail;
    }

    hr = st_pGraph->QueryInterface(IID_IVideoWindow, (void**)&st_pVideoWindow);
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Query VideoWindow HRESULT(0x%08x) error(%d)\n",hr,ret);
        goto fail;
    }

    hr = st_pGraph->QueryInterface(IID_IMediaEvent, (void **)&st_pEvent);
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Query MediaEvent HRESULT(0x%08x) error(%d)\n",hr,ret);
        goto fail;
    }

#ifdef _UNICODE
    ret = AnsiToUnicode(hogfile,&pHogWide,&hogwidesize);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    hr = st_pGraph->RenderFile(pHogWide,NULL);

#else
    pHogAnsi = hogfile;
    hr = st_pGraph->RenderFile(pHogAnsi,NULL);

#endif

    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Render File(%s) HRESULT(0x%08x) error(%d)\n",hogfile,hr,ret);
        goto fail;
    }

    hr = st_pMediaControl->Run();
    if(FAILED(hr))
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Run MediaControl HRESULT(0x%08x) error(%d)\n",hr,ret);
        goto fail;
    }

    st_pVideoWindow->put_WindowState(SW_HIDE);
    st_pVideoWindow->put_AutoShow(OAFALSE);
    st_pVideoWindow->put_Visible(OAFALSE);
    st_pVideoWindow->put_Top(-100);
    st_pVideoWindow->put_Left(-100);
    st_pVideoWindow->put_Width(0);
    st_pVideoWindow->put_Height(0);

    st_pMediaControl->Pause();




#ifdef _UNICODE
    AnsiToUnicode(hogfile,&pHogWide,&hogwidesize);
#else
    pHogAnsi = NULL;
#endif
    return 0;


fail:
#ifdef _UNICODE
    AnsiToUnicode(hogfile,&pHogWide,&hogwidesize);
#else
    pHogAnsi = NULL;
#endif
    __FiniHogVideo();
    SetLastError(ret);
    return -ret;
}

int InitHogVideo(const char * hogfile)
{
    __FiniHogVideo();
}


void FiniHogVideo()
{
    __FiniHogVideo();
    return ;
}
