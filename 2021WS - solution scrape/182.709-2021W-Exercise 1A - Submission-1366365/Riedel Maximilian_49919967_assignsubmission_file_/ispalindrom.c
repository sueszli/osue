/*
ispalindrom.c (ispalidrom) 
Version 1
@date 14.11.2021
@author: Maximilian Riedel (StudentID: 11736247)
@brief: checks if given string is a palindrom
@details: checks if a char * is a palindrom and returns " is a palindrom" or " is no palindrom" 
*/

#include "ispalindrom.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/errno.h>

/*
@brief: checks if given string is a palindrom
@details: checks if a char * is a palindrom
@params: char * str
@return: " is a palindrom" or " is no palindrom"
*/
char * ispalindrom(char* str){
	const char * prog_name = "ispalindrom";
	int i;
	char * inv_str;
	inv_str = malloc(sizeof(char) * (strlen(str)-1));
        	if (inv_str == NULL){
	                fprintf(stderr, "[%s] ERROR: memory allocation failed: %s\n", prog_name, strerror(errno));
                	exit(EXIT_FAILURE);
		}

	char * s = inv_str;
	char * t = str+strlen(str);

	for (i = 0; i<(strlen(str)); i++) {
		t--;
		*s = *t;
		s++;
	}
	*s = '\0';
	str = t;

	if(strcmp(str, inv_str) == 0){
		free(inv_str);
		return " is a paindrom\n";
	} else {
		free(inv_str);
		return " is no paindrom\n";
	}
}
