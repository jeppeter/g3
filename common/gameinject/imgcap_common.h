

#ifndef __IMG_CAP_COMMON_H__
#define __IMG_CAP_COMMON_H__


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



#endif /*__IMG_CAP_COMMON_H__*/

