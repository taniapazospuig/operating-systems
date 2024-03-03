#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>   // for usleep waits for microsec
#include <unistd.h> // for sleep: waits for seconds
#include "crc.h"
#include "fileLock.h"
#define N 4

typedef struct {
    int nBlock;
    int isGet; //if get 1, if generate 0
} Request;

typedef struct {
    int nBlock;
    unsigned short int crc;
} Result;

//put error handling messages!!!
int main(int argc, char * argv[]) {
    int pipeA[2], pipeB[2];
    // pipe B --> communication from child to parent
    // pipe A --> communication from parent to child
    // Create two pipes. Also remember to close the channels when needed, otherwise it will not work!

    if(pipe(pipeA) == -1){
        printf("Pipe creation failed\n"); 
        exit(1); 
    }

    if(pipe(pipeB) == -1){
        printf("Pipe creation failed\n"); 
        exit(1); 
    }

    for (int i = 0; i < N; ++i) { //we create 4 child processes --> each of the filds has 2 file descriptors
        int n = fork();
        if (n == -1){
            printf("An error ocurred with the fork\n"); 
            exit(1); 
        }
        if (n == 0) { //piece of code run by the child process
            //children will read from pipeA and write to pipeB
            close(pipeA[1]);  //close write for pipe A
            close(pipeB[0]);  //close read for pipe B
            
            int fd = open(argv[1], O_RDONLY); //open data file read only
            int fdCRC = open(argv[2], O_RDWR); //open crc file read and write
            //crcInit(); 
            Request r; // from the standard input --> either generate or get
            // we create an instance of r
            while(read(pipeA[0], &r, sizeof(Request)) >0) { //while there are requests to read
                if (!r.isGet) { //generate
                    off_t offset_data = (r.nBlock -1)*256; 
                    lseek(fd, offset_data, SEEK_SET); //to move the reading pointer to read the block
                    char buff[256]; 
                    int nBytesRead = read(fd, (unsigned char*)buff, 256); 
                    if(nBytesRead > 0){
                        off_t offset = (r.nBlock -1)*sizeof(short int); 
                        unsigned short int crcValue = crcSlow( (unsigned char*)buff, nBytesRead); 
                        printf("crc value generated %d\n", crcValue); 
                        file_lock_write(fdCRC, offset, sizeof(short int)); //we put a lock to write the recomputed crc value in the CRC file
                        lseek(fdCRC,offset, SEEK_SET); //move pointer to write 
                        write(fdCRC, &crcValue, sizeof(short int)); 
                        file_unlock(fdCRC, offset, sizeof(short int)); 
                    }
                    /* Recompute the CRC, use lseek to get the correct datablock,
                    and store it in the correct position of the CRC file. Remember to use approppriate locks! */
                    usleep(rand()%1000 *1000); // Make the computation a bit slower

                }
                else{ //get
                    usleep(rand()%1000 *1000);
                    Result res; 
                    res.nBlock = r.nBlock;
                    // Read the CRC from the CRC file, using lseek + read. Remember to use the correct locks!
                    off_t offset = (r.nBlock -1)*sizeof(short int); 
                    printf("offset %lld \n", offset); 
                    int flr = file_lock_read(fdCRC, offset, sizeof(short int)); 
                    printf("file lock read %d\n", flr); 
                    lseek(fdCRC, offset, SEEK_SET); 
                    int nBytesReadCRC = read(fdCRC, &res.crc, sizeof(short int)); 
                    printf("crc %d\n", res.crc); 
                    printf("bytes read %d\n", nBytesReadCRC); 
                    int fu = file_unlock(fdCRC, offset, sizeof(short int)); 
                    printf("file unlock %d\n", fu); 
                    
                    //Write the result in pipeB!
                    if(nBytesReadCRC > 0){
                        write(pipeB[1], &res, sizeof(Result)); 
                    }
                }
            }
            close(pipeA[0]); //close read pipe A 
            close(pipeB[1]); //close write pipe B
            close(fd); 
            close(fdCRC); 
            exit(0);
        }
    }

    //parent will only write to pipe A and read from pipe B
    close(pipeA[0]); //close read pipe A 
    close(pipeB[1]); //close write pipe B

    char s[100];
    int nBytesRead;
    /* Read from the standard input*/
    while((nBytesRead = read(0, s, 100) ) > 0) {
        char op[200];
        s[nBytesRead] = '\0';
        int nBlock;

        sscanf(s, "%s %d", op, &nBlock);
        Request r;
        r.nBlock = nBlock;
        r.isGet = strcmp(op, "get") == 0;
        // Write r in the pipe!
        write(pipeA[1], &r, sizeof(Request)); 
    }
    close(pipeA[1]);
    printf("FINISHED\n");
    while(wait(NULL) == -1);

    // Now that is finished, write all the results
    Result res;
    while((nBytesRead = read(pipeB[0], &res, sizeof(res)) ) > 0) {
        printf("The CRC of block #%d is %d \n", res.nBlock, res.crc);
    }

     
    close(pipeB[0]);  
}
