/*************************************************************************

x509\routin.h

	-by Miles Chen (stainboyx@hotmail.com) 2009-1-29

*************************************************************************/

#pragma once

#ifdef IMGCAPINJECT_EXPORTS
#ifndef IMGCAPINJECT_API
#define IMGCAPINJECT_API  extern "C" __declspec(dllexport)
#endif
#else  /*IMGCAPINJECT_EXPORTS*/
#ifndef IMGCAPINJECT_API
#define IMGCAPINJECT_API  extern "C" __declspec(dllimport)
#endif
#endif /*IMGCAPINJECT_EXPORTS*/

int Routine();
int Cleanup();




//EXTERN_C __declspec(dllexport) void AceCrash();
IMGCAPINJECT_API int Capture3DBackBuffer(const char* filetosave);
IMGCAPINJECT_API int Capture2DBackBufferD11(const char* filetosave);