#include <pthread.h>
#include <sys/time.h>

#include "topicQueue.h"

int spawnProducers(topicQueue* TQ, int producers, int writes);

int spawnConsumers(topicQueue* TQ, int consumers, int reads);

int spawnCleanup(topicQueue* TQ);


/*------------------------------------------------------------------------*/


void* produce(void* input);

void* consume(void* input);

void* cleanup(void* input);


/*------------------------------------------------------------------------*/
/*test: test.c test.h topicEntry.h topicQueue.h
	$(CC) -c test.c $(cflag) */



/*
	//init a topic entry
	//
	topicEntry TE;

	getEntry(topicStore[0], 0, &TE);	

	printf("name-%s  entries-%d  head-%d  tail-%d full-%d length-%d\n", 
			topicStore[0]->name, topicStore[0]->entries,
			topicStore[0]->head, topicStore[0]->tail,
			topicStore[0]->full, topicStore[0]->length);

	enqueue(topicStore[0], &TE);

	printf("name-%s  entries-%d  head-%d  tail-%d full-%d length-%d\n", 
			topicStore[0]->name, topicStore[0]->entries,
			topicStore[0]->head, topicStore[0]->tail,
			topicStore[0]->full, topicStore[0]->length);
	
	getEntry(topicStore[0], 0, &TE);	

	printf("name-%s  entries-%d  head-%d  tail-%d full-%d length-%d\n", 
			topicStore[0]->name, topicStore[0]->entries,
			topicStore[0]->head, topicStore[0]->tail,
			topicStore[0]->full, topicStore[0]->length);

	
	
	//read from empty
	//spawnConsumers(topicStore[0], 1, 1);

	//write to empty
	spawnProducers(topicStore[0], 2, 4);

	//write to full
	//spawnProducers(topicStore[0], 1, 1);

	//read from full
	//spawnConsumers(topicStore[0], 1, 1);

	sleep(2);
	
	//run cleanup
	spawnCleanup(topicStore[0]);
*/	