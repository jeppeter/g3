#ifndef __IMGCAPCOMMON_H__
#define __IMGCAPCOMMON_H__

#include <stdint.h>

enum IMGCAPPER_STATE
{
	IMGCAPCTRL_STATE_OPEN = 0,			 // ע��ɹ�
	IMGCAPCTRL_STATE_CLOSE, 			 // δע���δע�����
};

enum IMGCAPPER_OPERATION
{
	IMGCAPCTRL_OPERATION_NONE = 0,		 // �޿ɽ�ȡ���ݻ�֪���ɽ�ȡ��������ʲô��ʽ���ֵ�
	IMGCAPCTRL_OPERATION_GDI,
	IMGCAPCTRL_OPERATION_DIRECTX,
	IMGCAPCTRL_OPERATION_OPENGL,
};

typedef struct
{
    unsigned int processid;
	int getresult;
	int filledlen;
    void * data;
    unsigned int datalen;
    unsigned int format;
    unsigned int width;
    unsigned int height;
} imgcap_buffer_t;

typedef unsigned long ptr_t;

#endif /*__DLL_CAPTURE_H__*/


