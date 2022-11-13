/**
 * @file	myexpand.c
 * @author	Alexander Stummer 11777763
 * @date	8.11.2021
 * 
 * @brief main and only module
 *
 * This program takes files (or if non given input from stdin)
 * and prints the lines with the tabs replaced with a given
 * amount of spaces (8 if not given). its printed to an output
 * file if defined, alternatively to stdout
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>


const char* processLine(char* string, int tabstop);
/**
 * This function is almost the entire program
 * @brief	replaces tabs with spaces
 * @details	no global variables
 * @param	argc The argument counter
 * @param	argv The argument vector
 * @return	returns EXIT_SUCCESS on success or EXIT_FAILURE
 		on any error
 */
int main(int argc, char** argv){
	
	int distance 	= 8;
	//outfile, not used if no outfile specified
	FILE *outfile	= stdout;
	//inflag counts the infiles
	int inflag 	= 0;
	//this array stores the infiles - the array size is the
	//amount of arguments as that size is the max amount
	//possible of infiles. 
	char* infiles[argc];
	
	
	int c;
	char *ptr;	//necessary for strtol
	
	while((c = getopt(argc, argv, "t:o:")) != -1)
		switch(c){
		
		case't':
			distance = strtol(optarg, &ptr, 10);
			break;	
		case'o':
			outfile = fopen(optarg, "w+");
			if(!outfile){
				fprintf(stderr, "fopen error for output file\n %s\n", strerror(errno));
				fclose(outfile);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr,"unknown option\n");
			exit(EXIT_FAILURE);
			break;
	}
	while(optind < argc){
		infiles[inflag++] = argv[optind++];
	}
	
	/*
	 * end of processing options
	 * 
	 * initialize most of variables needed for loop
	 */
	
	//buffer set getline
	char* line_buf		= NULL;
	//buffer size, changed by getline
	size_t line_buf_size	= 0;
	//string length size returned by getline
	ssize_t line_size	= 0;
	//true if files to read remain
	bool fileflag	= true;
	//true if more lines to read
	bool lineflag	= true;
	//this will store which file to read from
	//initialized with stdin for if no files given
	FILE *fp	= stdin;
	//stores which infile is currently processed
	int incounter = 0;
	
	/*
	 * start of main loop
	 * first loop goes through the infiles
	 * or alternatively reads from stdin
	 * inner loop goes through lines of file/stdin
	 */
	
	while(fileflag == true){
		//true if no infiles given
		if(inflag<1){
			//stdin is the only file to read from, ends outer loop
			fileflag=false;
		}
		//if reached infiles > 0
		else{
			fp = fopen(infiles[incounter++], "r+");
			if(!fp){
				fprintf(stderr, "fopen error when trying to open an input file\n%s\n", strerror(errno));
			}
			if(incounter >= inflag){
				fileflag=false;
			}
			lineflag = true;
		}
		
		//begin of inner loop
		//processes all lines of given file
		while(lineflag == true){
			line_buf = NULL;
			line_buf_size = 0;
			line_size = getline(&line_buf, &line_buf_size, fp);
			
			//true if line is to be processed
			if(line_size >= 0){
				const char* outline = processLine(line_buf, distance);
				if(outline==NULL){
					free(line_buf);
					fclose(outfile);
					fclose(fp);
					exit(EXIT_FAILURE);
				}
				int fputsret = fputs(outline, outfile);
				free((char*)outline);
				if(fputsret<0){
					fprintf(stderr,"fputs error when trying to put line to outfile/stdout\n%s\n", strerror(errno));
					free(line_buf);
					fclose(fp);
					fclose(outfile);
					exit(EXIT_FAILURE);
				}
			}
		 	//if reached getline did not get a line
		 	//due to either error or EOF
			else{
				lineflag = false;
				//true if getline had an error
				if(!feof(fp)){
					fprintf(stderr,"getline error when trying to get \n%s\n", strerror(errno));
					free(line_buf);
					fclose(fp);
					fclose(outfile);
					exit(EXIT_FAILURE);
				}
			}
			free(line_buf);
		}
		fclose(fp);
		
	}
	fclose(outfile);
	
	exit(EXIT_SUCCESS);
}


const char* processLine(char* string, int tabstop){

	int stringLength = strlen(string);
	
	for(int i=0; i<strlen(string); i++){
		if(string[i]== '\t'){
			stringLength += (tabstop-(i%tabstop));
		}
	}
	
	char* retString = (char*) malloc(stringLength * sizeof(char));
	if(retString==NULL){
		fprintf(stderr,"malloc error, could not create memory space for string\n%s\n", strerror(errno));
		return NULL;
	}
	retString[stringLength-1] = '\0';
	
	//j tracks position of new String, i tracks position in old string
	int j=0;
	for(int i=0; i<strlen(string); i++){
		if(string[i] != '\t'){
			retString[j] = string[i];
			j++;
		}
		else{
			do{
				retString[j] = ' ';
				j++;
			} while((j%tabstop) != 0);
			/*
			for(int k=0; k<tabstop; k++){
				retString[j] = ' ';
				j++;
			}
			*/
		}
	}
	
	return &retString[0];

}
