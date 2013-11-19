
#ifndef __IO_CAP_COMMON_H__
#define __IO_CAP_COMMON_H__

#include <injectcommon.h>

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
    KEYBOARD_CODE_E,
    KEYBOARD_CODE_F,
    KEYBOARD_CODE_G,
    KEYBOARD_CODE_H,
    KEYBOARD_CODE_I,
    KEYBOARD_CODE_J,
    KEYBOARD_CODE_K,
    KEYBOARD_CODE_L,
    KEYBOARD_CODE_M,
    KEYBOARD_CODE_N,
    KEYBOARD_CODE_O,
    KEYBOARD_CODE_P,
    KEYBOARD_CODE_Q,
    KEYBOARD_CODE_R,
    KEYBOARD_CODE_S,
    KEYBOARD_CODE_T,
    KEYBOARD_CODE_U,
    KEYBOARD_CODE_V,
    KEYBOARD_CODE_W,
    KEYBOARD_CODE_X,
    KEYBOARD_CODE_Y,
    KEYBOARD_CODE_Z,             /*25*/
    KEYBOARD_CODE_0,
    KEYBOARD_CODE_1,
    KEYBOARD_CODE_2,
    KEYBOARD_CODE_3,
    KEYBOARD_CODE_4,
    KEYBOARD_CODE_5,
    KEYBOARD_CODE_6,
    KEYBOARD_CODE_7,
    KEYBOARD_CODE_8,
    KEYBOARD_CODE_9,             /*35*/
    KEYBOARD_CODE_ESCAPE,
    KEYBOARD_CODE_MINUS,
    KEYBOARD_CODE_EQUALS,         /*equals */
    KEYBOARD_CODE_BACK,           /*back space*/     
    KEYBOARD_CODE_TAB,            /**/                   /*40*/
    KEYBOARD_CODE_LBRACKET,       /*[*/
    KEYBOARD_CODE_RBRACKET,       /*]*/
    KEYBOARD_CODE_RETURN,
    KEYBOARD_CODE_LCONTROL,
    KEYBOARD_CODE_SEMICOLON,       /*;*/
    KEYBOARD_CODE_APOSTROPHE,      /*`*/
    KEYBOARD_CODE_GRAVE,           /*'*/
    KEYBOARD_CODE_LSHIFT,
    KEYBOARD_CODE_BACKSLASH,       /*\\*/
    KEYBOARD_CODE_COMMA,           /*,*/                  /*50*/
    KEYBOARD_CODE_PERIOD,          /*.*/
    KEYBOARD_CODE_SLASH,           /* / */
    KEYBOARD_CODE_RSHIFT,          
    KEYBOARD_CODE_NUM_MULTIPLY,    /* numpad * */
    KEYBOARD_CODE_LALT,
    KEYBOARD_CODE_SPACE,
    KEYBOARD_CODE_CAPITAL,         /*caps lock*/
    KEYBOARD_CODE_F1,
    KEYBOARD_CODE_F2,
    KEYBOARD_CODE_F3,                                     /*60*/
    KEYBOARD_CODE_F4,                                      
    KEYBOARD_CODE_F5,
    KEYBOARD_CODE_F6,
    KEYBOARD_CODE_F7,
    KEYBOARD_CODE_F8,
    KEYBOARD_CODE_F9,
    KEYBOARD_CODE_F10,
    KEYBOARD_CODE_F11,
    KEYBOARD_CODE_F12,
    KEYBOARD_CODE_NUMLOCK,                                /*70*/
    KEYBOARD_CODE_SCROLL,
    KEYBOARD_CODE_NUM_7,
    KEYBOARD_CODE_NUM_8,
    KEYBOARD_CODE_NUM_9,
    KEYBOARD_CODE_SUBTRACT,         /*-*/
    KEYBOARD_CODE_NUM_4,
    KEYBOARD_CODE_NUM_5,
    KEYBOARD_CODE_NUM_6,
    KEYBOARD_CODE_NUM_ADD,
    KEYBOARD_CODE_NUM_1,                                   /*80*/
    KEYBOARD_CODE_NUM_2,                                    
    KEYBOARD_CODE_NUM_3,
    KEYBOARD_CODE_NUM_0,
    KEYBOARD_CODE_DECIMAL,          /* . on numeric keypad */
    KEYBOARD_CODE_OEM_102,          /* <> or \| on RT 102-key keyboard (Non-U.S.) */
    KEYBOARD_CODE_F13,
    KEYBOARD_CODE_F14,
    KEYBOARD_CODE_F15,
    KEYBOARD_CODE_KANA,             /* (Japanese keyboard)            */
    KEYBOARD_CODE_ABNT_C1,          /* /? on Brazilian keyboard */             /*90*/
    KEYBOARD_CODE_CONVERT,          /* (Japanese keyboard)            */        
    KEYBOARD_CODE_NONCONVERT,       /* (Japanese keyboard)            */
    KEYBOARD_CODE_YEN,              /* (Japanese keyboard)            */
    KEYBOARD_CODE_ABNT_C2,          /* Numpad . on Brazilian keyboard */
    KEYBOARD_CODE_NUM_EQUALS,       /* = on numeric keypad (NEC PC98) */
    KEYBOARD_CODE_PREV_TRACK,       /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
    KEYBOARD_CODE_AT,               /*                     (NEC PC98) */
    KEYBOARD_CODE_COLON,            /*                     (NEC PC98) */
    KEYBOARD_CODE_UNDERLINE,        /*                     (NEC PC98) */
    KEYBOARD_CODE_KANJI,            /* (Japanese keyboard)            */           /*100*/
    KEYBOARD_CODE_STOP,             /*                     (NEC PC98) */            
    KEYBOARD_CODE_AX,               /*                     (Japan AX) */
    KEYBOARD_CODE_UNLABELED,        /*                        (J3100) */
    KEYBOARD_CODE_NEXT_TRACK,       /* Next Track */
    KEYBOARD_CODE_NUM_ENTER,
    KEYBOARD_CODE_RCONTROL,
    KEYBOARD_CODE_MUTE,
    KEYBOARD_CODE_CALCULATOR,
    KEYBOARD_CODE_PLAY_PAUSE,
    KEYBOARD_CODE_MEDIA_STOP,                                                  /*110*/
    KEYBOARD_CODE_VOLUME_DOWN,                                                
    KEYBOARD_CODE_VOLUME_UP,
    KEYBOARD_CODE_WEB_HOME,
    KEYBOARD_CODE_NUM_COMMA,        /* , on numeric keypad (NEC PC98) */
    KEYBOARD_CODE_NUM_DIVIDE,       /* / on numeric keypad */
    KEYBOARD_CODE_SYSRQ,
    KEYBOARD_CODE_RALT,
    KEYBOARD_CODE_PAUSE,
    KEYBOARD_CODE_HOME,
    KEYBOARD_CODE_UP,                                                           /*120*/
    KEYBOARD_CODE_PRIOR,            /*pageup*/                                
    KEYBOARD_CODE_LEFT,
    KEYBOARD_CODE_RIGHT,
    KEYBOARD_CODE_END,
    KEYBOARD_CODE_DOWN,
    KEYBOARD_CODE_NEXT,             /*PAGEDOWN*/
    KEYBOARD_CODE_INSERT,
    KEYBOARD_CODE_DELETE,
    KEYBOARD_CODE_LWIN,
    KEYBOARD_CODE_RWIN,                                                          /*130*/
    KEYBOARD_CODE_APPMENU,                                                      
    KEYBOARD_CODE_POWER,
    KEYBOARD_CODE_SLEEP,
    KEYBOARD_CODE_WAKE,
    KEYBOARD_CODE_WEB_SEARCH,
    KEYBOARD_CODE_WEB_FAVORITES,
    KEYBOARD_CODE_WEB_REFRESH,
    KEYBOARD_CODE_WEB_STOP,
    KEYBOARD_CODE_WEB_FORWARD,
    KEYBOARD_CODE_WEB_BACK,                                                       /*140*/
    KEYBOARD_CODE_MY_COMPUTER,                                                   
    KEYBOARD_CODE_MAIL,
    KEYBOARD_CODE_MEDIA_SELECT,

   KEYBOARD_CODE_NULL = 255
} IO_KEYBOARD_CODE_t,*PIO_KEYBOARD_CODE_t;


typedef enum IO_KEYBOARD_EVENT
{
	KEYBOARD_EVENT_DOWN = 0, 
	KEYBOARD_EVENT_UP, 
} IO_KEYBOARD_EVENT_t,*PIO_KEYBOARD_EVENT_t;

typedef enum IO_MOUSE_CODE
{
	MOUSE_CODE_MOUSE = 0,			//moving
	MOUSE_CODE_LEFTBUTTON, 
	MOUSE_CODE_RIGHTBUTTON, 
	MOUSE_CODE_MIDDLEBUTTON, 
} IO_MOUSE_CODE_t,*PIO_MOUSE_CODE_t;


typedef enum IO_MOUSE_EVENT
{
	MOUSE_EVENT_KEYDOWN = 0, 
	MOUSE_EVENT_KEYUP, 
	MOUSE_EVNET_MOVING, 
	MOUSE_EVENT_SLIDE,			// SLIDE
} IO_MOUSE_EVENT_t,*PIO_MOUSE_EVENT_t;


typedef struct 
{
	int devtype;
	ptr_t devid;
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





#endif /*__IO_CAP_COMMON_H__*/

