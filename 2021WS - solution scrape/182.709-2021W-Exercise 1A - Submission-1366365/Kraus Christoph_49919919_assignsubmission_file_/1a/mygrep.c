/**
 * @file mygrep.c
 * @author Christoph Kraus (11776854) 
 * @date 11.11.2021
 * 
 * @brief allows a user to search through strings and print the string if anything was found. 
 * @details allows a user to search through strings and print the string if anything was found. The user can either use stdin as input or define one or multiple input files.
 * results can be either printed in stdout or printed in a defines output file
 * 
 * */


#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char* PROGRAM_LABEL;

/**
 * @brief displays error message. Will terminate afterwards.
 * @details Displays an error message via stderr with the program label as prefix
 * 
 * @param message to print via stderr
 * */
static void error(char* message){
    fprintf(stderr, "%s, %s\n", PROGRAM_LABEL, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief displays usage message. Will terminate afterwards.
 * @details Displays an usage message via stderr with the program label as prefix. Will be shown when a users inputs wrong arguments.
 * */
static void usage(void){
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", PROGRAM_LABEL);
    exit(EXIT_FAILURE);
}

/**
 * @brief will transform a string into a lowercase string.
 * @details Uses tolower to transform the char pointer bit by bit into a lowercase string
 * 
 * @param char* to transform into lowercase.
 * */
static void lowercase(char *const str){
    for(size_t i = 0; str[i]; ++i){
        str[i] = tolower(str[i]);
    }
}

/**
 * @brief searches a string keyword in strings and will print strings where it was found. 
 * @details searches the keyword in the input. The input can be either a file or stdin. When the keyword is found a single time in the input, it will be printed in the defined output. 
 * 
 * @param input defines input, non stdin files will be free d afterwards
 * @param keyword used to search through our input
 * @param output defines output
 * @param case_sensitive if this is true, keyword and input will be parsed to lowercase
 * */
static void mygrep(FILE *input, const char *const keyword, FILE *output, bool case_sensitive){
    char *buffer = NULL;
    size_t buffer_length = 0;
    ssize_t read_length;

    while((read_length = getline(&buffer, &buffer_length, input)) != -1){
        char buffer_copy[read_length];
        strcpy(buffer_copy, buffer);
        char *keyword_buffer = (char *const) keyword;

        if(!case_sensitive){
            lowercase(buffer_copy);
            lowercase(keyword_buffer);
        }

        if(strstr(buffer_copy, keyword_buffer) != NULL){
            if(fputs(buffer, output) == EOF){
                fprintf(stderr, "[%s] ERROR, couldnt write into output: %s\n", PROGRAM_LABEL, strerror(errno));
                free(buffer);
                exit(EXIT_FAILURE);
            }
        }
    }
    if(input != stdin){
        if(fclose(input) != 0){
                error("could't close input file");
            }
    }
    if(buffer != NULL){
        free(buffer);
    }

}
/**
 * @brief entry point of the program
 * @details This is the entry point of the program. First it will validate flags and react to them.
 * Afterwards it checks if there is a keyword, otherwise usage will be printed.
 * if no input file is declared via parameters, the user can enter the input line by line. 
 * The input will be searched through with mygrep.
 * if no output file is declared via parameters, the program will print in stdout
 * 
 * @param argc amount of args
 * @param argv array with args
 * */
int main(int argc, char **argv){

    PROGRAM_LABEL = argv[0];
    char* outfile = NULL;
    bool case_sensitive = true;
    int count_i = 0, count_o = 0;

    int c;
    while ((c = getopt(argc, argv,"i::o:")) != -1){

        switch (c){
            case 'i':
                case_sensitive = false;
                count_i++;
                break;
            case 'o':
                count_o++;
                outfile = optarg;
                break;

            case '?':
            default:
                usage();
        }
    }

    if(count_i > 1 ||  count_o > 1){
        usage();
    }

    FILE *output_file = stdout;
    if(outfile != NULL){
        output_file = fopen(outfile, "w");
        if(output_file == NULL){
            error("Cannot open outputfile");
        }
    }

    int argInd = optind;
    if(argInd >= argc || argc < 2){
        usage();
    }

    char* keyword = argv[argInd++];

    FILE* in = stdin;
    if(argInd == argc || argc == 1){
        mygrep(in, keyword, output_file, case_sensitive);
    }else{
        for(; argInd < argc; argInd++){
            in = fopen(argv[argInd], "r");
            if(in != NULL){
                mygrep(in, keyword, output_file, case_sensitive);

            }else{
                error("could't open input file to read, does it exists?");
            }

        }
    }

    if(outfile != NULL){
        if(fclose(output_file) != 0){
            error("could't close output file");
        }
    }
    return 0;
}
