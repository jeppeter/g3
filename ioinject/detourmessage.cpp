
#include "detourmessage.h"
#include <output_debug.h>
#include <detours/detours.h>

typedef BOOL (WINAPI *GetMessageFunc_t)(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax
);

typedef BOOL (WINAPI *PeekMessageFunc_t)(
    LPMSG lpMsg,
    HWND hWnd,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg
);


#include "detourmessage_debug.cpp"


BOOL DetourMessageInputInit(void)
{
	__MessageDetour();
	return TRUE;
}
void DetourMessageInputFini(void)
{
	return ;
}


