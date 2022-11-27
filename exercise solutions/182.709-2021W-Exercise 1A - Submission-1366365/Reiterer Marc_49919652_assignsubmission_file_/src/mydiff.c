/**
 * @file mydiff.c
 * @author Marc Reiterer <e12020640@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief program to compare the content of 2 files
 * @details In this program two files are compared line by line
 * the difference in characters is printed to stdout
 * or a file
 **/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

 
static char *pgm_name; /**< The program name. used error output */

static FILE *f1; /**< file1, used in comparison */
static FILE *f2; /**< file2, used in comparison*/
static FILE *outputFile = NULL; /**< only set when option -o is used*/
 
/**
 * @brief Outputs usage information of mydiff
 * @details global variables: pgm_name
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-i] [-o outputfile] file1 file2\n",pgm_name);
    (void) fprintf(stderr, "Compare file1 and file 2 line by line and provide amount of different characters to stdout\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "  -i              not differentiating between upper and lower case letters\n");
	(void) fprintf(stderr, "  -o outputfile   output to file instead of stdout\n");

	exit(EXIT_FAILURE);
}

/**
 * @brief prints the formated linenumber and character difference
 * @details outputs difference information to the standard output or the specified output file
 * global variables: outputFile
 * @param lineNum the current line being inspected
 * @param cDiff the number of different characters found in this line
 */
void printDiff(int lineNum,int cDiff){
	if(cDiff>0){
		if(outputFile==NULL){	
			fprintf(stdout,"Line: %i, characters:%i\n",lineNum,cDiff);
		}else{
			fprintf(outputFile,"Line: %i, characters:%i\n",lineNum,cDiff);
		}
	}
	
}

/**
 * @brief closes all used FILE streams
 * @details will close and remove references to the
 * used FILE streams
 * global variables: f1,f2,outputFile
*/
static void closeAll(void){
	if(f1!=NULL){
		fclose(f1);
		f1 = NULL;
	}	
	if(f2!=NULL){
		fclose(f2);
		f2 = NULL;
	}
	if(outputFile!=NULL){
		fclose(outputFile);
		outputFile = NULL;
	}
}


/**
 * Program entry point.
 * @brief finds the line-wise characterdifference between two files
 * @details counts the number of different characters in each line
 * of two files. results are either printed to stdout or a file
 * global variables: f1,f2,outputFile,pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector. requires two file names to work
 * @return returns EXIT_SUCCESS on successfull execution.
 */
int main(int argc, char **argv){
	 
	pgm_name = argv[0]; 
	char *outfile = NULL; /**< read filename for outputFile  */
	int ignoreCase = 0; 


	int c;
	while( (c = getopt(argc,argv,"o:i"))!=-1){
		switch(c){
			case 'i': ignoreCase = 1;
			 	break;
			case 'o': outfile = optarg;
				break;
			case '?': 
				(void)fprintf(stderr,"getopt failed: %s\n",strerror(errno));
				usage();
				break;
			default: usage();
				break;
		}
	}

	if((argc - optind) != 2){
		usage();
	}
	
	char *file1Name = argv[optind];
	char *file2Name = argv[optind + 1];

	f1 = fopen(file1Name,"r");
	if (f1 == NULL) {
		fprintf(stderr, "fopen if file1 failed: %s\n", strerror(errno));
		closeAll();
		exit(EXIT_FAILURE);
	}
	f2 = fopen(file2Name,"r");
	if (f2 == NULL) {
		fprintf(stderr, "fopen of file2 failed: %s\n", strerror(errno));
		closeAll();
		exit(EXIT_FAILURE);
	}
	if(outfile!= NULL){
		outputFile = fopen(outfile,"w");
		if (outputFile == NULL) {
			fprintf(stderr, "fopen of output file failed: %s\n", strerror(errno));
			closeAll();
			exit(EXIT_FAILURE);
		}
	}

	// 1 character for comparison, 1 character for \0 termination = 2
	char f1Buffer[2]=" "; /* using char[] instead of int, because of strcmp */
	char f2Buffer[2]=" ";

	char *result; /* used for checking if fgets has an error/end of line*/

	int lineNumber = 1;
	int differences = 0;

	while((feof(f1)==0) && (feof(f2)==0)){
		result = fgets(f1Buffer,2,f1);
		if(ferror(f1)!=0){
			fprintf(stderr, "fgetc of file1 failed: %s\n", strerror(errno));
			closeAll();
			exit(EXIT_FAILURE);
		}
		if(result == NULL){
			// error already handled, has to be end of file
			printDiff(lineNumber,differences);
			break;
		}

		result = fgets(f2Buffer,2,f2);	
		if(ferror(f2)!=0){
			fprintf(stderr, "fgetc of file2 failed: %s\n", strerror(errno));
			closeAll();
			exit(EXIT_FAILURE);
		}
		if(result == NULL){
			printDiff(lineNumber,differences);
			break;
		}

		if((f1Buffer[0] == '\n') || (f2Buffer[0]=='\n')){
			// end of line reached, consume rest of line
			while(f1Buffer[0] != '\n'){
				result = fgets(f1Buffer,2,f1);
				if(ferror(f1)!=0){
					fprintf(stderr, "fgetc of file1 failed: %s\n", strerror(errno));
					closeAll();
					exit(EXIT_FAILURE);
				}		
				if(result == NULL){
					// error already handled, has to be end of file
					printDiff(lineNumber,differences);
					break;
				}
			}
			while(f2Buffer[0] != '\n'){
				result = fgets(f2Buffer,2,f2);
				if(ferror(f2)!=0){
					fprintf(stderr, "fgetc of file2 failed: %s\n", strerror(errno));
					closeAll();
					exit(EXIT_FAILURE);
				}		
				if(result == NULL){
					// error already handled, has to be end of file
					printDiff(lineNumber,differences);
					break;
				}
			}
			printDiff(lineNumber,differences);
			lineNumber++;
			differences = 0;	
		}else{
			// compare

			if(ignoreCase == 0){
				if(strcmp(f1Buffer,f2Buffer)!= 0){
					differences++;
				}
			}else{
				if(strcasecmp(f1Buffer,f2Buffer)!= 0){
					differences++;
				}
			}
		}
	}	
	closeAll();
	return EXIT_SUCCESS;

}


