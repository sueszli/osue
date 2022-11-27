/**
 * @file smgrep.c
 * @author Stefan Moser 12025955
 * @date 9.11.2021
 *
 * @brief implements the grepping
 *
 * @details smgrep provides functions for either reading from stdin or given input files
 * grepFiles reads from input Files, if there are none grepStdin is called
 * For further details of the functions, please read documentation of smgrep.h!
 * The static functions are documented below!
 **/

#define _GNU_SOURCE /*required for use of function getline(3)*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readArgs.h"

/**
 *@brief find needle in haystack (case insensitive search)
 *
 *@details strstri copies haystack and needle to temporarily allocated memory
 * converts all characters in said copy to uppercase
 * and then searches the needle copy in haystack by calling
 * the standard c library function strstr(3).
 *
 *@param *haystack points at examined line
 *@param *needle points at keyword
 *@return char * line if found or NULL
 **/
static char *strstri(const char *haystack, const char *needle){
	char *haystackCopy;
	char *needleCopy;
	char *pFound;
	/*allocate memory for copies of haystack and needle*/
	haystackCopy = malloc(strlen(haystack)  + 1);
	if(haystackCopy == NULL){
		fprintf(stderr, "[%s] ERROR: memory allocation failed at line %u\n",__FILE__,__LINE__);
		exit(EXIT_FAILURE);
	}

	needleCopy = malloc(strlen(needle)  + 1);
	if(needleCopy == NULL){
		fprintf(stderr, "[%s] ERROR: memory allocation failed at line %u\n",__FILE__,__LINE__);
		exit(EXIT_FAILURE);
	}
	/*copy haystack and needle*/
	strcpy(haystackCopy, haystack);
	strcpy(needleCopy, needle);

	/*convert all characters in haystack and needle copy to their uppercase equivalent*/
	for( int i = 0; i < strlen(haystackCopy); i++) haystackCopy[i]=toupper(haystackCopy[i]);
	for( int i = 0; i < strlen(needleCopy); i++) needleCopy[i]=toupper(needleCopy[i]);

	/*find needle in haystack*/
	pFound=strstr(haystackCopy,needleCopy);

	/*free memory of copies*/
	free(haystackCopy);
	free(needleCopy);

	return pFound;
}

/**
 *@brief examines line for specified keyword
 *
 *@details examineLine differentiates between case senisitive/insensitive and
 *then calls the appropraite functions strstr/strstri which checks the for the keyword in given line
 *
 *@param *pA input arguments
 *@param *pLine string that represents line from input
 *@return char * references said line or NULL
 **/
static char *examineLine(const char *pLine, InputArguments_t *pA){
	if(pA->caseSensitive) return strstr(pLine, pA->pKeyword);
	else return strstri(pLine, pA->pKeyword);
}



void grepFiles(InputArguments_t *pA){
	char *pLine = NULL;
	char *pFound;
	size_t n=0;
	ssize_t nread;
	/*write either to oufile or stdout*/
	FILE *oF = pA->outfile == NULL ? stdout : pA->outfile;

	/*iterate over input files*/
	for( int i = 0; i < pA->countInputFiles; i++){
		if(pA->inputFiles[i]) {

			fprintf(oF,"\n%s:\n",pA->pInputFiles[i]);

			/*read, examine and print all lines in file at i*/
			while((nread = getline(&pLine, &n, pA->inputFiles[i])) != -1){
				pFound =examineLine(pLine, pA);
				if(pFound) fprintf(oF,"%s",pLine);
			}
		}
	}
	if(pLine) free(pLine);
}


void grepStdin(InputArguments_t *pA){
	char *pLine = NULL;
	char *pFound;
	size_t n=0;
	ssize_t nread;
	/*write either to oufile or stdout*/
	FILE *oF = pA->outfile == NULL ? stdout : pA->outfile;

	printf("?>");
	while((nread = getline(&pLine, &n, stdin)) != -1 )
  	{
		pFound = examineLine(pLine, pA);
		if(pFound) fprintf(oF,"%s",pLine);
		/*signify waiting for input*/
		printf("?>");
	}
	if(pLine) free(pLine);
}


