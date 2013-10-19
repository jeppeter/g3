
#ifndef __TIME_TICKS_H__
#define __TIME_TICKS_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/



EXPORT_C_FUNC BOOL InitializeTicks(unsigned int*pStartTick,unsigned int *pCurTick,unsigned int *pEndTick,int timeout);
EXPORT_C_FUNC BOOL GetCurrentTicks(unsigned int *pCurTick);
EXPORT_C_FUNC unsigned int LeftTicks(unsigned int *pStartTick,unsigned int *pCurTick,unsigned int *pEndTick,int timeout);



#endif /*__TIME_TICKS_H__*/

