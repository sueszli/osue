/**
 * @author Artem Chornyi. 11922295
 * @brief Program that finds specified string in files/stdin
 * @details Output can be written to file.
 * 
 *          Search can be case insensitive
 * 
 * @date 10-th November 2021 (10.11.2021)
 */

#ifdef DEBUG
#define debug(fmt, ...) \
(void) fprintf(stderr, "[%s:%d] " fmt "\n", \
__FILE__, __LINE__, \
##__VA_ARGS__)
#else
#define debug(non) 
#endif

// Length of buffer to store file's characters/strings
#define BUFFER_LENGTH 1024

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

// Indicates if search should be case insensitive 
int i_opt = 0;

// global argv[0]
char* filename;


/**
 * @brief Prints correct usage of a program.
 * 
 */
void usage(void){
    fprintf(stderr, "Usage: %s [-i] [-o outfiles] searched_word [file1 [file2, [...]]]\n", filename);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints contents of 'buff' variable if 'searchWord' variable is in it.
 * 
 * @details toupper() function is being used to performe case insensitive search.
 * 
 * @param buff buffer containing line/characters to be printed, if 'searchWord' is in 'buff'
 * @param searchWord string that is to be found in buff.
 * @param outputFd file descriptor to output destination (file/stdout).
 */
void print_if_contains(char *buff, char *searchWord, FILE *outputFd){
    if(i_opt == 1){
        int strLength = strnlen(buff, BUFFER_LENGTH);
        int searchWordLength = strlen(searchWord);
        char localBuff[strLength+1], localSearchWord[searchWordLength+1];
        
        for (int i = 0; i < strLength; i++)
        {
            localBuff[i] = toupper(buff[i]);
        }
        localBuff[strLength] = 0;
        
        for (int i = 0; i < searchWordLength; i++)
        {
            localSearchWord[i] = toupper(searchWord[i]);
        }
        localSearchWord[searchWordLength] = 0;

        if(strstr(localBuff, localSearchWord) != NULL){
            fprintf(outputFd, "%s", buff);
        }
    } else {
        if(strstr(buff, searchWord) != NULL){
            fprintf(outputFd, "%s", buff);
        }
    }
}

/**
 * @brief Reads null-terminated strings and prints them if 'searchWord' variable
 *        is in them.
 * 
 * @param f file descriptor of a file to read.
 * @param searchWord string to search for.
 * @param outputFd file descriptor to output destination (file/stdout).
 */
void print_lines(FILE* f, char *searchWord, FILE *outputFd){
    char buff[BUFFER_LENGTH];
    while(1){
        if(fgets(buff, BUFFER_LENGTH, f) == NULL){
            break;
        }
        print_if_contains(buff, searchWord, outputFd);
    }
}

/**
 * @brief Opens file and prints lines containing 'searchWord' variable.
 * 
 * @param path path to file to read.
 * @param searchWord string to search for.
 * @param outputFd file descriptor to output destination (file/stdout).
 */
void search_in_file(char* path, char* searchWord, FILE *outputFd){
    FILE *f;
    if((f = fopen(path, "r")) == NULL){
        fprintf(stderr, "[%s] ERROR: fopen failed: (%s) %s\n", filename, path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    print_lines(f, searchWord, outputFd);
    fclose(f);
}

/**
 * @brief Print lines from stdin containing 'searchWord'
 * 
 * @details toupper() function is being used to performe case insensitive search.
 * 
 * @param searchWord string to search for.
 * @param outputFd file descriptor to output destination (file/stdout).
 */
void search_in_stdin(char *searchWord, FILE *outputFd){
    print_lines(stdin, searchWord, outputFd);
}

int main(int argc, char **argv){
    filename = argv[0];
    
    // save how many times o_opt is specified.
    char o_opt = 0;
    
    // string to search.
    char *searchWord;

    // output destination
    FILE *outputFd = stdout;
    {
        int c;
        while((c = getopt(argc, argv, "o:i")) != -1){
            switch(c){
                case 'i':
                    i_opt = 1;
                    break;
                case 'o':
                    if(o_opt == 1){
                        fprintf(stderr, "[%s] ERROR: Only one -o should be specified!\n\n", filename);
                        fclose(outputFd);
                        usage();
                    } else {
                        o_opt = 1;
                    }
                    if(strlen(optarg) != 0 && optarg[0] == '-'){
                        fprintf(stderr, "[%s] ERROR: Incorrect -o was specified!\n", filename);
                        usage();
                    }
                    if((outputFd = fopen(optarg, "w")) == NULL){
                        fprintf(stderr, "[%s] ERROR: failed opening %s: %s\n", filename, optarg, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    if(outputFd != stdout && outputFd != NULL){
                        fclose(outputFd);
                    }
                    usage();
            }
        }
    }

    {
        int posArgsCount = argc - optind;
        // If no positional args are specified.
        if(posArgsCount <= 0){
            if(outputFd != stdout && outputFd != NULL){
                fclose(outputFd);
            }
            usage();
        } else {
            searchWord = argv[optind];
            // if search files are specified
            if(posArgsCount > 1){
                for (int i = 1; i < posArgsCount; i++)
                {
                    search_in_file(argv[optind + i], searchWord, outputFd);
                }
            } // search for 'searchWord' variable in stdin.
            else {
                search_in_stdin(searchWord, outputFd);
            }
        }
    }
    if(outputFd != stdout){
        fclose(outputFd);
    }

    exit(EXIT_FAILURE);
}

