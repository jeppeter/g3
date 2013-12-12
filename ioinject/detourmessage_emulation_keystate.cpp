
static unsigned int st_VirtKeyState[256]={0};
static CRITICAL_SECTION st_EmuKeyStateCS;
static int IsAltPressed()
{
}


static int EmulationKeyStateInit(HMODULE hModule)
{
	InitializeCriticalSection(&st_EmuKeyStateCS);
	return 0;
}
