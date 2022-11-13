#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

/**
 * @file mydiff.c
 * @author Steven Ludwig, 11914281
 * @brief Checks 2 files, line by line for differences and outputs them. 
 * 
 * @details Optionally the output may be printed into terminal or into into specific outputfile that will be created automatically if
 * necessary. With flag -i there is an option for turning Case-Insensitivity on
*/

/**
 * @brief Pointer that stores the filename
 * Filename
 */
char *progName = "mydiff";


/**Boolean that will be set if necessary */
bool caseInsensitive;

/**
 * Usage function.
 * @brief Function writes usage into terminal as stderr and terminates the programm with appropriate EXIT_FAILURE.
 * @details uses global progName
 */
static void usage(void){
	fprintf(stderr, "Usage: %s [-i] [-o outfile] file_1 file_2 \n",progName);
	exit(EXIT_FAILURE);
}

/**
 * checker function.
 * @brief Counts differences in each line, by checking for different characters on same place. Returns amount of Indifferences 
 * if end of file or line was reached. Shorter Line sets the length of maximum comparrisons, therefore operat and operating are identical.
 * 
 * @param content1  First fileLine to compare
 * @param content2  Sencond fileLine to compare
 *
 * @return int checker with amount of differences in line. If 0, then line will be ignored.
 */
int checker(char *content1, char *content2){
    if(*content1 == '\n' || *content2 == '\n' || *content1 == '\0' || *content2 == '\0') 
		return 0;

    int char1 = *content1;
    int char2 = *content2;

    if(caseInsensitive) {
        char1 = tolower(*content1);
        char2 = tolower(*content2);
    }

    
    if(char2 == char1)
        return checker(++content1,++content2);
        else{
            return 1 + checker(++content1,++content2);
        }

}


/**
 * @brief main function, therefor starting point
 * 
 * @param argc argument counter 
 * @param argv arguments
 * @return exit with appropriate int
 */
int main(int argc, char **argv)
{
progName = argv[0];

char *outName;
FILE *outPutFile;

int c;
	while ( (c = getopt(argc, argv, "io:")) != -1 ){
		switch (c) {
			case 'i': caseInsensitive = true;
				break;
			case 'o': outName = optarg;
				break;
			case '?': usage();
				break;
			default:
				fprintf(stderr, "[%s] ERROR: with getopt\n", progName);
				exit(EXIT_FAILURE);
		}
	}
    
	if (outName == NULL) {
		outPutFile = stdout; //set Output to standard terminal output
	}else{
		if ( (outPutFile = fopen(outName, "w")) == NULL ) {
			fprintf(stderr, "[%s] ERROR: fopen outPutFile failed: %s\n", progName, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

    //checks input file
    if ((argc - optind) != 2){
        int temp = argc-optind;
        fprintf(stderr, " ERROR: Must have 2 input files to work with. Got: %d",temp);
        usage();
        }
    
    char* check_file_1 = argv[optind];
    char* check_file_2 = argv[optind+1];
    
    if(check_file_1 == NULL || check_file_2 == NULL){
        fprintf(stderr, "[%s] ERROR: At least one input file is missing: %s\n", progName, strerror(errno));
    }
    FILE *file_1 = fopen(check_file_1, "r");
		if (file_1 == NULL) {
			fprintf(stderr, "Could not open file.\n");
			exit(EXIT_FAILURE);
		}
    FILE *file_2 = fopen(check_file_2, "r");
		if (file_2 == NULL) {
			fprintf(stderr, "Could not open file.\n");
			exit(EXIT_FAILURE);
		}
    	

    int line = 1;
    int differences = 0;

    char *bufferLineFile1;
    char *bufferLineFile2; 
    size_t n1 = 0;
    size_t n2 = 0;
    size_t read1 = 0;
    size_t read2 = 0;


    while((read1 = getline(&bufferLineFile1,&n1,file_1))!=-1 && (read2 = getline(&bufferLineFile2,&n2,file_2))!=-1){
        
        differences = checker(bufferLineFile1,bufferLineFile2);
        if(differences>0){
            fprintf(outPutFile,"Line: %d characters: %d\n",line, differences);
        }
        differences = 0;
        line++;
    }
    
    free(bufferLineFile1);
    free(bufferLineFile2);


    fclose(file_1);
    fclose(file_2);


return EXIT_SUCCESS;
}