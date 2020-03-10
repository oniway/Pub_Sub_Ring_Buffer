#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "topicQueue.h"


#define NUMPROXIES 5
#define MAX_FILE_NAME 50			//make length of file name


typedef struct{
	pthread_t thread;
	int id;
	int inUse;
	char fileName[MAX_FILE_NAME];
	pthread_mutex_t lock;
} threadEnt;



threadEnt producerPool[NUMPROXIES];
threadEnt consumerPool[NUMPROXIES];

//for cond_wait
pthread_mutex_t cond_mutex;
pthread_cond_t cond;


/*---------------------------- INIT FUNCTIONS ----------------------------*/

int initThreadPool( threadEnt* pool, int length, void *(*startR) (void *) );


int initThreadEnt( threadEnt* entry, int id, void *(*startR) (void *) );

int stopThreadPool( threadEnt* pool, int length );



/*--------------------------- THREAD FUNCTIONS ---------------------------*/

void* producer(void* input);

void* consumer(void* input);


/*------------------------------------------------------------------------*/
