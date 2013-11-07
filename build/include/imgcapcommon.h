#ifndef __IMGCAPCOMMON_H__
#define __IMGCAPCOMMON_H__

typedef struct
{
    unsigned int processid;
    void * data;
    unsigned int datalen;
    unsigned int format;
    unsigned int width;
    unsigned int height;
} imgcap_buffer_t;

typedef unsigned long ptr_t;

#endif /*__DLL_CAPTURE_H__*/


