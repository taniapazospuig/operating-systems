//
//  main.c
//  
//
//  Created by Julia Lopez Pinot on 22/1/24.
//

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "crc.h"
#include "timer.h"

// show options
void printMenu(){
    printf("Write one of the following options: 1. checksum <filename> -generate, 2. checksum <filename> -verify, 3. checksum <filename> -verify -maxNumErrors <numErrors> or 4. checksum <filename> -maxNumErrors <numErrors> -verify\n");
}

void generateCRCFile(char* filename){
    crcInit(); // initialize the CRCtable

    int fd = open(filename, O_RDONLY); //we open the file with read only mode
    if (fd == -1){
        printf("Error opening file\n"); //we write to the screen we could not open the file
        exit(5);
    }
    
    unsigned char buff[256];
    int bytesRead;
    crc crcValue;

    // creating new filename for CRC file by appending .crc to the data filename
    char crcFilename[50];
    strcpy(crcFilename, filename);
    strcat(crcFilename, ".crc");
    
    // open or create crc file (create or overwrite)
    int crcFd = open(crcFilename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(crcFd == -1){
        printf("Error opening or creating the CRC file\n");
        close(fd);
        exit(5);
    }
    
    long totalTimeRead = 0;
    long totalTimeCRC = 0;
    
    startTimer(); // timer to read input file
    bytesRead = read(fd, &buff, 256);
    totalTimeRead += endTimer(); // get reading time of first iteration
    while (bytesRead > 0){
        crcValue = INITIAL_REMAINDER; // reset crc value for each block

        startTimer(); // timer to compute CRC for block
        crcValue = crcSlow((unsigned char*)buff, bytesRead); // compute crc for this block
        totalTimeCRC += endTimer(); // stop timer for computing the crc and add the value

        if (write(crcFd, &crcValue, sizeof(crc)) == -1){ // writing he crc code of each block to crc file
            printf("Error writing to CRC file\n");
            close(fd); // close input file
            close(crcFd); // close crc file
            exit(5);
        }
        startTimer(); // add time reading for next iteration
        bytesRead = read(fd, &buff, 256);
        totalTimeRead += endTimer();
    }

    printf("Microseconds spent in read are %ld (generate)\n", totalTimeRead);
    printf("Microseconds spent in computing the CRC are %ld (generate)\n", totalTimeCRC);
    
    if (bytesRead == -1){
        printf("Error reading file\n");
        close(fd);
        close(crcFd);
        exit(5);
    }
    
    close(fd);
    close(crcFd);

}

void verifyCRC(char* filename, int maxNumErrors){
   
    char crcFilename[50];
    strcpy(crcFilename, filename);
    strcat(crcFilename, ".crc"); //we need to know the name of the crc file to extract the code
    
    int crcFd = open(crcFilename, O_RDONLY); // open the crc file in read mode
    if(crcFd == -1){
        printf("Error opening CRC file\n");
        exit(5);
    }
    
    crcInit();
    crc expectedCRC;
    crc actualCRC;
    int bytesRead;
    long totalTimeRead = 0;
    long totalTimeCRC = 0;
    int numErrors = 0;

    int fd = open(filename, O_RDONLY);
    if (fd == -1){
        printf("Error opening file\n");
        exit(5);
    } 
    unsigned char buff[256];

    while (1) {
        startTimer(); // timer for reading
        bytesRead = read(crcFd, &expectedCRC, sizeof(crc)); // read from crc file and store to expected crc
        totalTimeRead += endTimer(); // stop timer for reading

        if (bytesRead <= 0) break; // end of file

        startTimer();
        // Read block from input file
        bytesRead = read(fd, &buff, 256);
        totalTimeRead += endTimer();

        startTimer(); // timer for recomputing crc code
        actualCRC = crcSlow(buff, bytesRead);
        totalTimeCRC += endTimer(); // stop timer for recomputing crc code

        if(actualCRC != expectedCRC){
            numErrors += 1;
        }
    }

    close(fd);
    close(crcFd);

    printf("Microseconds spent in read are %ld (verify)\n", totalTimeRead);
    printf("Microseconds spent in computing the CRC are %ld (verify)\n", totalTimeCRC);
    
    if(maxNumErrors > 0 && numErrors > maxNumErrors){ //we have the argument maxNumErrors and file has more errors than permitted
        printf("File has %d errors\n", numErrors);
    } else if(maxNumErrors == 0 && numErrors != 0){ // we do not have the argument maxNumErrors and file has at least one error
        printf("File has %d errors\n", numErrors);
    } else {
        printf("File OK\n");
    }  
}


int main(int argc, char* argv[]){
    if (argc < 3 || argc > 5){
        printMenu();
        exit(22);
    }
    
    char* filename = argv[1]; // the name of the file is always the first argument (after the main)
    
    // flag to know options chosen
    int generate = 0;
    int verify = 0;
    int maxNumErrors = 0;

// parse through each arguments and compare them to the list of options
for(int i = 2; i < argc; i++){ //we start at i = 2 because the 2 first arguments are the main and the filename (always in that order)
        // assuming filename is correct
        if(strcmp(argv[i], "-generate") == 0){ // argument after filename is -generate
            generate = 1;
        }
        // generate and verify cannot be together
        else if(strcmp(argv[i], "-verify") == 0){ // argument after filename is -verify
            verify = 1;
        } else if (strcmp(argv[i], "-maxNumErrors") == 0){ // argument after filename is -maxNumErrors
            if(i+1 < argc){ //there is still another argument after the current one (the numErrors value)
                maxNumErrors = atoi(argv[i+1]); //convert string argument after -maxNumErrors to integer
                i++;  //skip next argument because we have already stored it (numErrors value)
            } else { // no numErrors value after -maxNumErrors
                printMenu();
                exit(22);
            }
        } else { // argument after filename is neither -generate, -verify, or -maxNumErrors
            printMenu();
            exit(22);
        }
    }
    
    if((generate && verify) || (!generate && !verify)){ //if both arguments appear or none of them appear, exit
        printMenu();
        exit(22);
    }
    
    if(generate){
        generateCRCFile(filename);
    } else if(verify){
        verifyCRC(filename, maxNumErrors);
    }
    
    return 0; 
}





