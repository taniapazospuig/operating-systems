#include "crc.h"
#include "fileManager.h"
#include "myutils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for sleep function : waits for seconds

FileManager fm;

void* worker_function(void * arg){
    while (1){
        // case 1: more to read, so continue reading 
        // case 2: nothing more to read so mark file as finished
        // case 3: unreserve for the file and do the computation
        dataEntry  d;
        unsigned char buff[256];
        crc crc;

        //place semaphore to manage the number of threads accessing the function concurrently
        sem_wait(&semaphore); 
        int res = getAndReserveFile(&fm, &d); // Reserves a file 
        sem_post(&semaphore); 

        // Case 1
        if (res == 0) {
            read(d.fdcrc, &crc, sizeof(short int)); // read crc file and put the value in variable crc
            int nBytesReadData = read(d.fddata, buff, 256); // read a block of data file and put it in the buffer

            if(nBytesReadData == 0){ // there is no more data to read in this file
                unreserveFile(&fm, &d); 
                markFileAsFinished(&fm, &d); 
                
            } 
            else{
                if (crc != crcSlow(buff, nBytesReadData)) { // compute the crc and compare it to the read crc
                    printf("CRC error in file %s\n", d.filename);
                }
                unreserveFile(&fm, &d); 
            }
            
        }
        
        // Case 2
        if (res == 1 && fm.nFilesRemaining > 0){ // no files available and not all files finished
           pthread_join(*(pthread_t *)arg, NULL); // we make the thread wait for a signal 
        }

        if (res == 1 && fm.nFilesRemaining == 0){
            pthread_exit(NULL); //all files are finished
        }
    }
}

int main(int argc, char ** argv) {
    initialiseFdProvider(&fm, argc, argv);
    pthread_t threadID[fm.nFilesTotal]; // create one thread per file
    for (int i = 0; i < fm.nFilesTotal; ++i) { 
        pthread_t thread;
        pthread_create(&threadID[i], NULL, worker_function, &threadID[i]); // threads execute worker function
        // we pass the threadID to the worker function
    }

    for (int i = 0; i < fm.nFilesTotal; ++i) { // wait until all threads have finished
        pthread_join(threadID[i], NULL);
    }

    destroyFdProvider(&fm);
    sem_destroy(&semaphore); 
    return 0; 
}