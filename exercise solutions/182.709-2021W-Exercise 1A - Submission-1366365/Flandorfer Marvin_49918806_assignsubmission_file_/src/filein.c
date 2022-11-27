/**
 * Filein module
 * @file filein.c
 * @author Marvin Flandorfer, 52004069
 * @date 25.10.2021
 * 
 * @brief Implementation of the filein module.
 * */

#include "misc.h"

/**
 * Function for reading content
 * @brief Reads the content from the given input allocates needed memory for storage of the input.
 * @details The input will be read via a filepointer that can either point to a file or to stdin.
 * The needed memory for storage of the input will be allocated in different function calls and will be freed within the function.
 * Only the very last allocation will be used for the result. This allocated memory needs to be freed after usage.
 * All possible errors will be handled within the function and error messages will be written accordingly.
 * 
 * @param fp Filepointer that can either point to an already opened file or to stdin.
 * @result On success returns a char pointer to the read input. On failure (due to an error) returns a null-pointer.
 * */
static char *read_content(FILE *fp){
    char c;                             /**< Read character*/
    char *content = NULL;               /**< Content of the input*/
    while((c = fgetc(fp)) != EOF){
        char s[2] = "\0\0";
        s[0] = c;
        char *tmp = concat(content,s);
        free(content);
        content = tmp;
    }
    if(ferror(fp) != 0){
        error_message("fgetc");
        free(content);
        return NULL;
    }
    return content;
}

char *read_content_from_stdin(void){
    return read_content(stdin);
}

char *read_content_from_file(char filepath[]){
    char *content = NULL;               /**< Content of input that will be received from other function calls*/
    int close;
    FILE *fp = fopen(filepath,"r");
    if(!fp){
        error_message("fopen");
        return NULL;
    }
    content = read_content(fp);
    close = fclose(fp);
    if(close != 0){
        error_message("fclose");
        return NULL;
    }
    return content;
}