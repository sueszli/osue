/**
* @file mygrep.c
* @author Jan Sobhani <e0411385@student.tuwien.ac.at>
* @date 09.11.2021
* @brief A reduced variation of the Unix-command grep
* @details Implementation of a reduced variation of the Unix-command grep. A C-program mygrep, which reads in
* several files and prints all lines containing a keyword.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#ifdef DEBUG
#define debug(fmt, ...) \
(void) fprintf(stderr, "[%s:%d] " fmt "\n", \
__FILE__, __LINE__, \
##__VA_ARGS__)
#else
#define debug(fmt, ...) /* NOP */
#endif // DEBUG

static char* PROG_NAME;

/**
 * @brief Prints an error-message (short description of the failed call) to stderr and exits the program with code: EXIT_FAILURE
 * @param message a String with the short error-message
 * @details: uses global variables: stderr, EXIT_FAILURE
 */
static void exit_error(char *message){
    fprintf(stderr, "[%s] ERROR in [%s]: %s\n",message ,
    PROG_NAME, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints a usage-message documenting the correct calling-interface and exits with code: EXIT_FAILURE
 * @details: uses global variables: stderr, EXIT_FAILURE
 */
static void usage(void) {
fprintf(stderr,"Usage: %s [-i] [-o file] keyword [file...]\n", PROG_NAME);
exit(EXIT_FAILURE);
}

/**
 * @brief Takes a String and changes its chars to upper-case
 * @param message a String
 * @return returnes an int; -1 if failed (String was NULL), 0 if successful
 */
static int to_upper_case(char* chars){
    if(!chars){
        return -1;
    }
    int i = 0;
        while (chars[i])
        {
            chars[i] = toupper(chars[i]);
            i++;
        }
        return 0;
}

/**
 * @brief Similar to the classic grep-function it searches an input(-file) for a keyword and writes the lines to the console or a file.
 * @param argc argument count
 * @param argv command-line arguments
 * @param output_filename filename where the result should be saved
 * @param write_to_file 0 if should be written to stdout, 1 for file.
 * @param case_insensitive 0 if case-sensitive, 1 for case-insensitive
 */
static void mygrep(int argc, char *argv[], char *output_filename, int write_to_file, int is_case_insensitive){
    char *keyword = argv[optind];
    FILE *fp_append = stdout;

    if(write_to_file){
        if((fp_append = fopen(output_filename, "a")) == NULL){
            exit_error("fopen failed");
        }
    }
    
    if(is_case_insensitive == 1){
        if(to_upper_case(keyword)<0){
            fclose(fp_append);
            exit_error("to_upper_case failed");
        };
    }

    /* means that we have only 1 pos-arg, namely the keyword => use stdin */
    if(argc - optind == 1){
        FILE *fp = stdin;
            char *line = NULL;
            size_t len = 0;
            
            while (getline(&line, &len, fp) != -1){
                if(is_case_insensitive){
                    char *duplicate = strdup(line);
                    if(to_upper_case(duplicate)<0){
                        fclose(fp_append);
                        exit_error("to_upper_case failed");
                    }
                    if(strstr(duplicate, keyword)){
                        fprintf(fp_append, line);
                    }
                }
                else{
                    if(strstr(line, keyword)){
                        fprintf(fp_append, line);
                    }
                }
            }
    }

    /* means that we have at least 1 file, possibly more */
    if(argc-(optind-1) >= 2){

        int ind = optind+1; /* all positional arguments after keyword are the textfiles */
        while(ind<=argc-1){
            debug("||Opening file|| %s\n", argv[ind]); 
            char *line = NULL;
            size_t len = 0;
            FILE *fp;
            if((fp = fopen(argv[ind], "r")) == NULL){
                exit_error("fopen failed");
            }
            
            while (getline(&line, &len, fp) != -1){
                if(is_case_insensitive){
                    char *duplicate = strdup(line);
                    if(to_upper_case(duplicate)<0){
                        fclose(fp_append);
                        exit_error("to_upper_case failed");
                    }
                    if(strstr(duplicate, keyword)){
                        fprintf(fp_append, line);
                    }
                }
                else{
                    if(strstr(line, keyword)){
                        fprintf(fp_append, line);
                    }
                }
            }
            ind++;
        } 
        if(write_to_file){
            /* cleanup by closing stream */
            fclose(fp_append);
        }
    }
}

/**
 * @brief main
 * The main function (entry-point). It uses getopt to parse all the command-line options and command-line arguments.
 * Also checks if there are issues with the options and positional arguments.
 * @param argc argument count
 * @param argv command-line arguments
 * @return EXIT_SUCCESS on success
 * @return EXIT_FAILURE on failure.
 * @details: uses global variables: EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
    int c = 0;
    int write_to_file = 0;
    int is_case_insensitive = 0;
    FILE *fp_write = NULL;
    char *output_filename = NULL;
    PROG_NAME = argv[0];
    while((c = getopt(argc, argv, "io:"))!= -1){
        switch(c){
            case 'i': is_case_insensitive++;
            break;
            case 'o': 
                    write_to_file++;
                    /*printf("Found filename: %s\n", optarg);*/
                    output_filename = optarg;
                    if((fp_write = fopen(optarg, "w")) == NULL){
                        exit_error("fopen failed");
                    }
                    /* cleanup by closing stream */
                    fclose(fp_write);
            break;
            case '?': usage();
            break;
            default: usage();
        }
    }

    if(argc - optind == 0){
        /* You need at least 1 positional argument, the keyword */ 
        /*printf("You need at least 1 positional argument, the keyword");*/
        usage();
    }


    mygrep(argc, argv, output_filename, write_to_file, is_case_insensitive);
    
    return EXIT_SUCCESS;
}

