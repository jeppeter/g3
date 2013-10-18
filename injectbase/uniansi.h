#ifndef __UNI_ANSI_H__
#define __UNI_ANSI_H__


#ifdef DLL_EXPORT
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC   EXTERN_C __declspec(dllexport)
#endif
#else   /*DLL_EXPORT*/
#ifndef EXPORT_C_FUNC
#define   EXPORT_C_FUNC  EXTERN_C
#endif
#endif   /*DLL_EXPORT*/


/**********************************************************
*    if pWideChar==NULL  will reset *ppChar=NULL and free memory
*    else set *ppChar by new and *pCharSize is the size of *ppChar
*    return value success >= 0 number of bytes in *ppChar
*    otherwise negative error code
**********************************************************/
EXPORT_C_FUNC int UnicodeToAnsi(wchar_t* pWideChar,char** ppChar,int*pCharSize);

/**********************************************************
*    if pChar==NULL  will reset *ppWideChar=NULL and free memory
*    else set *ppWideChar by new and *pWideCharSize is the size of *ppWideChar
*    return value success >= 0 number of bytes in *ppWideChar
*    otherwise negative error code
**********************************************************/
EXPORT_C_FUNC int AnsiToUnicode(char* pChar,wchar_t **pWideChar,int*pWideCharSize);

#endif /*__UNI_ANSI_H__*/
