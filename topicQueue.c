#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

//#include "topicEntry.h"
#include "topicQueue.h"

/*--------------------------- HELPER FUNCTIONS ---------------------------*/

int deleteLock(topicQueue* TQ){

	int error = pthread_mutex_destroy( &(TQ->lock) );
	if( error ){
		fprintf(stderr, "ERROR failed to delete lock of topicQueue %s\n", 
			TQ -> id);
		return 0;
	}

	return 1;
}

int copyTE(topicEntry* dst, topicEntry* src){
	dst -> entryNum = src -> entryNum;
	dst -> timeStamp = src -> timeStamp;
	dst -> pubID = src -> pubID;
	strcpy(dst -> photoURL, src -> photoURL);
	strcpy(dst -> photoCaption, src -> photoCaption);

	return 1;
}

/*-------------------------- REQUIRED FUNCTIONS --------------------------*/
/*-------------- access to topicQueues must be synchronized --------------*/

int enqueue(topicQueue* TQ, topicEntry* TE){

	// get lock
	int error = pthread_mutex_lock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to lock in enqueue \n");
		return 0;
	}

	// test for full
	if ( TQ -> full ){
/**/		printf("enqueue failed: queue is full\n");

		//unlock
		error = pthread_mutex_unlock( &( TQ -> lock ) );

		if(error){
			fprintf(stderr, "ERROR failed to unlock in enqueue \n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}
	
	// enqueue
	//insert into head++
/**/	printf("head before insert %d\n", TQ -> head );
	topicEntry* new = &(TQ -> buffer)[TQ -> head];
	TQ -> head = ++(TQ -> head) % (TQ -> length);
/**/	printf("head after insert %d\n", TQ -> head );

	if( TQ -> head == TQ -> tail ){
		TQ -> full = 1;
	}

	//set entryNum to entries++
/**/	printf("entries before insert %d\n", TQ -> entries );
	new -> entryNum = ++(TQ -> entries);
/**/	printf("entries after insert %d   entryNum %d\n", 
/**/			TQ -> entries, new -> entryNum );

	//set timeStamp
	error = gettimeofday( &(new -> timeStamp), NULL);
	if(error){
		fprintf(stderr, "ERROR failed to gettimeofday in enqueue \n");

		error = pthread_mutex_unlock( &( TQ -> lock ) );
		if(error){
			fprintf(stderr, "ERROR failed to unlock in enqueue \n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	//copy over pubID, URL, and Cap
	new -> pubID = TE -> pubID;
	strcpy(new -> photoURL, TE -> photoURL);
	strcpy(new -> photoCaption, TE -> photoCaption);

	// unlock
	error = pthread_mutex_unlock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to unlock in enqueue \n");
		exit(EXIT_FAILURE);
	}

	return 1;
}


int getEntry(topicQueue* TQ, int lastEntry, topicEntry* TE){ 

	int error = pthread_mutex_lock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to lock in getEntry \n");
		return 0;
	}

	//check if the topic queue is empty
	if( (TQ -> head == TQ -> tail) && !( TQ -> full ) ){
		printf("getEntry: topicQueue is empty\n");

		error = pthread_mutex_unlock( &( TQ -> lock ) );
		if(error){
			fprintf(stderr, "ERROR failed to unlock in getEntry \n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}

	//try to find the smallest entry number >lastEntry
	int curEnt = TQ -> tail;

	while(1){
		if(  ( (( TQ -> buffer )[curEnt]).entryNum ) > lastEntry  ){
			//copy topic entry
			copyTE(TE, &( TQ -> buffer )[curEnt]);
			
/**/			printf("read index: %d   entryNum: %d\n",
	       			curEnt, (( TQ -> buffer )[curEnt]).entryNum); 

			error = pthread_mutex_unlock( &( TQ -> lock ) );
			if(error){
				fprintf(stderr, "ERROR failed to unlock in getEntry \n");
				exit(EXIT_FAILURE);
			}
			
/**/			printf("return from get entry\n");
			return curEnt;
		}

		curEnt = (curEnt + 1) % (TQ -> length);

		if( curEnt == (TQ -> head) ){
			break;
/**/			printf("THIS SHOULD NOT PRINT\n");
		}
	}

	error = pthread_mutex_unlock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to unlock in getEntry \n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
	

int dequeue(topicQueue* TQ){ 

	int error = pthread_mutex_lock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to lock in dequeue \n");
		return 0;
	}

	//start at tail and check time deltas until we find something in bounds of being allowed to stay
	
	//get a time to compare against
	struct timeval time;
	error = gettimeofday( &time, NULL);
	if(error){
		fprintf(stderr, "ERROR failed to gettimeofday in dequeue \n");

		error = pthread_mutex_unlock( &( TQ -> lock ) );
		if(error){
			fprintf(stderr, "ERROR failed to unlock in dequeue \n");
			exit(EXIT_FAILURE);
		}

		return 0;
	}
	
	if( (TQ -> head == TQ -> tail) && !(TQ -> full) ){
/**/		printf("Nothing to dequeue: queue is empty\n");
	} else {

		int curEnt = TQ -> tail;

		//check if curEnt is to be deleted
		double diff = difftime(time.tv_sec, (TQ->buffer)[curEnt].timeStamp.tv_sec);
		if( diff >= DELTA ){
/**/			printf("dequeue entry in index %d\n", curEnt); 

			// handle queue starts full and ends up not full
			if( TQ -> full){
				TQ -> full = 0;
			}
			curEnt = (curEnt + 1) % (TQ -> length);
	
			while(curEnt != TQ -> head){
				diff = difftime(time.tv_sec, (TQ->buffer)[curEnt].timeStamp.tv_sec);

				if(diff < DELTA){ 
					//this entry doesn't get dequeued
					break;
				}
				printf("dequeue entry in index %d\n", curEnt); 
				curEnt = (curEnt + 1) % (TQ -> length);
	
			}

		} else {
			printf("not dequeueing. delta is %d   dif is %d\n", 
					DELTA, diff);
		}

		TQ -> tail = curEnt;	
	}

	error = pthread_mutex_unlock( &( TQ -> lock ) );
	if(error){
		fprintf(stderr, "ERROR failed to unlock in dequeue \n");
		exit(EXIT_FAILURE);
	}

	return 1;
}
	

/*------------------------------------------------------------------------*/

void* cleanup(void* input){

	//loop through topic store and call dequeue on each queue
	
	topicQueue** topicStore = (topicQueue**) input;

	int old;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);
	sched_yield();
	
	int i = 0;

	while(1){
		dequeue( topicStore[i] );

		if( ++i == topics ){
			i = 0;
			sched_yield();
		}
	}
}

