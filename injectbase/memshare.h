
#ifndef  __MEM_SHARE_H__
#define  __MEM_SHARE_H__

#include <Windows.h>

#ifdef INJECTBASE_EXPORTS
#ifndef INJECTBASE_API
#define   INJECTBASE_API   extern "C" __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef INJECTBASE_API
#define   INJECTBASE_API  extern "C" __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/



INJECTBASE_API int ReadShareMem(unsigned char* pBasePtr,int offset,unsigned char* pBuffer,int bufsize);
INJECTBASE_API int WriteShareMem(unsigned char* pBasePtr,int offset,unsigned char* pBuffer,int bufsize);
INJECTBASE_API HANDLE CreateMapFile(const char* pMapFileName,int size,int create);
INJECTBASE_API unsigned char* MapFileBuffer(HANDLE hMapFile,int size);
INJECTBASE_API void UnMapFileBuffer(unsigned char** ppBuffer);
INJECTBASE_API void CloseMapFileHandle(HANDLE *pHandle);






#endif /*__MEM_SHARE_H__*/

