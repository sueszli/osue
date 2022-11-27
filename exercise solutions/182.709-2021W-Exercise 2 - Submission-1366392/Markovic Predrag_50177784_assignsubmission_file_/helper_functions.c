#include "helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void checkCloseRes(const int RES) {
    if(RES < 0) {
        fprintf(stderr,"./forksort: Could not duplicate file descriptor: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void checkDubRes(const int RES) {
    if(RES < 0) {
        fprintf(stderr,"./forksort: Could not close file descriptor: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void checkPipeRes(const int RES) {
    if(RES < 0) {
        fprintf(stderr,"./forksort: Could not create pipe: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void writeStreamsToStdout(const int FD1, const int FD2) {

        FILE* readSource1 = fdopen(FD1, "r");
        FILE* readSource2 = fdopen(FD2, "r");

        if(readSource1 == NULL || readSource2 == NULL) {
            fprintf(stderr, "Could not read sorted stream from pipe!\n");
            exit(EXIT_FAILURE);
        }

        char readBuf1[BUFSIZ];
        char readBuf2[BUFSIZ];
        memset(readBuf1, 0, sizeof(readBuf1));
        memset(readBuf2, 0, sizeof(readBuf2));

        char* b1res = fgets(readBuf1, sizeof(readBuf1), readSource1);
        char* b2res = fgets(readBuf2, sizeof(readBuf2), readSource2);
        int block1 = 0;
        int block2 = 0;


        while(b1res != NULL && b2res != NULL) {

            if(readBuf1[0] < readBuf2[0]) {
                block1 = 0;
                fputs(readBuf1, stdout);
                block2 = 1;
            }
            else if(readBuf1[0] > readBuf2[0])  {
                block2 = 0;
                fputs(readBuf2, stdout);
                block1 = 1;
            }

            else {
                int compres = strcmp(readBuf1, readBuf2);

                if(compres < 0) {
                    block1 = 0;
                    fputs(readBuf1, stdout);
                    block2 = 1;
                }

                else if(compres > 0) {
                    block2 = 0;
                    fputs(readBuf2, stdout);
                    block1 = 1;
                }

                else {
                    block2 = 0;
                    fputs(readBuf1, stdout);
                    fputs(readBuf2, stdout);
                    block1 = 0;
                }


            }

            if(block1 != 1) b1res = fgets(readBuf1, sizeof(readBuf1), readSource1);
            if(block2 != 1) b2res = fgets(readBuf2, sizeof(readBuf2), readSource2);
        }

        if(b1res != NULL) {
            fputs(readBuf1, stdout);
            while(fgets(readBuf1, sizeof(readBuf1), readSource1) != NULL) {
                fputs(readBuf1, stdout);
            }
        }
        if(b2res != NULL) {
            fputs(readBuf2, stdout);
            while(fgets(readBuf2, sizeof(readBuf1), readSource2) != NULL) {
                fputs(readBuf2, stdout);
            }
        }

        fclose(readSource1);
        fclose(readSource2);
        close(FD1);
        close(FD2);
}

void freeArrayMemory(char* array[], int length) {
    for(int i = 0; i < length; i++)
        free(array[i]);

}
