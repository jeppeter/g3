
#ifndef __TIME_TICKS_H__
#define __TIME_TICKS_H__

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



INJECTBASE_API BOOL InitializeTicks(unsigned int*pStartTick,unsigned int *pCurTick,unsigned int *pEndTick,int timeout);
INJECTBASE_API BOOL GetCurrentTicks(unsigned int *pCurTick);
INJECTBASE_API unsigned int LeftTicks(unsigned int *pStartTick,unsigned int *pCurTick,unsigned int *pEndTick,int timeout);



#endif /*__TIME_TICKS_H__*/

