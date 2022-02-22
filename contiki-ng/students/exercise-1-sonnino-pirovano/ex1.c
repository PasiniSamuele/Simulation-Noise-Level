//
// Created by lucap on 11/12/2020.
//
#include "contiki.h"

#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


/*---------------------------------------------------------------------------*/
PROCESS(producer,
"Producer process");
PROCESS(consumer,
"Consumer process");
AUTOSTART_PROCESSES(&producer,&consumer);
/*---------------------------------------------------------------------------*/
static process_event_t produce_event;
static process_event_t consume_event;
typedef struct {
	int list[10];
} arrStruct;
static arrStruct* queue;
/*---------------------------------------------------------------------------*/

void print_list(int* list) {
	for(int i=0; i<10; i++) {
		printf("%d ", list[i]);
	}
	printf("\n");
}


PROCESS_THREAD(producer, ev, data)
{
static struct etimer timer;

PROCESS_BEGIN();

/* Make sure the other process has time to start... */
PROCESS_PAUSE();

/* Start the push process... */
produce_event = process_alloc_event();
etimer_set(&timer, CLOCK_SECOND * 0);
queue = calloc(1, sizeof(arrStruct));
while(1) {
/* If the array has some free spaces proceed with push */
if ((*(arrStruct*)queue).list[9] == 0) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    for(int i=0; i<10; i++) {
	if((*(arrStruct*)queue).list[i]==0) {
		(*(arrStruct*)queue).list[i] = 10;
		printf(ANSI_COLOR_RED "Pushing: %d\n" ANSI_COLOR_RESET,(*(arrStruct*)queue).list[i]);
		break;
	}
    }
    print_list((*(arrStruct*)queue).list);
    process_post(&consumer, produce_event, queue);
    /* Wait between pushes */
    etimer_set(&timer, CLOCK_SECOND * (rand() % (3 - 1 + 1) + 1));

} else {
    /* Wait a random delay before retrying */
    etimer_set(&timer, CLOCK_SECOND * (rand() % (3 - 1 + 1) + 1));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
}
}

PROCESS_END();

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(consumer, ev, data)
{
static struct etimer timer;
PROCESS_BEGIN();
/* Wait for at least one push */
PROCESS_WAIT_EVENT();
consume_event = process_alloc_event();
while(1) {
	if((*(arrStruct*)queue).list[0] != 0) {
	/* Wait for timer expiration. */
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
	/* Pull the latest element from the queue */
	for(int i=9; i>=0; i--) {
		if((*(arrStruct*)queue).list[i]!=0) {
			printf(ANSI_COLOR_GREEN "Pulling %d\n" ANSI_COLOR_RESET, (*(arrStruct*)queue).list[i]);
			(*(arrStruct*)queue).list[i] = 0;
			break;
		}
	}
	print_list((*(arrStruct*)queue).list);
	process_post(&producer, consume_event, queue);
	etimer_set(&timer, CLOCK_SECOND * (rand() % (3 - 1 + 1) + 1));
	}
	else {
		PROCESS_WAIT_EVENT();
	}
}

PROCESS_END();

}
/*---------------------------------------------------------------------------*/
