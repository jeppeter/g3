
#include "joystick_enum.h"

static CRITICAL_SECTION st_JoyConfigCS;

JoyStickBetopEnum::JoyStickBetopEnum()
{
}

JoyStickBetopEnum::~JoyStickBetopEnum()
{
}


HRESULT JoyStickBetopEnum::GetConfig(UINT uiJoy,LPDIJOYCONFIG pjc,DWORD dwFlags)
{
    HRESULT hr=DIERR_NOMOREITEMS;
    if(uiJoy != 0)
    {
        return DIERR_NOMOREITEMS;
    }

    hr = DIERR_INVALIDPARAM;
    switch(dwFlags)
    {
    case DIJC_GUIDINSTANCE:
        break;
    case DIJC_REGHWCONFIGTYPE:
        break;
    case DIJC_GAIN:
        break;
    case DIJC_CALLOUT:
        break;
    }

    return hr;
}

/* now we should */
HRESULT JoyStickBetopEnum::CallEnumTypes(LPDIJOYTYPECALLBACK lpCallback,LPVOID pvRef)
{
    return S_OK;
}






