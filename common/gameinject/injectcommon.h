
#ifndef __INJECT_COMMON_H__
#define __INJECT_COMMON_H__


#include <Windows.h>
#include <stdint.h>

#define  CURSOR_MASK_BITMAPINFO         101
#define  CURSOR_MASK_BITDATA            102
#define  CURSOR_COLOR_BITMAPINFO        103
#define  CURSOR_COLOR_BITDATA           104

typedef unsigned long ptr_t;

typedef struct
{
	HANDLE thread;
	unsigned long threadid;
	HANDLE exitevt;
	int running;
	int exited;	
} thread_control_t;

typedef struct __share_data
{
	unsigned int datalen;
	unsigned int datatype;
	unsigned char data[8];
} SHARE_DATA,*LPSHARE_DATA;



typedef DWORD (WINAPI *ThreadFunc_t)(LPVOID lpParam);

#define LAST_ERROR_CODE() ((int)(GetLastError() ? GetLastError() : 1))
#define SETERRNO(ret)  (SetLastError(ret))
#define GETERRNO()     ((int)(GetLastError() ? GetLastError() : 1))


#endif /*__INJECT_COMMON_H__*/


