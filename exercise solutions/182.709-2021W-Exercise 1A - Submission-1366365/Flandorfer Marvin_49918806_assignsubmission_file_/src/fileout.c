/**
 * Fileout module
 * @file fileout.c
 * @author Marvin Flandorfer, 52004069
 * @date 27.10.2021
 * 
 * @brief Implementation of the fileout module.
 * */

#include "misc.h"

int write_into_file(char filepath[], char str[]){
    int items_written;                  /**< Number of written characters*/
    int str_length = strlen(str);       /**< Length of the given string*/
    FILE *fp;                           /**< File pointer pointing to the output file*/
    fp = fopen(filepath, "w");
    if(!fp){
        error_message("fopen");
        return -1;
    }
    items_written = fwrite(str, 1, str_length, fp);
    if(items_written < str_length){
        error_message("fwrite");
        fclose(fp);
        return -1;
    }
    int close = fclose(fp);
    if(close != 0){
        error_message("fclose");
        return -1;
    }
    return 0;
}