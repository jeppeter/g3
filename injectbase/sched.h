

#ifndef __SCHED_H__
#define __SCHED_H__

#ifdef DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*DLL_EXPORT*/


int SchedOut();

#endif /*__SCHED_H__*/

