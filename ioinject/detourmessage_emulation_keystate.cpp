
static unsigned int st_VirtKeyState[256]= {0};
static int st_CapsLock=0;
static CRITICAL_SECTION st_EmuKeyStateCS;

#define  MAP_CHAR_NULL           0

/*for 00 -- 07  no map char*/

#define  MAP_CHAR_BACKSPACE      0x08
#define  MAP_CHAR_TAB            0x09

static uint8_t st_CapsChar[256] = {
		MAP_CHAR_NULL       ,MAP_CHAR_NULL       ,MAP_CHAR_NULL       ,MAP_CHAR_NULL          ,MAP_CHAR_NULL,            /*5*/
		MAP_CHAR_NULL       ,MAP_CHAR_NULL       ,MAP_CHAR_NULL       ,0x08
	};

static int __IsMenuPressedNoLock()
{
    return st_VirtKeyState[VK£ßMENU];
}


static int __SetVirtualKeyDownNoLock(int vk)
{
    int ret;
    if(vk >= 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    ret = 1;

    if(vk == VK_LMENU || vk == RMENU)
    {
        st_VirtKeyState[VK_MENU] = 1;
        ret ++ ;
    }
    else if(vk == VK_LCONTROL || vk == VK_RCONTROL)
    {
        ret ++;
        st_VirtKeyState[VK_CONTROL] = 1;
    }
    else if(vk == VK_LSHIFT || vk == VK_RSHIFT)
    {
        ret ++ ;
        st_VirtKeyState[VK_SHIFT]  =1;
    }

    st_VirtKeyState[vk] = 1;
    /*now to set the key*/
    return ret;
}

static int __SetVirtualKeyUpNoLock(int vk)
{
    int ret;
    int cnt;

    if(vk >= 256)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return -ret;
    }

    ret = 1;

    if(vk == VK_RMENU || vk == VK_LMENU)
    {
        /*now to test whether just it */
        cnt = st_VirtKeyState[VK_LMENU] + st_VirtKeyState[VK_RMENU];
        if(cnt == 1 && st_VirtKeyState[vk] == 1)
        {
            st_VirtKeyState[VK_MENU] = 0;
            ret ++;
        }
    }
    else if(vk == VK_RSHIFT || vk == VK_LSHIFT)
    {
        cnt = st_VirtKeyState[VK_RSHIFT] + st_VirtKeyState[VK_LSHIFT];
        if(cnt == 1 && st_VirtKeyState[vk] == 1)
        {
            st_VirtKeyState[VK_SHIFT] = 0;
            ret ++;
        }
    }
    else if(vk == VK_RCONTROL || vk == VK_LCONTROL)
    {
        cnt = st_VirtKeyState[VK_RCONTROL] + st_VirtKeyState[VK_LCONTROL];
        if(cnt == 1 && st_VirtKeyState[vk] == 1)
        {
            st_VirtKeyState[VK_CONTROL] =0 ;
            ret ++;
        }
    }
    else if(vk == VK_CAPITAL)
    {
        st_CapsLock ++;
        if(st_CapsLock >= 2)
        {
            st_CapsLock = 0;
        }
    }

    st_VirtKeyState[vk] = 0;
    return ret;
}




static int EmulationKeyStateInit(HMODULE hModule)
{
    InitializeCriticalSection(&st_EmuKeyStateCS);
    return 0;
}
