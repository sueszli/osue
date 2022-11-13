/**
 * @file ispalindrom.c
 * @author noah capek
 * @brief a program to check if a given string is a palindrom
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

char *myprog;

void usage(void){
    fprintf(stderr, "Usage %s [-o optarg] [-s] [-i] file...\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Programm entry point
 * @brief The program start here. This function evaluates the given parameters and takes action according to them, overall purpose of this function is
 * to check wether given strings are palindroms
 * @param argc The argument counter
 * @param argv The argument vector
 * @return Returns EXIT_SUCCESS 
 */
int main(int argc, char *argv[]){
    myprog = argv[0];
    int opt_s = 0;
    int opt_i = 0;
    int opt_o = 0;
    char *o_arg = NULL;
    int c;
    while ((c = getopt(argc, argv, "sio:h")) != -1){
        switch (c)
        {
        case 's': opt_s++;
            break;

        case 'i': opt_i++;
            break;
        
        case 'o': o_arg = optarg; opt_o++;
            break;
        case 'h': //help option becuase how do you call it incorrectly?
            usage();
            break;
        default:
            break;
        }
    }

    if(argv[optind] == NULL){
        fromCommandLine(argv, opt_i, opt_s, opt_o, o_arg);
    }
    else{

    for(int i = optind;i < argc; i++){
        FILE *file;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        FILE *outFile;
        if(opt_o >= 1){     
            outFile = fopen(o_arg, "a");
        }

        file = fopen(argv[i], "r");
        if(file == NULL){
            fprintf("fopen failed %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        while((read = getline(&line, &len, file)) != -1){
            int begin = 0;
            int count = strlen(line) - 2;
            
            if(opt_i >= 1){
                everythingToLower(line, count);
            }

            if(opt_s >= 1){
                removeBlanks(line);
            }

            bool isPalindrome = 1;
            for (begin = 0; begin < count; begin++){
                if (line[begin] != line[count] && opt_o == 0){
                    isPalindrome = 0;
                    line[strcspn(line, "\n")] = 0;
                    printf("%s is not a palindrome \n", line);
                    break;
                }
                if(line[begin] != line[count] && opt_o >= 1){                   
                    isPalindrome = 0;
                    toFile(line, outFile);
                    break;
                }
                count--;
            }

            if(isPalindrome && opt_o == 0){
                line[strcspn(line, "\n")] = 0;
                printf("%s is a palindrome \n", line);
            }

            if(isPalindrome && opt_o >= 1){
                toFile(line, outFile);
            }
        }
        if(opt_o >= 1){     
            fclose(outFile);
        }
        fclose(file);
    }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief removes blanks from a given line
 * 
 * @param line line of a file or command line input
 */
void removeBlanks(char *line){
    int i = 0, j = 0;
    while (line[i]){
        if(line[i] != ' '){
            line[j++] = line[i];
        }
        i++;
    }
    line[j] = '\0';
}

/**
 * @brief converts the line to all lowercase if the case does not matter
 * 
 * @param line line of a file or stdin
 * @param count of chars
 */
void everythingToLower(char* line, int count){
    for(int i = 0; i < count + 1; i++){
        line[i] = tolower(line[i]);
    }
}


/**
 * @brief function to save to a file if needed
 * 
 * @param line line of a file or stdin
 * @param outfile file where the line is saved to
 */
void toFile(char* line, FILE *outfile){
    line[strcspn(line, "\n")] = 0;
    fprintf(outfile, "%s is a Palindrome\n", line);
}

/**
 * @brief if no positional arguments are given the function takes stdin instead of a file
 * 
 * @param argv argument vector
 * @param opt_i variable to determine if case matters
 * @param opt_s variable to determine if blanks count
 * @param opt_o variable to determine if output is saved to a file
 * @param o_arg path of the file to save too
 */
void fromCommandLine(char *argv[], int opt_i, int opt_s, int opt_o, char *o_arg){
        FILE *oFile;
        
        for(;;)
        {
            if(opt_o >= 1){
                oFile = fopen(o_arg, "a");
            }
            char *line;
            size_t len = 0;
            getline(&line, &len, stdin);
            bool isPalindrome = 1;
            
            int begin = 0;
            int count = strlen(line) - 2;

            if(opt_i >= 1){
                everythingToLower(line, count);
            }

            if(opt_s >= 1){
                removeBlanks(line);
            }
                
            for (begin = 0; begin < count; begin++){
                if (line[begin] != line[count]){
                    isPalindrome = 0;
                    line[strcspn(line, "\n")] = 0;
                    if(opt_o >= 1){
                        toFile(line, oFile);
                        fclose(oFile);
                    }
                    printf("%s is not a palindrome \n", line);
                    break;
                }
                count--;
            }
            if(isPalindrome)
            {
                line[strcspn(line, "\n")] = 0;
                if(opt_o >= 1){
                    toFile(line, oFile);
                    fclose(oFile);
                }
                printf("%s is a palindrome \n", line);
            }
        } 
}

