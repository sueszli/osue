/**
 * @file readIn.c
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 06.12.2021
 *  
 * @brief Implementation file for functions declared in readIn.h
 *
 */


#include "readIn.h"

//check if the input values are hex numbers (with ascii table)
void checkInputValues(char* firstLine, char* secondLine, char** argv) {
     
    char* wrongSymbolArray; 
    if((wrongSymbolArray = malloc(strlen(firstLine) + strlen(secondLine) + 1)) == NULL) {
        fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    
 
    int wrong_symbol = 0;
    for(int i=0; i<strlen(firstLine); ++i) {
        int ascii_first = (int)firstLine[i];
        int ascii_second = (int)secondLine[i];
        
        
        if(((ascii_first >= 48) && (ascii_first <= 57)) || ((ascii_first >= 65) && (ascii_first <= 70))) {
            //firstLine[i] is hex digit
        } else {
            char str[2];
            str[1] = '\0';
            str[0] = firstLine[i];
            strcat(wrongSymbolArray, str);
            wrong_symbol = 1;
        }
        
        
        if(((ascii_second >= 48) && (ascii_second <= 57)) || ((ascii_second >= 65) && (ascii_second <= 70))) {
            //secondLine[i] is hex digit
        } else {
            char str[2];
            str[1] = '\0';
            str[0] = secondLine[i];
            strcat(wrongSymbolArray, str);
            wrong_symbol = 1;
        }
        
        
        if(wrong_symbol == 1) {
            //some digit is not hexadecimal
            free(firstLine);
            free(secondLine);
            fprintf(stderr, "Error in %s -> invalid input :%s\n",argv[0], wrongSymbolArray);
            free(wrongSymbolArray);
            exit(EXIT_FAILURE);
        }
        
    }
    free(wrongSymbolArray);
}


//read input
void readLines(char* firstLine, char* secondLine, char** argv) {
    
    char c;
    int newMemory = 2;
    
    
    //read in first line
    while((c = fgetc(stdin)) != '\n') {
        
    
        //create string from char to append to firstLine
        char str[2];
        str[1] = '\0';
        str[0] = c;
        strcat(firstLine, str);
        
        //check if new memory has to be allocated
        if(strlen(firstLine) == BYTES) {
             
            if((firstLine = realloc(firstLine,sizeof(char) * BYTES * newMemory++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
                free(firstLine);
                free(secondLine);
                exit(EXIT_FAILURE);
            } 
        }
    }
    
    //check for errors
    if(ferror(stdin)) {
        fprintf(stderr, "Error in %s -> fgetc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        exit(EXIT_FAILURE);
    }
    
    
    
    newMemory = 2;
    //read in second line
    while((c = fgetc(stdin)) != EOF) {
        
        
        //create string from char to append to secondLine
        char str[2];
        str[1] = '\0';
        str[0] = c;
        strcat(secondLine, str);
        
        //check if new memory has to be allocated
        if(strlen(secondLine) == BYTES) {
            if((secondLine = realloc(secondLine,sizeof(char) * BYTES * newMemory++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
                free(firstLine);
                free(secondLine);
                exit(EXIT_FAILURE);
            } 
        }
        
    }
    
    //check for errors
    if(ferror(stdin)) {
        fprintf(stderr, "Error in %s -> fgetc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        exit(EXIT_FAILURE);
    }
    
    
    //set last character of secondLine to '\0'
    secondLine[strlen(secondLine)-1] = '\0';
    
   
}