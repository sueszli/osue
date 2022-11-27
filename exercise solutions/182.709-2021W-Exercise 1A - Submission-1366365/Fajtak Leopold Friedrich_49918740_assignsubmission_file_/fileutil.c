/**
 * @file fileutil.c
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @brief Utilities for mygrep
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "fileutil.h"

FILE* openFile(char* path, char* mode){
    FILE *f = fopen(path, mode);
    if(f==NULL){
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        return NULL;
    }
    return f;
}

int closeFile(FILE* file){
    if(ferror(file)){
        fprintf(stderr, "something went wrong when processing file\n");
        clearerr(file);
    }
    if(fclose(file)!=0){
            fprintf(stderr, "fclose failed: %s\n", strerror(errno));
            return 1;
    }
    return 0;
}
