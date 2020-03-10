#include <sys/time.h>

#define URLSIZE 100
#define CAPSIZE 100

typedef struct {
	int entryNum;
	struct timeval timeStamp;
	int pubID;
	char photoURL[URLSIZE];
	char photoCaption[CAPSIZE];
} topicEntry;

