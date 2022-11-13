/**
 * @file ispalindrom.c
 * @author Claudia Kößldorfer <e11825357@student.tuwien.ac.ar>
 * @date 30.10.2021

* @brief Reads and checks input files line by line for palindroms.
 * Returns each line and either "is a palindrom" or "is not a palindrom".
 * @details If -o is provided output will be written in the given output file otherwise it will be written to stdout.
 * An arbitrary number of input files can be given, which will be read from. If none are provided the program reads from stdin.
 * If -s is provided whitespaces will be ignored for the check.
 * If -i is provided the check will not differentiate between upper and lower case letters.
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include "ispalindrom.h"

/**
 * @brief checks for each line of input file if it is a palindrom
 * @details Check is affected by whether or not the flags are set.
 * @param input File from which will be read.
 * @param output File to which will be written.
 * @param sflag Flag which is 1 if -s was provided and otherwise 0.
 * @param iflag Flag which is 1 if -i was provided and otherwise 0.
**/
static void palindrom(FILE* input, FILE* output, int sflag, int iflag) {
    char line [1000];
    //get each line of input seperately
    while(fgets(line,sizeof line,input)!= NULL) {
        line[strcspn(line, "\n")] = '\0';
        int n = strlen(line);
        //create copy of line so it can be altered without affecting the output later
        char copy [n];
        strncpy(copy,line,n);

        char *s = copy;
        //check if -i was provided ergo the check is not case sensitive
        if(iflag == 1) {
            int j = 0;
            for(j=0; j<n; j++) {
                //turn all chars lower case
                s[j] = tolower((unsigned char) copy[j]);
            }
        }

        char *p = copy;
        int h = 0;
        //flag showing if line is palindrom
        int palindrom = 1;
        //check if -s was provided ergo the check ignores whitespaces
        if(sflag == 1) {
            //indexes going from front to back and back to front
            int iFront = 0;
            int iBack = n - 1;
            while(1) {
                //if at either index there is a whitespace the index is incremented
                while(copy[iFront] == ' ') {
                    iFront++;
                }
                while(copy[iBack] == ' ') {
                    iBack--;
                }
                //if the indexes are equal or have passes each other the whole line was checked
                if (iBack <= iFront) break;
                //the chars are compared with each other and if not equal the line is not a palindrom, so the flag is set to 0
                if(copy[iFront]!= copy[iBack]) {
                    palindrom = 0;
                }
                iFront++;
                iBack--;
            }
        } else {
            for(h=0; h<n/2; h++) {
                if(p[h]!= p[n-h-1]) {
                    palindrom = 0;
                }
            }
        }
        //prints output for the line that was checked
        if(palindrom==1) {
            fprintf(output, "%s is a palindrom \n", line);
        } else {
            fprintf(output, "%s is not a palindrom \n", line);
        }
    }
}

/**
 * @brief checks for each line of input file if it is a palindrom
 * @details Check either input files/s provieded through programm call or if none are given check stdin line by line if it's a palindrom.
 * Write checked lines and for each also whether they're a palindrom or not to either output file if -o is provided or to stdout.
 * If -s is provided whitespaces will be ignored for the check.
 * If -i is provided the check will not differentiate between upper and lower case letters.
 * @param argc The argument counter.
 * @param argv The argument vector.
**/
int main(int argc, char *argv[]) {
    //set default input and output
    FILE *input = stdin;
    FILE *output = stdout;
    //counter to check how many options/files were provided used to read input files
    int files = 1;
    //set flags by default to 0
    int sflag = 0;
    int iflag = 0;
    //check provided options with getopt and if necessary set flags or output
    int c;
    while((c = getopt(argc,argv, "sio:")) != -1) {
        switch(c) {
        case 's':
            sflag = 1;
            files += 1;
            break;
        case 'i':
            iflag = 1;
            files += 1;
            break;
        case 'o':
            output = fopen(optarg,"a");
            if(output == NULL) {
                fprintf(stderr, "%s: fopen ('%s') failed: (%s)\n", argv[0], optarg, strerror(errno));
                return EXIT_FAILURE;
            }
            files += 2;
            break;
        case '?':
        default :
            break;
        }
    }

    //determine if there are input files and open them
    if((argc - files) != 0) {
        int i = 0;
        i += sflag;
        i += iflag;
        if(output == stdout) {
            i += 1;
        } else {
            i += 3;
        }
        for(i; i < argc; i++) {
            input = fopen(argv[i], "r");
            if(input == NULL) {
                fprintf(stderr, "%s: fopen ('%s') failed: (%s)\n", argv[0], argv[i], strerror(errno));
                return EXIT_FAILURE;
            }
            //call palindrom for each input file
            palindrom(input,output, sflag, iflag);
        }
    } else {
        //if no input file was given call palindrom with stdin as input
        palindrom(input, output, sflag, iflag);
    }

    fclose(input);
    fclose(output);
    exit(EXIT_SUCCESS);
}
