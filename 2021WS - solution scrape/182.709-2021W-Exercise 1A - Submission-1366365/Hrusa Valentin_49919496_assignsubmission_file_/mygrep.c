/**
 * @file mygrep.c
 * @author Valentin Hrusa 11808205
 * @brief Program that returns your input if a keyword was found
 * @details mygrep reads your input line by line, either from stdin or from specified files and returns them 
 *          if a specified keyword was found. The search can be case independent if the -i tag is passed.
 * 
 * 
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

/*global var for the name of the program used for error-messages*/
char *name;
/*global var for storing the filepointer to the input-file used for reading the lines*/
FILE *input = NULL;
/*global vat for storing the filepointer to the output-file used for writing the lines which contain the keyword*/
FILE *output = NULL;

/**
 * @brief correctly closes opened Filepointers and frees memory
 * @details global variables input and output are closed if data isn't read 
 *          and returned over stdin and stdout.
 * 
 * @param pointer to allocated memory which needs to be freed
 */
static void ressourceClose(char *pointer);

/**
 * @brief signal-handler for closing the application
 * @details if a SIGINT or SIGTERM is received while reading lines
 *          ressourceClose is called, exit-message is printed and the
 *          program is exited. 
 * 
 * @param signum representation of the Signal received
 */
static void sig_handler(int signum);

/**
 * @brief exits the program after error
 * @details if an error is encountered in the program this function
 *          prints correct usage and exits with failure
 * 
 */
static void errexit(void);

/**
 * @brief checks if a line contains a keyword. If this is the case, the line gets printed
 * @details if ignoreCase is true, noCaseLine, which is a representation of line with lowered cases
 *          is checked for keyword. Otherwise line is checked. If keyword was found, line is
 *          printed to the global Filepointer output
 * 
 * @param line current original input-line 
 * @param noCaseLine current input-line without case
 * @param ignoreCase if -i was specified this flag is set
 * @param keyword string which has to be found in line or noCaseLine, cases have to be lowered if
 *                ignoreCase is set. 
 */
static void checkLine(char *line, char *noCaseLine, int ignoreCase, char *keyword);

/**
 * @brief reads lines from input and continuously calls checkLine
 * @details continuously reads a line from global-input, if ignoreCase is set builds a new String
 *          with only lowered-cases of the input-line then calls checkLine
 * 
 * @param keyword specified keyword which has to be found, passed through to checkLine
 * @param ignoreCase if -i was specified this flag is set
 */
static void readLines(char *keyword, int ignoreCase);

int main(int argc, char **argv)
{
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    name = argv[0];
    int c, nextarg;
    int iflag = 0;
    int oflag = 0;
    char *outfile = NULL;
    char *keyword = NULL;

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            if (iflag) errexit();
            iflag = 1;
            break;
        case 'o':
            if (oflag) errexit();
            oflag = 1;
            outfile = optarg;
            break;
        default:
            errexit();
        }
    }

    nextarg = optind;

    if (nextarg >= argc)
    {
        fprintf(stderr, "Keyword is missing!\n");
        errexit();
    }

    if(oflag){
        output = fopen(outfile, "w");
    }else{
        output = stdout;
    }

    /*reduces keyword to lowercase if iflag was set*/
    if(iflag){
        keyword = (char *)malloc(strlen(argv[nextarg]));
        if(keyword == NULL){
            fprintf(stderr, "ERROR in %s, malloc failed allocating ressources", name);
            ressourceClose(NULL);
            exit(EXIT_FAILURE);
        }
        for(size_t i = 0; i<strlen(argv[nextarg]); i++){
            keyword[i] = tolower(argv[nextarg][i]);
        }
    }else{
        keyword = argv[nextarg];
    }
    
    /*if no input files are specified readLines is called with stdin*/
    nextarg++;
    if(nextarg >= argc){
        input = stdin;
        readLines(keyword, iflag);
        if(iflag){
            ressourceClose(keyword);
        }else{
            ressourceClose(NULL);
        }
        //fprintf(stdout, ">successful exit!\n");
        exit(EXIT_SUCCESS);
    }

    /*if input files are specified, this loop iterates over them and calls readLines for every file*/
    while(nextarg < argc){
        if(input != NULL) fclose(input);
        input = fopen(argv[nextarg], "r");
        if(input == NULL){
            fprintf(stderr, "ERROR in %s: failed to open %s\n", name, argv[nextarg]);
            nextarg++;
            continue;
        }
        readLines(keyword, iflag);
        nextarg++;
    }

    if(iflag){
        ressourceClose(keyword);
    }else{
        ressourceClose(NULL);
    }
    //fprintf(stdout, ">successful exit!\n");
    exit(EXIT_SUCCESS);
}

void readLines(char *keyword, int ignoreCase){
    char *line = NULL;
    char *noCaseLine = NULL;
    size_t len = 0;
    ssize_t nread;
    while((nread = getline(&line, &len, input)) != -1){
        if(ignoreCase){
            noCaseLine = (char *)malloc(nread);
            if(noCaseLine == NULL){
                fprintf(stderr, "ERROR in %s, malloc failed allocating ressources", name);
                ressourceClose(line);
                exit(EXIT_FAILURE);
            }
            for(size_t i = 0; i<strlen(line); i++){
                noCaseLine[i] = tolower(line[i]);
            }
        }
        checkLine(line, noCaseLine, ignoreCase, keyword);
    }   
    if(line != NULL) free(line);
    if(noCaseLine != NULL) free(noCaseLine);
}

void checkLine(char *line, char *noCaseLine, int ignoreCase, char *keyword){
    char *pointer;
    if(ignoreCase){
        pointer = strstr(noCaseLine, keyword);
    }else{
        pointer = strstr(line, keyword);
    }        
    if(pointer != NULL){
        fprintf(output, "%s", line);
        if(line[strlen(line)-1] != '\n') fprintf(output, "\n");
    }
    fflush(output);    
}

void errexit()
{ 
    char *usage = "[-i] [-o outfile] keyword [file...]";
    fprintf(stderr, "Error in %s, USAGE: %s\n", name, usage);
    exit(EXIT_FAILURE);
}

void sig_handler(int signum){
    ressourceClose(NULL);
    //fprintf(stdout, ">successful exit with a signal!\n");
    exit(EXIT_SUCCESS);
}

void ressourceClose(char *pointer){
    if(pointer != NULL){
        free(pointer);
    }
    if((input != NULL) && (input != stdin)){
        fclose(input);
    }
    if((output != NULL) && (output != stdout)){
        fclose(output);
    }
}
