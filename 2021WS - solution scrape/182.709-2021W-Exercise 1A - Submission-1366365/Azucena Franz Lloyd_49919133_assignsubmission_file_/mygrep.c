#include <stdio.h>		// needed for fopen, fclose, fprintf:x
#include <string.h>		// needed for strlen
#include <stdlib.h> 	// needed for exit-status
#include <stdbool.h>	// needed for bool
#include <unistd.h>		// needed for optarg
#include <ctype.h>		// needed for tolower and isspace

/**
 * @file mygrep.c
 * @author Franz Lloyd Azucena <e1425044@student.tuwien.ac.at>
 * @date 01.11.2021
 *
 * @brief Main program module for mygrep
 *
 * @details This program can read either from input files or stdin and checks every line of the input if it contains the keyword or not. Output can be either written to output file or to stdout (arg: "-o"). Arg "-i" (caseinsensitive).
 **/

/**
 * configuration
 * @details saves wether casesensitive or not, the keyword, the output and the program name
 **/
typedef struct Config {
	char* keyword;	// keyword we are looking for
	bool i;		// if true -> caseinsencitive
	FILE* output;	// output, can be a file or stdout
	char* name;	// name of program
} Config;

/**
 * @brief prints an error
 * @details prints an error with programm name and the message to stderr
 * @param name program name
 * @param message error message
 * @return void
 **/
static void printError(char* name, char* message) {
	fprintf(stderr, "%s ERROR: %s\n", name, message);
}

/**
 * @brief checks wether a string is NULL or empty
 * @details checks if the string is NULL or starts with '\0'
 * @return true, if the string is NULL or empty, false otherwise
 **/
static bool isEmpty(char* str) {
	if(str==NULL || strlen(str)==0) {
		return true;
	}
	return false;
}

/**
 * @details reads from given input and writes into given output, considering given configuration
 * @brief reads from input and writes into output
 * @param input input file
 * @param config configuration
 **/
static void doYourThing(FILE* input, Config* config) {
	char* line = NULL;
	size_t size = 0;
	int res = 0;
	
	while(true) {
		res = getline(&line, &size, input);
		
		if(res == -1) {
			break;
		}
		
		// copy line, just in case of option "-i"
		// the line should remain unchanged even if optoin "-i" is true
		char* dup = strdup(line);
		
		if(config->i) {
			for(int i = 0; i<strlen(dup); i++) {
				dup[i] = tolower(dup[i]);
			}
		}
		
		//				haystack, needle
		char* substrFound = strstr(dup, config->keyword);
		// strstr returns NULL if the substring is not found!
		if(substrFound!=NULL) {
			fprintf(config->output, "%s", line);
		}
		
	} //while(res != -1);
}

/**
 * @brief main function
 * @detail reads arguments and puts them into the configuration that will be given to other functions
 * @param argc argument counter
 * @param argv arguments
 * @return status int (either EXIT_FAILURE or EXIT_SUCCESS)
 **/
int main(int argc, char** argv) {

	Config config = {NULL, false, stdout, argv[0]};

	int option;	// option index
	char *ovalue = NULL;
	

	// TODO keyword muss noch ausgelesen und übergeben werden!
	// oder soll ich das keyword in die config schreiben .. aber kann ich als platzhalter einen leerstring oder sogar NULL rein geben?

	// When getopt() return -1, indicating no more options are present, the loop terminates.
	// the ":" indicates that this option requires an argument!
	// that means, that "o:" indicates, that the option "o" requires an argument
	// "i" does not require an argument
	while((option=getopt(argc, argv, "o:i"))!=-1) {
		switch(option) {
			case 'o':
				ovalue = optarg;
				break;
			case 'i':
				(&config) -> i = true;
				break;
			default:
				fprintf(stderr, "SYNOPSIS: %s [-i] [-o outfile] keyword [file...]", (&config)->name);
				// exit(int status) terminates the calling process immediately.
				// any open file descriptors belonging to the process are closed 
				// and any children of the process are inherited by process 1, init, 
				// and the process parent is sent a SIGCHLD singnal.
				// SIGCHLD: when a child process stops or terminates, SIGCHLD is sent tot the parent process.
				exit(EXIT_FAILURE);
		}
	}
	
	if(!isEmpty(ovalue)) {
		(&config) -> output = fopen(ovalue, "w");
	}

	// get keyword
	int index = optind;
	
	if(index >= argc) {
		// no keyword found!
		(&config) -> keyword = "";	// TODO oder bleibt keyword einfach NULL?
	} else {
		(&config) -> keyword = argv[index]; 
	}
	index++;
	
	// check for input files
	if(index >= argc) {
		// no input files found, read from stdin
		doYourThing(stdin, &config);
	} else {
		// input files found, read from input files
		for(; index<argc;index++) {
			FILE* input = fopen(argv[index], "r");
			if(input == NULL) {
				char* message = strcat(argv[index], " could not be opened");
				printError((&config) -> name, message);
				continue;
			}	
			doYourThing(input, &config);
			fclose(input);
		}
	}
	
	exit(EXIT_SUCCESS);
}