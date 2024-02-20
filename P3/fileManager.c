#include "fileManager.h"
#include "myutils.h"

// since myutils.h already includes pthread.h we do not include them again

void  initialiseFdProvider(FileManager * fm, int argc, char **argv) {
    // Complete the initialisation
    /* Your rest of the initialisation comes here*/
    fm->nFilesTotal = argc -1;
    fm->nFilesRemaining = fm->nFilesTotal;
    // Initialise enough memory to  store the arrays
    fm->fdData = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fdCRC= malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileFinished = malloc(sizeof(int) * fm->nFilesTotal);
    fm->fileAvailable = malloc(sizeof(int) * fm->nFilesTotal);

    // Initialize synchronization tools
    my_sem_init(&semaphore, fm->nFilesTotal); // Semaphore to ensure threads executing concurrently
    pthread_mutex_init(&lock, NULL); // Lock to ensure only one thread is reading from the same file

    int i;
    for (i = 1; i < fm->nFilesTotal +1; ++i) {
        char path[100];
        strcpy(path, argv[i]);
        strcat(path, ".crc");
        fm->fdData[i] = open(argv[i], O_RDONLY); // storing file descriptor of data file to struct
        fm->fdCRC[i] = open(path, O_RDONLY); // storing file descriptor of crc file to struct

        fm->fileFinished[i] = 0; // file not completely read
        fm->fileAvailable[i] = 1; // file is available
    }
}

void destroyFdProvider(FileManager * fm) { // frees all alocated memory and closes open files
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
    // return information (fds crc and data, index of filemanager struct) in struct dataEntry
    // mark file as not available (fileAvailable 0)

    int i;
    for (i = 0; i < fm->nFilesTotal; ++i) {
        
        if (fm->fileAvailable[i] && !fm->fileFinished[i]) {

            pthread_mutex_lock(&lock); // lock to make sure only one thread is executing this section
            fm->fileAvailable[i] = 0; // mark that the file is not available 

            d->fdcrc = fm->fdCRC[i];
            d->fddata = fm->fdData[i];
            d->index = i;
            pthread_mutex_unlock(&lock);

            return 0;
        }
    }

    return 1; // if no file available, return 1
}

void unreserveFile(FileManager *fm,dataEntry * d) { // call when thread finishes reading the block in the specified file
    fm->fileAvailable[d->index] = 1; 
}

void markFileAsFinished(FileManager * fm, dataEntry * d) { // call when file completely read 
    fm->fileFinished[d->index] = 1;
    fm->nFilesRemaining--; // mark that a file has finished
    if (fm->nFilesRemaining == 0) {
        printf("All files have been processed\n");
        //TO COMPLETE: unblock all waiting threads, if needed
        // signal for all waitinig threads 
        // unlock mutex

    }
}
