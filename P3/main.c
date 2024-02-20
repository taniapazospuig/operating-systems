#include "crc.h"
#include "fileManager.h"
#include "myutils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep function : waits for seconds

#define N 4

FileManager fm;

void* worker_function(void * arg){
    while (1){
        // case 1: more to read, so continue reading 
        // case 2: nothing more to read so mark file as finished
        // case 3: unreserve for the file and do the computation
        dataEntry  d;
        char * buff[256];
        short int crc;
        int res = getAndReserveFile(&fm, &d); // Reserves a file. The release is missing. Where should you put it?

        read(d.fdcrc, &crc, sizeof(short int)); // read crc file and put the value in variable crc
        int nBytesReadData = read(d.fddata, buff, 256); // read a block of data file and put it in the buffer

        if (crc != crcSlow(buff, nBytesReadData)) {
            printf("CRC error in file %d\n", d.filename);
            
        }

    }
}

int main(int argc, char ** argv) {
    initialiseFdProvider(&fm, argc, argv);
    pthread_t threadID[N];
    for (int i = 0; i < N; ++i) { // create 4 threads
        pthread_t thread;
        pthread_create(&threadID[i], NULL, worker_function, NULL); // threads execute worker function
    }

    for (int i = 0; i < N; ++i) { // wait until all threads have finished
        pthread_join(threadID[i], NULL);
    }
    destroyFdProvider(&fm);
}