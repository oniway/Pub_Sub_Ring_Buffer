#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "threadPool.h"
//#include "topicQueue.h"


/*---------------------------- INIT FUNCTIONS ----------------------------*/

//init threadPool (will take function as argument so we can use same for pub and sub)
int initThreadPool( threadEnt* pool, int length, void *(*startR) (void *) ){

	int i = 0;
	while( i < length ){
//		printf("attempt to init %d of pool\n", i);
		initThreadEnt( &pool[i], i, startR );
		i++;
	}
}

int initThreadEnt( threadEnt* entry, int id, void *(*startR) (void *) ){

	entry -> id = id;
	entry -> inUse = 0;
	pthread_mutex_init( &entry -> lock, NULL);
	
	int* input = malloc(sizeof(int));
	*input = id;

//	printf("lets try and create thread %d \n", id);
	int error = pthread_create( &(entry -> thread), NULL, startR, (void*) input );
	if(error){
		fprintf(stderr, "ERROR failed to create thread %d of type %s\n", 
			id, (startR == producer) ? "producer" : "consumer" );
		return 0;
	}

//	printf("started thread %d of type %s\n", 
//		id, (startR == producer) ? "producer" : "consumer" );

}

int stopThreadPool( threadEnt* pool, int length){
	int i = 0;

	while( i < length){
		pthread_mutex_destroy( &pool[i].lock );
		pthread_cancel( pool[i].thread );
		i++;
	}
}


/*------------------------------------------------------------------------*/
