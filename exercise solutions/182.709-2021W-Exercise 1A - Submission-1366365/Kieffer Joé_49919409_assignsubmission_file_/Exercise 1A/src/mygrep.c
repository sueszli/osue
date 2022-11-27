/**
 * @file mygrep.c
 * @author Kieffer Joé <e11814254@student.tuwien.ac.at>
 * @date 25.10.2021
 * 
 * @brief Main program module
 * 
 * This program implements a light version of the linux grep function. The program reads lines from the terminal or
 * a file and returns the line if it contains a specified keyword to the terminal or a file if specified.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

int insensitive = 0;
const char *progName;
FILE *outputFile = NULL;
char *keyword;


/**
 * @brief Method to close open files
 *
 * @details This method gets a file and closes it, if it is unsuccessful an error message is displayed and
 * the @function cleanup is called.
 *
 * @param pFile the file which should be closed
 */
void closeFile(FILE *pFile){
    int success = fclose(pFile);
    if(success == EOF){
        fprintf(stderr, "[%s:%i] ERROR: Couldn't close file, fclose failed: %s\n", progName, __LINE__, strerror(errno));
        free(keyword);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This method cleanup the resources
 *
 * @details If a file for the output is open, @function closeFile is called for this file. The reserved memory
 * allocation for the keyword is freed.
 */
void cleanup(void){
    if(outputFile!= NULL){
        closeFile(outputFile);
    }
    free(keyword);
}

/**
 * @brief This method is responsible for the output.
 *
 * @details The method gets a line and prints it out to the terminal or a file depending on the options given at the
 * program start.
 *
 * @param line The line which should be printed out.
 */
void out(char *line){
    FILE *out;
    if(outputFile == NULL){
        out = stdout;
    } else {
        out = outputFile;
    }
    fprintf(out, "%s", line);
    if(line[strlen(line)-1] != '\n'){
        fprintf(out, "\n");
    }
}

/**
 * @brief This method checks all the occurrences of the keyword in the given configuration.
 *
 * @details This method reads line after line from the given input stream (Terminal or File) and looks if it contains
 * the keyword. If the option -i was given at program start the search is case insensitive. If a line contains the
 * keyword @function out is called.
 *
 * @param input the input stream to read from
 */
void mygrep(FILE *input){
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, input) != -1){
        char *workLine = malloc(strlen(line) * sizeof(char)); 
        if(insensitive) {
            for(int i=0; i < strlen(line); i++){
                workLine[i] = toupper(line[i]);
            }
        } else {
            strcpy(workLine, line);
        }
        char *key = strstr(workLine, keyword);
        if(key != NULL){
            out(line);
        }
        free(workLine);
    }
}

/**
 * @brief This method opens a given file.
 *
 * @details This methods open a given file, if it succeeds the @function mygrep is called and then the
 * @function closeFile. If the method can't open the file an error message is printed and the @function cleanup
 * is called
 *
 * @param inputFile the filename of the file to open
 */
void openInputfile(char *inputFile){
    FILE *input = fopen(inputFile, "r");
    if(input == NULL){
        fprintf(stderr, "[%s:%i] ERROR: Couldn't open file %s, fopen failed: %s\n", progName, __LINE__, inputFile, strerror(errno));
        cleanup();
        exit(EXIT_FAILURE);
    }else{
        mygrep(input);
        closeFile(input);
    }
}

/**
 * @brief Main method
 *
 * @details The program starts here, first the method reads the given options and sets the @var insensitive if the
 * option is given. The method creates the file for the output too if the option is given. If an error occurs a error
 * message gets printed out and the program exits with EXIT_FAILURE. The keyword gets set too in uppercase if the
 * option for insensitive case is given else the keyword gets set as given. If the program is started with
 * valid arguments depending on the options the @function mygrep is calls or @function openInputfile is called to open
 * one after the other input file(s). At the end @function cleanup is called and the program ends successful.
 *
 * @param argc the argument counter
 * @param argv the given arguments
 * @return EXIT_SUCCESS or EXIT_FAILURE depending how the program ends.
 */
int main(int argc, char *argv[]){
    int opt;
    progName = argv[0];
    while((opt=getopt(argc, argv, "io:"))!=-1){
        switch(opt){
            case 'i':
                insensitive = 1;
                break;
            case 'o':
                outputFile = fopen(optarg ,"w");
                if(outputFile == NULL){
                    fprintf(stderr, "[%s:%i] ERROR: Couldn't create file %s, fopen failed: %s\n", progName, __LINE__, optarg, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "[%s:%i] ERROR: mygrep [-i] [-o outfile] keyword [file...]\n", progName, __LINE__);
                exit(EXIT_FAILURE);
        }
    }
    if(optind >= argc){
        fprintf(stderr, "[%s:%i] ERROR: mygrep [-i] [-o outfile] keyword [file...]\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    keyword = malloc(strlen(argv[optind]) * sizeof(char));
    if(insensitive){
        for(int i=0; i < strlen(argv[optind]); i++){
            keyword[i] = toupper(argv[optind][i]);
        }
        optind++;
    }else{
        strcpy(keyword, argv[optind++]);
    }
    if(optind >= argc){
        mygrep(stdin);
    }else{
        while(optind < argc){
            openInputfile(argv[optind++]);
        }
    }
    cleanup();
    return EXIT_SUCCESS;
}