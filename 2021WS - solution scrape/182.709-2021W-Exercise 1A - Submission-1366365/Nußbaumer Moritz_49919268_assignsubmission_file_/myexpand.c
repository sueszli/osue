/**
 * @file myexpand.c
 * @author Nußbaumer Moritz (12026652)
 * @brief takes an input text, replaces tabs with spaces, writes them
 * @details input is eiter from one file, multiple files or stdin; output is either written to a specified file
 * or to stdout; the amount of spaces a tab gets replaced can be modified through an option; there also exists an option
 * to specify where the output is written to
 * @date 2021-11-11
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#define TAB_LENGTH 8

//global pointer so that usage() can access the program name
char *myprog;

/**
 * @brief reads the lines of the specified input, replaces tabs with spaces, and writes the text to the specified output
 * @details reads the characters of a line one by one; if the character is a tab, it gets replaces with spaces, otherwise
 * the character is directly written.
 * @param input a pointer to where the input is taken from
 * @param output a pointer to where the output is written to
 * @param tabLength the number of spaces a tab is replaced with
 */
static void expand (FILE* input, FILE* output, int tabLength){

    ssize_t read = NULL;
    size_t len = 0;
    char* line = NULL;
    int pos = 0, index = 0, spaces = 0, write = 0;
    /*pos: indicates where the next character needs to be written after spaces are added
    spaces: indicates how many spaces need to be added, depending on tabLength and at which position the last
            character was written to
    write: the position the next character needs to be written to, if no tab is read
    */

    while((read = getline(&line, &len, input)) != -1){
        //at the end of the for-loop a line-break is read and thus a new line started. write needs to be set to 0 to start writing
        //at the beginning of the new line
        write = 0;
        for (index = 0; index < read; index++){
            if(line[index] == '\t'){
                pos = tabLength * ((write / tabLength) + 1);
                //calculating how many spaces need to be filled in to get to the position where the next character is written to
                spaces = pos - write; 
                write = pos;
                fprintf(output, "%*s", spaces, " ");
            } else {
                fprintf(output, "%c", line[index]);
                write++;
            }
        }
    }
    free(line);
}

/**
 * @brief gets called when an incorrect command-line argument is entered and returns an explanation of how a command-line
 * argument should look like
 */
static void usage(void){
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile]  [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief reads the commanand-line arguments and calls myexpand with the correct values
 * @details depending on the arguments provided, the method will modify where the input is taken from, the output is written to
 * and how many spaces a tab is replaced with as specified in the command-line argument
 * TAB_LENGTH is the standard tab length, which is uses if no other length is specified through -t
 * @param argc the number of provided arguments + 1;
 * @param argv a char array filled with the provided arguments; argv[0] is the program name
 * @return int returns EXIT_SUCCESS upon completing the task, as long as the command-line arguments were entered correctly
 */
int main (int argc, char **argv){

    myprog = argv[0];
    int x = 0, files = 0;
    long newTabLength = TAB_LENGTH;
    char* optT_ = NULL,* optO_ = NULL,* tabErr_ = NULL;
    FILE* output_ = NULL,* input_ = NULL;

    while((x = getopt(argc, argv, "t:o:")) != -1){
        switch(x) {
            case 'o': optO_ = optarg;
            break;
            case 't': optT_ = optarg;
            break;
            default: usage();
            break;
        }
    }

    //sets output to the specified output file if option -o was read, otherwise sets it to stdout
    if(optO_){
        output_ = fopen(optO_, "w");
        if(output_ == NULL){
            fprintf(stderr, "[%s] couldn't write to file %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE); 
        }
    } else output_ = stdout;

    //sets the tablength to the specified value if option -t was read
    if(optT_){
        newTabLength = strtol(optT_, &tabErr_, 10);
        if(*tabErr_){
            fprintf(stderr, "[%s] Invalid tabstop %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    //argc == optind if no input file(s) was/were provided -> reads from stdin
    if(argc == optind){
        input_ = stdin;
        expand(input_, output_, newTabLength);
        fclose(input_);
        fclose(output_);
    } else {
        //iterates through all given files and call my expand with the input of each
        for(files = optind; files < argc; files++){
            input_ = fopen(argv[files], "r");
            if (input_ == NULL){
                fprintf(stderr, "[%s] couldn't read from file %s\n", argv[0], strerror(errno));
                fclose(input_);
                fclose(output_);
                exit(EXIT_FAILURE);
            }
            expand(input_, output_, newTabLength);
        }
        fclose(input_);
        fclose(output_);
    }
    return EXIT_SUCCESS;
}