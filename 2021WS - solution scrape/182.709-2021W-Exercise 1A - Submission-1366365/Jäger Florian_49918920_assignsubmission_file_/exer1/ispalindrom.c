#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "ispalindrom.h"

/**
 * @author Florian Jäger 11810847
 * @brief Outputs if the input is a palindrom
 * @date 2021-11-12
 *  */

#define MAXLINESIZE 1024

/**
    @brief prints result
    @param line: line, which was getting checked
    @param equal: 1, if palindrom;0, if not
    @param o_arg: output-filename if set
    @return returns if output worked
*/
static int printResult(char* line, int equal, char* o_arg) {
    if(o_arg == NULL) {

        if (equal == 1) {
            printf("%s is a palindrom\n",line);
        } else {
            printf("%s is not a palindrom\n",line);
        }

        return 0;

    } else {
        FILE *out = fopen(o_arg,"a");
        if (out != NULL ) {
            
            if (equal == 1) {
                char* connect = malloc(sizeof(line)+30);
                strcpy(connect,line);
                char *text = " is a palindrom\n";
                strcat(connect,text);
                fputs(connect,out);
                free(connect);
                
                
            } else {
                char* connect = malloc(sizeof(line)+30);
                strcpy(connect,line);
                char *text = " is not a palindrom\n";
                strcat(connect,text);
                fputs(connect,out);
                free(connect);
                
            }
            fclose(out);
        
            return 0;
        }
        return 1;
        
    }
}
/** 
    @brief checks if line is a palindrom
    @param line: line which gets checked
    @param spaces: option if spaces are ignored
    @param cases: option if line should be case insensitive
    @return 1, if palindrom; 0, if not
*/
static int isPalindrom(char* line, int spaces, int cases) {
    int n = strlen(line);
    char *output = line;
    //process ignore space option
    if(spaces == 1) {
        int j = 0;
        for(int i = 0; i < n; i++,j++) {
            if (line[i]!=' ') {
                output[j] = line[i];
            } else {
                j--;
            }
        }
        output[j] = 0;
        line = output;
    }

    n = strlen(line);
    //process ignore case option
    if(cases == 1) {
        for(int i = 0; i < n; i++) {
            line[i] = tolower(line[i]);
        }
    }
    //compare first and last,etc.
    for(int i = 0; i < n; i++) {
        if (line[i] != line[n - i -1]) {
            return 0;
            }
        }
    return 1;
}

int main(int argc, char* argv[]) {

    char *o_arg = NULL;
    int caseInsensitive = 0;
    int ignoreSpaces = 0;
    int outputSet = 0;

    int c;
    //process arguments
    while((c = getopt(argc, argv, "o:is")) != -1) {
        switch(c) {
            case 'i': 
                caseInsensitive = 1;
                break;
            case 's':
                ignoreSpaces = 1;
                break;
            case 'o':
                outputSet = 2;
                o_arg = optarg;
                break;
            default:
                assert(0);
        }
    }

    int optionLength = caseInsensitive + ignoreSpaces + outputSet;


    FILE *f;
    char* line = (char*) malloc(sizeof(char) * MAXLINESIZE);

    if((argc - optionLength) == 1) {
        //get input from stdin
        fgets(line, sizeof(line), stdin);
        //delete \n
        line[strcspn(line,"\n")] = 0;
        //create copy
        char* copy = malloc(MAXLINESIZE*sizeof(char));
        strcpy(copy,line);
        printResult(line,isPalindrom(copy,ignoreSpaces,caseInsensitive),o_arg);
        free(copy);
        free(line);
        return EXIT_SUCCESS;

    } else {
        //get input from given file(s)
        for (int i = 1 + optionLength; i < argc; i++) {
            f = fopen(argv[i], "r");
            
            if(f == NULL) {
                return EXIT_FAILURE;
            }

            while (fgets(line,MAXLINESIZE, f) != NULL) {
                //delete \n
                line[strcspn(line,"\n")] = 0;
                //create copy
                char* copy = malloc(MAXLINESIZE*sizeof(char));
                strcpy(copy,line);
                printResult(line,isPalindrom(copy,ignoreSpaces,caseInsensitive),o_arg);
                free(copy);
                
                }
                
            fclose(f);
        }
        free(line);
        return EXIT_SUCCESS;
            
    }
}

