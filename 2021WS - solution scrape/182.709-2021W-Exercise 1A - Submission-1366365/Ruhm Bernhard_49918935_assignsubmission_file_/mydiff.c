/**
*@file mydiff.c
*@author Bernhard Ruhm
*@date 13.11.2021
*@brief This program compares two files
*@details The program will read from two files and
*compare each file line by line. The line number and number of
*differing characters are either printed or written in a file.
*
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


char *myprog;

/**
*@brief This function prints an usage message 
*@details global variables: myprog
*
**/
static void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2 \n", myprog);
    exit(EXIT_FAILURE);
}

/**
*@brief This function provides the functionality of the program
*@details The function verifies the arguments and reads lines from two given files.
*The lines will be compared line by line and depending on the arguments the results
*are printed in a file or to stdout
*@param argc argument counter
*@param argv argument vector
*@return 0
**/
int main(int argc, char *argv[]) {

    char *o_arg = NULL;
    int opt_i = 0;
    int c;
    bool ignoreCase = false;
    myprog = argv[0];

    while ((c = getopt(argc, argv, "o:i")) != -1){
        switch (c)
        {
            case 'o': o_arg = optarg;
                break;
            case 'i': opt_i++;
                ignoreCase = true;
                break;
            case '?': usage();
                break;
        }
    }

    if (opt_i > 1)
        usage();

    if ((argc - optind) != 2)
        usage();

    char *file1Name = argv[optind];
    char *file2Name = argv[optind+1];
    FILE *file1;
    FILE *file2;
    FILE *outfile;

    if ((file1 = fopen(file1Name, "r")) == NULL ) {
        fprintf(stderr, "[%s] ERROR: fopen failes : %s\n", file1Name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((file2 = fopen(file2Name, "r")) == NULL ) {
        fprintf(stderr, "[%s] ERROR: fopen failes : %s\n", file2Name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (o_arg != NULL) {
        if ((outfile = fopen(o_arg, "w")) == NULL ) {
            fprintf(stderr, "[%s] ERROR: fopen failes : %s\n", o_arg, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    char *buff1 = NULL;
    char *buff2 = NULL;
    size_t len = 0;
    ssize_t nread1 = 0;
    ssize_t nread2 = 0;
    int line = 1;


    while((nread1 = getline(&buff1, &len, file1)) != -1 &&
         ((nread2 = getline(&buff2, &len, file2)) != -1)) {

        int s1 = strlen(buff1);
        int s2 = strlen(buff2);
        int size = 0;
        int cNum = 0;

        if (s1 > s2)
            size = s2;
        else
            size = s1;

        if (ignoreCase){
            for (int i = 0; i < size - 1; i++) {
                if (strncasecmp(buff1, buff2, 1) != 0)
                    cNum++;
                buff1++;
                buff2++;
            }
        }
        else {
            for (int i = 0; i < size - 1; i++) {
                if (strncmp(buff1, buff2, 1) != 0)
                    cNum++;
                buff1++;
                buff2++;
            }
        }

        if (cNum != 0 && o_arg == NULL) {
            printf("Line: %d, characters: %d\n", line, cNum );
        }
        else if(cNum != 0) {
            fprintf(outfile, "Line: %d, characters: %d\n", line, cNum);
        }
        line++;
    }

    fclose(file1);
    fclose(file2);
    if (o_arg != NULL)
        fclose(outfile);

    return 0;
}
