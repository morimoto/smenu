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

#include "tevent.h"

#define X 0  // X position of touch coordinate
#define Y 1  // Y position of touch coordinate
#define A 2  // radius at touch coordinate
#define M 3  // match flags

#define MATCH_X ( 1 << 0 )  // X coordinate was matched
#define MATCH_Y ( 1 << 1 )  // Y coordinate was matched

//----------------------
// ABS_MT_POSITION_*
//----------------------
#ifdef ABS_MT_POSITION_X
#  define CASE_ABS_MT_POSITION_X  case ABS_MT_POSITION_X:
#else
#  define CASE_ABS_MT_POSITION_X
#endif

#ifdef ABS_MT_POSITION_Y
#  define CASE_ABS_MT_POSITION_Y case ABS_MT_POSITION_Y:
#else
#  define CASE_ABS_MT_POSITION_Y
#endif

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
#define MAX 256
static bool arg_analyze ( struct list_head *pLhead,
                          struct list_head *pDevhead,
                          const char **pTitle,
                          bool *pError,
                          int nArgc, char *pstrArgv[] )
{
    int opt;
    struct event_table *pos;
    struct device_table *dev;
    char cmd[MAX];

    while ((opt = getopt(nArgc, pstrArgv, "et:")) != -1) {
        switch (opt) {
            case 'e':
                *pError = true;
                break;
            case 't':
                *pTitle = optarg;
                break;
            default:
                usage( "Invalid argument" );
                return false;
        }
    }

    //------------------------------
    // need at least the device name
    //------------------------------
    if ( optind >= nArgc ) {
        usage( "Missing event source name" );
        return false;
    }

    //----------------------
    // get device name
    // note that only a single device is currently supported, because the X/Y
    // event matching going on in decide() doesn't keep track of the source
    // of events
    //----------------------
    dev = malloc(sizeof( struct device_table ));
    if (!dev) {
        printf("malloc error\n");
        return false;
    }
    INIT_LIST_HEAD( &dev->lst );
    list_add_tail( &dev->lst, pDevhead );
    dev->device = pstrArgv[ optind++ ];

    //----------------------
    // get x-y radius definition
    //----------------------
    for ( ; optind < nArgc ; optind += 4 ) {

        if ( (optind+4) > nArgc ) {
            usage( "Need 4 arguments per entry" );
            return false;
        }

        pos = malloc(sizeof( struct event_table ));
        if (!pos) {
            printf("malloc error\n");
            return false;
        }

        INIT_LIST_HEAD( &pos->lst );
        list_add_tail( &pos->lst, pLhead );

        pos->value[X]  = atoi( pstrArgv[ optind + 0 ] );
        pos->value[Y]  = atoi( pstrArgv[ optind + 1 ] );
        pos->value[A]  = atoi( pstrArgv[ optind + 2 ] );
        pos->value[M]  = 0;
        pos->script = pstrArgv[ optind + 3 ];

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
static bool event(struct input_event *pEvent)
{
    return ( EV_ABS == pEvent->type ) || ( EV_SYN == pEvent->type );
}

//=====================================
//
//          decide
//
//=====================================
#define  match(t, b, r) ((t < (b + r)) && (t > (b - r)))
static bool decide(struct input_event *pEvent,
                   struct event_table *pPos)
{
    int v = 0;
    bool rc = false;


    if ( EV_SYN == pEvent->type ) {
        //----------------------
        // clear out any old stored match flags
        //----------------------
        pPos->value[M] = 0;
        return rc;
    }

    //----------------------
    // decide main
    //----------------------
    switch( pEvent->code ) {
    case ABS_X:
    CASE_ABS_MT_POSITION_X

        if ( match(pEvent->value , pPos->value[X], pPos->value[A] ))
            v = MATCH_X;
        break;

    case ABS_Y:
    CASE_ABS_MT_POSITION_Y

        if ( match(pEvent->value , pPos->value[Y], pPos->value[A] ))
            v = MATCH_Y;
        break;

    default:
        return rc; // ignore this useless event altogether
    }

    //----------------------
    // Match only if the previous event (probably X)
    // and this event (probably Y) both match
    //----------------------
    rc = ((MATCH_X | MATCH_Y) == (pPos->value[M] | v));
    if (rc)
        pPos->value[M] = 0; // clear this to get ready for the next match
    else
        pPos->value[M] = v; // maybe the next event will provide the second match

    return rc;
}

//=====================================
//
//          main
//
//=====================================
int main ( int nArgc, char *pstrArgv[] )
{
    struct list_head lhead, devhead;
    struct event_table *pos, *npos;
    struct device_table *dev, *ndev;
    const char *title = NULL;
    bool exit_error = false;
    int rc = ERROR_EXIT; // assume error

    //----------------------
    // init list head
    //----------------------
    INIT_LIST_HEAD( &lhead );
    INIT_LIST_HEAD( &devhead );

    if ( !arg_analyze( &lhead, &devhead, &title, &exit_error, nArgc, pstrArgv ))
        goto main_end;

    //----------------------
    // open event file
    //----------------------
    if( !EventOpen( &devhead )) {
        usage ( "open failed" );
        goto main_end;
    }

    //----------------------
    // start scan
    //----------------------
    rc = 0;
    ScanLoop( &lhead, title, exit_error, event, decide );
    EventClose(  );

main_end:

    list_for_each_entry_safe(pos, npos, &lhead, lst)
        free ( pos );

    list_for_each_entry_safe(dev, ndev, &devhead, lst)
        free ( dev );

    return rc;

}
