
#ifndef __DETOUR_DINPUT_H__
#define __DETOUR_DINPUT_H__

#include <Windows.h>
#include <ioinject.h>


#ifdef __cplusplus
extern "C" {
#endif
BOOL DetourDirectInputInit(HMODULE hModule);
void DetourDirectInputFini(HMODULE hModule);

int DetourDirectInputControl(PIO_CAP_CONTROL_t pControl);



#ifdef __cplusplus
};
#endif



#endif /*__DETOUR_DINPUT_H__*/

