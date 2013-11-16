
#ifndef __INJECT_COMMON_H__
#define __INJECT_COMMON_H__


#include <Windows.h>

typedef struct
{
    HANDLE m_hFillEvt;
    int m_Error;
    int m_Idx;
    pcmcap_ptr_t m_BaseAddr;
    pcmcap_ptr_t m_Offset;
    unsigned int size;
} EVENT_LIST_t;


#endif /*__INJECT_COMMON_H__*/


