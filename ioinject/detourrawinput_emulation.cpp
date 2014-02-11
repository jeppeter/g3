

#include <vector>
#define   DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#include "detourmessage.h"
#include <assert.h>
#include "detourdinput.h"
#include "ioinject_thread.h"
#include <injectbase.h>

#define  DEVICE_GET_INFO   0x1
#define  DEVICE_GET_NAMEA  0x2
#define  DEVICE_GET_NAMEW  0x3

#define  KEY_STATE_SIZE   256

#define  RAW_INPUT_MAX_INPUT_SIZE  100

static int st_RawinputEmulationInit=0;

static CRITICAL_SECTION st_EmulationRawinputCS;
static std::vector<RAWINPUT*> st_KeyRawInputVecs;
static RID_DEVICE_INFO* st_KeyRawInputHandle=NULL;
static uint8_t *st_KeyRawInputName=NULL;
static wchar_t *st_KeyRawInputNameWide=NULL;
static int st_KeyLastInfo=DEVICE_GET_INFO;
static int st_KeyRegistered=0;
static HWND st_KeyHwnd=NULL;
static std::vector<RAWINPUT*> st_MouseRawInputVecs;
static RID_DEVICE_INFO* st_MouseRawInputHandle=NULL;
static uint8_t *st_MouseRawInputName=NULL;
static wchar_t *st_MouseRawInputNameWide=NULL;
static int st_MouseLastInfo=DEVICE_GET_INFO;
static int st_MouseRegistered=0;
static HWND st_MouseHwnd=NULL;


static uint16_t st_KeyStateArray[KEY_STATE_SIZE]= {0};
static int st_KeyDownStateArray[KEY_STATE_SIZE]= {0};
static uint16_t st_AsyncKeyStateArray[KEY_STATE_SIZE]= {0};
static int st_KeyLastStateArray[KEY_STATE_SIZE]= {0};
static BYTE st_UcharKeyboardStateArray[KEY_STATE_SIZE]= {0};
static BYTE st_UcharLastKeyDown=0;
static POINT st_RawInputMouseLastPoint= {0,0};

RegisterRawInputDevicesFunc_t RegisterRawInputDevicesNext=RegisterRawInputDevices;
GetRawInputDataFunc_t GetRawInputDataNext=GetRawInputData;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoANext=GetRawInputDeviceInfoA;
GetRawInputDeviceInfoFunc_t GetRawInputDeviceInfoWNext=GetRawInputDeviceInfoW;
GetRawInputDeviceListFunc_t GetRawInputDeviceListNext=GetRawInputDeviceList;
GetKeyStateFunc_t GetKeyStateNext=GetKeyState;
GetAsyncKeyStateFunc_t GetAsyncKeyStateNext=GetAsyncKeyState;
GetKeyboardStateFunc_t GetKeyboardStateNext=GetKeyboardState;

#define  DETOURRAWINPUT_DEBUG_BUFFER_FMT  DEBUG_BUFFER_FMT
#define  DETOURRAWINPUT_DEBUG_INFO      DEBUG_INFO

#define DIK_NULL  0xff


static int st_CodeMapDik[256] =
{
    DIK_A             ,DIK_B              ,DIK_C              ,DIK_D              ,DIK_E              ,  /*5*/
    DIK_F             ,DIK_G              ,DIK_H              ,DIK_I              ,DIK_J              ,  /*10*/
    DIK_K             ,DIK_L              ,DIK_M              ,DIK_N              ,DIK_O              ,  /*15*/
    DIK_P             ,DIK_Q              ,DIK_R              ,DIK_S              ,DIK_T              ,  /*20*/
    DIK_U             ,DIK_V              ,DIK_W              ,DIK_X              ,DIK_Y              ,  /*25*/
    DIK_Z             ,DIK_0              ,DIK_1              ,DIK_2              ,DIK_3              ,  /*30*/
    DIK_4             ,DIK_5              ,DIK_6              ,DIK_7              ,DIK_8              ,  /*35*/
    DIK_9             ,DIK_ESCAPE         ,DIK_MINUS          ,DIK_EQUALS         ,DIK_BACK           ,  /*40*/
    DIK_TAB           ,DIK_LBRACKET       ,DIK_RBRACKET       ,DIK_RETURN         ,DIK_LCONTROL       ,  /*45*/
    DIK_SEMICOLON     ,DIK_APOSTROPHE     ,DIK_GRAVE          ,DIK_LSHIFT         ,DIK_BACKSLASH      ,  /*50*/
    DIK_COMMA         ,DIK_PERIOD         ,DIK_SLASH          ,DIK_RSHIFT         ,DIK_MULTIPLY       ,  /*55*/
    DIK_LMENU         ,DIK_SPACE          ,DIK_CAPITAL        ,DIK_F1             ,DIK_F2             ,  /*60*/
    DIK_F3            ,DIK_F4             ,DIK_F5             ,DIK_F6             ,DIK_F7             ,  /*65*/
    DIK_F8            ,DIK_F9             ,DIK_F10            ,DIK_F11            ,DIK_F12            ,  /*70*/
    DIK_F13           ,DIK_F14            ,DIK_F15            ,DIK_NUMLOCK        ,DIK_SCROLL         ,  /*75*/
    DIK_SUBTRACT      ,DIK_NUMPAD0        ,DIK_NUMPAD1        ,DIK_NUMPAD2        ,DIK_NUMPAD3        ,  /*80*/
    DIK_NUMPAD4       ,DIK_NUMPAD5        ,DIK_NUMPAD6        ,DIK_NUMPAD7        ,DIK_NUMPAD8        ,  /*85*/
    DIK_NUMPAD9       ,DIK_ADD            ,DIK_DECIMAL        ,DIK_OEM_102        ,DIK_KANA           ,  /*90*/
    DIK_ABNT_C1       ,DIK_CONVERT        ,DIK_NOCONVERT      ,DIK_YEN            ,DIK_ABNT_C2        ,  /*95*/
    DIK_NUMPADEQUALS  ,DIK_PREVTRACK      ,DIK_AT             ,DIK_COLON          ,DIK_UNDERLINE      ,  /*100*/
    DIK_KANJI         ,DIK_STOP           ,DIK_AX             ,DIK_UNLABELED      ,DIK_NEXTTRACK      ,  /*105*/
    DIK_NUMPADENTER   ,DIK_RCONTROL       ,DIK_MUTE           ,DIK_CALCULATOR     ,DIK_PLAYPAUSE      ,  /*110*/
    DIK_MEDIASTOP     ,DIK_VOLUMEDOWN     ,DIK_VOLUMEUP       ,DIK_WEBHOME        ,DIK_NUMPADCOMMA    ,  /*115*/
    DIK_DIVIDE        ,DIK_SYSRQ          ,DIK_RMENU          ,DIK_PAUSE          ,DIK_HOME           ,  /*120*/
    DIK_UP            ,DIK_PRIOR          ,DIK_LEFT           ,DIK_RIGHT          ,DIK_END            ,  /*125*/
    DIK_DOWN          ,DIK_NEXT           ,DIK_INSERT         ,DIK_DELETE         ,DIK_LWIN           ,  /*130*/
    DIK_RWIN          ,DIK_APPS           ,DIK_POWER          ,DIK_SLEEP          ,DIK_WAKE           ,  /*135*/
    DIK_WEBSEARCH     ,DIK_WEBFAVORITES   ,DIK_WEBREFRESH     ,DIK_WEBSTOP        ,DIK_WEBFORWARD     ,  /*140*/
    DIK_WEBBACK       ,DIK_MYCOMPUTER     ,DIK_MAIL           ,DIK_MEDIASELECT    ,DIK_NULL           ,  /*144*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*150*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*155*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*160*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*165*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*170*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*175*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*180*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*185*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*190*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*195*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*200*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*205*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*210*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*215*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*220*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*225*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*230*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*235*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*240*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*245*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*250*/
    DIK_NULL          ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,DIK_NULL           ,  /*255*/
    DIK_NULL
};


#define  ASYNC_KEY_PRESSED_STATE   0x8000
#define  ASYNC_KEY_UNPRESSED_STATE 0x7fff
#define  ASYNC_KEY_TOGGLED_STATE   0x1
#define  ASYNC_KEY_UNTOGGLE_STATE  0xfffe

#define  KEY_PRESSED_STATE         0xff80
#define  KEY_UNPRESSED_STATE       0x007f
#define  KEY_TOGGLE_STATE          0x1
#define  KEY_UNTOGGLE_STATE        0xfffe

#define  UCHAR_KEY_PRESSED_STATE   0x80
#define  UCHAR_KEY_UNPRESSED_STATE 0x7f
#define  UCHAR_KEY_TOGGLE_STATE    0x01
#define  UCHAR_KEY_UNTOGGLE_STATE  0xfe

int SetKeyState(UINT vsk,int keydown)
{
    UINT vk[2];
    int vknum=1;
    int i;
    vk[0] = vsk;
    assert(vsk >= 0 && vsk < 256);
    if(vk[0] == 0)
    {
        /*if not return ,just not set*/
        return 0;
    }

    if(vk[0] == VK_LSHIFT ||
            vk[0] == VK_RSHIFT)
    {
        vknum = 2;
        vk[1] = VK_SHIFT;
    }
    else if(vk[0] == VK_LCONTROL ||
            vk[0] == VK_RCONTROL)
    {
        vknum = 2;
        vk[1] = VK_CONTROL;
    }
    else if(vk[0] == VK_LMENU ||
            vk[0] == VK_RMENU)
    {
        vknum = 2;
        vk[1] = VK_MENU;
    }



    EnterCriticalSection(&st_EmulationRawinputCS);
    if(keydown)
    {
        for(i=0; i<vknum; i++)
        {
            st_KeyStateArray[vk[i]] |= KEY_PRESSED_STATE;
            st_UcharKeyboardStateArray[vk[i]] |= UCHAR_KEY_PRESSED_STATE;
            st_KeyDownStateArray[vk[i]] ++;
            st_AsyncKeyStateArray[vk[i]] |= ASYNC_KEY_PRESSED_STATE;
            st_AsyncKeyStateArray[vk[i]] |= ASYNC_KEY_TOGGLED_STATE;
            /*zero means up*/
            if(st_KeyLastStateArray[vk[i]] == 0)
            {
                /*we change the state array for it will from up to down*/
                if(st_KeyStateArray[vk[i]] & KEY_TOGGLE_STATE)
                {
                    st_KeyStateArray[vk[i]] &= KEY_UNTOGGLE_STATE;
                }
                else
                {
                    st_KeyStateArray[vk[i]] |= KEY_TOGGLE_STATE;
                }

                if(st_UcharKeyboardStateArray[vk[i]] & UCHAR_KEY_TOGGLE_STATE)
                {
                    st_UcharKeyboardStateArray[vk[i]] &= UCHAR_KEY_UNTOGGLE_STATE;
                }
                else
                {
                    st_UcharKeyboardStateArray[vk[i]] |= UCHAR_KEY_TOGGLE_STATE;
                }
            }

            st_KeyLastStateArray[vk[i]] = 1;
        }

        if(vsk >= VK_BACK)
        {
            st_UcharLastKeyDown = vsk;
        }
    }
    else
    {
        for(i=0; i<vknum; i++)
        {
            st_KeyStateArray[vk[i]] &= KEY_UNPRESSED_STATE;
            st_UcharKeyboardStateArray[vk[i]] &= UCHAR_KEY_UNPRESSED_STATE;
            st_AsyncKeyStateArray[vk[i]] &= ASYNC_KEY_UNPRESSED_STATE;
            st_AsyncKeyStateArray[vk[i]] |= ASYNC_KEY_TOGGLED_STATE;
            st_KeyLastStateArray[vk[i]] = 0;
        }
        st_UcharLastKeyDown = 0;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return 0;
}


USHORT __InnerGetKeyState(UINT vk)
{
    USHORT uret;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }


    EnterCriticalSection(&st_EmulationRawinputCS);
    uret = st_KeyStateArray[vk];
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return uret;
}


USHORT __InnerGetAsynState(UINT vk)
{
    USHORT uret;
    int expanded=0;
    if(vk >= KEY_STATE_SIZE)
    {
        return 0;
    }
    EnterCriticalSection(&st_EmulationRawinputCS);
    {
        uret = st_AsyncKeyStateArray[vk];
        st_AsyncKeyStateArray[vk] &= ASYNC_KEY_UNTOGGLE_STATE;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return uret;
}


SHORT WINAPI GetKeyStateCallBack(
    int nVirtKey
)
{
    SHORT sret;

    sret = __InnerGetKeyState(nVirtKey);

    //DEBUG_INFO("GetKeyState 0x%08x(%d) sret(0x%08x:%d)\n",nVirtKey,nVirtKey,sret,sret);
    return sret;
}

SHORT WINAPI GetAsyncKeyStateCallBack(
    int vKey
)
{
    SHORT sret;

    sret = __InnerGetAsynState(vKey);

    DEBUG_INFO("GetAsyncKeyState 0x%08x(%d) sret(0x%08x:%d)\n",vKey,vKey,sret,sret);
    return sret;
}


#define   VK_NULL                 0
#define   VK_1                    0x31
#define   VK_2                    0x32
#define   VK_3                    0x33
#define   VK_4                    0x34
#define   VK_5                    0x35
#define   VK_6                    0x36
#define   VK_7                    0x37
#define   VK_8                    0x38
#define   VK_9                    0x39
#define   VK_0                    0x30
#define   VK_A                    0x41
#define   VK_B                    0x42
#define   VK_C                    0x43
#define   VK_D                    0x44
#define   VK_E                    0x45
#define   VK_F                    0x46
#define   VK_G                    0x47
#define   VK_H                    0x48
#define   VK_I                    0x49
#define   VK_J                    0x4a
#define   VK_K                    0x4b
#define   VK_L                    0x4c
#define   VK_M                    0x4d
#define   VK_N                    0x4e
#define   VK_O                    0x4f
#define   VK_P                    0x50
#define   VK_Q                    0x51
#define   VK_R                    0x52
#define   VK_S                    0x53
#define   VK_T                    0x54
#define   VK_U                    0x55
#define   VK_V                    0x56
#define   VK_W                    0x57
#define   VK_X                    0x58
#define   VK_Y                    0x59
#define   VK_Z                    0x5a


static unsigned int st_ScanNumLockVk[256]=
{
    VK_NULL          ,VK_ESCAPE        ,VK_1             ,VK_2             ,VK_3             ,    /* 005 */
    VK_4             ,VK_5             ,VK_6             ,VK_7             ,VK_8             ,    /* 010 */
    VK_9             ,VK_0             ,VK_OEM_MINUS     ,VK_OEM_PLUS      ,VK_BACK          ,    /* 015 */
    VK_TAB           ,VK_Q             ,VK_W             ,VK_E             ,VK_R             ,    /* 020 */
    VK_T             ,VK_Y             ,VK_U             ,VK_I             ,VK_O             ,    /* 025 */
    VK_P             ,VK_OEM_4         ,VK_OEM_6         ,VK_RETURN        ,VK_CONTROL       ,    /* 030 */
    VK_A             ,VK_S             ,VK_D             ,VK_F             ,VK_G             ,    /* 035 */
    VK_H             ,VK_J             ,VK_K             ,VK_L             ,VK_OEM_1         ,    /* 040 */
    VK_OEM_7         ,VK_OEM_3         ,VK_SHIFT         ,VK_OEM_5         ,VK_Z             ,    /* 045 */
    VK_X             ,VK_C             ,VK_V             ,VK_B             ,VK_N             ,    /* 050 */
    VK_M             ,VK_OEM_COMMA     ,VK_OEM_PERIOD    ,VK_OEM_2         ,VK_SHIFT         ,    /* 055 */
    VK_MULTIPLY      ,VK_MENU          ,VK_SPACE         ,VK_CAPITAL       ,VK_F1            ,    /* 060 */
    VK_F2            ,VK_F3            ,VK_F4            ,VK_F5            ,VK_F6            ,    /* 065 */
    VK_F7            ,VK_F8            ,VK_F9            ,VK_F10           ,VK_NUMLOCK       ,    /* 070 */
    VK_SCROLL        ,VK_NUMPAD7       ,VK_NUMPAD8       ,VK_NUMPAD9       ,VK_SUBTRACT      ,    /* 075 */
    VK_NUMPAD4       ,VK_NUMPAD5       ,VK_NUMPAD6       ,VK_ADD           ,VK_NUMPAD1       ,    /* 080 */
    VK_NUMPAD2       ,VK_NUMPAD3       ,VK_NUMPAD0       ,VK_DECIMAL       ,VK_NULL          ,    /* 085 */
    VK_NULL          ,VK_NULL          ,VK_F11           ,VK_F12           ,VK_NULL          ,    /* 090 */
    VK_NULL          ,VK_LWIN          ,VK_RWIN          ,VK_APPS          ,VK_NULL          ,    /* 095 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 100 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 105 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 110 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 115 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 120 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 125 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 130 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 135 */
    VK_SNAPSHOT      ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 140 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 145 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 150 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 155 */
    VK_NULL          ,VK_RETURN        ,VK_CONTROL       ,VK_NULL          ,VK_NULL          ,    /* 160 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 165 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 170 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 175 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 180 */
    VK_NULL          ,VK_DIVIDE        ,VK_NULL          ,VK_NULL          ,VK_MENU          ,    /* 185 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 190 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 195 */
    VK_NULL          ,VK_NULL          ,VK_PRINT         ,VK_NULL          ,VK_HOME          ,    /* 200 */
    VK_UP            ,VK_PRIOR         ,VK_NULL          ,VK_LEFT          ,VK_NULL          ,    /* 205 */
    VK_RIGHT         ,VK_NULL          ,VK_END           ,VK_DOWN          ,VK_NEXT          ,    /* 210 */
    VK_INSERT        ,VK_DELETE        ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 215 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 220 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 225 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 230 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 235 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 240 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 245 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 250 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 255 */
    VK_NULL          ,
};


static unsigned int st_ScanNumNoLockvk[256]=
{
    VK_NULL          ,VK_ESCAPE        ,VK_1             ,VK_2             ,VK_3             ,    /* 005 */
    VK_4             ,VK_5             ,VK_6             ,VK_7             ,VK_8             ,    /* 010 */
    VK_9             ,VK_0             ,VK_OEM_MINUS     ,VK_OEM_PLUS      ,VK_BACK          ,    /* 015 */
    VK_TAB           ,VK_Q             ,VK_W             ,VK_E             ,VK_R             ,    /* 020 */
    VK_T             ,VK_Y             ,VK_U             ,VK_I             ,VK_O             ,    /* 025 */
    VK_P             ,VK_OEM_4         ,VK_OEM_6         ,VK_RETURN        ,VK_CONTROL       ,    /* 030 */
    VK_A             ,VK_S             ,VK_D             ,VK_F             ,VK_G             ,    /* 035 */
    VK_H             ,VK_J             ,VK_K             ,VK_L             ,VK_OEM_1         ,    /* 040 */
    VK_OEM_7         ,VK_OEM_3         ,VK_SHIFT         ,VK_OEM_5         ,VK_Z             ,    /* 045 */
    VK_X             ,VK_C             ,VK_V             ,VK_B             ,VK_N             ,    /* 050 */
    VK_M             ,VK_OEM_COMMA     ,VK_OEM_PERIOD    ,VK_OEM_2         ,VK_SHIFT         ,    /* 055 */
    VK_MULTIPLY      ,VK_MENU          ,VK_SPACE         ,VK_CAPITAL       ,VK_F1            ,    /* 060 */
    VK_F2            ,VK_F3            ,VK_F4            ,VK_F5            ,VK_F6            ,    /* 065 */
    VK_F7            ,VK_F8            ,VK_F9            ,VK_F10           ,VK_NUMLOCK       ,    /* 070 */
    VK_SCROLL        ,VK_HOME          ,VK_UP            ,VK_PRIOR         ,VK_SUBTRACT      ,    /* 075 */
    VK_LEFT          ,VK_CLEAR         ,VK_RIGHT         ,VK_ADD           ,VK_END           ,    /* 080 */
    VK_DOWN          ,VK_NEXT          ,VK_INSERT        ,VK_DELETE        ,VK_NULL          ,    /* 085 */
    VK_NULL          ,VK_NULL          ,VK_F11           ,VK_F12           ,VK_NULL          ,    /* 090 */
    VK_NULL          ,VK_LWIN          ,VK_RWIN          ,VK_APPS          ,VK_NULL          ,    /* 095 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 100 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 105 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 110 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 115 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 120 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 125 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 130 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 135 */
    VK_SNAPSHOT      ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 140 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 145 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 150 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 155 */
    VK_NULL          ,VK_RETURN        ,VK_CONTROL       ,VK_NULL          ,VK_NULL          ,    /* 160 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 165 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 170 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 175 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 180 */
    VK_NULL          ,VK_DIVIDE        ,VK_NULL          ,VK_NULL          ,VK_MENU          ,    /* 185 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 190 */
    VK_NULL          ,VK_NULL          ,VK_PRINT         ,VK_NULL          ,VK_NULL          ,    /* 195 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_HOME          ,    /* 200 */
    VK_UP            ,VK_PRIOR         ,VK_NULL          ,VK_LEFT          ,VK_NULL          ,    /* 205 */
    VK_RIGHT         ,VK_NULL          ,VK_END           ,VK_DOWN          ,VK_NEXT          ,    /* 210 */
    VK_INSERT        ,VK_DELETE        ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 215 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 220 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 225 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 230 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 235 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 240 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 245 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 250 */
    VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,VK_NULL          ,    /* 255 */
    VK_NULL          ,
};

static unsigned char st_VkFlagMap[256]=
{
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*005*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*010*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*015*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*020*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*025*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*030*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*035*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*040*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*045*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*050*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*055*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*060*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*065*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*070*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*075*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*080*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*085*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*090*/
    RI_KEY_MAKE    ,RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_MAKE    ,     /*095*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*100*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*105*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*110*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*115*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*120*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*125*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*130*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*135*/
    RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*140*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*145*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*150*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*155*/
    RI_KEY_MAKE    ,RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*160*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*165*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*170*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*175*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*180*/
    RI_KEY_MAKE    ,RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_E0      ,     /*185*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*190*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*195*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_E0      ,     /*200*/
    RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_E0      ,RI_KEY_MAKE    ,     /*205*/
    RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_E0      ,     /*210*/
    RI_KEY_E0      ,RI_KEY_E0      ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*215*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*220*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*225*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*230*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*235*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*240*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*245*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*250*/
    RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,RI_KEY_MAKE    ,     /*255*/
    RI_KEY_MAKE    ,
};

static unsigned int st_VkMsgMap[256]=
{
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*005*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*010*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*015*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*020*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*025*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*030*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*035*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*040*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*045*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*050*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*055*/
    0x100    ,0x104    ,0x100    ,0x100    ,0x100    ,     /*060*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*065*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*070*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*075*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*080*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*085*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*090*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*095*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*100*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*105*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*110*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*115*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*120*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*125*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*130*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*135*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*140*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*145*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*150*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*155*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*160*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*165*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*170*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*175*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*180*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x104    ,     /*185*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*190*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*195*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*200*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*205*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*210*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*215*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*220*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*225*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*230*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*235*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*240*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*245*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*250*/
    0x100    ,0x100    ,0x100    ,0x100    ,0x100    ,     /*255*/
    0x100
};


static unsigned int st_VkExtMap[256]=
{
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 005 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 010 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 015 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 020 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 025 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 030 */
    0x0    ,0x0    ,0x0    ,0x1    ,0x1    ,    /* 035 */
    0x1    ,0x1    ,0x1    ,0x1    ,0x1    ,    /* 040 */
    0x1    ,0x0    ,0x0    ,0x0    ,0x1    ,    /* 045 */
    0x1    ,0x1    ,0x0    ,0x0    ,0x0    ,    /* 050 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 055 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 060 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 065 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 070 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 075 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 080 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 085 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 090 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 095 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 100 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 105 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 110 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 115 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 120 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 125 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 130 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 135 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 140 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 145 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 150 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 155 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 160 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 165 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 170 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 175 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 180 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 185 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 190 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 195 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 200 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 205 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 210 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 215 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 220 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 225 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 230 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 235 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 240 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 245 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 250 */
    0x0    ,0x0    ,0x0    ,0x0    ,0x0    ,    /* 255 */
    0x0    ,
};

static unsigned char st_VkScancodeMap[256]=
{
    0x00    ,0x01    ,0x02    ,0x03    ,0x04    ,     /*005*/
    0x05    ,0x06    ,0x07    ,0x08    ,0x09    ,     /*010*/
    0x0a    ,0x0b    ,0x0c    ,0x0d    ,0x0e    ,     /*015*/
    0x0f    ,0x10    ,0x11    ,0x12    ,0x13    ,     /*020*/
    0x14    ,0x15    ,0x16    ,0x17    ,0x18    ,     /*025*/
    0x19    ,0x1a    ,0x1b    ,0x1c    ,0x1d    ,     /*030*/
    0x1e    ,0x1f    ,0x20    ,0x21    ,0x22    ,     /*035*/
    0x23    ,0x24    ,0x25    ,0x26    ,0x27    ,     /*040*/
    0x28    ,0x29    ,0x2a    ,0x2b    ,0x2c    ,     /*045*/
    0x2d    ,0x2e    ,0x2f    ,0x30    ,0x31    ,     /*050*/
    0x32    ,0x33    ,0x34    ,0x35    ,0x36    ,     /*055*/
    0x37    ,0x38    ,0x39    ,0x3a    ,0x3b    ,     /*060*/
    0x3c    ,0x3d    ,0x3e    ,0x3f    ,0x40    ,     /*065*/
    0x41    ,0x42    ,0x43    ,0x44    ,0x45    ,     /*070*/
    0x46    ,0x47    ,0x48    ,0x49    ,0x4a    ,     /*075*/
    0x4b    ,0x4c    ,0x4d    ,0x4e    ,0x4f    ,     /*080*/
    0x50    ,0x51    ,0x52    ,0x53    ,0x54    ,     /*085*/
    0x55    ,0x56    ,0x57    ,0x58    ,0x59    ,     /*090*/
    0x5a    ,0x5b    ,0x5c    ,0x5d    ,0x5e    ,     /*095*/
    0x5f    ,0x60    ,0x61    ,0x62    ,0x63    ,     /*100*/
    0x64    ,0x65    ,0x66    ,0x67    ,0x68    ,     /*105*/
    0x69    ,0x6a    ,0x6b    ,0x6c    ,0x6d    ,     /*110*/
    0x6e    ,0x6f    ,0x70    ,0x71    ,0x72    ,     /*115*/
    0x73    ,0x74    ,0x75    ,0x76    ,0x77    ,     /*120*/
    0x78    ,0x79    ,0x7a    ,0x7b    ,0x7c    ,     /*125*/
    0x7d    ,0x7e    ,0x7f    ,0x80    ,0x81    ,     /*130*/
    0x82    ,0x83    ,0x84    ,0x85    ,0x86    ,     /*135*/
    0x37    ,0x88    ,0x89    ,0x8a    ,0x8b    ,     /*140*/
    0x8c    ,0x8d    ,0x8e    ,0x8f    ,0x90    ,     /*145*/
    0x91    ,0x92    ,0x93    ,0x94    ,0x95    ,     /*150*/
    0x96    ,0x97    ,0x98    ,0x99    ,0x9a    ,     /*155*/
    0x9b    ,0x1c    ,0x1d    ,0x9e    ,0x9f    ,     /*160*/
    0xa0    ,0xa1    ,0xa2    ,0xa3    ,0xa4    ,     /*165*/
    0xa5    ,0xa6    ,0xa7    ,0xa8    ,0xa9    ,     /*170*/
    0xaa    ,0xab    ,0xac    ,0xad    ,0xae    ,     /*175*/
    0xaf    ,0xb0    ,0xb1    ,0xb2    ,0xb3    ,     /*180*/
    0xb4    ,0x35    ,0xb6    ,0xb7    ,0x38    ,     /*185*/
    0xb9    ,0xba    ,0xbb    ,0xbc    ,0xbd    ,     /*190*/
    0xbe    ,0xbf    ,0xc0    ,0xc1    ,0xc2    ,     /*195*/
    0xc3    ,0xc4    ,0xc5    ,0xc6    ,0x47    ,     /*200*/
    0x48    ,0x49    ,0xca    ,0x4b    ,0xcc    ,     /*205*/
    0x4d    ,0xce    ,0x4f    ,0x50    ,0x51    ,     /*210*/
    0x52    ,0x53    ,0xd4    ,0xd5    ,0xd6    ,     /*215*/
    0xd7    ,0xd8    ,0xd9    ,0xda    ,0xdb    ,     /*220*/
    0xdc    ,0xdd    ,0xde    ,0xdf    ,0xe0    ,     /*225*/
    0xe1    ,0xe2    ,0xe3    ,0xe4    ,0xe5    ,     /*230*/
    0xe6    ,0xe7    ,0xe8    ,0xe9    ,0xea    ,     /*235*/
    0xeb    ,0xec    ,0xed    ,0xee    ,0xef    ,     /*240*/
    0xf0    ,0xf1    ,0xf2    ,0xf3    ,0xf4    ,     /*245*/
    0xf5    ,0xf6    ,0xf7    ,0xf8    ,0xf9    ,     /*250*/
    0xfa    ,0xfb    ,0xfc    ,0xfd    ,0xfe    ,     /*255*/
    0xff    ,
};



static int  __MapVirtualKeyNoLock(int scancode)
{
    uint16_t numlock;
    int vk;
    numlock  = st_KeyStateArray[VK_NUMLOCK];

    if(numlock & KEY_TOGGLE_STATE)
    {
        vk = st_ScanNumLockVk[scancode];
    }
    else
    {
        vk = st_ScanNumNoLockvk[scancode];
    }
    return vk;
}

int MapVirtualKeyEmulation(int scancode)
{
    int vk = 0;
    if(scancode >= 256)
    {
        return vk;
    }
    EnterCriticalSection(&st_EmulationRawinputCS);
    vk = __MapVirtualKeyNoLock(scancode);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return vk;
}

#define   NO_EXTENDED_KEY     0
#define   EXTENDED_KEY_DOWN   1
#define   EXTENDED_KEY_UP     2

int IsExtendedKey(int vk,int down)
{
    int ret =NO_EXTENDED_KEY;
    return ret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    if(down)
    {
        /*now to test whether it is extended key ,if so ,and it is the first time to press down ,so send the key*/
        if(st_VkExtMap[vk] && (!(st_KeyStateArray[vk] & KEY_PRESSED_STATE)))
        {
            ret = EXTENDED_KEY_DOWN;
        }
        else if(st_VkExtMap[vk] == 0 && (!(st_KeyStateArray[vk] & KEY_PRESSED_STATE)))
        {
            /*not the extended key ,and it is the first time to press down ,so test if the last key is extended key ,so we do this*/
            if(st_VkExtMap[st_UcharLastKeyDown])
            {
                ret = EXTENDED_KEY_UP;
            }

        }
    }
    else
    {
        /*if it is the up state ,so we should test whether it is the extended key ,if so ,we just*/
        if(st_VkExtMap[vk])
        {
            if(st_UcharLastKeyDown == vk)
            {
                ret = EXTENDED_KEY_UP;
            }
        }
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);
    return ret;
}


LONG __InsertKeyboardInput(RAWINPUT* pInput,HWND* pHwnd)
{
    LONG lret=0;
    RAWINPUT *pRemove=NULL;
    RAWKEYBOARD *pKeyboard=NULL;

    pKeyboard = &(pInput->data.keyboard);
    ERROR_INFO("(0x%08x)GetInsert Keyboard MakeCode(0x%04x:%d) Flags(0x%04x) VKey(0x%04x) Message (0x%08x:%d) ExtraInformation(0x%08x:%d)\n",GetTickCount(),
               pKeyboard->MakeCode,pKeyboard->MakeCode,
               pKeyboard->Flags,
               pKeyboard->VKey,
               pKeyboard->Message,pKeyboard->Message,
               pKeyboard->ExtraInformation,pKeyboard->ExtraInformation);

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_KeyRawInputHandle && st_KeyRegistered > 1)
    {
        pInput->header.hDevice = st_KeyRawInputHandle;
        pInput->header.wParam = RIM_INPUT;
        lret =(LONG) st_KeyRawInputHandle;
        if(st_KeyRawInputVecs.size() >= RAW_INPUT_MAX_INPUT_SIZE)
        {
            pRemove = st_KeyRawInputVecs[0];
            st_KeyRawInputVecs.erase(st_KeyRawInputVecs.begin());
        }
        st_KeyRawInputVecs.push_back(pInput);
        *pHwnd = st_KeyHwnd;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(pRemove)
    {
        pKeyboard = &(pRemove->data.keyboard);
        ERROR_INFO("Remove Keyboard MakeCode(0x%04x:%d) Flags(0x%04x) VKey(0x%04x) Message (0x%08x:%d) ExtraInformation(0x%08x:%d)\n",
                   pKeyboard->MakeCode,pKeyboard->MakeCode,
                   pKeyboard->Flags,
                   pKeyboard->VKey,
                   pKeyboard->Message,pKeyboard->Message,
                   pKeyboard->ExtraInformation,pKeyboard->ExtraInformation);
        free(pRemove);
    }
    pRemove = NULL;
    return lret;
}

int __RawInputInsertKeyStruct(int scank,int vk,int flag,int msg,int down)
{
    int ret;
    MSG InputMsg;
    HWND hwnd;
    DWORD lparam;
    RAWINPUT *pKeyInput=NULL;
    pKeyInput = (RAWINPUT*)calloc(1,sizeof(*pKeyInput));
    if(pKeyInput == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }
    pKeyInput->header.dwType = RIM_TYPEKEYBOARD;
    pKeyInput->header.dwSize = sizeof(pKeyInput->header) + sizeof(pKeyInput->data.keyboard);
    pKeyInput->data.keyboard.MakeCode = scank;
    if(down)
    {
        pKeyInput->data.keyboard.Flags = flag;
    }
    else
    {
        pKeyInput->data.keyboard.Flags = (flag | 1);
    }
    pKeyInput->data.keyboard.Message = msg;

    pKeyInput->data.keyboard.Reserved = 0;
    pKeyInput->data.keyboard.VKey = vk;
    pKeyInput->data.keyboard.ExtraInformation = 0;

    ret =  __InsertKeyboardInput(pKeyInput,&hwnd);
    if(ret == 0)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return -ret;
    }
    pKeyInput = NULL;

    lparam = ret;
    InputMsg.hwnd = hwnd;
    InputMsg.message = WM_INPUT;
    InputMsg.wParam = RIM_INPUT;
    InputMsg.lParam = lparam;
    InputMsg.time = GetTickCount();
    InputMsg.pt.x = 0;
    InputMsg.pt.y = 0;

    ret = InsertEmulationMessageQueue(&InputMsg,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    return 0;
fail:
    if(pKeyInput)
    {
        free(pKeyInput);
    }
    pKeyInput = NULL;
    SetLastError(ret);
    return -ret;
}

int __Scan4cCodeInput(int down)
{
    int ret;

    if(down)
    {
        ret = __RawInputInsertKeyStruct(0x1d,0x13,0x4,WM_KEYDOWN,down);
        if(ret == 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Keyboard Not Exist\n");
            goto fail;
        }


        ret = __RawInputInsertKeyStruct(0x45,0xff,0x0,WM_KEYDOWN,down);
        if(ret == 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Keyboard Not Exist\n");
            goto fail;
        }
    }
    else
    {

        ret = __RawInputInsertKeyStruct(0x1d,0x13,0x4,WM_KEYUP,down);
        if(ret == 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Keyboard Not Exist\n");
            goto fail;
        }


        ret = __RawInputInsertKeyStruct(0x45,0xff,0x0,WM_KEYUP,down);
        if(ret == 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Keyboard Not Exist\n");
            goto fail;
        }
    }

    SetKeyState(0x2a,down);

    return 0;

fail:
    /*because we should record the key down and set state at last not at first because some times will give the extended to no*/
    SetKeyState(0x2a,down);
    SetLastError(ret);
    return -ret;
}


int __RawInputInsertKeyboardEvent(LPDEVICEEVENT pDevEvent)
{
    int ret;
    int scank;
    int vk=VK_NULL;
    int down;
    int hasext;
    int putscank;
    int flag;
    int msg;
    int hasinsertvk=0;


    scank = st_CodeMapDik[pDevEvent->event.keyboard.code];
    if(scank >= DIK_NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> code (%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.code);
        goto fail;
    }

    if(pDevEvent->event.keyboard.event != KEYBOARD_EVENT_DOWN &&
            pDevEvent->event.keyboard.event != KEYBOARD_EVENT_UP)
    {
        ret = ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> event (%d) not valid\n",pDevEvent,pDevEvent->event.keyboard.event);
        goto fail;
    }

    DEBUG_INFO("EventCode(0x%02x) scancode 0x%02x %s\n",pDevEvent->event.keyboard.code,scank,pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN ? "down" : "up");

    down = pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN ? 1 : 0;
    if(scank == 0x4c)
    {
        return __Scan4cCodeInput(down);
    }

    vk = MapVirtualKeyEmulation(scank);
    if(vk == VK_NULL)
    {
        goto fail;
    }

    /*now we should first check whether we should send for */
    if(down)
    {
        hasext = IsExtendedKey(vk,down);
        if(hasext == EXTENDED_KEY_DOWN)
        {
            /*now first to send the extend code*/
            ret = __RawInputInsertKeyStruct(0x2a,0xff,0x2,WM_KEYDOWN,down);
            if(ret < 0)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("<0x%p> Insert Extended down Error(%d)\n",pDevEvent,ret);
                goto fail;
            }
        }
        else if(hasext == EXTENDED_KEY_UP)
        {
            ret = __RawInputInsertKeyStruct(0x2a,0xff,0x2,WM_KEYUP,0);
            if(ret < 0)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("<0x%p> Insert Extended up Error(%d)\n",pDevEvent,ret);
                goto fail;
            }
        }

        putscank = st_VkScancodeMap[scank];
        msg = st_VkMsgMap[scank];
        flag = st_VkFlagMap[scank];

        if(vk == VK_NULL)
        {
            assert(hasext == 0);
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> code 0x%08x:%d down vk error\n",pDevEvent,scank,scank);
            goto fail;
        }

        ret= __RawInputInsertKeyStruct(putscank,vk,flag,msg,down);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("<0x%p> Insert key Error(%d)\n",pDevEvent,ret);
            goto fail;
        }

    }
    else
    {
        putscank = st_VkScancodeMap[scank];
        vk = MapVirtualKeyEmulation(scank);
        flag = st_VkFlagMap[scank];

        if(vk == VK_NULL)
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> code 0x%08x:%d up vk error\n",pDevEvent,scank,scank);
            goto fail;
        }

        ret= __RawInputInsertKeyStruct(putscank,vk,flag,WM_KEYUP,down);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("<0x%p> Insert key Error(%d)\n",pDevEvent,ret);
            goto fail;
        }

        hasext = IsExtendedKey(vk,down);
        if(hasext == EXTENDED_KEY_UP)
        {
            ret = __RawInputInsertKeyStruct(0x2a,0xff,0x2,WM_KEYUP,down);
            if(ret < 0)
            {
                ret = LAST_ERROR_CODE();
                ERROR_INFO("<0x%p> Insert Extend Error(%d)\n",pDevEvent,ret);
                goto fail;
            }
        }
    }

    /*if we are not successful in the insertkeyboardinput ,we can record the keydown or keyup event*/
    SetKeyState(vk,pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN ? 1 : 0);
    hasinsertvk = 1;


    return 0;
fail:
    if(vk != VK_NULL && hasinsertvk == 0)
    {
        /*this put here  because the */
        SetKeyState(vk,pDevEvent->event.keyboard.event == KEYBOARD_EVENT_DOWN ? 1 : 0);
        hasinsertvk = 1;
    }
    assert(ret > 0);
    SetLastError(ret);
    return -ret;
}

LONG __InsertMouseInput(RAWINPUT * pInput,HWND *pHwnd)
{
    LONG lret=0;
    RAWINPUT *pRemove=NULL;
    RAWMOUSE *pMouse=NULL;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_MouseRawInputHandle && st_MouseRegistered > 1)
    {
        lret = (LONG) st_MouseRawInputHandle;
        pInput->header.hDevice = (HANDLE) st_MouseRawInputHandle;
        pInput->header.wParam = RIM_INPUT;
        if(st_MouseRawInputVecs.size() >= RAW_INPUT_MAX_INPUT_SIZE)
        {
            pRemove = st_MouseRawInputVecs[0];
            st_MouseRawInputVecs.erase(st_MouseRawInputVecs.begin());
        }
        st_MouseRawInputVecs.push_back(pInput);
        *pHwnd = st_MouseHwnd;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pRemove)
    {
        pMouse = &(pRemove->data.mouse);
        ERROR_INFO("Remove Mouse Input Flags(0x%08x:%d) usButtonData(%d)  x:y(%d:%d)\n",
                   pMouse->usFlags,pMouse->usFlags,
                   pMouse->usButtonData,pMouse->lLastX,pMouse->lLastY);
        free(pRemove);
    }
    pRemove = NULL;
    return lret;
}

int __RawInputInsertMouseEvent(LPDEVICEEVENT pDevEvent)
{
    RAWINPUT* pMouseInput=NULL,*pMouseAddition=NULL;

    int ret;
    LONG lparam;
    MSG InputMsg= {0};
    HWND hwnd;
    POINT pt;
    UINT vk=0;
    int down=0;
    int movx,movy;


    pMouseInput = (RAWINPUT*)calloc(1,sizeof(*pMouseInput));
    if(pMouseInput == NULL)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    pMouseInput->header.dwType = RIM_TYPEMOUSE;
    pMouseInput->header.dwSize = sizeof(pMouseInput->header) + sizeof(pMouseInput->data.mouse);

    pMouseInput->data.mouse.usFlags = MOUSE_MOVE_RELATIVE;
    pMouseInput->data.mouse.ulButtons = 0;
    BaseGetMousePointAbsolution(&pt);
    if(pDevEvent->event.mouse.code == MOUSE_CODE_MOUSE)
    {
        /*no buttons push*/
        if(pDevEvent->event.mouse.event == MOUSE_EVNET_MOVING)
        {
            movx = pDevEvent->event.mouse.x;
            movy = pDevEvent->event.mouse.y;
            EnterCriticalSection(&st_EmulationRawinputCS);
            if((st_RawInputMouseLastPoint.x + movx) != pt.x)
            {
                movx = pt.x - st_RawInputMouseLastPoint.x;
                ERROR_INFO("Adjust rawinput RelMouseX %d => %d\n",pDevEvent->event.mouse.x,movx);
            }
            if((st_RawInputMouseLastPoint.y + movy) != pt.y)
            {
                movy = pt.y - st_RawInputMouseLastPoint.y;
                ERROR_INFO("Adjust rawinput RelMouseY %d => %d\n",pDevEvent->event.mouse.y,movy);
            }
            LeaveCriticalSection(&st_EmulationRawinputCS);
            pMouseInput->data.mouse.usButtonFlags = 0;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = movx;
            pMouseInput->data.mouse.lLastY = movy;
            pMouseInput->data.mouse.ulExtraInformation = 0;

        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_ABS_MOVING)
        {
            if(pDevEvent->event.mouse.x == IO_MOUSE_RESET_X && pDevEvent->event.mouse.y == IO_MOUSE_RESET_Y)
            {
                /*now first to move the upper top-left point*/
                pMouseInput->data.mouse.usButtonFlags = 0;
                pMouseInput->data.mouse.usButtonData = 0;
                pMouseInput->data.mouse.ulRawButtons = 0;
                pMouseInput->data.mouse.lLastX = IO_MOUSE_RESET_X_MOV;
                pMouseInput->data.mouse.lLastY = IO_MOUSE_RESET_Y_MOV;
                pMouseInput->data.mouse.ulExtraInformation = 0;


                pMouseAddition = calloc(1,sizeof(*pMouseAddition));
                if(pMouseAddition == NULL)
                {
                    ret = GETERRNO();
                    goto fail;
                }

                /*now to give header ok*/
                pMouseAddition->header.dwType = RIM_TYPEMOUSE;
                pMouseAddition->header.dwSize = sizeof(pMouseAddition->header) + sizeof(pMouseAddition->data.mouse);
                pMouseAddition->data.mouse.usFlags = MOUSE_MOVE_RELATIVE;
                pMouseAddition->data.mouse.ulButtons = 0;


                /*to set for the reset ok*/
                pMouseAddition->data.mouse.usButtonFlags = 0;
                pMouseAddition->data.mouse.usButtonData = 0;
                pMouseAddition->data.mouse.ulRawButtons = 0;
                pMouseAddition->data.mouse.lLastX = pt.x;
                pMouseAddition->data.mouse.lLastY = pt.y;
                pMouseAddition->data.mouse.ulExtraInformation = 0;
            }
            else
            {
                /*now it is ok just move for the absolution poition*/
                movx = 0;
                movy = 0;
                EnterCriticalSection(&st_EmulationRawinputCS);
                if((st_RawInputMouseLastPoint.x) != pt.x)
                {
                    movx = pt.x - st_RawInputMouseLastPoint.x;
                    ERROR_INFO("Adjust rawinput AbsMouseX %d => %d\n",st_RawInputMouseLastPoint.x,pt.x);
                }
                if((st_RawInputMouseLastPoint.y) != pt.y)
                {
                    movy = pt.y - st_RawInputMouseLastPoint.y;
                    ERROR_INFO("Adjust rawinput AbsMouseY %d => %d\n",st_RawInputMouseLastPoint.y,pt.y);
                }
                LeaveCriticalSection(&st_EmulationRawinputCS);
                pMouseInput->data.mouse.usButtonFlags = 0;
                pMouseInput->data.mouse.usButtonData = 0;
                pMouseInput->data.mouse.ulRawButtons = 0;
                pMouseInput->data.mouse.lLastX = movx;
                pMouseInput->data.mouse.lLastY = movy;
                pMouseInput->data.mouse.ulExtraInformation = 0;
            }
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_SLIDE)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_WHEEL;
            pMouseInput->data.mouse.usButtonData = pDevEvent->event.mouse.x;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_LEFTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_DOWN;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_LBUTTON;
            down = 1;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_UP;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_LBUTTON;
            down = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_RIGHTBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_DOWN;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_RBUTTON;
            down = 1;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_UP;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_RBUTTON;
            down = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else if(pDevEvent->event.mouse.code == MOUSE_CODE_MIDDLEBUTTON)
    {
        if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYDOWN)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_DOWN;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_MBUTTON;
            down = 1;
        }
        else if(pDevEvent->event.mouse.event == MOUSE_EVENT_KEYUP)
        {
            pMouseInput->data.mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_UP;
            pMouseInput->data.mouse.usButtonData = 0;
            pMouseInput->data.mouse.ulRawButtons = 0;
            pMouseInput->data.mouse.lLastX = 0;
            pMouseInput->data.mouse.lLastY = 0;
            pMouseInput->data.mouse.ulExtraInformation = 0;
            vk = VK_MBUTTON;
            down = 0;
        }
        else
        {
            ret = ERROR_INVALID_PARAMETER;
            ERROR_INFO("<0x%p> Mouse invalid code(%d) event(%d)\n",pDevEvent,pDevEvent->event.mouse.code,
                       pDevEvent->event.mouse.event);
            goto fail;
        }
    }
    else
    {
        ret =ERROR_INVALID_PARAMETER;
        ERROR_INFO("<0x%p> Mouse Invalid Input event(%d) code(%d)\n",pDevEvent,pDevEvent->event.mouse.event,
                   pDevEvent->event.mouse.code);
        goto fail;
    }

    EnterCriticalSection(&st_EmulationRawinputCS);
    /*to set for the last point */
    st_RawInputMouseLastPoint = pt;
    LeaveCriticalSection(&st_EmulationRawinputCS);

    lparam = __InsertMouseInput(pMouseInput,&hwnd);
    if(lparam == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        //ERROR_INFO("Insert Mouse Not Exist\n");
        goto fail;
    }
    pMouseInput = NULL;



    InputMsg.hwnd = hwnd ;
    InputMsg.message = WM_INPUT;
    InputMsg.wParam = RIM_INPUT;
    InputMsg.lParam = lparam;
    InputMsg.time = GetTickCount();
    InputMsg.pt.x = 0;
    InputMsg.pt.y = 0;
    //DEBUG_INFO("Message (0x%08x:%d) wparam (0x%08x:%d) lparam (0x%08x:%d)\n",
    //           InputMsg.message,
    //           InputMsg.wParam,InputMsg.wParam,
    //           InputMsg.lParam,InputMsg.lParam);

    ret = InsertEmulationMessageQueue(&InputMsg,1);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        goto fail;
    }

    if(pMouseAddition)
    {
        hwnd = NULL;
        lparam = __InsertMouseInput(pMouseAddition,&hwnd);
        if(lparam == 0)
        {
            ret = ERROR_DEV_NOT_EXIST;
            goto fail;
        }
        pMouseAddition = NULL;

        InputMsg.hwnd = hwnd ;
        InputMsg.message = WM_INPUT;
        InputMsg.wParam = RIM_INPUT;
        InputMsg.lParam = lparam;
        InputMsg.time = GetTickCount();
        InputMsg.pt.x = 0;
        InputMsg.pt.y = 0;
        //DEBUG_INFO("Message (0x%08x:%d) wparam (0x%08x:%d) lparam (0x%08x:%d)\n",
        //           InputMsg.message,
        //           InputMsg.wParam,InputMsg.wParam,
        //           InputMsg.lParam,InputMsg.lParam);

        ret = InsertEmulationMessageQueue(&InputMsg,1);
        if(ret < 0)
        {
            ret = GETERRNO();
            goto fail;
        }

    }


    SetKeyState(vk,down);

    return 0;
fail:
    assert(ret > 0);
    if(pMouseAddition)
    {
        free(pMouseAddition);
    }
    pMouseAddition = NULL;
    if(pMouseInput)
    {
        free(pMouseInput);
    }
    pMouseInput = NULL;
    SetLastError(ret);
    return -ret;

}


int RawInputEmulationInit(LPVOID pParam,LPVOID pInput)
{
    int ret = -1;
    RAWINPUT *pRawInput=NULL;
    if(st_RawinputEmulationInit)
    {
        ret = 0;
        EnterCriticalSection(&st_EmulationRawinputCS);
        while(st_MouseRawInputVecs.size() > 0)
        {
            assert(pRawInput == NULL);
            pRawInput = st_MouseRawInputVecs[0];
            st_MouseRawInputVecs.erase(st_MouseRawInputVecs.begin());
            free(pRawInput);
            pRawInput = NULL;
        }

        while(st_KeyRawInputVecs.size() > 0)
        {
            assert(pRawInput == NULL);
            pRawInput = st_KeyRawInputVecs[0];
            st_KeyRawInputVecs.erase(st_KeyRawInputVecs.begin());
            free(pRawInput);
            pRawInput = NULL;
        }
        ZeroMemory(st_KeyStateArray,sizeof(st_KeyStateArray));
        ZeroMemory(st_KeyLastStateArray,sizeof(st_KeyLastStateArray));
        ZeroMemory(st_AsyncKeyStateArray,sizeof(st_AsyncKeyStateArray));
        ZeroMemory(st_UcharKeyboardStateArray,sizeof(st_UcharKeyboardStateArray));
        st_UcharLastKeyDown = 0;
        LeaveCriticalSection(&st_EmulationRawinputCS);
    }
    return ret;
}

int IsRawInputKeyboardRegistered()
{
    int ret = 0;
    if(st_RawinputEmulationInit)
    {
        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_KeyRegistered > 1)
        {
            ret = 1;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);
    }
    return ret;
}

int IsRawInputMouseRegistered()
{
    int ret = 0;
    if(st_RawinputEmulationInit)
    {
        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_MouseRegistered > 1)
        {
            ret = 1;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);
    }
    return ret;
}

static int RawInputEmulationInsertEventList(LPVOID pParam,LPVOID pInput)
{
    LPDEVICEEVENT pDevEvent = (LPDEVICEEVENT)pInput;
    int ret;
    if(pDevEvent->devtype == DEVICE_TYPE_KEYBOARD)
    {
        //DEBUG_INFO("Input Keyboard Event\n");
        return __RawInputInsertKeyboardEvent(pDevEvent);
    }
    else if(pDevEvent->devtype == DEVICE_TYPE_MOUSE)
    {
        return __RawInputInsertMouseEvent(pDevEvent);
    }
    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("<0x%p>devtype %d not supported\n",pDevEvent,pDevEvent->devtype);
    SetLastError(ret);
    return -ret;

}

void __UnRegisterKeyboardHandle(HWND hwnd)
{
    RID_DEVICE_INFO *pKeyboardInfo=NULL;
    uint8_t *pKeyName=NULL;
    wchar_t *pKeyUnicode=NULL;
    std::vector<RAWINPUT*> removekeyrawinput;
    RAWINPUT *pRemoveInput=NULL;
    int cleared=0;
    EnterCriticalSection(&st_EmulationRawinputCS);
    pKeyboardInfo = st_KeyRawInputHandle;
    pKeyName = st_KeyRawInputName;
    pKeyUnicode = st_KeyRawInputNameWide;
    if(pKeyboardInfo && st_KeyRegistered > 0)
    {
        st_KeyRegistered -- ;
    }
    if(st_KeyRegistered == 0 && pKeyboardInfo)
    {
        st_KeyRawInputHandle = NULL;
        st_KeyRawInputName = NULL;
        st_KeyRawInputNameWide = NULL;
        removekeyrawinput = st_KeyRawInputVecs;
        st_KeyHwnd = NULL;
        st_KeyRawInputVecs.clear();
        cleared = 1;
    }
    if(hwnd == st_KeyHwnd)
    {
        st_KeyHwnd = NULL;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(cleared == 0)
    {
        return ;
    }

    while(removekeyrawinput.size() > 0)
    {
        pRemoveInput = removekeyrawinput[0];
        removekeyrawinput.erase(removekeyrawinput.begin());
        free(pRemoveInput);
        pRemoveInput = NULL;
    }
    if(pKeyboardInfo)
    {
        free(pKeyboardInfo);
    }
    pKeyboardInfo = NULL;
    if(pKeyName)
    {
        free(pKeyName);
    }
    pKeyName = NULL;
    if(pKeyUnicode)
    {
        free(pKeyUnicode);
    }
    pKeyUnicode = NULL;
    return ;
}

HANDLE __RegisterKeyboardHandle(HWND hwnd)
{
    RID_DEVICE_INFO *pKeyboardInfo=NULL,*pAllocInfo=NULL;
    uint8_t *pKeyName=NULL;
    wchar_t *pKeyUnicode=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pKeyboardInfo = st_KeyRawInputHandle;
    if(pKeyboardInfo)
    {
        st_KeyRegistered ++;
    }
    if(pKeyboardInfo && hwnd)
    {
        st_KeyHwnd = hwnd;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pKeyboardInfo == NULL)
    {
        pAllocInfo = (RID_DEVICE_INFO*)calloc(1,sizeof(*pAllocInfo));
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pKeyName = (uint8_t*)calloc(256,1);
        if(pKeyName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s((char*)pKeyName,256,"\\\\?\\KeyBoard_Emulate",_TRUNCATE);
        pKeyUnicode = (wchar_t*)calloc(256,2);
        if(pKeyUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pKeyName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pKeyUnicode,256,L"\\\\?\\KeyBoard_Emulate",_TRUNCATE);

        pAllocInfo->cbSize = sizeof(pAllocInfo->cbSize) + sizeof(pAllocInfo->dwType) + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEKEYBOARD;
        pAllocInfo->keyboard.dwType = 81;
        pAllocInfo->keyboard.dwSubType = 0;
        pAllocInfo->keyboard.dwKeyboardMode = 1;
        pAllocInfo->keyboard.dwNumberOfFunctionKeys = 12;
        pAllocInfo->keyboard.dwNumberOfIndicators = 3;
        pAllocInfo->keyboard.dwNumberOfKeysTotal = 101;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_KeyRawInputHandle == NULL)
        {
            inserted = 1;
            st_KeyRawInputHandle = pAllocInfo;
            pKeyboardInfo = pAllocInfo;
            st_KeyRawInputName = pKeyName;
            st_KeyRawInputNameWide = pKeyUnicode;
        }
        else
        {
            pKeyboardInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        st_KeyRegistered ++;
        if(hwnd)
        {
            st_KeyHwnd = hwnd;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pKeyboardInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
            free(pKeyName);
            pKeyName = NULL;
            free(pKeyUnicode);
            pKeyUnicode = NULL;
        }
    }

    return pKeyboardInfo;
}


void __UnRegisterMouseHandle(HWND hwnd)
{
    RID_DEVICE_INFO *pMouseInfo=NULL;
    uint8_t *pMouseName=NULL;
    wchar_t *pMouseUnicode=NULL;
    std::vector<RAWINPUT*> removemouserawinput;
    RAWINPUT *pRemoveInput=NULL;
    int cleared = 0;
    EnterCriticalSection(&st_EmulationRawinputCS);
    pMouseInfo = st_MouseRawInputHandle;
    pMouseName = st_MouseRawInputName;
    pMouseUnicode = st_MouseRawInputNameWide;
    if(pMouseInfo && st_MouseRegistered > 0)
    {
        st_MouseRegistered -- ;
    }
    if(st_MouseRegistered == 0 && pMouseInfo)
    {
        st_MouseRawInputHandle = NULL;
        st_MouseRawInputName = NULL;
        st_MouseRawInputNameWide = NULL;
        removemouserawinput = st_MouseRawInputVecs;
        st_MouseHwnd = NULL;
        st_MouseRawInputVecs.clear();
        cleared = 1;
    }
    if(hwnd == st_MouseHwnd)
    {
        st_MouseHwnd = NULL;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(cleared == 0)
    {
        return ;
    }

    while(removemouserawinput.size() > 0)
    {
        pRemoveInput = removemouserawinput[0];
        removemouserawinput.erase(removemouserawinput.begin());
        free(pRemoveInput);
        pRemoveInput = NULL;
    }
    if(pMouseInfo)
    {
        free(pMouseInfo);
    }
    pMouseInfo = NULL;
    if(pMouseName)
    {
        free(pMouseName);
    }
    pMouseName = NULL;
    if(pMouseUnicode)
    {
        free(pMouseUnicode);
    }
    pMouseUnicode = NULL;
    return ;
}

HANDLE __RegisterMouseHandle(HWND hwnd)
{
    RID_DEVICE_INFO *pMouseInfo=NULL,*pAllocInfo=NULL;
    uint8_t *pMouseName=NULL;
    wchar_t *pMouseUnicode=NULL;
    int inserted = 0,ret;


    EnterCriticalSection(&st_EmulationRawinputCS);
    pMouseInfo = st_MouseRawInputHandle;
    if(pMouseInfo)
    {
        st_MouseRegistered ++;
    }
    if(pMouseInfo && hwnd)
    {
        st_MouseHwnd = hwnd;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(pMouseInfo == NULL)
    {
        pAllocInfo = (RID_DEVICE_INFO*)calloc(1,sizeof(*pAllocInfo));
        if(pAllocInfo == NULL)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return NULL;
        }

        pMouseName = (uint8_t*)calloc(256,1);
        if(pMouseName == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            SetLastError(ret);
            return NULL;
        }
        strncpy_s((char*)pMouseName,256,"\\\\?\\Mouse_Emulate",_TRUNCATE);
        pMouseUnicode = (wchar_t*)calloc(256,2);
        if(pMouseUnicode == NULL)
        {
            ret = LAST_ERROR_CODE();
            free(pAllocInfo);
            free(pMouseName);
            SetLastError(ret);
            return NULL;
        }
        wcsncpy_s(pMouseUnicode,256,L"\\\\?\\Mouse_Emulate",_TRUNCATE);

        pAllocInfo->cbSize = sizeof(pAllocInfo->cbSize) + sizeof(pAllocInfo->dwType) + sizeof(pAllocInfo->keyboard);
        pAllocInfo->dwType = RIM_TYPEMOUSE;
        pAllocInfo->mouse.dwId = 2;
        pAllocInfo->mouse.dwNumberOfButtons = 2;
        pAllocInfo->mouse.dwSampleRate = 60;
        pAllocInfo->mouse.fHasHorizontalWheel = 0;

        EnterCriticalSection(&st_EmulationRawinputCS);
        if(st_MouseRawInputHandle == NULL)
        {
            inserted = 1;
            st_MouseRawInputHandle = pAllocInfo;
            pMouseInfo = pAllocInfo;
            st_MouseRawInputName = pMouseName;
            st_MouseRawInputNameWide = pMouseUnicode;
        }
        else
        {
            pMouseInfo = st_KeyRawInputHandle;
            inserted = 0;
        }
        st_MouseRegistered ++;
        if(hwnd)
        {
            st_MouseHwnd = hwnd;
        }
        LeaveCriticalSection(&st_EmulationRawinputCS);

        if(inserted == 0)
        {
            DETOURRAWINPUT_DEBUG_INFO("To Free Keyboard DEV INFO 0x%p\n",pMouseInfo);
            free(pAllocInfo);
            pAllocInfo = NULL;
            free(pMouseName);
            pMouseName = NULL;
            free(pMouseUnicode);
            pMouseUnicode = NULL;
        }
    }

    return pMouseInfo;
}

int __CopyKeyboardDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret=0;
    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_KeyRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_KeyRawInputHandle;
        pRawList->dwType = RIM_TYPEKEYBOARD;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;


}

int __CopyMouseDeviceList(PRAWINPUTDEVICELIST pRawList)
{
    int ret=0;

    EnterCriticalSection(&st_EmulationRawinputCS);

    if(st_MouseRawInputHandle)
    {
        pRawList->hDevice = (HANDLE) st_MouseRawInputHandle;
        pRawList->dwType = RIM_TYPEMOUSE;
    }
    else
    {
        ret = -ERROR_DEV_NOT_EXIST;
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);

    if(ret < 0)
    {
        SetLastError(-ret);
    }
    else
    {
        SetLastError(0);
    }
    return ret;
}


BOOL __GetDeviceInfoNoLock(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        ret = 0 ;
        bret = TRUE;
        st_KeyLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = sizeof(pInfo->cbSize) + sizeof(pInfo->dwType) + sizeof(pInfo->keyboard);
        pInfo->dwType = RIM_TYPEKEYBOARD;
        CopyMemory(&(pInfo->keyboard),&(st_KeyRawInputHandle->keyboard),sizeof(pInfo->keyboard));
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        ret = 0;
        bret = TRUE;
        st_MouseLastInfo = DEVICE_GET_INFO;
        pInfo->cbSize = sizeof(pInfo->cbSize) + sizeof(pInfo->dwType) + sizeof(pInfo->mouse);
        pInfo->dwType = RIM_TYPEMOUSE;
        CopyMemory(&(pInfo->mouse),&(st_MouseRawInputHandle->mouse),sizeof(pInfo->mouse));
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceInfo(HANDLE hDevice,RID_DEVICE_INFO* pInfo)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceInfoNoLock(hDevice,pInfo);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}



BOOL __GetDeviceNameANoLock(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen((const char*)st_KeyRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen((const char*)st_KeyRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen((const char*)st_KeyRawInputName) + 1;
            strncpy_s((char*)pData,*pcbSize,(const char*)st_KeyRawInputName,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEA;
        if(*pcbSize <= strlen((const char*)st_MouseRawInputName) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = strlen((const char*)st_MouseRawInputName) + 1;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = strlen((const char*)st_MouseRawInputName) + 1;
            strncpy_s((char*)pData,*pcbSize,(const char*)st_MouseRawInputName,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}

BOOL __GetDeviceNameA(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameANull(HANDLE hDevice,UINT* pcbSize)
{
    BOOL bret;
    int ret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameANoLock(hDevice,NULL,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        if(ret == ERROR_INSUFFICIENT_BUFFER)
        {
            bret = TRUE;
        }
    }
    return bret;
}

BOOL __GetDeviceNameWNoLock(HANDLE hDevice,void * pData,UINT * pcbSize)
{
    BOOL bret=FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    if(hDevice == (HANDLE)st_KeyRawInputHandle)
    {
        st_KeyLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_KeyRawInputNameWide)*2 +1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_KeyRawInputNameWide)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_KeyRawInputNameWide)*2 + 2;
            wcsncpy_s((wchar_t*)pData,(*pcbSize)/2,st_KeyRawInputNameWide,_TRUNCATE);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        st_MouseLastInfo = DEVICE_GET_NAMEW;
        if(*pcbSize <= (wcslen(st_MouseRawInputNameWide)*2+1) || pData == NULL)
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = wcslen(st_MouseRawInputNameWide)*2 + 2;
        }
        else
        {
            ret = 0;
            bret = TRUE;
            *pcbSize = wcslen(st_MouseRawInputNameWide)*2 + 2;
            wcsncpy_s((wchar_t*)pData,(*pcbSize)/2,st_MouseRawInputNameWide,_TRUNCATE);
        }
    }
    SetLastError(ret);
    return bret;
}

int __GetRawInputDeviceNum()
{
    int retnum=0;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(st_KeyRawInputHandle)
    {
        retnum ++;
    }
    if(st_MouseRawInputHandle)
    {
        retnum ++;
    }
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return retnum;
}


BOOL __GetDeviceNameW(HANDLE hDevice,void* pData, UINT* pcbSize)
{
    BOOL bret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    return bret;
}

BOOL __GetDeviceNameWNull(HANDLE hDevice,UINT* pcbSize)
{
    BOOL bret;
    int ret;
    EnterCriticalSection(&st_EmulationRawinputCS);
    bret = __GetDeviceNameWNoLock(hDevice,NULL,pcbSize);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(!bret)
    {
        ret = LAST_ERROR_CODE();
        if(ret == ERROR_INSUFFICIENT_BUFFER)
        {
            bret = TRUE;
        }
    }
    return bret;
}

int __GetDeviceInfoLast(HANDLE hDevice,void* pData,UINT* pcbSize)
{
    BOOL bret = FALSE;
    int ret=ERROR_DEV_NOT_EXIST;
    int copiedlen=-1;

    EnterCriticalSection(&st_EmulationRawinputCS);
    if(hDevice == (HANDLE) st_KeyRawInputHandle)
    {
        if(st_KeyLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                if(pData == NULL && ret == ERROR_INSUFFICIENT_BUFFER)
                {
                    copiedlen = 0;
                }
            }
            else
            {
                copiedlen = strlen((const char*)pData);
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                if(pData == NULL && ret == ERROR_INSUFFICIENT_BUFFER)
                {
                    copiedlen = 0;
                }
            }
            else
            {
                copiedlen = wcslen((const wchar_t*)pData)*2;
                ret = 0;
            }
        }
        else if(st_KeyLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
                copiedlen = 0;
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                if(pData)
                {
                    bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                    if(!bret)
                    {
                        ret = LAST_ERROR_CODE();
                    }
                    else
                    {
                        copiedlen = sizeof(RID_DEVICE_INFO);
                        ret = 0;
                    }
                }
                else
                {
                    ret = ERROR_INSUFFICIENT_BUFFER;
                    *pcbSize = sizeof(RID_DEVICE_INFO);
                    copiedlen = 0;
                }
            }
        }
        else
        {
            /*we could not arrive here*/
            assert(0!=0);
        }
    }
    else if(hDevice == (HANDLE)st_MouseRawInputHandle)
    {
        if(st_MouseLastInfo == DEVICE_GET_NAMEA)
        {
            bret = __GetDeviceNameANoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                if(pData == NULL && ret == ERROR_INSUFFICIENT_BUFFER)
                {
                    copiedlen = 0;
                }
            }
            else
            {
                copiedlen = strlen((const char*)pData);
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_NAMEW)
        {
            bret = __GetDeviceNameWNoLock(hDevice,pData,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                if(pData == NULL && ret == ERROR_INSUFFICIENT_BUFFER)
                {
                    copiedlen = 0;
                }
            }
            else
            {
                copiedlen = wcslen((const wchar_t*)pData)*2;
                ret = 0;
            }
        }
        else if(st_MouseLastInfo == DEVICE_GET_INFO)
        {
            if(pData == NULL)
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = sizeof(RID_DEVICE_INFO);
                copiedlen = 0;
            }
            else if(*pcbSize != sizeof(RID_DEVICE_INFO))
            {
                ret = ERROR_INVALID_PARAMETER;
            }
            else
            {
                bret = __GetDeviceInfoNoLock(hDevice,(RID_DEVICE_INFO*)pData);
                if(!bret)
                {
                    ret = LAST_ERROR_CODE();
                }
                else
                {
                    copiedlen = sizeof(RID_DEVICE_INFO);
                    ret = 0;
                }
            }
        }
        else
        {
            /*we could not arrive here*/
            assert(0!=0);
        }
    }

    LeaveCriticalSection(&st_EmulationRawinputCS);
    SetLastError(ret);
    return copiedlen;
}

BOOL WINAPI RegisterRawInputDevicesCallBack(
    PCRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize
)
{
    RAWINPUTDEVICE *pDevice=NULL;
    int ret;
    UINT i;
    RID_DEVICE_INFO *pMouse=NULL,*pKeyBoard=NULL;
    HWND mousehwnd=NULL,keyhwnd=NULL;

    DEBUG_INFO("\n");

    /*now first check for device is supported*/
    if(cbSize != sizeof(*pDevice) || pRawInputDevices == NULL || uiNumDevices == 0)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return FALSE;
    }
    DEBUG_INFO("\n");

    for(i=0; i<uiNumDevices; i++)
    {
        pDevice = (RAWINPUTDEVICE*)&(pRawInputDevices[i]);
        /*we only supported keyboard and mouse ,others are not supported*/
        if(pDevice->usUsagePage != 0x1 || (pDevice->usUsage != 0x2 && pDevice->usUsage != 0x6))
        {
            ret = ERROR_NOT_SUPPORTED;
            SetLastError(ret);
            return FALSE;
        }
    }
    DEBUG_INFO("\n");

    for(i=0; i<uiNumDevices; i++)
    {
        pDevice = (RAWINPUTDEVICE*)&(pRawInputDevices[i]);

        if(pDevice->usUsage == 2)
        {
            pMouse =(RID_DEVICE_INFO*) __RegisterMouseHandle(pDevice->hwndTarget);
            if(pMouse == NULL)
            {
                ret = LAST_ERROR_CODE();
                if(pKeyBoard)
                {
                    __UnRegisterKeyboardHandle(keyhwnd);
                }
                pKeyBoard = NULL;
                SetLastError(ret);
                return FALSE;
            }
            keyhwnd = pDevice->hwndTarget;
        }

        if(pDevice->usUsage == 0x6)
        {
            pKeyBoard = (RID_DEVICE_INFO*)__RegisterKeyboardHandle(pDevice->hwndTarget);
            if(pKeyBoard== NULL)
            {
                ret = LAST_ERROR_CODE();
                if(pMouse)
                {
                    __UnRegisterMouseHandle(mousehwnd);
                }
                pMouse = NULL;
                SetLastError(ret);
                return FALSE;
            }
            mousehwnd = pDevice->hwndTarget;
        }
    }
    DEBUG_INFO("\n");

    /*now all is ok  ,so we do not need any more*/
    return TRUE;
}


UINT WINAPI GetRawInputDeviceListCallBack(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize
)
{
    int ret;
    int retnum=0;
    int num=0;

    /*now check for the input */
    if(puiNumDevices == NULL || cbSize != sizeof(*pRawInputDeviceList))
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    num = *puiNumDevices;

    if(st_RawinputEmulationInit == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        SetLastError(ret);
        return (UINT)-1;
    }

    retnum = __GetRawInputDeviceNum();
    if(pRawInputDeviceList == NULL)
    {
        *puiNumDevices = retnum;
        return (UINT) 0;
    }

    if((int)*puiNumDevices < retnum)
    {
        ret=  ERROR_INSUFFICIENT_BUFFER;
        *puiNumDevices = retnum;
        SetLastError(ret);
        return (UINT) -1;
    }

    num = 0;
    /*now we should copy the memory*/
    ret = __CopyKeyboardDeviceList(&(pRawInputDeviceList[num]));
    if(ret >= 0)
    {
        num ++;
    }

    if(num >= retnum)
    {
        ret = ERROR_INSUFFICIENT_BUFFER;
        *puiNumDevices = num + 1;
        SetLastError(ret);
        return (UINT) -1;
    }

    ret = __CopyMouseDeviceList(&(pRawInputDeviceList[num]));
    if(ret >=  0)
    {
        num ++;
    }

    *puiNumDevices = num;

    return num;
}

UINT WINAPI GetRawInputDeviceInfoACallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    BOOL bret;
    int ret;

    if(pcbSize == NULL || hDevice == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    if(st_RawinputEmulationInit == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        SetLastError(ret);
        return (UINT)-1;
    }

    /*now first to copy the data*/
    if(uiCommand == RIDI_DEVICENAME)
    {
        if(pData == NULL)
        {
            bret = __GetDeviceNameANull(hDevice,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                SetLastError(ret);
                return (UINT)-1;
            }
            return 0;
        }
        bret= __GetDeviceNameA(hDevice,pData,pcbSize);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return strlen((const char*)pData);
    }
    else if(uiCommand == RIDI_DEVICEINFO)
    {
        if(*pcbSize != sizeof(RID_DEVICE_INFO))
        {
            ret = ERROR_INVALID_PARAMETER;
            SetLastError(ret);
            return (UINT) -1;
        }

        if(pData == NULL)
        {
            return 0;
        }

        bret= __GetDeviceInfo(hDevice,(RID_DEVICE_INFO*)pData);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return sizeof(RID_DEVICE_INFO);
    }
    else if(uiCommand == RIDI_PREPARSEDDATA)
    {
        ret = __GetDeviceInfoLast(hDevice,pData,pcbSize);
        return (UINT)ret;
    }

    /*now nothing find ,so we should return not supported*/
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return (UINT) -1;
}


UINT WINAPI GetRawInputDeviceInfoWCallBack(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize
)
{
    BOOL bret;
    int ret;

    if(pcbSize == NULL || hDevice == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    if(st_RawinputEmulationInit == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        SetLastError(ret);
        return (UINT)-1;
    }

    /*now first to copy the data*/
    if(uiCommand == RIDI_DEVICENAME)
    {
        if(pData == NULL)
        {
            bret = __GetDeviceNameWNull(hDevice,pcbSize);
            if(!bret)
            {
                ret = LAST_ERROR_CODE();
                SetLastError(ret);
                return (UINT)-1;
            }
            return 0;
        }

        bret= __GetDeviceNameW(hDevice,pData,pcbSize);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return (wcslen((const wchar_t*)pData)*2);
    }
    else if(uiCommand == RIDI_DEVICEINFO)
    {
        if(*pcbSize != sizeof(RID_DEVICE_INFO))
        {
            ret = ERROR_INVALID_PARAMETER;
            *pcbSize = sizeof(RID_DEVICE_INFO);
            SetLastError(ret);
            return (UINT) -1;
        }

        if(pData == NULL)
        {
            return 0;
        }

        bret= __GetDeviceInfo(hDevice,(RID_DEVICE_INFO*)pData);
        if(!bret)
        {
            ret = LAST_ERROR_CODE();
            SetLastError(ret);
            return (UINT) -1;
        }
        return sizeof(RID_DEVICE_INFO);
    }
    else if(uiCommand == RIDI_PREPARSEDDATA)
    {
        ret = __GetDeviceInfoLast(hDevice,pData,pcbSize);
        return (UINT)ret;
    }

    /*now nothing find ,so we should return not supported*/
    ret = ERROR_NOT_SUPPORTED;
    SetLastError(ret);
    return (UINT) -1;
}

UINT __GetRawInputDataNoLock(HRAWINPUT hRawInput,
                             UINT uiCommand,
                             LPVOID pData,
                             PUINT pcbSize,
                             UINT cbSizeHeader)
{
    RAWINPUT *pRawInput=NULL;
    int ret;
    int copied = -1;
    if(pData == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        if(uiCommand == RID_HEADER)
        {
            *pcbSize = sizeof(pRawInput->header);
            copied = 0;
        }
        else if(uiCommand == RID_INPUT)
        {
            if(hRawInput == (HRAWINPUT)st_KeyRawInputHandle)
            {
                *pcbSize = sizeof(pRawInput->header) + sizeof(pRawInput->data.keyboard);
                copied = 0;
            }
            else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
            {
                *pcbSize = sizeof(pRawInput->header) + sizeof(pRawInput->data.mouse);
                copied = 0;
            }
            else
            {
                ret = ERROR_DEV_NOT_EXIST;
            }
        }
        else
        {
            ret = ERROR_NOT_SUPPORTED;
        }
        SetLastError(ret);
        return (UINT) copied;
    }

    if(uiCommand == RID_HEADER)
    {
        if(*pcbSize < sizeof(pRawInput->header))
        {
            ret = ERROR_INSUFFICIENT_BUFFER;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }

        if(cbSizeHeader != sizeof(pRawInput->header))
        {
            ret = ERROR_INVALID_PARAMETER;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }

        if(hRawInput == (HRAWINPUT) st_KeyRawInputHandle)
        {
            if(st_KeyRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No keyboard data for <0x%p>\n",st_KeyRawInputHandle);
                *pcbSize = sizeof(pRawInput->header);
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_KeyRawInputVecs[0];
            *pcbSize = sizeof(pRawInput->header);
            /*not to remove the vectors ,for next read*/
            CopyMemory(pData,&(pRawInput->header),sizeof(pRawInput->header));
            return sizeof(pRawInput->header);
        }
        else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
        {
            if(st_MouseRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No Mouse data for <0x%p>\n",st_MouseRawInputHandle);
                *pcbSize = sizeof(pRawInput->header);
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_MouseRawInputVecs[0];
            *pcbSize = sizeof(pRawInput->header);
            /*not to remove the vectors ,for next read*/
            CopyMemory(pData,&(pRawInput->header),sizeof(pRawInput->header));
            return sizeof(pRawInput->header);
        }
        else
        {
            ret = ERROR_DEV_NOT_EXIST;
            *pcbSize = sizeof(pRawInput->header);
            SetLastError(ret);
            return (UINT) -1;
        }
    }
    else if(uiCommand == RID_INPUT)
    {

        if(hRawInput == (HRAWINPUT) st_KeyRawInputHandle)
        {
            if(*pcbSize < (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard)))
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(cbSizeHeader != (sizeof(pRawInput->header)))
            {
                ret = ERROR_INVALID_PARAMETER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(st_KeyRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No keyboard data for <0x%p>\n",st_KeyRawInputHandle);
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard));
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_KeyRawInputVecs[0];
            /*remove this input handle*/
            st_KeyRawInputVecs.erase(st_KeyRawInputVecs.begin());
            CopyMemory(pData,pRawInput,(sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard)));
            *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard));
            /*free memory ,and not let it memory leak*/
            free(pRawInput);
            pRawInput = NULL;
            return (sizeof(pRawInput->header)+sizeof(pRawInput->data.keyboard));
        }
        else if(hRawInput == (HRAWINPUT) st_MouseRawInputHandle)
        {
            if(*pcbSize < (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse)))
            {
                ret = ERROR_INSUFFICIENT_BUFFER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(cbSizeHeader != (sizeof(pRawInput->header)))
            {
                ret = ERROR_INVALID_PARAMETER;
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            if(st_MouseRawInputVecs.size() == 0)
            {
                ret = ERROR_NO_DATA;
                ERROR_INFO("No Mouse data for <0x%p>\n",st_MouseRawInputHandle);
                *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse));
                SetLastError(ret);
                return (UINT) -1;
            }
            pRawInput = st_MouseRawInputVecs[0];
            st_MouseRawInputVecs.erase(st_MouseRawInputVecs.begin());
            CopyMemory(pData,pRawInput,(sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse)));
            *pcbSize = (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse));
            /*free memory ,and not let it memory leak*/
            free(pRawInput);
            pRawInput = NULL;
            return (sizeof(pRawInput->header)+sizeof(pRawInput->data.mouse));
        }
        else
        {
            ret = ERROR_DEV_NOT_EXIST;
            ERROR_INFO("hRawInput<0x%08x> not exist\n",hRawInput);
            SetLastError(ret);
            return (UINT) -1;
        }
    }
    ret = ERROR_NOT_SUPPORTED;
    ERROR_INFO("Not Supported uiCommand 0x%08x\n",uiCommand);
    SetLastError(ret);
    return (UINT) -1;
}


UINT WINAPI GetRawInputDataCallBack(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader)
{
    UINT uret;
    int ret;
    RAWINPUT *pRawinput=NULL;


    if(pcbSize == NULL || hRawInput == NULL)
    {
        ret = ERROR_INVALID_PARAMETER;
        SetLastError(ret);
        return (UINT) -1;
    }

    if(st_RawinputEmulationInit == 0)
    {
        ret = ERROR_DEV_NOT_EXIST;
        SetLastError(ret);
        return (UINT)-1;
    }

    EnterCriticalSection(&st_EmulationRawinputCS);
    uret = __GetRawInputDataNoLock(hRawInput,uiCommand,pData,pcbSize,cbSizeHeader);
    LeaveCriticalSection(&st_EmulationRawinputCS);
    if(uret != (UINT) -1 && pData)
    {
        pRawinput = (RAWINPUT*)pData;
        if(uiCommand == RID_INPUT && pRawinput->header.dwType == RIM_TYPEKEYBOARD)
        {
            RAWKEYBOARD *pKeyboard=NULL;

            pKeyboard = &(pRawinput->data.keyboard);
            ERROR_INFO("(0x%08x)Insert Keyboard MakeCode(0x%04x:%d) Flags(0x%04x) VKey(0x%04x) Message (0x%08x:%d) ExtraInformation(0x%08x:%d)\n",GetTickCount(),
                       pKeyboard->MakeCode,pKeyboard->MakeCode,
                       pKeyboard->Flags,
                       pKeyboard->VKey,
                       pKeyboard->Message,pKeyboard->Message,
                       pKeyboard->ExtraInformation,pKeyboard->ExtraInformation);
        }
    }
    return uret;
}

BOOL WINAPI GetKeyboardStateCallBack(PBYTE pByte)
{
    UINT i;
    static BYTE st_LastKeyState[256];
    if(pByte == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    EnterCriticalSection(&st_EmulationRawinputCS);
    CopyMemory(pByte,st_UcharKeyboardStateArray,sizeof(st_UcharKeyboardStateArray));
    LeaveCriticalSection(&st_EmulationRawinputCS);

    for(i=0; i<256; i++)
    {
        if(st_LastKeyState[i] != pByte[i])
        {
            DEBUG_INFO("[%d] state(0x%02x) != laststate(0x%02x)\n",
                       i,pByte[i],st_LastKeyState[i]);
            st_LastKeyState[i] = pByte[i];
        }
    }

    return TRUE;
}


int __RawInputDetour(void)
{
    int ret;
    HANDLE hHandle=NULL;
    InitializeCriticalSection(&st_EmulationRawinputCS);
    hHandle = __RegisterKeyboardHandle(NULL);
    if(hHandle == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return 0;
    }

    hHandle = __RegisterMouseHandle(NULL);
    if(hHandle == NULL)
    {
        ret = LAST_ERROR_CODE();
        SetLastError(ret);
        return 0;
    }

    ret = RegisterEventListHandler(RawInputEmulationInsertEventList,NULL,RAWINPUT_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register Rawinput Emulation Error(%d)\n",ret);
        return ret;
    }

    ret = RegisterEventListInit(RawInputEmulationInit,NULL,RAWINPUT_EMULATION_PRIOR);
    if(ret < 0)
    {
        ret = LAST_ERROR_CODE();
        ERROR_INFO("Register Rawinput Init Error(%d)\n",ret);
        return ret;
    }


    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"Before RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"Before GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"Before GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"Before GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"Before GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"Before GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"Before GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    DEBUG_BUFFER_FMT(GetKeyboardStateNext,10,"Before GetKeyboardStateNext(0x%p)",GetKeyboardStateNext);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((PVOID*)&RegisterRawInputDevicesNext,RegisterRawInputDevicesCallBack);
    DetourAttach((PVOID*)&GetRawInputDataNext,GetRawInputDataCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoANext,GetRawInputDeviceInfoACallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceInfoWNext,GetRawInputDeviceInfoWCallBack);
    DetourAttach((PVOID*)&GetRawInputDeviceListNext,GetRawInputDeviceListCallBack);
    DetourAttach((PVOID*)&GetKeyStateNext,GetKeyStateCallBack);
    DetourAttach((PVOID*)&GetAsyncKeyStateNext,GetAsyncKeyStateCallBack);
    DetourAttach((PVOID*)&GetKeyboardStateNext,GetKeyboardStateCallBack);
    DetourTransactionCommit();
    DEBUG_BUFFER_FMT(RegisterRawInputDevicesNext,10,"After RegisterRawInputDeviceNext(0x%p)",RegisterRawInputDevicesNext);
    DEBUG_BUFFER_FMT(GetRawInputDataNext,10,"After GetRawInputDataNext(0x%p)",GetRawInputDataNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoANext,10,"After GetRawInputDeviceInfoANext(0x%p)",GetRawInputDeviceInfoANext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceInfoWNext,10,"After GetRawInputDeviceInfoWNext(0x%p)",GetRawInputDeviceInfoWNext);
    DEBUG_BUFFER_FMT(GetRawInputDeviceListNext,10,"After GetRawInputDeviceListNext(0x%p)",GetRawInputDeviceListNext);
    DEBUG_BUFFER_FMT(GetKeyStateNext,10,"After GetKeyStateNext(0x%p)",GetKeyStateNext);
    DEBUG_BUFFER_FMT(GetAsyncKeyStateNext,10,"After GetAsyncKeyStateNext(0x%p)",GetAsyncKeyStateNext);
    DEBUG_BUFFER_FMT(GetKeyboardStateNext,10,"After GetKeyboardStateNext(0x%p)",GetKeyboardStateNext);

    DEBUG_INFO("Rawinput Emulation\n");
    st_RawinputEmulationInit = 1;
    return 0;
}



