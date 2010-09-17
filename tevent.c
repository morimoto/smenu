/*************************************************************************

                                 tevent.c

Copyright (c) Kuninori Morimoto <morimoto.kuninori@renesas.com>

2009/09  morimoto
*************************************************************************/
#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>

#include "tevent.h"

//======================================================================
//
//
//                      static function
//
//
//======================================================================
#define EVNAME(a) [a] = #a

static const char *events[EV_MAX + 1] = {
    [0 ... EV_MAX] = NULL,
    EVNAME( EV_SYN ),		EVNAME( EV_KEY ),
    EVNAME( EV_REL ),		EVNAME( EV_ABS ),
    EVNAME( EV_MSC ),		EVNAME( EV_LED ),
    EVNAME( EV_SND ),		EVNAME( EV_REP ),
    EVNAME( EV_FF  ),		EVNAME( EV_PWR ),
    EVNAME( EV_FF_STATUS ),
};

static const char *keys[KEY_MAX + 1] = {
    [0 ... KEY_MAX] = NULL,
    EVNAME( KEY_RESERVED ),		EVNAME( KEY_ESC ),
    EVNAME( KEY_1 ),			EVNAME( KEY_2 ),
    EVNAME( KEY_3 ),			EVNAME( KEY_4 ),
    EVNAME( KEY_5 ),			EVNAME( KEY_6 ),
    EVNAME( KEY_7 ),			EVNAME( KEY_8 ),
    EVNAME( KEY_9 ),			EVNAME( KEY_0 ),
    EVNAME( KEY_MINUS ),		EVNAME( KEY_EQUAL ),
    EVNAME( KEY_BACKSPACE ),		EVNAME( KEY_TAB ),
    EVNAME( KEY_Q ),			EVNAME( KEY_W ),
    EVNAME( KEY_E ),			EVNAME( KEY_R ),
    EVNAME( KEY_T ),			EVNAME( KEY_Y ),
    EVNAME( KEY_U ),			EVNAME( KEY_I ),
    EVNAME( KEY_O ),			EVNAME( KEY_P ),
    EVNAME( KEY_LEFTBRACE ),		EVNAME( KEY_RIGHTBRACE ),
    EVNAME( KEY_ENTER ),		EVNAME( KEY_LEFTCTRL ),
    EVNAME( KEY_A ),			EVNAME( KEY_S ),
    EVNAME( KEY_D ),			EVNAME( KEY_F ),
    EVNAME( KEY_G ),			EVNAME( KEY_H ),
    EVNAME( KEY_J ),			EVNAME( KEY_K ),
    EVNAME( KEY_L ),			EVNAME( KEY_SEMICOLON ),
    EVNAME( KEY_APOSTROPHE ),		EVNAME( KEY_GRAVE ),
    EVNAME( KEY_LEFTSHIFT ),		EVNAME( KEY_BACKSLASH ),
    EVNAME( KEY_Z ),			EVNAME( KEY_X ),
    EVNAME( KEY_C ),			EVNAME( KEY_V ),
    EVNAME( KEY_B ),			EVNAME( KEY_N ),
    EVNAME( KEY_M ),			EVNAME( KEY_COMMA ),
    EVNAME( KEY_DOT ),			EVNAME( KEY_SLASH ),
    EVNAME( KEY_RIGHTSHIFT ),		EVNAME( KEY_KPASTERISK ),
    EVNAME( KEY_LEFTALT ),		EVNAME( KEY_SPACE ),
    EVNAME( KEY_CAPSLOCK ),
    EVNAME( KEY_F1 ),			EVNAME( KEY_F2 ),
    EVNAME( KEY_F3 ),			EVNAME( KEY_F4 ),
    EVNAME( KEY_F5 ),			EVNAME( KEY_F6 ),
    EVNAME( KEY_F7 ),			EVNAME( KEY_F8 ),
    EVNAME( KEY_F9 ),			EVNAME( KEY_F10 ),
    EVNAME( KEY_NUMLOCK ),		EVNAME( KEY_SCROLLLOCK ),
    EVNAME( KEY_KP7 ),			EVNAME( KEY_KP8 ),
    EVNAME( KEY_KP9 ),			EVNAME( KEY_KPMINUS ),
    EVNAME( KEY_KP4 ),			EVNAME( KEY_KP5 ),
    EVNAME( KEY_KP6 ),			EVNAME( KEY_KPPLUS ),
    EVNAME( KEY_KP1 ),			EVNAME( KEY_KP2 ),
    EVNAME( KEY_KP3 ),			EVNAME( KEY_KP0 ),
    EVNAME( KEY_KPDOT ),		EVNAME( KEY_ZENKAKUHANKAKU ),
    EVNAME( KEY_102ND ),		EVNAME( KEY_F11 ),
    EVNAME( KEY_F12 ),			EVNAME( KEY_RO ),
    EVNAME( KEY_KATAKANA ),		EVNAME( KEY_HIRAGANA ),
    EVNAME( KEY_HENKAN ),		EVNAME( KEY_KATAKANAHIRAGANA ),
    EVNAME( KEY_MUHENKAN ),		EVNAME( KEY_KPJPCOMMA ),
    EVNAME( KEY_KPENTER ),		EVNAME( KEY_RIGHTCTRL ),
    EVNAME( KEY_KPSLASH ),		EVNAME( KEY_SYSRQ ),
    EVNAME( KEY_RIGHTALT ),		EVNAME( KEY_LINEFEED ),
    EVNAME( KEY_HOME ),			EVNAME( KEY_UP ),
    EVNAME( KEY_PAGEUP ),		EVNAME( KEY_LEFT ),
    EVNAME( KEY_RIGHT ),		EVNAME( KEY_END ),
    EVNAME( KEY_DOWN ),			EVNAME( KEY_PAGEDOWN ),
    EVNAME( KEY_INSERT ),		EVNAME( KEY_DELETE ),
    EVNAME( KEY_MACRO ),		EVNAME( KEY_MUTE ),
    EVNAME( KEY_VOLUMEDOWN ),		EVNAME( KEY_VOLUMEUP ),
    EVNAME( KEY_POWER ),		EVNAME( KEY_KPEQUAL ),
    EVNAME( KEY_KPPLUSMINUS ),		EVNAME( KEY_PAUSE ),
    EVNAME( KEY_KPCOMMA ),		EVNAME( KEY_HANGUEL ),
    EVNAME( KEY_HANJA ),		EVNAME( KEY_YEN ),
    EVNAME( KEY_LEFTMETA ),		EVNAME( KEY_RIGHTMETA ),
    EVNAME( KEY_COMPOSE ),		EVNAME( KEY_STOP ),
    EVNAME( KEY_AGAIN ),		EVNAME( KEY_PROPS ),
    EVNAME( KEY_UNDO ),			EVNAME( KEY_FRONT ),
    EVNAME( KEY_COPY ),			EVNAME( KEY_OPEN ),
    EVNAME( KEY_PASTE ),		EVNAME( KEY_FIND ),
    EVNAME( KEY_CUT ),			EVNAME( KEY_HELP ),
    EVNAME( KEY_MENU ),			EVNAME( KEY_CALC ),
    EVNAME( KEY_SETUP ),		EVNAME( KEY_SLEEP ),
    EVNAME( KEY_WAKEUP ),		EVNAME( KEY_FILE ),
    EVNAME( KEY_SENDFILE ),		EVNAME( KEY_DELETEFILE ),
    EVNAME( KEY_XFER ),			EVNAME( KEY_PROG1 ),
    EVNAME( KEY_PROG2 ),		EVNAME( KEY_WWW ),
    EVNAME( KEY_MSDOS ),		EVNAME( KEY_COFFEE ),
    EVNAME( KEY_DIRECTION ),		EVNAME( KEY_CYCLEWINDOWS ),
    EVNAME( KEY_MAIL ),			EVNAME( KEY_BOOKMARKS ),
    EVNAME( KEY_COMPUTER ),		EVNAME( KEY_BACK ),
    EVNAME( KEY_FORWARD ),		EVNAME( KEY_CLOSECD ),
    EVNAME( KEY_EJECTCD ),		EVNAME( KEY_EJECTCLOSECD ),
    EVNAME( KEY_NEXTSONG ),		EVNAME( KEY_PLAYPAUSE ),
    EVNAME( KEY_PREVIOUSSONG ),		EVNAME( KEY_STOPCD ),
    EVNAME( KEY_RECORD ),		EVNAME( KEY_REWIND ),
    EVNAME( KEY_PHONE ),		EVNAME( KEY_ISO ),
    EVNAME( KEY_CONFIG ),		EVNAME( KEY_HOMEPAGE ),
    EVNAME( KEY_REFRESH ),		EVNAME( KEY_EXIT ),
    EVNAME( KEY_MOVE ),			EVNAME( KEY_EDIT ),
    EVNAME( KEY_SCROLLUP ),		EVNAME( KEY_SCROLLDOWN ),
    EVNAME( KEY_KPLEFTPAREN ),		EVNAME( KEY_KPRIGHTPAREN ),
    EVNAME( KEY_F13 ),			EVNAME( KEY_F14 ),
    EVNAME( KEY_F15 ),			EVNAME( KEY_F16 ),
    EVNAME( KEY_F17 ),			EVNAME( KEY_F18 ),
    EVNAME( KEY_F19 ),			EVNAME( KEY_F20 ),
    EVNAME( KEY_F21 ),			EVNAME( KEY_F22 ),
    EVNAME( KEY_F23 ),			EVNAME( KEY_F24 ),
    EVNAME( KEY_PLAYCD ),		EVNAME( KEY_PAUSECD ),
    EVNAME( KEY_PROG3 ),		EVNAME( KEY_PROG4 ),
    EVNAME( KEY_SUSPEND ),		EVNAME( KEY_CLOSE ),
    EVNAME( KEY_PLAY ),			EVNAME( KEY_FASTFORWARD ),
    EVNAME( KEY_BASSBOOST ),		EVNAME( KEY_PRINT ),
    EVNAME( KEY_HP ),			EVNAME( KEY_CAMERA ),
    EVNAME( KEY_SOUND ),		EVNAME( KEY_QUESTION ),
    EVNAME( KEY_EMAIL ),		EVNAME( KEY_CHAT ),
    EVNAME( KEY_SEARCH ),		EVNAME( KEY_CONNECT ),
    EVNAME( KEY_FINANCE ),		EVNAME( KEY_SPORT ),
    EVNAME( KEY_SHOP ),			EVNAME( KEY_ALTERASE ),
    EVNAME( KEY_CANCEL ),		EVNAME( KEY_BRIGHTNESSDOWN ),
    EVNAME( KEY_BRIGHTNESSUP ),		EVNAME( KEY_MEDIA ),
    EVNAME( KEY_UNKNOWN ),
    EVNAME( BTN_0 ),			EVNAME( BTN_1 ),
    EVNAME( BTN_2 ),			EVNAME( BTN_3 ),
    EVNAME( BTN_4 ),			EVNAME( BTN_5 ),
    EVNAME( BTN_6 ),			EVNAME( BTN_7 ),
    EVNAME( BTN_8 ),			EVNAME( BTN_9 ),
    EVNAME( BTN_LEFT ),			EVNAME( BTN_RIGHT ),
    EVNAME( BTN_MIDDLE ),		EVNAME( BTN_SIDE ),
    EVNAME( BTN_EXTRA ),		EVNAME( BTN_FORWARD ),
    EVNAME( BTN_BACK ),			EVNAME( BTN_TASK ),
    EVNAME( BTN_TRIGGER ),		EVNAME( BTN_THUMB ),
    EVNAME( BTN_THUMB2 ),		EVNAME( BTN_TOP ),
    EVNAME( BTN_TOP2 ),			EVNAME( BTN_PINKIE ),
    EVNAME( BTN_BASE ),			EVNAME( BTN_BASE2 ),
    EVNAME( BTN_BASE3 ),		EVNAME( BTN_BASE4 ),
    EVNAME( BTN_BASE5 ),		EVNAME( BTN_BASE6 ),
    EVNAME( BTN_DEAD ),			EVNAME( BTN_A ),
    EVNAME( BTN_B ),			EVNAME( BTN_C ),
    EVNAME( BTN_X ),			EVNAME( BTN_Y ),
    EVNAME( BTN_Z ),			EVNAME( BTN_TL ),
    EVNAME( BTN_TR ),			EVNAME( BTN_TL2 ),
    EVNAME( BTN_TR2 ),			EVNAME( BTN_SELECT ),
    EVNAME( BTN_START ),		EVNAME( BTN_MODE ),
    EVNAME( BTN_THUMBL ),		EVNAME( BTN_THUMBR ),
    EVNAME( BTN_TOOL_PEN ),		EVNAME( BTN_TOOL_RUBBER ),
    EVNAME( BTN_TOOL_BRUSH ),		EVNAME( BTN_TOOL_PENCIL ),
    EVNAME( BTN_TOOL_AIRBRUSH ),	EVNAME( BTN_TOOL_FINGER ),
    EVNAME( BTN_TOOL_MOUSE ),		EVNAME( BTN_TOOL_LENS ),
    EVNAME( BTN_TOUCH ),		EVNAME( BTN_STYLUS ),
    EVNAME( BTN_STYLUS2 ),		EVNAME( BTN_TOOL_DOUBLETAP ),
    EVNAME( BTN_TOOL_TRIPLETAP ),	EVNAME( BTN_GEAR_DOWN ),
    EVNAME( BTN_GEAR_UP ),		EVNAME( KEY_OK ),
    EVNAME( KEY_SELECT ),		EVNAME( KEY_GOTO ),
    EVNAME( KEY_CLEAR ),		EVNAME( KEY_POWER2 ),
    EVNAME( KEY_OPTION ),		EVNAME( KEY_INFO ),
    EVNAME( KEY_TIME ),			EVNAME( KEY_VENDOR ),
    EVNAME( KEY_ARCHIVE ),		EVNAME( KEY_PROGRAM ),
    EVNAME( KEY_CHANNEL ),		EVNAME( KEY_FAVORITES ),
    EVNAME( KEY_EPG ),			EVNAME( KEY_PVR ),
    EVNAME( KEY_MHP ),			EVNAME( KEY_LANGUAGE ),
    EVNAME( KEY_TITLE ),		EVNAME( KEY_SUBTITLE ),
    EVNAME( KEY_ANGLE ),		EVNAME( KEY_ZOOM ),
    EVNAME( KEY_MODE ),			EVNAME( KEY_KEYBOARD ),
    EVNAME( KEY_SCREEN ),		EVNAME( KEY_PC ),
    EVNAME( KEY_TV ),			EVNAME( KEY_TV2 ),
    EVNAME( KEY_VCR ),			EVNAME( KEY_VCR2 ),
    EVNAME( KEY_SAT ),			EVNAME( KEY_SAT2 ),
    EVNAME( KEY_CD ),			EVNAME( KEY_TAPE ),
    EVNAME( KEY_RADIO ),		EVNAME( KEY_TUNER ),
    EVNAME( KEY_PLAYER ),		EVNAME( KEY_TEXT ),
    EVNAME( KEY_DVD ),			EVNAME( KEY_AUX ),
    EVNAME( KEY_MP3 ),			EVNAME( KEY_AUDIO ),
    EVNAME( KEY_VIDEO ),		EVNAME( KEY_DIRECTORY ),
    EVNAME( KEY_LIST ),			EVNAME( KEY_MEMO ),
    EVNAME( KEY_CALENDAR ),		EVNAME( KEY_RED ),
    EVNAME( KEY_GREEN ),		EVNAME( KEY_YELLOW ),
    EVNAME( KEY_BLUE ),			EVNAME( KEY_CHANNELUP ),
    EVNAME( KEY_CHANNELDOWN ),		EVNAME( KEY_FIRST ),
    EVNAME( KEY_LAST ),			EVNAME( KEY_AB ),
    EVNAME( KEY_NEXT ),			EVNAME( KEY_RESTART ),
    EVNAME( KEY_SLOW ),			EVNAME( KEY_SHUFFLE ),
    EVNAME( KEY_BREAK ),		EVNAME( KEY_PREVIOUS ),
    EVNAME( KEY_DIGITS ),		EVNAME( KEY_TEEN ),
    EVNAME( KEY_TWEN ),			EVNAME( KEY_DEL_EOL ),
    EVNAME( KEY_DEL_EOS ),		EVNAME( KEY_INS_LINE ),
    EVNAME( KEY_DEL_LINE ),

#ifdef KEY_NUMERIC_POUND
    EVNAME( KEY_FN ),			EVNAME( KEY_FN_ESC ),
    EVNAME( KEY_FN_F1 ),		EVNAME( KEY_FN_F2 ),
    EVNAME( KEY_FN_F3 ),		EVNAME( KEY_FN_F4 ),
    EVNAME( KEY_FN_F5 ),		EVNAME( KEY_FN_F6 ),
    EVNAME( KEY_FN_F7 ),		EVNAME( KEY_FN_F8 ),
    EVNAME( KEY_FN_F9 ),		EVNAME( KEY_FN_F10 ),
    EVNAME( KEY_FN_F11 ),		EVNAME( KEY_FN_F12 ),
    EVNAME( KEY_FN_1 ),			EVNAME( KEY_FN_2 ),
    EVNAME( KEY_FN_D ),			EVNAME( KEY_FN_E ),
    EVNAME( KEY_FN_F ),			EVNAME( KEY_FN_S ),
    EVNAME( KEY_FN_B ),

    EVNAME( KEY_BRL_DOT1 ),		EVNAME( KEY_BRL_DOT2 ),
    EVNAME( KEY_BRL_DOT3 ),		EVNAME( KEY_BRL_DOT4 ),
    EVNAME( KEY_BRL_DOT5 ),		EVNAME( KEY_BRL_DOT6 ),
    EVNAME( KEY_BRL_DOT7 ),		EVNAME( KEY_BRL_DOT8 ),
    EVNAME( KEY_BRL_DOT9 ),		EVNAME( KEY_BRL_DOT10 ),
    EVNAME( KEY_NUMERIC_0 ),		EVNAME( KEY_NUMERIC_1 ),
    EVNAME( KEY_NUMERIC_2 ),		EVNAME( KEY_NUMERIC_3 ),
    EVNAME( KEY_NUMERIC_4 ),		EVNAME( KEY_NUMERIC_5 ),
    EVNAME( KEY_NUMERIC_6 ),		EVNAME( KEY_NUMERIC_7 ),
    EVNAME( KEY_NUMERIC_8 ),		EVNAME( KEY_NUMERIC_9 ),
    EVNAME( KEY_NUMERIC_STAR ),		EVNAME( KEY_NUMERIC_POUND ),
#endif

};

static const char *relatives[REL_MAX + 1] = {
    [0 ... REL_MAX] = NULL,
    EVNAME( REL_X ),			EVNAME( REL_Y ),
    EVNAME( REL_Z ),			EVNAME( REL_HWHEEL ),
    EVNAME( REL_DIAL ),			EVNAME( REL_WHEEL ),
    EVNAME( REL_MISC ),
};

static const char *absolutes[ABS_MAX + 1] = {
    [0 ... ABS_MAX] = NULL,
    EVNAME( ABS_X ),		EVNAME( ABS_Y ),
    EVNAME( ABS_Z ),		EVNAME( ABS_RX ),
    EVNAME( ABS_RY ),		EVNAME( ABS_RZ ),
    EVNAME( ABS_THROTTLE ),		EVNAME( ABS_RUDDER ),
    EVNAME( ABS_WHEEL ),		EVNAME( ABS_GAS ),
    EVNAME( ABS_BRAKE ),		EVNAME( ABS_HAT0X ),
    EVNAME( ABS_HAT0Y ),		EVNAME( ABS_HAT1X ),
    EVNAME( ABS_HAT1Y ),		EVNAME( ABS_HAT2X ),
    EVNAME( ABS_HAT2Y ),		EVNAME( ABS_HAT3X ),
    EVNAME( ABS_HAT3Y ),		EVNAME( ABS_PRESSURE ),
    EVNAME( ABS_DISTANCE ),		EVNAME( ABS_TILT_X ),
    EVNAME( ABS_TILT_Y ),		EVNAME( ABS_TOOL_WIDTH ),
    EVNAME( ABS_VOLUME ),		EVNAME( ABS_MISC ),
#ifdef ABS_MT_BLOB_ID
    EVNAME( ABS_MT_TOUCH_MAJOR ),	EVNAME( ABS_MT_TOUCH_MINOR ),
    EVNAME( ABS_MT_WIDTH_MAJOR ),	EVNAME( ABS_MT_WIDTH_MINOR ),
    EVNAME( ABS_MT_ORIENTATION ),	EVNAME( ABS_MT_TOOL_TYPE ),
    EVNAME( ABS_MT_POSITION_X ),	EVNAME( ABS_MT_POSITION_Y ),
    EVNAME( ABS_MT_BLOB_ID ),
#endif
};

static const char *misc[MSC_MAX + 1] = {
    [ 0 ... MSC_MAX] = NULL,
    EVNAME( MSC_SERIAL ),		EVNAME( MSC_PULSELED ),
    EVNAME( MSC_GESTURE ),		EVNAME( MSC_RAW ),
    EVNAME( MSC_SCAN ),
};

static const char *leds[LED_MAX + 1] = {
    [0 ... LED_MAX] = NULL,
    EVNAME( LED_NUML ),		EVNAME( LED_CAPSL ),
    EVNAME( LED_SCROLLL ),		EVNAME( LED_COMPOSE ),
    EVNAME( LED_KANA ),		EVNAME( LED_SLEEP ),
    EVNAME( LED_SUSPEND ),		EVNAME( LED_MUTE ),
    EVNAME( LED_MISC ),
#ifdef LED_CHARGING
    EVNAME( LED_CHARGING ),
#endif
};

static const char *repeats[REP_MAX + 1] = {
    [0 ... REP_MAX] = NULL,
    EVNAME( REP_DELAY ),
    EVNAME( REP_PERIOD ),
};

static const char *sounds[SND_MAX + 1] = {
    [0 ... SND_MAX] = NULL,
    EVNAME( SND_CLICK ),
    EVNAME( SND_BELL ),
    EVNAME( SND_TONE ),
};

static struct pollfd *g_PollList;
static nfds_t g_PollNum;

//======================================================================
//
//
//                      static function
//
//
//======================================================================
//=====================================
//
//          getevname
//
//=====================================
static const char* getevname( int nType, int nCode )
{
    const char **names;
    int size;

    switch ( nType ) {
    case EV_SYN: names = events;    size = ARRAY_SIZE( events );    break;
    case EV_KEY: names = keys;      size = ARRAY_SIZE( keys );      break;
    case EV_REL: names = relatives; size = ARRAY_SIZE( relatives ); break;
    case EV_ABS: names = absolutes; size = ARRAY_SIZE( absolutes ); break;
    case EV_MSC: names = misc;      size = ARRAY_SIZE( misc );      break;
    case EV_LED: names = leds;      size = ARRAY_SIZE( leds );      break;
    case EV_SND: names = sounds;    size = ARRAY_SIZE( sounds );    break;
    case EV_REP: names = repeats;   size = ARRAY_SIZE( repeats );   break;
    default:
        return "unknown type";
    }

    if ( nCode >= size ) {
        printf( "*** code error (%d - %d) ***\n", nCode, size );
        return "code error";
    }

    return names[ nCode ];
}

//=====================================
//
//          getevent
//
//=====================================
#define EVENTMAX 64
static int getevent( struct input_event **ppEvent )
{
    static struct input_event _event[EVENTMAX];
    int size, num;
    struct pollfd *p;

    if ( g_PollNum < 1 ) {
        printf( "EventOpen is needed\n" );
        return -1;
    }

    num = poll( g_PollList, g_PollNum, -1);
    if ( num < 1 )
        return -1;

    num = g_PollNum;
    for ( p=g_PollList; num && !p->revents; --num, ++p )
        ; /* search for the first fd with available data */

    if ( num <= 0 )
        return -1; /* nothing ready (should never happen) */

    size = read( p->fd, _event , sizeof(_event) );
    size /= (int)sizeof( struct input_event );

    if ( size < 1 )
        return -1;

    *ppEvent = _event;
    return  size;
}

//======================================================================
//
//
//                      global function
//
//
//======================================================================
//=====================================
//
//       EventOpen / EventClose
//
//=====================================
bool EventOpen( struct list_head *pDevhead )
{
    int num = 0;
    struct device_table *dev, *ndev;
    struct pollfd *p;

    if ( !pDevhead )
        return false;

    list_for_each_entry_safe(dev, ndev, pDevhead, lst)
        ++num;

    g_PollList = calloc(num, sizeof(struct pollfd));

    p = g_PollList;
    list_for_each_entry_safe(dev, ndev, pDevhead, lst) {
        p->fd = open( dev->device, O_RDONLY );
        p->events = POLLIN;
        if ( p->fd < 0 ) {
            free( g_PollList );
            g_PollList = NULL;
            g_PollNum = 0;
            return false;
        }
        ++g_PollNum;
        ++p;
    }
    return true;
}

void EventClose( void )
{
    int i;
    for ( i=g_PollNum; i--; )
        close( g_PollList[i].fd );
    free( g_PollList );
    g_PollList = NULL;
    g_PollNum = 0;
}

//=====================================
//
//          ScanLoop
//
//=====================================
void ScanLoop( struct list_head *pLhead,
               const char *pTitle,
               bool exit_error,
               bool (*hEvent)(struct input_event *event),
               bool (*hDecide)(struct input_event *event, struct event_table *pos))
{
    struct input_event *event;
    struct event_table *pos;
    int size, i;
    bool show = true;

    while (1) {

        //----------------------
        // printf title
        //----------------------
        if ( show && pTitle )
            printf( "%s\n", pTitle );

        show = false;

        //----------------------
        // get event data
        // in keysc, size is always 2
        //----------------------
        size = getevent( &event );

        //----------------------
        // if no key is registered,
        // print event value
        //----------------------
        if ( list_empty( pLhead )) {
            printf( "----------------\n" );
            for ( i=0 ; i<size ; i++ )
                printf("Event: type %d (%s), code %d (%s), value %d\n",
                       event[i].type, getevname( EV_SYN, event[i].type ),
                       event[i].code, getevname( event[i].type, event[i].code ),
                       event[i].value);
            continue;
        }

        //----------------------
        // loop through each event
        //----------------------
        for (; size; --size, ++event) {
            if ( !hEvent( event ))
                continue;

            //----------------------
            // check all list
            //----------------------
            list_for_each_entry( pos, pLhead, lst ) {

                //----------------------
                // if it is registered key,
                // run call back function
                // "0" mean return
                //----------------------
                if ( hDecide( event , pos )) {

                    if ( !pos->script )
                        return;

                    if ( system( pos->script ) && exit_error )
                        return;

                    show = true;
                    sleep( 1 );
                    goto quit_loop;
                }
            } // list_for_each_entry
        } // for
quit_loop:
        ;
    } // while
}

