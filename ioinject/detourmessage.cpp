
#include "detourmessage.h"
#include <injectbase.h>
#include <output_debug.h>
#include <detours/detours.h>
#include <iocapcommon.h>

#ifdef   IOCAP_DEBUG
#define  DETOUR_MESSAGE_DEBUG   1
#endif

#ifdef   IOCAP_EMULATION
#define  DETOUR_MESSAGE_EMULATION  1
#endif

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

typedef DWORD (WINAPI *GetMessagePosFunc_t)();

#if defined(DETOUR_MESSAGE_DEBUG) && defined(DETOUR_MESSAGE_EMULATION)
#error "Either define DETOUR_MESSAGE_DEBUG or DETOUR_MESSAGE_EMULATION can not both"
#endif

#ifdef DETOUR_MESSAGE_EMULATION
#pragma message("@message emulation@")
#elif DETOUR_MESSAGE_DEBUG
#pragma message("@message debug@")
#else
#error "must define one of DETOUR_MESSAGE_DEBUG and DETOUR_MESSAGE_EMULATION"
#endif

#ifdef DETOUR_MESSAGE_DEBUG
#include "detourmessage_debug.cpp"
#endif

#ifdef DETOUR_MESSAGE_EMULATION
#include "detourmessage_emulation.cpp"
#endif

BOOL DetourMessageInputInit(HMODULE hModule)
{
	__MessageDetour();
	return TRUE;
}
void DetourMessageInputFini(HMODULE hModule)
{
	return ;
}


