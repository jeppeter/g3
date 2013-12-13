
static unsigned int st_VirtKeyState[256]= {0};
static int st_CapsLock=0;
static CRITICAL_SECTION st_EmuKeyStateCS;

#define  MAP_CHAR_NULL           0

/*for 00 -- 07  no map char*/

#define  MAP_CHAR_BACKSPACE      0x08
#define  MAP_CHAR_TAB            0x09

#define  MAP_CHAR_ENTER          0x0d

#define  MAP_CHAR_ESCAPE         0x1b

#define  MAP_CHAR_SPACE          0x20

#define  MAP_CHAR_NUMPAD_STAR    0x2a
#define  MAP_CHAR_NUMPAD_PLUS    0x2b
#define  MAP_CHAR_NUMPAD_MINUS   0x2d
#define  MAP_CHAR_MINUS          0x2d
#define  MAP_CHAR_NUMPAD_SLASH   0x2f



#define  MAP_CHAR_0              0x30
#define  MAP_CHAR_1              0x31
#define  MAP_CHAR_2              0x32
#define  MAP_CHAR_3              0x33
#define  MAP_CHAR_4              0x34
#define  MAP_CHAR_5              0x35
#define  MAP_CHAR_6              0x36
#define  MAP_CHAR_7              0x37
#define  MAP_CHAR_8              0x38
#define  MAP_CHAR_9              0x39

#define  MAP_CHAR_COLON          0x3b
#define  MAP_CHAR_EQUAL          0x3d

#define  MAP_CHAR_A              0x41
#define  MAP_CHAR_B              0x42
#define  MAP_CHAR_C              0x43
#define  MAP_CHAR_D              0x44
#define  MAP_CHAR_E              0x45
#define  MAP_CHAR_F              0x46
#define  MAP_CHAR_G              0x47
#define  MAP_CHAR_H              0x48
#define  MAP_CHAR_I              0x49
#define  MAP_CHAR_J              0x4a
#define  MAP_CHAR_K              0x4b
#define  MAP_CHAR_L              0x4c
#define  MAP_CHAR_M              0x4d
#define  MAP_CHAR_N              0x4e
#define  MAP_CHAR_O              0x4f
#define  MAP_CHAR_P              0x50
#define  MAP_CHAR_Q              0x51
#define  MAP_CHAR_R              0x52
#define  MAP_CHAR_S              0x53
#define  MAP_CHAR_T              0x54
#define  MAP_CHAR_U              0x55
#define  MAP_CHAR_V              0x56
#define  MAP_CHAR_W              0x57
#define  MAP_CHAR_X              0x58
#define  MAP_CHAR_Y              0x59
#define  MAP_CHAR_Z              0x5a

#define  MAP_CHAR_LBRACKET       0x5b
#define  MAP_CHAR_BACKSLASH      0x5c
#define  MAP_CHAR_RBRACKET       0x5d
#define  MAP_CHAR_APOSTROPHY     0x60

static uint8_t st_CapsChar[256] =
{
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*5*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_BACKSPACE     ,MAP_CHAR_TAB           ,            /*10*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_ENTER         ,MAP_CHAR_NULL          ,            /*15*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL 	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*20*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL 	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*25*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_ESCAPE        ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*30*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_SPACE         ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*35*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*40*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL 	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*45*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_0             ,MAP_CHAR_1             ,            /*50*/
    MAP_CHAR_2          ,MAP_CHAR_3           ,MAP_CHAR_4             ,MAP_CHAR_5             ,MAP_CHAR_6             ,            /*55*/
    MAP_CHAR_7          ,MAP_CHAR_8           ,MAP_CHAR_9             ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*60*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*65*/
    MAP_CHAR_A          ,MAP_CHAR_B           ,MAP_CHAR_C             ,MAP_CHAR_D             ,MAP_CHAR_E             ,            /*70*/
    MAP_CHAR_F          ,MAP_CHAR_G           ,MAP_CHAR_H             ,MAP_CHAR_I             ,MAP_CHAR_J             ,            /*75*/
    MAP_CHAR_K          ,MAP_CHAR_L           ,MAP_CHAR_M             ,MAP_CHAR_N             ,MAP_CHAR_O             ,            /*80*/
    MAP_CHAR_P          ,MAP_CHAR_Q           ,MAP_CHAR_R             ,MAP_CHAR_S             ,MAP_CHAR_T             ,            /*85*/
    MAP_CHAR_U          ,MAP_CHAR_V           ,MAP_CHAR_W             ,MAP_CHAR_X             ,MAP_CHAR_Y             ,            /*90*/
    MAP_CHAR_Z          ,MAP_CHAR_NULL        ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*95*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL 	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*100*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL 	      ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*105*/
    MAP_CHAR_NULL       ,MAP_CHAR_NUMPAD_STAR ,MAP_CHAR_NUMPAD_PLUS   ,MAP_CHAR_NULL          ,MAP_CHAR_NUMPAD_MINUS  ,            /*110*/
    MAP_CHAR_NULL       ,MAP_CHAR_NUMPAD_SLASH,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*115*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*120*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*125*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*130*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*135*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*140*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*145*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*150*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*155*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*160*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*165*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*170*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*175*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*180*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*185*/
    MAP_CHAR_NULL       ,MAP_CHAR_COLON       ,MAP_CHAR_EQUAL         ,MAP_CHAR_NULL          ,MAP_CHAR_MINUS         ,            /*190*/
    MAP_CHAR_NULL       ,MAP_CHAR_NULL        ,MAP_CHAR_APOSTROPHY    ,MAP_CHAR_NULL          ,MAP_CHAR_NULL          ,            /*195*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*200*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*205*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*210*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*215*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_LBRACKET	  , 		   /*220*/
    MAP_CHAR_BACKSLASH  ,MAP_CHAR_RBRACKET    ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*225*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_APOSTROPHY	  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*230*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*235*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*240*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*245*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*250*/
    MAP_CHAR_NULL		,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  ,MAP_CHAR_NULL		  , 		   /*255*/
    MAP_CHAR_NULL
};

static int __IsMenuPressedNoLock()
{
    return st_VirtKeyState[VK��MENU];
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