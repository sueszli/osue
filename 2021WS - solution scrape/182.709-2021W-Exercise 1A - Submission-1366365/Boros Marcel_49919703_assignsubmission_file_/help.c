/**
 * @file help.c
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 3.11.2021
 *
 * 
 * @brief the implementation of the header help module. 
 * 
 * 
 **/


#include "help.h"

//separate input by newLine character
void separateLines(char* prog, char* line_buffer, int opt_s, int opt_i, int opt_o, char* o_arg) {
    char* line = NULL;
    FILE *outputFile = NULL;
    if(o_arg != NULL) {
        outputFile = fopen(o_arg, "a");
        if(outputFile == NULL) {
            fprintf(stderr, "fopen failed in %s: %s\n", prog,strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    //get first token
    line = strtok(line_buffer,"\n");
    while(line != NULL) {
        char output_string[strlen(line)];
        strcpy(output_string,line);
        char equal = reverse(line, opt_s, opt_i);
        
        //check of result whether palindrom or not
        if(equal == 'y') {
            //is a palindrom
            
            if(opt_o == 1) {
                //write to an output file
                
                char* output_text = NULL;
                output_text = malloc(sizeof(char) * (strlen(output_string)) + 20);
                strcat(output_text, output_string);
                strcat(output_text, " is a palindrom\n");
                if(fputs(output_text, outputFile) == EOF) {
                    fprintf(stderr, "fputs failed in %s: %s\n", prog,strerror(errno));
                    exit(EXIT_FAILURE);
                }
                free(output_text);
            
            } else {
                
                //write to stdout
                printf("%s is a palindrom\n", output_string);
            }
        
        
        } else {
            //not a palindrom
            
            if(opt_o == 1) {
                
                char* output_text = NULL;
                output_text = malloc(sizeof(char) * (strlen(output_string)) + 20);
                strcat(output_text, output_string);
                strcat(output_text, " is not a palindrom\n");
                if(fputs(output_text, outputFile) == EOF) {
                    fprintf(stderr, "fputs failed in %s: %s\n", prog,strerror(errno));
                    exit(EXIT_FAILURE);
                }
                free(output_text);
            
            } else {
                printf("%s is not a palindrom\n", output_string);
            }
        }
        line = strtok(NULL,"\n"); //get next token
    }
    
    
    
    if(outputFile != NULL) {
        fflush(outputFile); //write buffered string into file
        fclose(outputFile);
    }
}


//removes whitespaces from a string
char* removeWhitespaces(char* str) {
    int counter = 0, idx = 0;
    
    //count whitespaces
    for(int i=0; i<strlen(str); ++i) {
        if(str[i] == ' ') {
            ++counter;
        }
    }
    if(counter == 0) {
        return str;
    }
    
    //get size of new string reduced by whitespaces
    char newString[strlen(str)-counter];
    for(int i=0; i<strlen(str); ++i) {
        if(str[i] != ' ') {
            newString[idx++] = str[i];
        }
    }
    
    //set end of string
    newString[strlen(str)-counter] = '\0';
    str = newString;
    return str;
}



//returns a reversed string
char reverse(char* str,int opt_s, int opt_i) {
    
    
    //check active options
    if(opt_s == 1) {
        //ignore whitespaces
        str = removeWhitespaces(str);
    }
    
    if(opt_i == 1) {
        //make string to lowercase
        str = lowerCase(str);
    }
    
    
    //get size of reversed string
    char str_reversed[strlen(str)];
    int idx = 0;
    
    for(int i = strlen(str)-1; i>=0; --i) {
        str_reversed[idx++] = str[i];
    }
    str_reversed[strlen(str)] = '\0';     //set last character to '\0' 
    if(strcmp(str, str_reversed) == 0) {
        return 'y';
    } else {
        return 'n';
    }
}


//set string to lowercase
char* lowerCase(char* str) {
    for(int i=0; i<strlen(str); ++i) {
        str[i] = tolower(str[i]);
    }
    return str;
}