/*************************************************************************

x509\routin.h

	-by Miles Chen (stainboyx@hotmail.com) 2009-1-29

*************************************************************************/

#pragma once

#include "iris-int.h"

int Routine();
int Cleanup();


//EXTERN_C __declspec(dllexport) void AceCrash();
EXTERN_C __declspec(dllexport) int Capture3DBackBuffer(const char* filetosave);
EXTERN_C __declspec(dllexport) int Capture2DBackBufferD11(const char* filetosave);