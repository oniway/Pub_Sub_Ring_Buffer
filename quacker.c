#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "threadPool.h"

#define MAXTOPICS 20


topicQueue* topicStore[MAXTOPICS];
pthread_mutex_t topics_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_t cleaner;

/*---------------------------- MAIN FUNCTION -----------------------------*/
int main(int argc, char const *argv[])
{
	topics = 0;  //declared in topicQueue.h
/*
	printf("name-%s  entries-%d  head-%d  tail-%d full-%d length-%d\n", 
			topicStore[0]->name, topicStore[0]->entries,
			topicStore[0]->head, topicStore[0]->tail,
			topicStore[0]->full, topicStore[0]->length);
*/

	if(argc < 2){
		fprintf(stderr,"ERROR: no filename given");
		exit(EXIT_FAILURE);
	}

	initThreadPool(producerPool, NUMPROXIES, &producer);
	initThreadPool(consumerPool, NUMPROXIES, &consumer);

	//declarations for reading command file
	FILE* f;
	char* lineBuf = NULL;
	size_t bufLen = 0;
	ssize_t read;
	char* ptr;	//save pointer for strtok_r
	char* command;
	char* arg1;
	char* arg2;
	char* arg3;
	char* arg4;
	int num1;
	int num2;
	int gotNum1;
	int gotNum2;

	f = fopen(argv[1], "r");
		if(!f){
			fprintf(stderr, "failure to open file by publisher %d\n", argv[1]);
			return;
		}

		//read file 
		read = getline( &lineBuf, &bufLen, f );
/**/			printf("%s\n", lineBuf);
		while( (read != -1) ){
			//break line up
			//printf("\n next line: \n%s\n", lineBuf);

			command = strtok_r(lineBuf, " \n", &ptr);

			arg1 = strtok_r(NULL, " \"\n", &ptr);
			arg2 = strtok_r(NULL, " \"\n", &ptr);
			arg3 = strtok_r(NULL, " \"\n", &ptr);
			arg4 = strtok_r(NULL, " \"\n", &ptr);


			//identify command
			if(!strcmp(command, "create") && arg1 && arg2 && arg3 && arg4){
				// add a new queue
				gotNum1 = sscanf(arg2, "%d", &num1);
				gotNum2 = sscanf(arg4, "%d", &num2);

				if ( gotNum1 && gotNum2 ){
					addTopic(num1, arg3, num2);
				}

			}else if(!strcmp(command, "query") && arg1 ){

				if(!strcmp(arg1, "topics")){
					queryTopics();
				} else if(!strcmp(arg1, "publishers")){
					queryPublishers();
				} else if(!strcmp(arg1, "subscribers")){
					querySubscribers();
				}

			}else if(!strcmp(command, "add") && arg1 && arg2 ){

				if(!strcmp(arg1, "publisher")){
					addPublisher(arg2);
				} else if(!strcmp(arg1, "subscriber")){
					addSubscriber(arg2);
				}

			}else if (!strcmp(command, "delta") && arg1){

				if(arg1){
					sscanf(arg1, "%d", &num1);
					setDelta(num1);
				}

			}else if (!strcmp(command, "start") ){
				start();
			}
			read = getline( &lineBuf, &bufLen, f );
		}

		//close file
		fclose(f);
	
//end things	
	
	int i = 0;
/*	//so we don't end before the threads (hacky solutions but whatever)
	while( i < NUMPROXIES ){
		if (consumerPool[i].inUse || producerPool[i].inUse){
			sched_yield();
		} else {
			i++;
		}
	}
*/
	sched_yield();
	sched_yield();

	stopThreadPool(producerPool, NUMPROXIES);
	stopThreadPool(consumerPool, NUMPROXIES);

	pthread_cancel( cleaner);
	//pthread_mutex_destroy(&cond_mutex);
	//pthread_cond_destroy(&cond);


	i = 0;
	topicEntry* buff; 
	while( i < topics ){

		buff = topicStore[i]->buffer;
		free(buff);
		free(topicStore[i]);
		i++;
	}

	return 0;
}

/*-------------------------- HELPER FUNCTIONS ----------------------------*/
int addTopic(int id, char* name, int length){
	/*create topic <topic ID> "<topic name>" <queue length>
	 *Create a topic with ID (integer) and length. 
	 *This allocates a topic queue. */

	pthread_mutex_lock( &topics_mutex );

	//add topic to topicStore at topics and add one to topics
	//
	topicStore[topics] = malloc(sizeof(topicQueue));

	topicEntry* buf = malloc(sizeof(topicEntry) * length);

	topicStore[topics][0].id = id;
	topicStore[topics][0].buffer = buf;
	topicStore[topics][0].head = 0;
	topicStore[topics][0].tail = 0;
	topicStore[topics][0].full = 0;
	topicStore[topics][0].length = length;
	pthread_mutex_init( &topicStore[topics][0].lock, NULL );
	
	topics++;

	pthread_mutex_unlock( &topics_mutex );
}

int queryTopics(){
	/*query topics
	 *Print out all topic IDs and their lengths. */

	pthread_mutex_lock( &topics_mutex );

	int i = 0;

	while( i < topics ){
		//some printing

		printf("ID: %d   length: %d\n",
			topicStore[i]->id, topicStore[i]->length);
	i++;
	}

	pthread_mutex_unlock( &topics_mutex );
}

int addPublisher(char* file){
	/*add publisher "<publisher command file>"

	 *Create a publisher. 
	 *A free thread is allocated to be the “proxy" for the publisher. 
	 *When the publisher is started (see below), the thread reads its commands from the file.*/

	//loop through producerPool and trylock until you get one that isn't in use
	printf("in addPublisher\n");

	int i = 0;
	while( i < NUMPROXIES){

		printf("in addPublisher: before trylock\n");

		if( !pthread_mutex_trylock( &producerPool[i].lock ) ){
			printf("in addPublisher: before testing inuse\n");

			
			if( !( producerPool[i].inUse ) ){

				strcpy( producerPool[i].fileName, file );
				producerPool[i].inUse = 1;
				printf("added producer: %d  file: %s \n", i, file);
				break;
			}

		} 
		
		pthread_mutex_unlock( &producerPool[i].lock );

		i++;
	}

	if( i == NUMPROXIES ){
		return 0;
	} else {
		return 1;
	}
}

int queryPublishers(){
	/*query publishers
	 *Print out current publishers and their command file names.*/

	int i = 0;
	while( i < NUMPROXIES){

		//print id and filename
		printf("Publisher: %d   Command File: %s   inUse: %d\n", 
			producerPool[i].id , producerPool[i].fileName,
		     	producerPool[i].inUse );

		i++;
	}

}

int addSubscriber(char* file){
	/*add subscriber "<subscriber command file>"
	 *Create a subscriber. A free thread is allocated to be the "proxy" for the subscriber. 
	 *When the subscriber is started (see below), the thread reads its commands from the file.
	 */

	//loop through consumerPool and trylock until you get one that isn't in use
	int i = 0;
	while( i < NUMPROXIES){

		
		if( !pthread_mutex_trylock( &consumerPool[i].lock ) ){

			

			if( !( consumerPool[i].inUse ) ){

				strcpy( consumerPool[i].fileName, file );
				consumerPool[i].inUse = 1;

				break;
			}

		} 
		pthread_mutex_unlock( &consumerPool[i].lock ); 
		i++;
	}

	if( i == NUMPROXIES ){	return 0;
	} else {	return 1;}

}

int querySubscribers(){
	/*query subscribers
	 *Print out subscribers and their command file names.
	 */

	int i = 0;
	while( i < NUMPROXIES){

		//print id and filename
		printf("Consumer: %d   Command File: %s   inUse: %d\n", 
			consumerPool[i].id , consumerPool[i].fileName,
		        consumerPool[i].inUse );
		i++;
	}
}

int setDelta(int delta){
	/*delta <DELTA>
	 *Set DELTA to the value specified.
	 */

	DELTA = delta;
}

int start(){
	/*start
	 *Start all of the publishers and subscribers. 
	 *Just before this happens, the cleanup thread is started.
	 */

	
	int error = pthread_create( &cleaner , NULL, &cleanup, (void*) topicStore );
	if(error){
		fprintf(stderr, "ERROR failed to create cleanup thread\n" );
		return 0;
	}

	printf("Main program: send start signal\n");

	pthread_mutex_lock( &cond_mutex );
	pthread_cond_broadcast( &cond );
	pthread_mutex_unlock( &cond_mutex );

	return 1;
}

/*------------------------------------------------------------------------*/

int threadSleep(int length){

	struct timeval starttime;

	int error = gettimeofday( &starttime, NULL);
	if(error){
		fprintf(stderr, "ERROR failed to gettimeofday in threadSleep \n");

		return 0;
	}

	struct timeval nowtime;
	double diff;	
	while(1){
		int error = gettimeofday( &nowtime, NULL);
		if(error){
			fprintf(stderr, "ERROR failed to gettimeofday in threadSleep \n");

			return 0;
		}

		diff = difftime(nowtime.tv_sec, starttime.tv_sec);
		if(diff > length){
			break;
		}else{
			sched_yield();
		}
	}
}

topicQueue* findTQ(int id){
	int i = 0;
	while( (i < topics) ){
		if( topicStore[i]->id == id ){
			return topicStore[i];
		}
		i++;
	}
	return NULL;
}

/*--------------------------- THREAD FUNCTIONS ---------------------------*/

void* producer(void* input){

	int id = ((int*) input)[0];
	free(input);
	threadEnt* entry = &producerPool[id];

	FILE* f = NULL;
	char* lineBuf = NULL;
	size_t bufLen = 0;
	char* ptr = NULL;	//save pointer for strtok_r
	int num1 = 0;
	int hasNum = 0;

	printf("Proxy thread %d - type: Publisher \n", entry->id);
	
	while(1){
		
		pthread_mutex_lock( &cond_mutex );
		pthread_cond_wait( &cond, &(cond_mutex) );
		int error = pthread_mutex_unlock( &( cond_mutex ) );

		if(error){
			fprintf(stderr, 
				"ERROR failed to unlock in producer %d\n",
			       	entry->id);
			exit(EXIT_FAILURE);
		}

		pthread_mutex_lock( &(entry->lock) );

		if(entry -> inUse){

			int go = 1;

/**/		printf("thread %d: producer %d: started\n", pthread_self(), entry->id);

			//open file
			f = fopen(entry->fileName, "r");
			if(!f){
				fprintf(stderr, "failure to open file by publisher %d\n", entry->id);
				return;
			}

			//read file 
			ssize_t read = getline( &lineBuf, &bufLen, f );
/**/			printf("%s\n", lineBuf);
			while( (read != -1) & go ){
				//break line up
				printf("\n next line: \n%s\n", lineBuf);

				char* command = strtok_r(lineBuf, " \n", &ptr);
				char* arg1 = strtok_r(NULL, " \"\n", &ptr);
				char* arg2 = strtok_r(NULL, " \"\n", &ptr);
				char* arg3 = strtok_r(NULL, " \"\n", &ptr);

				if(arg1){
					hasNum = sscanf(arg1, "%d", &num1);
				} else {
					hasNum = 0;
				}

//				printf("%s-%s-%s-%s-%d\n", command, arg1, arg2, arg3, num1);

				//identify command
				if(!strcmp(command, "put") && hasNum && arg2 && arg3){
					//find correct queue
					topicQueue* TQ = findTQ(num1);
					if(TQ){
					
						//make entry
						topicEntry TE;
						TE.pubID = entry->id;
						strcpy(TE.photoURL, arg2);
						strcpy(TE.photoCaption, arg3);

						//enqueue entry
						enqueue(TQ, &TE);

					}
				}else if (!strcmp(command, "sleep") && hasNum){
					
					threadSleep(num1);
				}else if (!strcmp(command, "stop") ){
					go = 0;
				}

				read = getline( &lineBuf, &bufLen, f );
			}

			//close file
			fclose(f);

			entry -> inUse = 0;		
		} else {	//this thread is not to do anything yet
			sched_yield();
		}

		pthread_mutex_unlock( &(entry->lock) );	
	}

	free(lineBuf);
}

void* consumer(void* input){

	int id = ((int*) input)[0];
	free(input);

	threadEnt* entry = &consumerPool[id];

	//structure to track last entries
	int queues[MAXTOPICS];
	int LastEnt[MAXTOPICS];
	int queueCount;

	FILE* f = NULL;
	char* lineBuf = NULL;
	size_t bufLen = 0;
	char* ptr = NULL;
	int num1 = 0;
	int hasNum = 0;

	printf("Proxy thread %d - type: Consumer \n", entry->id);

	while(1){
		
		pthread_mutex_lock( &cond_mutex );
		pthread_cond_wait( &cond, &(cond_mutex) );
		int error = pthread_mutex_unlock( &( cond_mutex ) );

		if(error){
			fprintf(stderr, "ERROR failed to unlock in consumer %d \n", id);
			exit(EXIT_FAILURE);
		}

		// see what we have to do
		pthread_mutex_lock( &(entry->lock) );

		if(entry -> inUse){

			int go = 1;
			queueCount = 0;


/**/		printf("thread %d: consumer %d: started\n", pthread_self(), entry->id);


			//open file 
			f = fopen(entry ->fileName, "r");
			if(!f){
				fprintf(stderr, "failure to open file by consumer %d\n", entry->id);
				return;
			}

			ssize_t read = getline( &lineBuf, &bufLen, f );
/**/			printf("%s\n", lineBuf);
			while( (read != -1) & go ){
				//break line up
				printf("\n next line: \n%s\n", lineBuf);

				char* command = strtok_r(lineBuf, " \"\n", &ptr);

				char* arg1 = strtok_r(NULL, " \"\n", &ptr);

				if(arg1){
					hasNum = sscanf(arg1, "%d", &num1);
				} else {
					hasNum = 0;
				}

//				printf("%s-%s-%d\n", command, arg1,  num1);
				
				//find what command
				if(!strcmp(command, "get") && hasNum){
					//find or add topic ID to queues
					int i = 0;
					int found = 0;
					int last = -1;
					while( (i < queueCount) && !found ){
						if( queues[i] == num1 ){
							last = LastEnt[i];
							found = 1;
						}else{
							i++;
						}
					} if( !found ){
						queues[queueCount] = num1;
						i = queueCount;
						queueCount++;
					}
					topicQueue* TQ = findTQ(num1);
					if(TQ){
						topicEntry TE;
						last = getEntry(TQ, last, &TE);

						printf("subscriber %d: entryNum-%d pubID-%d photoCaption-%s", 
							entry->id, TE.entryNum, TE.pubID, TE.photoCaption );
					}else{
						fprintf(stderr, "ERROR: could not find queue %d\n", num1);
					}

					
					//call getEntry
					

				}else if (!strcmp(command, "sleep") && hasNum){
					
					threadSleep(num1);
				}else if (!strcmp(command, "stop") ){
					
					go = 0;
				}

				read = getline( &lineBuf, &bufLen, f );
			}


			//close file
			fclose(f);

			entry -> inUse = 0;
		} else {	//this thread is not to do anything yet
			sched_yield();
		}

		pthread_mutex_unlock( &(entry->lock) );

	}

	free(lineBuf);
}


/*------------------------------------------------------------------------*/

