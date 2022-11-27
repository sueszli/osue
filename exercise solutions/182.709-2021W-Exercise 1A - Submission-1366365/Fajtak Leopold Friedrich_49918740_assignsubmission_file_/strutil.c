/**
 * @file strutil.c
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "strutil.h"

int strchrocc(char* str, char c){
    if(str==NULL){
        fprintf(stdout, "WARNING invalid envocation of strcharocc");
        return -1;
    }
    int i=0;
    int count=0;
    for(; str[i]!='\0'; i++){
        if(str[i]==c)
            count++;
    }
    return count;
}
