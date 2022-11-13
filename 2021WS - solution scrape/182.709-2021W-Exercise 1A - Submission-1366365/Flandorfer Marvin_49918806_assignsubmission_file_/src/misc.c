/**
 * Miscellaneous module
 * @file misc.c
 * @author Marvin Flandorfer, 52004069
 * @date 27.10.2021
 * 
 * @brief Implementation of the miscellaneous module.
 * */

#include "misc.h"

extern char *program_name;

char *concat(char *s1, char *s2){
    if(!s1 && !s2){
        return NULL;
    }
    if(!s1){
        char *s = malloc(strlen(s2)+1);
        if(!s){
            error_message("malloc");
            return NULL;
        }
        (void) strcpy(s,s2);
        return s;
    }
    if(!s2){
        char *s = malloc(strlen(s1)+1);
        if(!s){
            error_message("malloc");
            return NULL;
        }
        (void) strcpy(s,s1);
        return s;
    }
    char *s = malloc(strlen(s1)+strlen(s2)+1);
    if(!s){
        error_message("malloc");
        return NULL;
    }
    (void) strcpy(s,s1);
    (void) strcat(s,s2);
    return s;
}

void usage(void){
    (void) fprintf(stderr,"%s Usage: mycompress [-o outfile] [file...]\n", program_name);
    exit(EXIT_FAILURE);
}

void error_message(char *str){
    (void) fprintf(stderr, "%s Error: %s failed: %s\n", program_name, str, strerror(errno));
}