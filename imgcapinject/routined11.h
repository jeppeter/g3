
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


extern "C" int RoutineDetourD11(void);
extern "C" void RotineClearD11(void);

IMGCAPINJECT_API int Capture2DBackBufferD11(const char* filetosave);




