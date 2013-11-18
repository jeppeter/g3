
#ifndef __INJECT_COMMON_H__
#define __INJECT_COMMON_H__


#include <Windows.h>

typedef unsigned long ptr_t;

typedef struct
{
	HANDLE thread;
	unsigned long threadid;
	HANDLE exitevt;
	int running;
	int exited;	
} thread_control_t;



#endif /*__INJECT_COMMON_H__*/


