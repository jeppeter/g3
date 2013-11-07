#ifndef __UNI_ANSI_H__
#define __UNI_ANSI_H__

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


/**********************************************************
*    if pWideChar==NULL  will reset *ppChar=NULL and free memory
*    else set *ppChar by new and *pCharSize is the size of *ppChar
*    return value success >= 0 number of bytes in *ppChar
*    otherwise negative error code
**********************************************************/
INJECTBASE_API int UnicodeToAnsi(wchar_t* pWideChar,char** ppChar,int*pCharSize);

/**********************************************************
*    if pChar==NULL  will reset *ppWideChar=NULL and free memory
*    else set *ppWideChar by new and *pWideCharSize is the size of *ppWideChar
*    return value success >= 0 number of bytes in *ppWideChar
*    otherwise negative error code
**********************************************************/
INJECTBASE_API int AnsiToUnicode(char* pChar,wchar_t **pWideChar,int*pWideCharSize);

#endif /*__UNI_ANSI_H__*/
