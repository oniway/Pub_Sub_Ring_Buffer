#include <pthread.h>

#include "topicEntry.h"

#define MAXENTRIES 10
#define MAXNAME 20

int DELTA;			//time in seconds after which cleanup will dequeue
int topics;

typedef pthread_mutex_t pm_t;

typedef struct {
	int id;
	topicEntry* buffer;
	pm_t lock;
	unsigned int entries;	//entryCounter
	int head;	//points to newest entry
	int tail;	//points to oldest entry
	int full;
	int length;
} topicQueue;

#define TQ_DEF(var, id, length)					\
	topicEntry var##_buf[MAXENTRIES];	\
	var = {					\
		.id = id,						\
		.buffer = var##_buf,			\
		.entries = 0,					\
		.head = 0,						\
		.tail = 0,						\
		.full = 0,						\
		.length = length			\
	};									\
	pthread_mutex_init(&(var.lock), NULL);

/*--------------------------- HELPER FUNCTIONS ---------------------------*/

int deleteLock(topicQueue* TQ); 

int copyTE(topicEntry* dst, topicEntry* src);

/*-------------------------- REQUIRED FUNCTIONS --------------------------*/

int enqueue(topicQueue* TQ, topicEntry* TE);
	/* access to topicQueues must be synchronized */


int getEntry(topicQueue* TQ, int lastEntry, topicEntry* TE);
	/* access to topicQueues must be synchronized */

int dequeue(topicQueue* TQ);
	/* access to topicQueues must be synchronized */

/*------------------------------------------------------------------------*/

void* cleanup(void* input);


/*------------------------------------------------------------------------*/

