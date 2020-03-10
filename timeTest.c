#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>


int main(int argc, char const *argv[])
{
	struct timeval time1;
	int error = gettimeofday( &time1, NULL);
	if(error){
		fprintf(stderr, "ERROR failed to gettimeofday in dequeue \n");
		return 0;
	}

	sleep(1);
	sched_yield();

	struct timeval time2;
	error = gettimeofday( &time2, NULL);
	if(error){
		fprintf(stderr, "ERROR failed to gettimeofday in dequeue \n");
		return 0;
	}

	printf("time1 seconds: %d   microseconds: %d \n", time1.tv_sec, time1.tv_usec );

	printf("time2 seconds: %d   microseconds: %d \n", time2.tv_sec, time2.tv_usec );
	
	return 0;
}
	
