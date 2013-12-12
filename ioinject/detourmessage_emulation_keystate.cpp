
static unsigned int st_VirtKeyState[256]={0};
static CRITICAL_SECTION st_EmuKeyStateCS;
static int IsMenuPressed()
{
	return st_VirtKeyState[VK£ßMENU];
}


static int EmulationKeyStateInit(HMODULE hModule)
{
	InitializeCriticalSection(&st_EmuKeyStateCS);
	return 0;
}
