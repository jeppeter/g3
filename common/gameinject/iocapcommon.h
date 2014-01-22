
#ifndef __IO_CAP_COMMON_H__
#define __IO_CAP_COMMON_H__

#include <injectcommon.h>

#define  LEFTBUTTON_IDX      0
#define  RIGHTBUTTON_IDX     1
#define  MIDBUTTON_IDX       2

#define  MOUSE_LEFT_BTN      1
#define  MOUSE_RIGHT_BTN     2
#define  MOUSE_MIDDLE_BTN    3
#define  MOUSE_MIN_BTN       1
#define  MOUSE_MAX_BTN       3


//#define  IOCAP_DEBUG         1
#define  IOCAP_EMULATION     1

typedef enum IO_DEVICE_TYPE
{
    DEVICE_TYPE_KEYBOARD = 0,
    DEVICE_TYPE_MOUSE,
    DEVICE_TYPE_GAMEPAD,
    DEVICE_TYPE_STEERWHEEL,
    DEVICE_TYPE_MAX
} IO_DEVICE_TYPE_t,*PIO_DEVICE_TYPE_t;


typedef enum IO_KEYBOARD_CODE
{
    KEYBOARD_CODE_A = 0,
    KEYBOARD_CODE_B,
    KEYBOARD_CODE_C,
    KEYBOARD_CODE_D,
    KEYBOARD_CODE_E,          /*5*/
    KEYBOARD_CODE_F,
    KEYBOARD_CODE_G,
    KEYBOARD_CODE_H,
    KEYBOARD_CODE_I,
    KEYBOARD_CODE_J,          /*10*/
    KEYBOARD_CODE_K,
    KEYBOARD_CODE_L,
    KEYBOARD_CODE_M,
    KEYBOARD_CODE_N,
    KEYBOARD_CODE_O,          /*15*/
    KEYBOARD_CODE_P,
    KEYBOARD_CODE_Q,
    KEYBOARD_CODE_R,
    KEYBOARD_CODE_S,
    KEYBOARD_CODE_T,          /*20*/
    KEYBOARD_CODE_U,
    KEYBOARD_CODE_V,
    KEYBOARD_CODE_W,
    KEYBOARD_CODE_X,
    KEYBOARD_CODE_Y,          /*25*/
    KEYBOARD_CODE_Z,
    KEYBOARD_CODE_0,
    KEYBOARD_CODE_1,
    KEYBOARD_CODE_2,
    KEYBOARD_CODE_3,          /*30*/
    KEYBOARD_CODE_4,
    KEYBOARD_CODE_5,
    KEYBOARD_CODE_6,
    KEYBOARD_CODE_7,
    KEYBOARD_CODE_8,          /*35*/
    KEYBOARD_CODE_9,
    KEYBOARD_CODE_ESCAPE,
    KEYBOARD_CODE_MINUS,
    KEYBOARD_CODE_EQUALS,         /*equals */
    KEYBOARD_CODE_BACK,           /*back space*/        /*40*/
    KEYBOARD_CODE_TAB,            /**/
    KEYBOARD_CODE_LBRACKET,       /*[*/
    KEYBOARD_CODE_RBRACKET,       /*]*/
    KEYBOARD_CODE_RETURN,
    KEYBOARD_CODE_LCONTROL,                             /*45*/
    KEYBOARD_CODE_SEMICOLON,       /*;*/
    KEYBOARD_CODE_APOSTROPHE,      /*`*/
    KEYBOARD_CODE_GRAVE,           /*'*/
    KEYBOARD_CODE_LSHIFT,
    KEYBOARD_CODE_BACKSLASH,       /*\\*/               /*50*/
    KEYBOARD_CODE_COMMA,           /*,*/
    KEYBOARD_CODE_PERIOD,          /*.*/
    KEYBOARD_CODE_SLASH,           /* / */
    KEYBOARD_CODE_RSHIFT,
    KEYBOARD_CODE_NUM_MULTIPLY,    /* numpad * */       /*55*/
    KEYBOARD_CODE_LALT,
    KEYBOARD_CODE_SPACE,
    KEYBOARD_CODE_CAPITAL,         /*caps lock*/
    KEYBOARD_CODE_F1,
    KEYBOARD_CODE_F2,                                    /*60*/
    KEYBOARD_CODE_F3,
    KEYBOARD_CODE_F4,
    KEYBOARD_CODE_F5,
    KEYBOARD_CODE_F6,
    KEYBOARD_CODE_F7,                                    /*65*/
    KEYBOARD_CODE_F8,
    KEYBOARD_CODE_F9,
    KEYBOARD_CODE_F10,
    KEYBOARD_CODE_F11,
    KEYBOARD_CODE_F12,                                    /*70*/
    KEYBOARD_CODE_F13,
    KEYBOARD_CODE_F14,
    KEYBOARD_CODE_F15,
    KEYBOARD_CODE_NUMLOCK,
    KEYBOARD_CODE_SCROLL,                                 /*75*/
    KEYBOARD_CODE_SUBTRACT,         /*-*/
    KEYBOARD_CODE_NUM_0,
    KEYBOARD_CODE_NUM_1,
    KEYBOARD_CODE_NUM_2,
    KEYBOARD_CODE_NUM_3,                                  /*80*/
    KEYBOARD_CODE_NUM_4,
    KEYBOARD_CODE_NUM_5,
    KEYBOARD_CODE_NUM_6,
    KEYBOARD_CODE_NUM_7,
    KEYBOARD_CODE_NUM_8,                                  /*85*/
    KEYBOARD_CODE_NUM_9,
    KEYBOARD_CODE_NUM_ADD,
    KEYBOARD_CODE_DECIMAL,          /* . on numeric keypad */
    KEYBOARD_CODE_OEM_102,          /* <> or \| on RT 102-key keyboard (Non-U.S.) */
    KEYBOARD_CODE_KANA,             /* (Japanese keyboard)            */    /*90*/
    KEYBOARD_CODE_ABNT_C1,          /* /? on Brazilian keyboard */
    KEYBOARD_CODE_CONVERT,          /* (Japanese keyboard)            */
    KEYBOARD_CODE_NONCONVERT,       /* (Japanese keyboard)            */
    KEYBOARD_CODE_YEN,              /* (Japanese keyboard)            */
    KEYBOARD_CODE_ABNT_C2,          /* Numpad . on Brazilian keyboard */   /*95*/
    KEYBOARD_CODE_NUM_EQUALS,       /* = on numeric keypad (NEC PC98) */
    KEYBOARD_CODE_PREV_TRACK,       /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
    KEYBOARD_CODE_AT,               /*                     (NEC PC98) */
    KEYBOARD_CODE_COLON,            /*                     (NEC PC98) */
    KEYBOARD_CODE_UNDERLINE,        /*                     (NEC PC98) */         /*100*/
    KEYBOARD_CODE_KANJI,            /* (Japanese keyboard)            */
    KEYBOARD_CODE_STOP,             /*                     (NEC PC98) */
    KEYBOARD_CODE_AX,               /*                     (Japan AX) */
    KEYBOARD_CODE_UNLABELED,        /*                        (J3100) */
    KEYBOARD_CODE_NEXT_TRACK,       /* Next Track */                       /*105*/
    KEYBOARD_CODE_NUM_ENTER,
    KEYBOARD_CODE_RCONTROL,
    KEYBOARD_CODE_MUTE,
    KEYBOARD_CODE_CALCULATOR,
    KEYBOARD_CODE_PLAY_PAUSE,                                             /*110*/
    KEYBOARD_CODE_MEDIA_STOP,
    KEYBOARD_CODE_VOLUME_DOWN,
    KEYBOARD_CODE_VOLUME_UP,
    KEYBOARD_CODE_WEB_HOME,
    KEYBOARD_CODE_NUM_COMMA,        /* , on numeric keypad (NEC PC98) */     /*115*/
    KEYBOARD_CODE_NUM_DIVIDE,       /* / on numeric keypad */
    KEYBOARD_CODE_SYSRQ,
    KEYBOARD_CODE_RALT,
    KEYBOARD_CODE_PAUSE,
    KEYBOARD_CODE_HOME,                                                     /*120*/
    KEYBOARD_CODE_UP,
    KEYBOARD_CODE_PRIOR,            /*pageup*/
    KEYBOARD_CODE_LEFT,
    KEYBOARD_CODE_RIGHT,
    KEYBOARD_CODE_END,                                                      /*125*/
    KEYBOARD_CODE_DOWN,
    KEYBOARD_CODE_NEXT,             /*PAGEDOWN*/
    KEYBOARD_CODE_INSERT,
    KEYBOARD_CODE_DELETE,
    KEYBOARD_CODE_LWIN,                                                      /*130*/
    KEYBOARD_CODE_RWIN,
    KEYBOARD_CODE_APPMENU,
    KEYBOARD_CODE_POWER,
    KEYBOARD_CODE_SLEEP,
    KEYBOARD_CODE_WAKE,                                                      /*135*/
    KEYBOARD_CODE_WEB_SEARCH,
    KEYBOARD_CODE_WEB_FAVORITES,
    KEYBOARD_CODE_WEB_REFRESH,
    KEYBOARD_CODE_WEB_STOP,
    KEYBOARD_CODE_WEB_FORWARD,                                                /*140*/
    KEYBOARD_CODE_WEB_BACK,
    KEYBOARD_CODE_MY_COMPUTER,
    KEYBOARD_CODE_MAIL,
    KEYBOARD_CODE_MEDIA_SELECT,

    KEYBOARD_CODE_NULL = 255
} IO_KEYBOARD_CODE_t,*PIO_KEYBOARD_CODE_t;


typedef enum IO_KEYBOARD_EVENT
{
	KEYBOARD_EVENT_DOWN = 0, 
	KEYBOARD_EVENT_UP, 
	KEYBOARD_EVENT_MAX,
} IO_KEYBOARD_EVENT_t,*PIO_KEYBOARD_EVENT_t;

typedef enum IO_MOUSE_CODE
{
	MOUSE_CODE_MOUSE = 0,			//moving
	MOUSE_CODE_LEFTBUTTON, 
	MOUSE_CODE_RIGHTBUTTON, 
	MOUSE_CODE_MIDDLEBUTTON, 
	MOUSE_CODE_MAX,
} IO_MOUSE_CODE_t,*PIO_MOUSE_CODE_t;


typedef enum IO_MOUSE_EVENT
{
	MOUSE_EVENT_KEYDOWN = 0, 
	MOUSE_EVENT_KEYUP, 
	MOUSE_EVNET_MOVING, 
	MOUSE_EVENT_SLIDE,			// SLIDE
	MOUSE_EVENT_ABS_MOVING,     //  absolute position sending ,this is for checking position
	MOUSE_EVENT_MAX,
} IO_MOUSE_EVENT_t,*PIO_MOUSE_EVENT_t;


typedef struct 
{
	int devtype;
	int devid;
	union
	{
		struct
		{
			IO_KEYBOARD_CODE_t code;		// keyboard code
			IO_KEYBOARD_EVENT_t event;		// keyboard event
		} keyboard;

		struct
		{
			IO_MOUSE_CODE_t code;		
			IO_MOUSE_EVENT_t event;		
			int x;			// for start
			int y;			// for add 
		} mouse;	

	} event;
} DEVICEEVENT, *LPDEVICEEVENT;

typedef struct
{
	unsigned long long seqid;
	DEVICEEVENT devevent;
} SEQ_DEVICEEVENT,*LPSEQ_DEVICEEVENT;




#define  IO_INJECT_STOP            0
#define  IO_INJECT_START           1
#define  IO_INJECT_ADD_DEVICE      2
#define  IO_INJECT_REMOVE_DEVICE   3


#define  IO_INJECT_HIDE_CURSOR     101
#define  IO_INJECT_NORMAL_CURSOR   102
#define  IO_INJECT_ENABLE_SET_POS  103
#define  IO_INJECT_DISABLE_SET_POS 104
#define  IO_INJECT_GET_CURSOR_BMP  105

#define  IO_NAME_MAX_SIZE          256


typedef struct
{
    uint32_t opcode;
    uint32_t devtype;
    uint32_t devid;
    uint8_t memsharename[IO_NAME_MAX_SIZE];
    uint32_t memsharesize;
    uint32_t memsharesectsize;                       /*section size*/
    uint32_t memsharenum;
    uint8_t freeevtbasename[IO_NAME_MAX_SIZE];
    uint8_t inputevtbasename[IO_NAME_MAX_SIZE];
} IO_CAP_CONTROL_t,*PIO_CAP_CONTROL_t;



#endif /*__IO_CAP_COMMON_H__*/

