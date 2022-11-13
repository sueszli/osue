#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/**
 * @name: myexpand
 * @author: Moritz Hammerer, 11902102
 * @date: 30.10.2021
 * 
 * @brief Function transforming tabs to spaces using tabstops
 * 
 * @details With myexpand tabs (\t) get transformed to spaces snapping to the next tabstop
 * with can be given as argument. Input and Output both can be used with terminal or files
 * 
 * @param [-t tabstop]          if set define number of spaces instead of tabstop
 * @param [-o outputfile]       if set a path to an file is given, otherwise Output to terminal
 * @param [inputfiles]          if set a path to one file (or more) is given, otherwise input with terminal
 * 
 */

int main(int argc, char* argv[]) {	
    int tabstop = 8;
    char *outputPath = NULL;
    
    //Argument processing
    int opt;
    
    while ( (opt = getopt(argc, argv, "o:t:")) != -1) {
        switch (opt) {
        case 'o': 
            outputPath = optarg;
            break;
        case 't': 
            tabstop = strtol(optarg, NULL, 10);
            break;
        default:
            fprintf(stderr, "[%s] Usage Error: %s [-t tabstop] [-o outputPath] [inputfiles]\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (tabstop < 0)
    {
        fprintf(stderr, "[%s] Usage Error: tabstop has to be a readPositive integer\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (optind > argc)
    {
        fprintf(stderr, "[%s] Usage Error: %s [-t tabstop] [-o outputPath] [inputfiles]\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }


    //Setting Output destination
    FILE *outputFile;
    if (outputPath != NULL)
    {
        if((outputFile = fopen(outputPath, "w")) == NULL) {
            fprintf(stderr, "[%s] Opening of %s failed: %s\n", argv[0], outputPath, strerror(errno));
            fclose(outputFile);
            exit(EXIT_FAILURE);
        }
    } else {
        outputFile = stdout;
    }


    //Read Input
    int countOfInputFiles = argc-optind;
    fprintf(stdout, "Number of files: %i\n", countOfInputFiles);
    
    if(countOfInputFiles == 0){ //Input terminal
        fprintf(stdout, "Enter text to reformat:\n");

        char* line = NULL;
        size_t len = 0;

        while (getline(&line, &len, stdin) > 0) {
            int readPos = 0;    //Keeps track on reading word
            int writePos = 0;   //Keeps track on writing word
            while (line[readPos] != '\n') {
                if (line[readPos] == '\t'){
                    int p = tabstop * ((writePos/tabstop)+1);
                    for (; writePos < p ; writePos++) {
                        fprintf(outputFile,"%c", ' ');
                    }
                    readPos++;
                } else {
                    fprintf(outputFile,"%c", line[readPos]);
                    readPos++;
                    writePos++;
                }    
            }
            fprintf(outputFile,"%c", '\n');
        }
        free(line);

    }else{ //Input per file
        for (int i = 0; i < countOfInputFiles; i++){ //Each inputfile
            FILE *inputFile;

            if((inputFile = fopen(argv[optind+i], "r")) == NULL) { //open inputfile
                fprintf(stderr, "[%s] Opening of %s failed: %s\n", argv[0], argv[optind+i], strerror(errno));
                fclose(inputFile);
                exit(EXIT_FAILURE);
            } else
            {
                char* line = NULL;
                size_t len = 0;

                while (getline(&line, &len, inputFile) > 0) {
                    strcat(line, "\n");
                    int readPos = 0;    //Keeps track on reading word
                    int writePos = 0;   //Keeps track on writing word
                    while (line[readPos] != '\n') {
                        if (line[readPos] == '\t'){
                            int p = tabstop * ((writePos/tabstop)+1);
                            for (; writePos < p ; writePos++) {
                                fprintf(outputFile,"%c", ' ');
                            }
                            readPos++;
                        } else {
                            fprintf(outputFile,"%c", line[readPos]);
                            readPos++;
                            writePos++;
                        }    
                    }
                    fprintf(outputFile,"%c", '\n');
                }
                free(line);

                fclose(inputFile); //close inputfile
            }
        }
        
    }
    fclose(outputFile); //close outputfile

	exit(EXIT_SUCCESS);
}
