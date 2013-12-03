
#ifndef  __DETOUR_MESSAGE_H__
#define  __DETOUR_MESSAGE_H__

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL DetourMessageInputInit(void);
void DetourMessageInputFini(void);
int InsertEmulationMessageQueue(LPMSG lpMsg,int back);

#ifdef __cplusplus
};
#endif

#endif /*__DETOUR_MESSAGE_H__*/

