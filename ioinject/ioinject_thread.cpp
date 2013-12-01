
#include "ioinject_thread.h"

typedef struct
{
    uint32_t m_Started;
    thread_control_t m_ThreadControl;
    uint32_t m_Bufnumm;
    uint32_t m_BufSectSize;
    uint8_t m_MemShareBaseName[IO_NAME_MAX_SIZE];
    HANDLE m_hMapFile;
    ptr_t m_pMemShareBase;
    uint8_t m_FreeEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pFreeEvts;
    uint8_t m_InputEvtBaseName[IO_NAME_MAX_SIZE];
    HANDLE *m_pInputEvts;
    EVENT_LIST_t* m_pEventListArray;
    CRITICAL_SECTION m_ListCS;
    int m_ListCSInited;
    std::vector<EVENT_LIST_t*>* m_pFreeList;
} DETOUR_DIRECTINPUT_STATUS_t,*PDETOUR_DIRECTINPUT_STATUS_t;


static DETOUR_THREAD_STATUS_t *st_pDetourStatus=NULL;

