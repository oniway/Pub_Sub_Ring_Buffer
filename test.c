#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#include "test.h"
//#include "topicEntry.h"
//#include "topicQueue.h"

#define PASSES 5

int spawnProducers(topicQueue* TQ, int producers, int writes){

	pthread_t threads[producers];

	int nums[1]; // contains number of writes and spawn number of the thread
	nums[0] = writes;

	void* input[2] = { TQ, &nums };
	//input[0] = TQ;
	//input[1] = &nums;

	int error;

	// spawn threads
	int i = 0;
	while( i < producers ){
	
		error = pthread_create( &threads[i], NULL, &produce, input );
		if(error){
			fprintf(stderr, "ERROR failed to create producer thread %d\n", i);
			return 0;
		}
		i++;
	}

	// join threads
	i = 0;
	while( i < producers ){
		error = pthread_join( threads[i], NULL );
		if(error){
			fprintf(stderr, "ERROR failed to join producer thread %d\n", i);
			return 0;
		}
		i++;
	}

	return 1;
}

int spawnConsumers(topicQueue* TQ, int consumers, int reads){

	pthread_t threads[consumers];

	int nums[1]; // contains number of writes and spawn number of the thread
	nums[0] = reads;

	void* input[2];
	input[0] = TQ;
	input[1] = &nums;

	int error;

	// spawn threads
	int i = 0;
	while( i < consumers ){
		
		printf("create consumer thread %d\n", i);
		error = pthread_create( &threads[i], NULL, &consume, input );
		if(error){
			fprintf(stderr, "ERROR failed to create consumer thread %d\n", i);
			return 0;
		}
		i++;
	}

	// join threads
	i = 0;
	while( i < consumers ){
		error = pthread_join( threads[i], NULL );
		if(error){
			fprintf(stderr, "ERROR failed to join consumer thread %d\n", i);
			return 0;
		}
		i++;
	}
	return 1;
}

int spawnCleanup(topicQueue* TQ){

	pthread_t thread;
	int error;

	error = pthread_create( &thread, NULL, &cleanup, &TQ );
	if(error){
			fprintf(stderr, "ERROR failed to join cleanup thread\n");
			return 0;
	}

	error = pthread_join( thread, NULL );
	if(error){
			fprintf(stderr, "ERROR failed to join cleanup thread\n");
			return 0;
	
	}
	return 1;
}



void* produce(void* input){
	
	topicQueue* TQ = ( (topicQueue**) input )[0];
	int writes = ( (int** ) input )[1][0];
	pthread_t id = pthread_self();
	topicEntry TE = { .pubID = (int) id };

	int i = 0;
	while( i < writes ){
		enqueue(TQ, &TE);
		i++;
		printf("producer: %d  numberPublished: %d \n", id, i);
	}
	
}

void* consume(void* input){

	topicQueue* TQ = ( (topicQueue**) input )[0];
	int reads = ( (int** ) input )[1][0];
	int id = pthread_self();

	printf("topicQueue %s    reads %d\n", TQ -> name, reads);

	int lastEntry = 0;
	int currentEntry = 0;

	topicEntry TE;

	int i = 0;
	while( i < reads ){
		lastEntry = getEntry(TQ, currentEntry, &TE);

		if(lastEntry){
			currentEntry = lastEntry;
			printf("consumer: %d  numberConsumed: %d 	pubID: %d 	entryNum: %d \n", 
				id, i, TE.pubID, TE.entryNum);
		}
		i++;
	}
	
}

void* cleanup(void* input){
	topicQueue* TQ = ( (topicQueue**) input)[0];
	dequeue(TQ);

}
