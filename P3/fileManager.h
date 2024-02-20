#pragma once
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct {
    int fdcrc;
    int fddata;
    int index;
    char * filename;
} dataEntry;

typedef struct {
    int * fdData;
    int * fdCRC;
    int * fileFinished;
    int * fileAvailable;

    int nFilesRemaining;
    int nFilesTotal;
} FileManager;

// Synchronization tools as global variables
pthread_mutex_t lock;
sem_t semaphore;

void initialiseFdProvider(FileManager * fm, int argc, char **argv);
void destroyFdProvider(FileManager * fm);
int getAndReserveFile(FileManager *fm, dataEntry * d);
void unreserveFile(FileManager *fm,dataEntry * d);
void markFileAsFinished(FileManager * fm, dataEntry * d);
