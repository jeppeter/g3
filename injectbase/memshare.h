
#ifndef  __MEM_SHARE_H__
#define  __MEM_SHARE_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/



EXPORT_C_FUNC int ReadShareMem(unsigned char* pBasePtr,int offset,unsigned char* pBuffer,int bufsize);
EXPORT_C_FUNC int WriteShareMem(unsigned char* pBasePtr,int offset,unsigned char* pBuffer,int bufsize);
EXPORT_C_FUNC HANDLE CreateMapFile(const char* pMapFileName,int size,int create);
EXPORT_C_FUNC unsigned char* MapFileBuffer(HANDLE hMapFile,int size);
EXPORT_C_FUNC void UnMapFileBuffer(unsigned char** ppBuffer);
EXPORT_C_FUNC void CloseMapFileHandle(HANDLE *pHandle);






#endif /*__MEM_SHARE_H__*/

