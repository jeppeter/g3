
#ifndef __INJECT_CTRL_H__
#define __INJECT_CTRL_H__

#include <injectcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

void StopThreadControl(thread_control_t *pThrControl);
int StartThreadControl(thread_control_t *pThrControl,ThreadFunc_t pStartFunc,LPVOID pParam,int startnow);
int ResumeThreadControl(thread_control_t *pThrControl);



#ifdef __cplusplus
};
#endif


#endif /*__INJECT_CTRL_H__*/

