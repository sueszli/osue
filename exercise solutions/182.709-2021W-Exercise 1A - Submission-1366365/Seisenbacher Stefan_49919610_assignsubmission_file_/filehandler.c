#include "filehandler.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

extern char *PROGRAM_NAME;
/**
 * Writes a string to a file
 * @brief Simple writes to a file
 * @details Simple writes to a file
 * @param output This will be written to the file
 * @param file To this file will be written
 */
void writeToFile(char *output, char *file){    
    FILE* fp = fopen(file, "w");
    if (fp == NULL){
        fprintf(stderr, "%s: Error open [%s] to write.\n",PROGRAM_NAME,file);
        exit(EXIT_FAILURE);
    }
        
    fwrite(output, sizeof(output), 1, fp);
    fclose(fp);
}
/**
 * Reads from a file
 * @brief Simple reads to a file
 * @details Simple reads to a file 
 * @param file From this file will be written
 * @return Content from the file
 */
char* readFile(char *file){
    FILE * fp;
    char * line = NULL;
    char *fileInput = malloc(sizeof(char) * 1+1);
    size_t line_length = 0;
    ssize_t read;

    fp = fopen(file, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: Error open [%s] to read.\n",PROGRAM_NAME,file);
        exit(EXIT_FAILURE);
    }        

    while ((read = getline(&line, &line_length, fp)) != -1) {

        fileInput =  realloc(fileInput,strlen(fileInput)*sizeof(char)+sizeof(char)*line_length+1);       
        strcat(fileInput, line);
    }
    fclose(fp);
    if (line){
        free(line); 
    }
    return fileInput;
}