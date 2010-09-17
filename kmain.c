/*************************************************************************

                                 kmain

Copyright (c) Kuninori Morimoto <morimoto.kuninori@renesas.com>

2009/09  morimoto
*************************************************************************/
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tevent.h"

#define K 0  // key id
#define V 1  // value

//=====================================
//
//          usage
//
//=====================================
static void usage( void )
{
    printf( "Usage: (%s)\n"
            "kmenu /dev/input/eventX [-e] [-t TITLE] [k v script]\n"
            "\n"
            "k      : event number\n"
            "v      : event value\n"
            "script : call back script\n",
            K_VERSION );
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
                usage( );
                return false;
        }
    }

    //------------------------------
    // need at least the device name
    //------------------------------
    if ( optind >= nArgc ) {
        usage( );
        return false;
    }

    //----------------------
    // get device name
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
    // get key definitions
    //----------------------
    for ( ; optind < nArgc ; optind += 3 ) {

        // need exactly 3 arguments per entry
        if ( (optind+3) > nArgc ) {
            usage();
            return false;
        }

        pos = malloc(sizeof( struct event_table ));
        if (!pos) {
            printf("malloc error\n");
            return false;
        }

        INIT_LIST_HEAD( &pos->lst );
        list_add_tail( &pos->lst, pLhead );

        pos->value[K] = atoi( pstrArgv[ optind + 0 ] );
        pos->value[V] = atoi( pstrArgv[ optind + 1 ] );
        pos->script = pstrArgv[ optind + 2 ];

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
    if ( EV_KEY != pEvent->type )
        return false;

    return true;
}

//=====================================
//
//          decide
//
//=====================================
static bool decide(struct input_event *pEvent,
                   struct event_table *pPos)
{
    if (( pEvent->code  == pPos->value[K] ) &&
        ( pEvent->value == pPos->value[V] ))
        return true;

    return false;
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
        printf ("Cannot open device\n");
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
