/*************************************************************************

                                 tmain

Copyright (c) Kuninori Morimoto <morimoto.kuninori@renesas.com>

2009/09  morimoto
*************************************************************************/
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./tevent.h"

#define X 0
#define Y 1
#define A 2

//=====================================
//
//          usage
//
//=====================================
static void usage( const char *pMsg )
{
    printf( "-- %s --\n"
            "Usage: (%s)\n"
            "tmenu /dev/input/eventX [-e] [-t TITLE] [x y a script]\n"
            "\n"
            "x      : x position\n"
            "y      : y position\n"
            "a      : x-y area\n"
            "script : call back script\n",
            pMsg,
            T_VERSION );
}

//=====================================
//
//          arg_analyze
//
// analyze argument, and return device name if success
//=====================================
#define ARGINC(x) i += x; nArgc -= x;
#define MAX 256
static bool arg_analyze ( struct list_head *pLhead,
                          const char **pDev,
                          const char **pTitle,
                          bool *pError,
                          int nArgc, char *pstrArgv[] )
{
    int i = 0;
    struct event_table *pos;
    char cmd[MAX];

    //----------------------
    // nArgc must over 2
    //----------------------
    if ( nArgc < 2 ) {
        usage( "argc must over2" );
        return false;
    }
    ARGINC(1);

    //----------------------
    // get device name
    //----------------------
    *pDev = pstrArgv[ i ];
    ARGINC(1);

    //-----------------------
    // get error flag
    //-----------------------
    if ( nArgc &&
         0 == strcmp( "-e", pstrArgv[ i ] )) {
        ARGINC(1);
        *pError = true;
    }

    //----------------------
    // get title if it have
    //----------------------
    if ( nArgc &&
         0 == strcmp( "-t", pstrArgv[ i ] )) {
        if ( nArgc < 2 ) {
            usage( "-t option" );
            return false;
        }
        ARGINC(1);

        *pTitle = pstrArgv[ i ];
        ARGINC(1);
    }

    //----------------------
    // get x-y radius definition
    //----------------------
    for ( ; nArgc ; ) {

        // nArgc should over 4
        if ( nArgc < 4 ) {
            usage( "argc should over 4" );
            return false;
        }

        pos = malloc(sizeof( struct event_table ));
        if (!pos) {
            printf("malloc error\n");
            return false;
        }

        INIT_LIST_HEAD( &pos->lst );
        list_add_tail( &pos->lst, pLhead );

        pos->value[X]  = atoi( pstrArgv[ i + 0 ] );
        pos->value[Y]  = atoi( pstrArgv[ i + 1 ] );
        pos->value[A]  = atoi( pstrArgv[ i + 2 ] );
        pos->script = pstrArgv[ i + 3 ];
        ARGINC(4);

        if ( strlen(pos->script) > MAX - 1 ) {
            printf( "too long script\n" );
            return false;
        }
        memset( cmd, 0, MAX );
        sscanf( pos->script, "%s", cmd );

        if ( 0 == strcmp("0", cmd ) ) {
            // script NULL mean quit
            pos->script = NULL;
        }
        else if ( (cmd[0] == '/' || cmd[0] == '.') && // if cmd have path,
                  access( cmd, F_OK | X_OK ) < 0 ) {  // it should be checked
            printf( "can not use callback script (%s)\n" , pos->script );
            return false;
        }
    }

    return true;
}

//=====================================
//
//          event
//
//=====================================
static bool event(struct input_event *pEvent, int nSize)
{
    int i;

    for ( i=0 ; i<nSize ; i++ ) {
        switch ( pEvent[i].type ) {
        case EV_KEY:
        case EV_ABS:
        case EV_SYN:
            break;
        default:
            printf( "unknown type %d\n" , pEvent[i].type );
            return false;
        }
    }

    return true;
}

//=====================================
//
//          decide
//
//=====================================
#define  match(t, b, r) ((t < (b + r)) && (t > (b - r)))
static bool decide(struct input_event *pEvent,
                   int                 nSize,
                   struct event_table *pPos)
{
    int i, v = 0;

    //----------------------
    // key down only
    //----------------------
    if (( EV_KEY != pEvent[0].type ) &&
        ( 1      != pEvent[0].value ))
        return false;

    //----------------------
    // decide main
    //----------------------
    for ( i=1 ; i<nSize ; i++ ) {

        if (pEvent[i].type != EV_ABS)
            continue;

        switch( pEvent[i].code ) {
        case ABS_X:
            if ( match(pEvent[i].value , pPos->value[X], pPos->value[A] ))
                v |= 0x1;
            break;
        case ABS_Y:
            if ( match(pEvent[i].value , pPos->value[Y], pPos->value[A] ))
                v |= 0x10;
            break;
        }
    }

    return (0x11 == v);
}

//=====================================
//
//          main
//
//=====================================
int main ( int nArgc, char *pstrArgv[] )
{
    struct list_head lhead;
    struct event_table *pos, *npos;
    const char *dev;
    const char *title = NULL;
    bool exit_error = false;

    //----------------------
    // init list head
    //----------------------
    INIT_LIST_HEAD( &lhead );

    if ( !arg_analyze( &lhead, &dev, &title, &exit_error, nArgc, pstrArgv ))
        goto main_end;

    //----------------------
    // open event file
    //----------------------
    if( !EventOpen( dev )) {
        usage ( "open failed" );
        goto main_end;
    }

    //----------------------
    // start scan
    //----------------------
    ScanLoop( &lhead, title, exit_error, event, decide );
    EventClose(  );

main_end:

    list_for_each_entry_safe(pos, npos, &lhead, lst)
        free ( pos );

    return 0;

}
