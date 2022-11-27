/**
 * @file myexpand.c
 * @author Fabian Hagmann (12021352)
 * @brief This file contains the solution for exercise1a (myexpand). It exchanges tabstops with spaces.
 * @details Synopsis: myexpand [-t tabstop] [-o outfile] [file...]. <br>
 * Default output, if not provided, is stdout. If no inputfiles are provided it will be queried from stdin
 * @date 2021-10-27
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#define TABSTOP_DISTANCE_DEFAULT 8

static int getTabstopDistance(char* argument);
static int checkAndProcessFile(char *input, FILE *outputFile);
static int processFile(FILE *inputFile, FILE *outputFile);
static FILE* getOutputStream(char* argument);
void usage(void);

int tabstopDistance = -1;
char *myprog;

/**
 * @brief Main function for expand functionality
 * @details parses the arguments (as seen in usage), processes all specified input files and writes the result in the specified output file)<br>
 * Global variables:
 * <ol>
 * 	<li>tabstopDistance (size of the tabstops)</li>
 *	<li>myprog (name of the programm as called)</li>
 * </ol>
 * @param argc argument count
 * @param argv argument values
 * @return exit code
 */
int main (int argc, char *argv[]) {
	// parse command line arguments
	myprog = argv[0];
	int c;
	char *outputFileName;
	FILE* outputFile = stdout;
	while ((c = getopt(argc, argv, "t:o:")) != -1) {
		switch(c) {
			case 't':
				tabstopDistance = getTabstopDistance(optarg);
				break;
			case 'o':
				outputFileName = optarg;
				outputFile = getOutputStream(optarg);
				break;
			default:
				usage();
		}
	}

	// set tabstop distance if not set before by parameters
	if (tabstopDistance == -1) {
		getTabstopDistance(NULL);
	}
	
	// print information on output target 
	if (outputFile == stdout) {
		fprintf(stderr, "[%s] No file set as output. Resorting to stdout\n", myprog);
	} else {
		fprintf(stdout, "[%s] Results will be written to %s\n", myprog, outputFileName);
	}
	
	if ((argc - optind) > 0) {
		// if input file(s) are providied
		int i;
		for (i = optind; i < argc; i++) {
			// process for every file
			int returnCode = checkAndProcessFile(argv[i], outputFile);
			if (returnCode == EXIT_FAILURE) {
				fprintf(stderr, "[%s] Skipping File %s. Continuing to next\n", myprog, argv[i]);
			}
		}
	} else {
		// if no input file(s) are provided... read input file from stdin
		fprintf(stdout, "[%s] No input file provided. Input on stdin (Ctrl+D to end)!\n", myprog);
		processFile(stdin,outputFile);
	}
	
	// close last open file and finish successfully
	fclose(outputFile);
	return EXIT_SUCCESS;
}

/**
 * @brief Converts the command line argument to the tabstop size (int)
 * @details If no argument was specified, resort to default TABSTOP_DISTANCE_DEFAULT.
 * If argument cannot be converted to int or is <= 0, also resort to default TABSTOP_DISTANCE_DEFAULT
 * Otherwise get value from the argument
 * @param argument contains the size as char* or NULL if it was not specified
 * @return converted size as int or default TABSTOP_DISTANCE_DEFAULT
 */
static int getTabstopDistance(char* argument) {
	// declare default tabstop size to TABSTOP_DISTANCE_DEFAULT
	tabstopDistance = TABSTOP_DISTANCE_DEFAULT;
	// if tabstop size was not provided...print info message and return default
	if (argument == NULL) {
		fprintf(stdout, "[%s] No tabstop distance inputed. Resorting to default: %d\n", myprog, TABSTOP_DISTANCE_DEFAULT);
		return tabstopDistance;
	} 

	// convert tabstop size to integer
	char *controll;
	tabstopDistance = (int)strtol(argument, &controll, 10);
	
	// check if conversion result is valid
	if (controll == argument || *controll != '\0') {
		fprintf(stdout, "[%s] Invalid tabstop distance inputed (<=0 or not a integer). Resorting to default: %d\n", 
			myprog, TABSTOP_DISTANCE_DEFAULT);
		tabstopDistance = TABSTOP_DISTANCE_DEFAULT;
	} else {
		fprintf(stdout, "[%s] Tabstop distance set to: %d\n", myprog, tabstopDistance);
	}

	return tabstopDistance;
}

/**
 * @brief convertes input argument to FILE* and opens it in append mode 
 * @details If argument was not specified it resorts to default stdout.
 * If the file cannot be opened in append mode it also resort to stdout.
 * Otherwise it opens the FILE* via fopen
 * @param argument contains the path to the file or NULL if it was not specified
 * @return the already opened FILE*
 */
static FILE* getOutputStream(char* argument) {
	if (argument != NULL) {
		FILE* foundFile = fopen(argument, "a");
		if (foundFile == NULL) {
			fprintf(stderr, "[%s] fopen failed (%s): %s\n", myprog, argument, strerror(errno));
			return stdout;
		}
		return foundFile;
	}
	return stdout;
}

/**
 * @brief checks for a input to be openable and processes it
 * @details opens the input in read mode. If sucess full cales the processFile() method the process it
 * @param input name of the input file
 * @param outputFile already opened FILE* to the output file
 * @return EXIT_FAILURE if input cannot be opened or a error happend during parsing.
 * EXIT_SUCCESS if input was successfuly opend and parsed
 */
static int checkAndProcessFile(char *input, FILE *outputFile) {
	FILE *inputFile = fopen(input, "r");
	if (inputFile == NULL) {
		// if file could not be opened for reading
		fprintf(stderr, "[%s] fopen failed (%s): %s. Continuing to next file\n", myprog, input, strerror(errno));
		return EXIT_FAILURE;
	} else {
		// process file and check results
		int processResult = processFile(inputFile, outputFile);
		if (processResult == EXIT_SUCCESS) {
			fprintf(stdout, "[%s] File \"%s\" successfully parsed\n", myprog, input);
		} else {
			fprintf(stderr, "[%s] File \"%s\" could not be parsed\n", myprog, input);
			return EXIT_FAILURE;
		}
		fclose(inputFile);
	}
	return EXIT_SUCCESS;
}

/**
 * @brief processes all characters present in the input file (stream) and writes the result in the output file (stream)
 * @details processes the input stream character for character and replaces tabstops with the appropriate
 * number of spaces. All input characters (except tabstops) and spaces will be written to the output stream
 * @param inputFile already opened input file (in read mode)
 * @param outputFile already opened output file (in append mode)
 * @return EXIT_FAILURE if character(s) could not be written to the output file. Otherwise EXIT_SUCESS
 */
static int processFile(FILE *inputFile, FILE *outputFile) {
	int c;
	int lineSize = 0;
	// read file character for character until the EOF
	while ((c = fgetc(inputFile)) != EOF) {
		int nextCharacterIndex = tabstopDistance * ((lineSize / tabstopDistance) + 1);
		//fprintf(stderr, "New char\n");
		switch(c) {
			case '\t':
				// add spaces until we reach the next tabstop
				while (lineSize < nextCharacterIndex) {
					if (fputc(' ', outputFile) == EOF) {
						fprintf(stderr, "[%s] fput failes: character could not be written to output file\n", myprog);
						return EXIT_FAILURE;
					}
					lineSize++;
				}
				break;
			case '\n':
				if (fputc(c, outputFile) == EOF) {
					fprintf(stderr, "[%s] fput failes: character could not be written to output file\n", myprog);
					return EXIT_FAILURE;
				}
				lineSize = 0;
				break; 
			default:
				if (fputc(c, outputFile) == EOF) {
					fprintf(stderr, "[%s] fput failes: character could not be written to output file\n", myprog);
					return EXIT_FAILURE;
				}
				lineSize++;
				break;
		}
	}
	if (fputc('\n', outputFile) == EOF) {
		fprintf(stderr, "[%s] fput failes: character could not be written to output file\n", myprog);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * @brief prints usage
 * @details prints usage (as specified in the exercise) to stderr
 */
void usage(void) {
	fprintf(stderr,"Usage %s [-t tabstop] [-o outfile] [file...]\n", myprog);
	exit(EXIT_FAILURE);
}
