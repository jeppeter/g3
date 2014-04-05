
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
    if(uiJoy != 0)
    {
        return DIERR_NOMOREITEMS;
    }

	
}

/* now we should */
HRESULT JoyStickBetopEnum::CallEnumTypes(LPDIJOYTYPECALLBACK lpCallback,LPVOID pvRef)
{
    return S_OK;
}






