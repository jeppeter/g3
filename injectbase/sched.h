

#ifndef __SCHED_H__
#define __SCHED_H__

#ifdef INJECTBASE_EXPORTS
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/


int SchedOut();

#endif /*__SCHED_H__*/

