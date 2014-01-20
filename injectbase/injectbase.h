
#ifndef  __INJECT_BASE_H__
#define  __INJECT_BASE_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef INJECTBASE_API
#define   INJECTBASE_API   extern "C" __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef INJECTBASE_API
#define   INJECTBASE_API  extern "C" __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/

#include <detours/detours.h>
#include <uniansi.h>
#include <output_debug.h>
#include <evt.h>
#include <memshare.h>
#include <StackWalker.h>
#include <sched.h>
#include <procex.h>
#include <timeticks.h>
#include <injectcommon.h>
#include <funclist.h>

typedef struct
{
    HANDLE m_hFillEvt;
    int m_Error;
    int m_Idx;
    ptr_t m_BaseAddr;
    ptr_t m_Offset;
    unsigned int size;
} EVENT_LIST_t,*PEVENT_LIST_t;


INJECTBASE_API HWND GetCurrentProcessActiveWindow();
INJECTBASE_API int InsertModuleFileName(HMODULE hModule);
INJECTBASE_API void SetUnHandlerExceptionDetour();
INJECTBASE_API void StopThreadControl(thread_control_t *pThrControl);
INJECTBASE_API int StartThreadControl(thread_control_t *pThrControl,ThreadFunc_t pStartFunc,LPVOID pParam,int startnow);
INJECTBASE_API int ResumeThreadControl(thread_control_t *pThrControl);
INJECTBASE_API int RegisterCreateWindowFunc(FuncCall_t pFunc,LPVOID pParam,int prior);
INJECTBASE_API int UnRegisterCreateWindowFunc(FuncCall_t pFunc);
INJECTBASE_API int RegisterDestroyWindowFunc(FuncCall_t pFunc,LPVOID pParam,int prior);
INJECTBASE_API int UnRegisterDestroyWindowFunc(FuncCall_t pFunc);
INJECTBASE_API int ShowCursorHandle();
INJECTBASE_API int SetShowCursorHide();
INJECTBASE_API int SetShowCursorNormal();
INJECTBASE_API int BaseSetKeyMouseState(LPVOID pParam,LPVOID pInput);
INJECTBASE_API int BaseSetWindowsRect(HWND hWnd);
INJECTBASE_API int BaseScreenMousePoint(HWND hwnd,POINT* pPoint);
INJECTBASE_API int GetBaseMouseState(UINT *pMouseBtnState,UINT btns,POINT *pPoint,UINT* pMouseZ);
INJECTBASE_API int InitBaseKeyState(void);
INJECTBASE_API int InitBaseMouseState(void);
INJECTBASE_API int GetBaseKeyState(unsigned char *pKeyState,UINT keys);
INJECTBASE_API int BasePressKeyDownTimes(UINT scancode);
INJECTBASE_API int BaseMouseBtnDown(UINT btn);
INJECTBASE_API int BaseGetMousePointAbsolution(POINT *pPoint);




int InjectBaseModuleInit(HMODULE hModule);
void InjectBaseModuleFini(HMODULE hModule);


#endif /*__INJECT_BASE_H__*/

