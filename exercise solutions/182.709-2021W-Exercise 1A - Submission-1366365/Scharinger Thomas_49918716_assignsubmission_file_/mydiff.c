/**
 * @name mydiff
 * @author  Thomas Scharinger, MatrNr: 11777710
 * @date 12.11.2021
 * @brief This is an implementation of "mydiff", an exercise given in the course "Betriebssysteme UE".
 * @details This program can be used to compare two files and the number of differences between each line.
 * 	Usage: mydiff [-i] [-o] file1 file2
 * 	The option -i makes the comparison case independent.
 * 	The option -o writes the results into an output file.
 * 	Be aware that the function only compares characters until one line in a file has ended. 
 *  The additional characters in the other line aren't counted as differences.
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


char *myprog;

/**
 *@brief This is the usage error-message function.
 *@details Every time this program is used incorrectily this function will be executed and 
 * a description of the correct use of this function is written into stderr.
 *@param none
 *@returns EXIT_FAILURE
 */

void usage(void)	{
	fprintf(stderr, "Usage: %s [-i] [-o file] file file\n", myprog);
	exit(EXIT_FAILURE);
}

/**
 *@brief This is the main function.
 *@details This function executes the comparison between two files.
 * It reads out the command from stdin, gets the options, checks if the input is correct,
 * openes two files, gets two lines, compares them, and writes the number of differences in stdout
 * or an outputfile
 *@param int argc, char *argv[]
 *@returns EXIT_FAILURE, EXIT_SUCCESS
 */
int main (int argc, char *argv[]) { 
	myprog = argv[0];
	int i = 0;
	int out = 0;
	char *o_arg = NULL;
	char *file1name = NULL;
	char *file2name = NULL;
	int c;
	
/**
 * Here, the inputs and paramaters are checked.  
 * The outputfiles name is copied.
 */
	while( (c=getopt(argc,argv,"io:")) != -1){
		switch(c) {
			case 'i':	
				i = 1;
				break;
			case 'o':
				out = 1;
				o_arg = optarg;
				break;
			case '?':
				usage();
				
				break;
			default :
				break;
		}
	}
/**
 * In this part the correct use of the inputs is checked. 
 * In case of a errornous use of this program the programm uses the function "usage()" to terminate
 * and write an errormessage in stderr.
 * Furthermore the names of file1 and file2 are extracted from the input
 */
	switch(argc){ //controlling the correct usage of the inputs
		case 1:
		case 2:
			usage();
		break;
		case 3:
			if((i > 0) || (out > 0)){
				usage();
			}
			file1name = argv[1];
			file2name = argv[2];
		break;
		case 4:
			if((i !=1 ) || (out != 0)){
				usage();
			}
			file1name = argv[2];
			file2name = argv[3];
		break;
		case 5:
			if((i !=0) || (out !=1)){
				usage();
			}
			file1name = argv[3];
			file2name = argv[4];
		break;
		case 6:
			if((i != 1) || (out != 1)){
				usage();
			}
			file1name = argv[4];
			file2name = argv[5];
		break;
		default :
			usage();
		break;
	}
/**
 * Here, both the inputfiles and in case -o also the outputfiles are opened.
 * In case of an error the programm terminates and wirtes an errormessage in stderr.
 */
	
	FILE *file1;
	if((file1 = fopen(file1name, "r"))==NULL){
		fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
		exit(EXIT_FAILURE);
	}
	FILE *file2;
	if((file2 = fopen(file2name, "r"))==NULL){
		fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
		exit(EXIT_FAILURE);
	}	
	FILE *outfile;
	if(out==1){
		
		if((outfile = fopen(o_arg, "w"))==NULL){
			fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
			exit(EXIT_FAILURE);
		}	
	}

	int lineNr = 1;
	int diffs = 0;
	char* line1 = NULL;
	char* line2 = NULL;
	size_t len1 = 0;
	size_t len2 = 0;
	size_t shorterlen = 0;
	ssize_t nread1 = 0;
	ssize_t nread2 = 0;
	
/**
 * In this part, the whole comparison between the lines and the ouput is executed.
 * Essentially, this is a while-loop that only terminates when getting an EOF.
 */
	while( ((nread1 = getline(&line1,&len1, file1)) != -1) && (((nread2 = getline(&line2,&len2, file2)) != -1)) ){
	/**
	 * Here i compare the length of both lines and copy the shorter one into the variable "shorterlen".
	 * THIS PROGRAM ONLY COMPARES THE LINES UP TO THE LENGTH OF THE SHORTER LINE!
	 */	
		if(nread1>=nread2){
			shorterlen = nread2;
		}
		else{
			shorterlen = nread1;
		}
		
		
	/**
	 * This if{...}else{...} section compare the lines and searches for the number of differences.
	 * In both parts, firstly the first "shorterlen" characters in the lines are compared 
	 * and ONLY if there is a difference the comparison section is executed. 
	 * The number of differences are written into the variable "diffs".
	 * 
	 * In the if{...} part the case sensitive option is executed.
	 * I basically simply compare the characters on the same position in both lines.
	 * 
	 * In the else{...} part the case insensitve option is executded.
	 * I use strncasecmp with n=1 to compare the characters in the same positions.
	 * Although I didn't want to call a function for every single character in a line,
	 * I couldn't come up with a different case insensitive solution.
	 */
		if(i==0){
			if(strncmp(line1, line2, shorterlen) != 0){
			
				for( int i = 0; i < shorterlen; i++){
					if(line1[i] != line2[i]) diffs++;
				
				}
			}
		}
		else{	
			if(strncasecmp(line1, line2, shorterlen) != 0){
			
				for( int i = 0; i < shorterlen; i++){
					if(strncasecmp(line1 + i, line2 + i, 1) != 0) diffs++;
				
				}
			}
			
		}
		
		/**
		 * In this part, if the number of differences in a line are greater than 0,
		 * the amount of differences is written into stdout or the outputfile.
		 */
		if(diffs != 0){
			if(out==1){
				fprintf(outfile,"Line: %i, characters: %i\n", lineNr,diffs);
			}
			else{
				printf("Line: %i, characters: %i\n", lineNr,diffs);
			}
		}
		
		
		lineNr++;
		diffs = 0;
	} 
		/**
		 * Here i clean and close up everything. 
		 */

	fclose(file1);
	fclose(file2);
	if(out==1){
		fclose(outfile);
	}
	
	exit(EXIT_SUCCESS);
}
		
			
