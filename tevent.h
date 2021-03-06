/*************************************************************************

                                 tevent.h

Copyright (c) Kuninori Morimoto <morimoto.kuninori@renesas.com>

2009/09  morimoto
*************************************************************************/
#ifndef TEVENT_H
#define TEVENT_H

#include <stdbool.h>
#include "list.h"

#define K_VERSION "0.0.2"  // see VERSION file for detail
#define T_VERSION "0.0.1"  // see VERSION file for detail

#define ERROR_EXIT 1       // program exit code in case of error

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define VMAX 4
struct event_table {
    struct list_head lst;
    int value[VMAX];
    const char *script;
};

struct device_table {
    struct list_head lst;
    const char *device;
};

bool EventOpen( struct list_head *pDevhead );
void EventClose( void );
void ScanLoop( struct list_head *pLhead,
               const char *pTitle,
               bool exit_error,
               bool (*hEvent)(struct input_event *event),
               bool (*hDecide)(struct input_event *event, struct event_table *pos));



#endif
