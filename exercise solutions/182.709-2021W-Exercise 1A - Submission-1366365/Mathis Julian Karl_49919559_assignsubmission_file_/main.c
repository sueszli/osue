/**
 * @file main.c
 * @author Julian Mathis 12024756
 * @brief program checks if the given word in stdin or files are palindroms, and put it into a file or stdout
 * @version 0.1
 * @date 2021-11-14
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

const char* prog_name;
/**
 * @brief when user inputs the wrong inputs for this programm
 * 
 */
void usage(void){
    fprintf(stderr, "USAGE: %s [-s] [-i] [-o outfile] [file...]", prog_name);
    exit(EXIT_FAILURE);
} 

/**
 * @brief changes alls letters in a String to lower case
 * 
 * @param input String thats needs to be changed
 */
static void ignore_case(char * input){
    for(; *input != '\0'; input++){
        *input = tolower(*input);
    }
}

/**
 * @brief removes all " " from a string
 * 
 * @param input String that need to be changed
 */
static void ignore_spaces(char * input){
    char * copy = (char *)strdup(input);
    for(; *copy != '\0'; copy++){
        if(*copy != ' '){
            *input = *copy;
            input++;
        }
    }
    *input = '\0';
}

/**
 * @brief checks if String is a palindrom with different options 
 * 
 * @param input STring that needs to be checked
 * @param i ignores Upper and lower letter differences
 * @param s ignorer " " in String
 * @return int 1 or 0 for boolean 
 */
static int isPalindrom(char * input, int i, int s){
    // duplicate string to leave orignal intact for output
    char * word = (char *)strdup(input);

    if(word == NULL){
        exit(EXIT_FAILURE);
    }

    // made own functions to avoid problems with the word pointer
    if(i) {
        ignore_case(word);
    }

    if(s) {
        ignore_spaces(word);
    }
    
    int front = 0;
    int back = strlen(word)-1;

    while (back >= 0){
        if (word[front++]== word[back--]){
            ;
        } else {
            return 0;
        }
    }


    return 1;
}


/**
 * @brief 
 * 
 * @param argc size of argv
 * @param argv Array of terminal inputs
 * @return int never happens, there is always an Exit() before
 */
int main(int argc, char *argv[]){

    int opt_s = 0;
    int opt_i = 0;
    FILE *ofile = NULL;
    char * palindrom = NULL;
    prog_name = argv[0];
    int c;
    size_t len;
    while((c = getopt(argc, argv, "sio:")) != -1){
        switch(c){
            case 's':
                opt_s = 1;
                break;
            case 'i':
                opt_i = 1;
                break;
            case 'o':
                if(ofile == NULL)
                    ofile = fopen(optarg, "w");
                break;
            case '?':
                usage();
                break;
        }
    }
    if (optind>=argc){ // use stdin
            while (getline(&palindrom, &len, stdin) != -1) {
                // remove unnecessary "\n"
                if(palindrom[ strlen(palindrom)-1] == '\n'){
                    palindrom[ strlen(palindrom)-1] = '\0';
                }
                // check if palindrom and give output
                if(isPalindrom(palindrom, opt_i, opt_s)){
                    if(ofile == NULL)
                        printf("%s is a palindrom \n", palindrom);
                    else
                        fprintf(ofile ,"%s is a palindrom \n", palindrom);
                        
                } else {
                    if(ofile == NULL)
                        printf("%s is not a palindrom \n", palindrom);
                    else
                        fprintf(ofile ,"%s is not a palindrom \n", palindrom);
                }
            }
            free(palindrom);
            if(ferror(stdin)){
                fprintf(stderr, "[%s] Error: fgets failed with %s", prog_name, strerror((long)ferror));
                exit(EXIT_FAILURE);
            }
    } else { // use files
        while (optind < argc){
            FILE * toread = fopen (argv[optind], "r");
            if (toread == NULL){
                fprintf(stderr, "[%s] Error: toread file failed", prog_name);
            }

            while (getline(&palindrom, &len, toread) != -1) {
                // remove unnecessary "\n"
                if(palindrom[ strlen(palindrom)-1] == '\n'){
                    palindrom[ strlen(palindrom)-1] = '\0';
                }
                // check if palindrom and give output
                if(isPalindrom(palindrom, opt_i, opt_s)){
                    if(ofile == NULL)
                        fprintf(stdout, "%s is a palindrom \n", palindrom);
                    else
                        fprintf(ofile ,"%s is a palindrom \n", palindrom);
                        
                } else {
                    if(ofile == NULL)
                        fprintf(stdout,"%s is not a palindrom \n", palindrom);
                    else
                        fprintf(ofile ,"%s is not a palindrom \n", palindrom);
                }
            }
            fclose(toread); //close input file
            
        optind++;
        }
    }

    fclose(ofile);
    exit(EXIT_SUCCESS);
}