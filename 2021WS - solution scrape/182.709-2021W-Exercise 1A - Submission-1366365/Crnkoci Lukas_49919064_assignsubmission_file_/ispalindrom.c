/**
 * @file    ispalindrom.c
 * @author  Lukas Crnkoci (01529217)
 * @brief   program that checks files or stdin if a line is a palindrom or not
 * @details program handles parameters, given options and checks if lines are palindroms or not and prints the results to stdout or an specified outfile.
 * @date    14.11.2021
*/

//includes
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

//prototypes
static void usage(char *programname);
static void remove_spaces(char* s);
static void upper_string(char* s);
static void checkLine(FILE *in_file, FILE *out_file, int opt_i, int opt_s);
static int checkPalindrom(char * s);
/**
 * @brief   main method.
 * @details given options and parameters are handled with getopt(). method checkLine() is called to carry out ispalindrom checks for given lines.
 * @param   argc as program argument count (number of parsed arguments)
 * @param   argv as argument vector (pointer array that points to each argument)
 * @return  exit-Code (as integer):
 *          -EXIT_SUCCESS (0) if program was processed correctly
 *          -EXIT_FAILURE (1) if an error occured 
*/
int main(int argc, char *argv[]){
    char *program_name = argv[0];
    int opt_s = 0;
    int opt_i = 0;
    char *o_arg = NULL;
    FILE *ifp;
    FILE *ofp;
    int c; 
    while ((c = getopt(argc, argv, "sio:")) != -1){
        switch (c)
        {
        case 's':
            opt_s++;
            break;
        case 'i':
            opt_i++;
            break;
        case 'o':
            o_arg=optarg;
            break;
        case '?':
            usage(program_name);
        default:
            break;
        }
    }

    if(o_arg != NULL){
        ofp = fopen(o_arg, "a");
        if(ofp==NULL){
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            fclose(ofp);
            exit(EXIT_FAILURE);
        }
    }else{
        ofp = stdout;
    }

    if((argc - optind)==0){
        ifp = stdin;
        checkLine(stdin,ofp, opt_i, opt_s);
        fclose(ifp);
        fclose(ofp);
    }
    if((argc - optind)>0){
        for(int i = optind; i < argc; i++){
            ifp = fopen(argv[i], "r");
            if(ifp==NULL){
                fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            checkLine(ifp,ofp,opt_i,opt_s);
            fclose(ifp);
            fclose(ofp);
        }
    }
    return 0;
}
/**
 * @brief   method for checking lines if they are a palindrom
 * @details method checks if an input line (stdin or fileinputs) is a palindrom and
 *          writes the new compressed filecontent to stdout or an specified outfile.
 * @param   infile as the filepointer of the given input-file (or stdin if not specified).
 * @param   outfile as the filepointer of the given output-file (or stdout of not specidief). 
 * @param   opt_i as the -i option 
 * @param   opt_s as the -s option
*/
void checkLine(FILE *ifp, FILE *ofp, int opt_i, int opt_s){
    size_t bufsize = 32;
    char * buffer = NULL;
    char * line = NULL;
    
    buffer = (char*)malloc(bufsize * sizeof(char));
    if(buffer == NULL){
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    line = (char*)malloc(bufsize * sizeof(char));
    if(buffer == NULL){
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (getline(&buffer, &bufsize, ifp)!=-1){
        
        if(buffer[strlen(buffer)-1] == '\n'){
            buffer[strlen(buffer)-1] = '\0';
        }
        line = (char*) realloc(line, strlen(line) + strlen(buffer));
        strcat(line, buffer);
        
        if(opt_i>0){
            remove_spaces(line);
        }
        if(opt_s>0){
            upper_string(line);
        }
        //upper_string(buffer);
        int is_palindrom = checkPalindrom(line);
        if(is_palindrom==1){
            fprintf(ofp, "%s is a palindrom\n",buffer);
            fflush(ofp);
        }else{
            fprintf(ofp, "%s is not a palindrom\n",buffer);
            fflush(ofp);
        }
        
    }
    free(line);
}
/**
 * @brief   method for printing the usage message
 * @details method prints the usage message to stderr if an error occured and exits the program with exit-code "1"
 * @param   void
 * @return  void
*/
static void usage(char* programname){
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", programname);
    exit(EXIT_FAILURE);
}

/**
 * @brief   method for removing (white)spaces
 * @details method removes whitespaces from the string - given as a parameter.
 * @param   s as a char pointer of the current line of text
 * @return  void
*/
static void remove_spaces(char* s) {
    char *buffer = (char*) malloc(strlen(s));

    int pos = 0;
    for (int i = 0; i < strlen(s); i++)
    {
        if(s[i] != ' '){
            buffer[pos] = s[i];
            pos++;
        }
    }
    s = (char*) realloc(s, strlen(buffer));
    if(s == NULL){
        fprintf(stderr, "realloc failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    strcpy(s, buffer);
    free(buffer);
}
/**
 * @brief   method for removing (white)spaces
 * @details method removes whitespaces from the string - given as a parameter.
 * @param   s as a char pointer of the current line of text
 * @return  void
*/
static void upper_string(char * s) {
  while (*s) {
    *s = toupper( *s);
    s++;
  }
}
/**
 * @brief   method for checking if a string is a palindrom
 * @details method checks if the given string (as a parameter) is a palindrom. 
 * @param   s as a char pointer of the current line of text
 * @return  int
 *          - 0 if string is not a palindrom
 *          - 1 if string is a palindrom
*/
static int checkPalindrom(char *s){
    int left = 0;
    int right = strlen(s) - 1;
    while (left <= right){
        if(s[left++] != s[right--]){
            return 0;
        }
    }
    return 1;
}
