CC=gcc
cflag= -g -pthread
main: topicQueue.o threadPool.o quacker.o 
	$(CC) topicQueue.o threadPool.o quacker.o -g -o quacker -pthread
topicQueue: topicQueue.c topicQueue.h topicEntry.h
	$(CC) -c topicQueue.c $(cflag) 
quacker: quaker.c test.h topicEntry.h topicQueue.h threadPool.h
	$(CC) -c quacker.c $(cflag)
threadPool: threadPool.c threadPool.h topicEntry.h topicQueue.h
	$(CC) -c threadPool.c $(cflag)
clean:
	rm -r *.o quacker
