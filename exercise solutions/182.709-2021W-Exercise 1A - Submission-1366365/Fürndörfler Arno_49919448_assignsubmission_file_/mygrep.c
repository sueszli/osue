/*
	Author: Arno Fürndörfler
	Number: 11739704
	E-Mail: e11739704@student.tuwien.ac.at

	Last-Modified: 14.11.2021


	SYNOPSIS:
		mygrep [-i] [-o outfile] keyword [file1, file2, ...]

	Short Description:
		-default: no options and no files given to the program 
			search (keyword): caesensitive (Test != TEST)
			output: standard ouput 
			input:  standard input	
			
		-i search is caseinsensitive (Test = TEST = TeSt = test)
		-o outcome of the search will be send to an outputfile 
	
	Use-Examples:

	(1) 
		$ ./mygrep rat
		Trap a rat!
		Trap a rat!
		Never odd or even
		Rating System
		Operating Systems
		Operating Systems
	(2)
		$ ./mygrep -i rat
		Rating System
		Rating System
		Operating Systems
		Operating Systems
	(3)
		$ cat example.in
		Operating Systems
		Rating System
		$ ./mygrep -o example.out rat example.in
		$ cat example.out
		Operating Systems
		
*/

//All Libraries are from the C Standard Library
#include <stdio.h> 	//Files (output File -o and input Files)
#include <stdlib.h> 	//EXIT_FAILURE and EXIT_SUCCESS
#include <getopt.h> 	//Option Handling
#include <string.h> 	//strstr()
#include <ctype.h> 	//tolower()
#include <signal.h>	//Sigin Handling (for stdio termination)

//C99 Standard limit of 4095 characters.
#define MAX_LENGTH 4095

//no options set
void readFile(char* Filename, char* Keyword);
//option o set
void readFileOutput(char* Filename, char* Keyword, char* Outputfilename);
//option i set
void readFileCaseSesitive(char* Filename, char* Keyword);
//option o and i set
void readFileCaseSesitiveOutput(char* Filename, char* Keyword, char* Outputfilename);
//case sensitive search, destroys case information
char* stri(char* line, char* keyword);


//Error-Handling:
//usage message when SYNOPSIS is wrong and EXIT_FAILURE
void usage();
//error message = what went wrong and EXIT_FAILURE
void errormessage(char* error);

//GlobalVariable that saves the name of the program 
char* nameOfProgram;

//GlobalVariable that saves the signal status
volatile sig_atomic_t quit = 0;
//Signal-Handling:
void handle_signal(int signal){quit=1;}



int main(int argc, char **argv){
	nameOfProgram = argv[0];
	char *keyword = NULL;
	char *o_arg = NULL;
	int opt_i = 0;
	int c = 0; 
	int readFromStdIn = 0;

	struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler= handle_signal;
        sigaction(SIGINT, &sa, NULL);


//Option Handling
	while ( (c = getopt(argc, argv, "o:i")) != -1){
		switch (c) {
			case 'o': 
				o_arg = optarg;
				break;
			case 'i': 
				opt_i++;
				break;
			default:
				//invalid option was given
				usage();
		}
	}


	//not more than 1 -i option 
	if(opt_i>1){
		printf("wrong usage of option -i. See Synopsis:");
		usage();
	}

	//no keyword was given
	if(argv[optind] == NULL){
		printf("no keyword was entered. See Synopsis:");
		usage();
	}
	
	keyword = argv[optind];

	//check if files are given after the keyword 	
	if(argc > optind+1) {
		readFromStdIn = 0;
	}else{
	//No files then read from StdIn
		readFromStdIn = 1;
	}


//Program:
	//No options: Default
	if(o_arg == NULL && opt_i == 0){
		if(readFromStdIn == 1){
			FILE *stream = stdin;
			char *line = NULL;
			size_t len = 0;
			ssize_t nread;

			if (stream == NULL) {
				errormessage("Coudn't open standard input");	
			}

			

			while ((nread = getline(&line, &len, stream)) != -1 && quit==0) {
				if(strstr(line, keyword) != NULL){
					printf("%s", line);
				}
			nread = 0;
			}

			free(line);
			fclose(stream);
			exit(EXIT_SUCCESS);

		}else if(readFromStdIn == 0){
			for(int i = 1; optind+i < argc; i++){
				readFile(argv[optind+i], keyword);
				}
			exit(EXIT_SUCCESS);
			}

	//Option: -i 
	}else if(o_arg == NULL && opt_i == 1){
		if(readFromStdIn == 1){
			FILE *stream = stdin;
			char *line = NULL;
			size_t len = 0;
			ssize_t nread;

			if (stream == NULL) {
				errormessage("Coudn't open standard input");	
			}
			
			char *lineSave = NULL;

			while ((nread = getline(&line, &len, stream)) != -1 && quit==0) {
				//saves case information because stri destroys it
				//case information of keyword can be destroyd 
				//because case ifnormation of keyword is later not needed
				lineSave = strdup(line);

				if(stri(line, keyword) != NULL){
					printf("%s", lineSave);
				}

				free(lineSave);
				nread = 0;
			}

			free(line);
			fclose(stream);
			exit(EXIT_SUCCESS);

		}else if(readFromStdIn == 0){
			printf("Reads from files");

			for(int i = 1; optind+i < argc; i++){
					readFileCaseSesitive(argv[optind+i], keyword);
				}
			exit(EXIT_SUCCESS);
			}

	//Option: -o filename
	}else if(o_arg != NULL && opt_i == 0){ 
		if(readFromStdIn == 1){
			FILE *stream = stdin;
			char *line = NULL;
			size_t len = 0;
			ssize_t nread;
			FILE *out = fopen(o_arg , "a");

			if (stream == NULL) {
				errormessage("Coudn't open standard input");	
			}
			
			while ((nread = getline(&line, &len, stream)) != -1 && quit==0) {
				if(strstr(line, keyword) != NULL){
					//write in the file: 
					fputs(line, out);	
				}
				nread = 0;
			}
			
			fclose(out);
			free(line);
			fclose(stream);
			exit(EXIT_SUCCESS);

		}else if(readFromStdIn == 0){
			for(int i = 1; optind+i < argc; i++){
				readFileOutput(argv[optind+i], keyword, o_arg);
				}
			exit(EXIT_SUCCESS);
			}

	//Option: -i -o filename
	}else if(o_arg != NULL && opt_i == 1){ 
		if(readFromStdIn == 1){
			FILE *stream = stdin;
			char *line = NULL;
			size_t len = 0;
			ssize_t nread;
			FILE *out = fopen(o_arg , "a");

			if (stream == NULL) {
				errormessage("Coudn't open standard input");	
			}

			char *lineSave = NULL;

			while ((nread = getline(&line, &len, stream)) != -1 && quit==0) {
				//saves case information because stri destroys it
				//case information of keyword can be destroyd 
				//because case ifnormation of keyword is later not needed
				lineSave = strdup(line);

				if(stri(line, keyword) != NULL){
					fputs(lineSave, out);	
				}

				free(lineSave);
				nread = 0;
			}

			fclose(out);
			free(line);
			fclose(stream);
			exit(EXIT_SUCCESS);

		}else if(readFromStdIn == 0){
			for(int i = 1; optind+i < argc; i++){
				readFileCaseSesitiveOutput(argv[optind+i], keyword, o_arg);
				}
			exit(EXIT_SUCCESS);
			}
	}
	
return 0;
}


//Read file
void readFile(char* Filename, char* Keyword){
	FILE * file = fopen(Filename, "r");
	char line[MAX_LENGTH];

	if(file == NULL) {
		errormessage("Coudn't open the inputfile");	
	}

	while(fgets(line, sizeof(line), file)) {
		if(strstr(line, Keyword) != NULL){
			printf("%s", line);
		}
	}
	fclose(file);
}


//Warning! destroys upper/lower case information of given line and keyword 
//Better save information before 
char* stri(char* line, char* keyword){
	//no check on keyword because, Keyword can't be NULL (check above)
	if(line == NULL){
		errormessage("Input error, Line==NULL");	
	}
	 
	int lengthKeyword = strlen(keyword);
	int lengthLine = strlen(line);
	 
	 //make all lowercase 
	for(int i = 0; i < lengthKeyword; i++){
		keyword[i] = tolower(keyword[i]);
	}
	for(int j = 0; j < lengthLine; j++){    
		 line[j] = tolower(line[j]);
	}
	  
	 
	if(strstr(line, keyword) == NULL){
		 return NULL;
	}else{
		  return line;
	}
}
void readFileOutput(char* Filename, char* Keyword, char* OutputFilename){
	FILE * file = fopen(Filename, "r");
	FILE * out = fopen(OutputFilename, "a");
	char line[MAX_LENGTH];

	if(file == NULL) {
		errormessage("Coudn't open the inputfile");	
	}

	if(out == NULL) {
		errormessage("Coudn't open the Outputfile");
	}

	while(fgets(line, sizeof(line), file)) {
		if(strstr(line, Keyword) != NULL){
			fputs(line, out);
		}
	}

	fclose(out);
	fclose(file);
	}
void readFileCaseSesitive(char* Filename, char* Keyword){
	FILE * file = fopen(Filename, "r");
	char line[MAX_LENGTH];

	if(file == NULL) {
		errormessage("Coudn't open the inputfile");	
	}

	char *lineSave = NULL;	

	while(fgets(line, sizeof(line), file)) {
		//Case information of line saved 
		lineSave = strdup(line);
		
		if(stri(line, Keyword) != NULL){
			printf("%s", lineSave);
		}
	}
	fclose(file);
}

void readFileCaseSesitiveOutput(char* Filename, char* Keyword, char* OutputFilename){
	FILE * file = fopen(Filename, "r");
	FILE * out = fopen(OutputFilename, "a");
	char line[MAX_LENGTH];

	if(file == NULL) {
		errormessage("Coudn't open the inputfile");	
	}

	if(out == NULL) {
		errormessage("Coudn't open the Outputfile");	
	}

	char *lineSave = NULL;	

	while(fgets(line, sizeof(line), file)) {
		//Case information of line saved 
		lineSave = strdup(line);
		
		if(stri(line, Keyword) != NULL){
			//write in File
			fputs(lineSave, out);
		}
	}

	fclose(out);
	fclose(file);
}

//Error-Handling
void usage(){
	fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file1, file2, ...]", nameOfProgram);
	exit(EXIT_FAILURE);
}

void errormessage(char* error){
	fprintf(stderr, "Name of Program: %s, Problem: %s", nameOfProgram, error);
	exit(EXIT_FAILURE);
}
