/*************************************************************************

                                 tevent.h

Copyright (c) Kuninori Morimoto <morimoto.kuninori@renesas.com>

2009/09  morimoto
*************************************************************************/
#ifndef TEVENT_H
#define TEVENT_H

#include <stdbool.h>
#include "./list.h"

#define K_VERSION "0.0.2"  // see VERSION file for detail
#define T_VERSION "0.0.1"  // see VERSION file for detail

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define VMAX 3
struct event_table {
    struct list_head lst;
    int value[VMAX];
    const char *script;
};

bool EventOpen( const char *pDevice );
void EventClose( void );
void ScanLoop( struct list_head *pLhead,
               const char *pTitle,
               bool (*hEvent)(struct input_event *event, int size),
               bool (*hDecide)(struct input_event *event, int size, struct event_table *pos));



#endif
