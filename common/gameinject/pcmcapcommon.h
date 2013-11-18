#ifndef  __PCMCAPCOMMON_H__
#define  __PCMCAPCOMMON_H__

#include <windows.h>
#include <injectcommon.h>

#define    PCMCAPPER_OPERATION_NONE      0 	// 不对进程声音进行抓取或播放的动作
#define    PCMCAPPER_OPERATION_CAPTURE   1	// 对进程声音进行抓取的动作
#define    PCMCAPPER_OPERATION_RENDER    2 	// 对进程声音进行播放的动作（播放交给系统去完成，不予干预）
#define    PCMCAPPER_OPERATION_BOTH   	 3  // 对上述两者同时进行


typedef struct
{
	int pointer;
    int datalen;
    int datasize;
    unsigned char data[4];
} pcmcap_data_t;

#define  FORMAT_EXTEND_SIZE   128

typedef struct
{
	float volume;
	unsigned char format[FORMAT_EXTEND_SIZE];
} pcmcap_format_t;

typedef struct
{
	pcmcap_format_t format;
	pcmcap_data_t data;
} pcmcap_buffer_t;

#define  PCMCAP_AUDIO_NONE             0
#define  PCMCAP_AUDIO_CAPTURE          1
#define  PCMCAP_AUDIO_RENDER           2
#define  PCMCAP_AUDIO_BOTH             3

typedef struct
{
	unsigned int  operation;       /*operation code*/
	unsigned int  timeout;         /*the timeout for handle waiting*/
	unsigned char mem_sharename[128];
	unsigned int  mem_sharesize;
	unsigned int  packsize;        /*this is for the one packet size*/
	unsigned int  packnum;        /*packets to do*/
	unsigned char freelist_semnamebase[128]; /*the semphore for free in the injected one get the signalled ,the pointer of packet will into the */
	unsigned char filllist_semnamebase[128]; /*the semaphore for release in the injected one ,to signal the filled list */
	unsigned char startevt_name[128];        /*for start event notify name*/
	unsigned char stopevt_name[128];         /*for stop event notify name*/
} pcmcap_control_t;


#endif /*__PCM_CAP_COMMON_H__*/

