

#ifndef  __EVT_H__
#define  __EVT_H__

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

HANDLE GetEvent(const char* pName,int create);
HANDLE GetMutex(const char* pName,int create);

#ifdef __cplusplus
};
#endif


#endif /*__EVT_H__*/

