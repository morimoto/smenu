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

#include "./tevent.h"

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
        usage( );
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

    //-----------------------
    // get title if available
    //-----------------------
    if ( nArgc &&
         0 == strcmp( "-t", pstrArgv[ i ] )) {
        if ( nArgc < 2 ) {
            usage( );
            return false;
        }
        ARGINC(1);

        *pTitle = pstrArgv[ i ];
        ARGINC(1);
    }

    //----------------------
    // get key definition
    //----------------------
    for ( ; nArgc ; ) {

        // nArgc should over 3
        if ( nArgc < 3 ) {
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

        pos->value[K] = atoi( pstrArgv[ i + 0 ] );
        pos->value[V] = atoi( pstrArgv[ i + 1 ] );
        pos->script = pstrArgv[ i + 2 ];
        ARGINC(3);

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
    struct list_head lhead;
    struct event_table *pos, *npos;
    const char *dev;
    const char *title = NULL;
    bool exit_error = false;
    int rc = ERROR_EXIT; // assume error

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
        usage ();
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

    return rc;

}
