#ifndef __DLL_CAPTURE_H__
#define __DLL_CAPTURE_H__


#include <libavutil/pixfmt.h>

typedef struct _capture_buffer
{
    unsigned int m_Processid;
    void* m_Data;
    unsigned int m_DataLen;
    unsigned int m_Format;
    unsigned int m_Width;
    unsigned int m_Height;
} capture_buffer_t;

typedef unsigned long ptr_t;


#endif /*__DLL_CAPTURE_H__*/


