/**
 * @file ispalindrom.c
 * @author Marinko Todorovic <e11809906@student.tuwien.ac.at>
 * @date 07.11.2021
 *
 * @brief a program ispalindrom, which checks whether strings are palindroms
 * @details 
 * 
 **/


#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<stdbool.h>
#include<string.h>
#include<signal.h>
#include<ctype.h>

#define ERROR_EXIT(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(EXIT_FAILURE); }

typedef struct 
{
    bool i;
    bool s;
} Flags;

static char* pgm_name = "ispalindrom";
static FILE *outputFile;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details prints out usage information like flags and files
 * global variables: pgm_name
 */
static void USAGE(void){
    fprintf(stdout,"USAGE: %s [-s] [-i] [-o outfile] [file...]", pgm_name);
    exit(EXIT_FAILURE);
}
/**
 * is_palindrom function.
 * @brief This function evaluates a char array if it contents is a palindrom.
 * global variables: pgm_name
 */
static bool is_palindrom(char *line){
    char * right = line + strlen(line)-1;
    char * left = line;

    while(left < right){
        if(*left != *right)
            return false;

        left++; right--;    
    }
    return true;
}
/**
 * remove_spaces function.
 * @brief This function manipulates a char array by removing all whitespaces.
 * global variables: pgm_name
 */
static void remove_spaces(char* s) {
    // To keep track of non-space character count
    int count = 0;
 
    // Traverse the given string. If current character
    // is not space, then place it at index 'count++'
    for (int i = 0; s[i]; i++)
        if (s[i] != ' ')
            s[count++] = s[i];
    
    s[count] = '\0';
}
/**
 * handle_input function.
 * @brief This function handles an input file and it's contents.
 * @details goes through the file line by line, checks if flags are set,
 * if so, then remove spaces or ignore case. Evaluated strings are printed
 * as results to output file and closed.
 * @param input_file use lines from file.
 * @param output_file to write output to. 
 * @param flags stored flags.
 * 
 */
static void handle_input(FILE *input_file, FILE *output_file, Flags * flags){
    char *line = NULL;
    size_t size = 0;
    while(getline(&line, &size, input_file) != -1){
        // because of interference remove newline char
        if(line[strlen(line) - 1] == '\n'){
			line[strlen(line) - 1] = '\0';
		}

        if(strlen(line) == 0)
            continue;

        char *original_line = (char*)malloc(sizeof(char)*strlen(line));
        strncpy(original_line, line, sizeof(char)*strlen(line));

        if(flags->s)
        {
            // ignore spaces
            remove_spaces(line);
        }

        if(flags->i)
        {
            // ignore case
            char* start = line;
            for ( ; *line; ++line) *line = tolower(*line);
            line = start;
        }

        if(is_palindrom(line) == true){
            fprintf(output_file, "%s is a palindrom.\n", original_line);
        }else{
            fprintf(output_file, "%s is not a palindrom.\n", original_line);
        }
        fflush(output_file);
    }
    free(line);
}
/**
 * myInterruptHandler  
 * @brief handles SIGINT interrupt 
 * @details close output file before exiting program 
 * global variables: pgm_name
 */
static void myInterruptHandler (int signum) { 
    fclose(outputFile);
    exit(1);
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters and flags and files
 * are parsed via getopt. 
 * @details 
 * Multiple inputfiles can be parsed, as well as an output file. If no input
 * file is specified, stdin will be used and stdout for no output file specified.
 * Special signal handler for SIGINT for interupts in order to close output file.
 * global variables: pgm_name, outputFile
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
    int opt;
    Flags flags = {false, false};

    signal(SIGINT, myInterruptHandler);

    // FILE * outputFile = stdout;
    outputFile = stdout;
    while((opt=getopt(argc, argv, "sio:")) != -1){
        switch (opt)
        {
        case 's':
            flags.s = true;
            break;
        case 'i':
            flags.i = true;
            break;
        case 'o':
            outputFile = fopen(optarg, "w");
            break;
        default:
            USAGE();
        }
    }

    int current_arg = optind;
    
    if(outputFile == NULL){
        ERROR_EXIT("Error opening output file.");
    }

    if(current_arg >= argc){
        handle_input(stdin, outputFile, &flags);
    }else{
        // TODO: parse one or multiple files afterwards
        while(current_arg<argc)
        {
            FILE *inputFile = fopen(argv[current_arg++], "r");
            if(inputFile == NULL){
                fclose(outputFile);
                free(inputFile);
                ERROR_EXIT("Error reading file %s\n", argv[current_arg-1]);
            }
            handle_input(inputFile, outputFile, &flags);
            fclose(inputFile);
        }
    }

    fclose(outputFile);

    signal(SIGINT, SIG_DFL);
    return 0;
}

