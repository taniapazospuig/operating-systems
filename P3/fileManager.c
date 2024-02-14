#include "fileManager.h"
#include "myutils.h"

void  initialiseFdProvider(FileManager * fm, int argc, char **argv) {
    // Complete the initialisation
    /* Your rest of the initialisation comes here*/
    my_semaphore semaphore;
    my_sem_init(&semaphore, 1); // critical section can only be executed by one thread at a time

    fm->nFilesTotal = argc -1;
    fm->nFilesRemaining = fm->nFilesTotal;
    // Initialise enough memory to  store the arrays
    fm->fdData = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fdCRC= malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileFinished = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileAvailable = malloc(sizeof(int) * fm->nFilesTotal);

    int i;
    for (i = 1; i < fm->nFilesTotal +1; ++i) {
        // This is the critical section, as 2 threads might get the same index and thus try to store the file descriptors of the two files in the same
        // position of the arrays of the fm struct
        my_sem_wait(&semaphore);
        char path[100];
        strcpy(path, argv[i]);
        strcat(path, ".crc");
        fm->fdData[i] = open(argv[i], O_RDONLY); // storing file descriptor of data file to struct
        fm->fdCRC[i] = open(path, O_RDONLY); // storing file descriptor of crc file to struct

        fm->fileFinished[i] = 0; // file not completely read
        fm->fileAvailable[i] = 1; // file being read by a thread
        my_sem_signal(&semaphore);
    }
}

void  destroyFdProvider(FileManager * fm) {
    int i;
    for (i = 0; i < fm->nFilesTotal; i++) {
        close(fm->fdData[i]);
        close(fm->fdCRC[i]);
    }
    free(fm->fdData);
    free(fm->fdCRC);
    free(fm->fileFinished);
}
int getAndReserveFile(FileManager *fm, dataEntry * d) {
    // This function needs to be implemented by the students
    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        if (fm->fileAvailable[i] && !fm->fileFinished[i]) {
            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;

            // You should mark that the file is not available 
            ??

            return 0;
        }
    }             
    return 1;
}
void unreserveFile(FileManager *fm,dataEntry * d) {
    fm->fileAvailable[d->index] = 1; 
}

void markFileAsFinished(FileManager * fm, dataEntry * d) {
    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; //mark that a file has finished
    if (fm->nFilesRemaining == 0) {
        printf("All files have been processed\n");
        //TO COMPLETE: unblock all waiting threads, if needed
    }
}
