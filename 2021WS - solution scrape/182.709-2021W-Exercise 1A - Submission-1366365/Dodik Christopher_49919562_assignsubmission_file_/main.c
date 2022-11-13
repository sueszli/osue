/**
 *@file main.c
 *@author Christopher Dodik <e1325036@student.tuwien.ac.at>
 *@date 14.11.2021
 *
 *@brief Main program module for ispalindrom
 *
 *Description
 *
 *This program reads direct user or file input and checks if a line or word is a palindrom.
 *Options -s ignores whitespace and option -i ignores case sensitivity 
 *
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

void usage(char *myprog);
static void lowercase(char* line);
static void remwhitespace(char *line);
void remnewline(FILE *fpOUT, char* word, char* original);
int getcharnum(char* word);
void readfileinput(FILE *fpIN, FILE *fpOUT, int sens, int whitespace);
int palindrom(FILE *fpOUT, char* word, char* original);

/**
 *Mandatory usage function
 *@brief this function prints a usage message to stdout 
 *@details to avoid using global variables, the program name is handed as the parameter to the usage function
 *@param myprog get the argv[0] that contains the name of the program
 **/

void usage(char *myprog){
	fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...] \n", myprog);
	exit(EXIT_FAILURE);
}

/**
 *@brief this function puts a line to lower case 
 *@param line 	current line that needs to be turned to lower case
 **/

static void lowercase(char* line){

	int characters = getcharnum(line);

	while(*line != '\0'){
		*line = tolower(*line);
		++line;
	}
	line = line - characters;	
}

/**
 *@brief this function removes whitespace from a line 
 *@param line 	current line where whitespace needs to be removed
 **/

static void remwhitespace(char *line){

	char* words = line;
	int characters = getcharnum(line);

	while(*words != '\0'){
		if(*words != ' '){
			*line = *words;
			++line;
		}
		++words;

	}
	*line = '\0';
	line = line - characters;
}

/**
 *@brief this function returns the number of characters from a word or line
 *@param word 	current word where the characters are counted
 **/

int getcharnum(char* word){
	
	char* c1;
	int characters = 0;

	c1 = word; 

	while(*c1 != '\0'){
		++c1;
		++characters;
	}

	return characters;
}

/**
 *@brief this function reads the input from a file and calls (if stated) the options entered by the user
 *@param fpIN 		File where the input is read from
 *@param fpOUT 		File where the output is saved, if stated by the option
 *@param sens 		option case insensitivity
 *@param whitespace 	option to remove / ignore whitespace
 **/

void readfileinput(FILE *fpIN, FILE *fpOUT, int sens, int whitespace){

	char* line = NULL;
	size_t len = 0;

	while(getline(&line, &len, fpIN) != -1){
		char* uinput = strdup(line);
		if(whitespace == 1){
			remwhitespace(uinput);
		}
		if(sens == 1){
			lowercase(uinput);
		}
		remnewline(fpOUT, uinput, line);
		free(uinput);
	}
}

/**
 *@brief this function prepares word / line for the palindrom function by removing the '\n'
 *@param fpOUT 		File where the output is saved, if stated by the option
 *@param word 		word, possibly modified by option functions
 *@param original 	unmodified word, needed for output print
 **/

void remnewline(FILE *fpOUT, char* word, char* original){

	char* words = word;
	int characters = getcharnum(words);
	int i = 0;

	char nonewline[characters + 1];

	while(*words != '\n'){
		nonewline[i] = *words;
		++i;
		++words;
	}
	
	
	char* orig = original;
	characters = getcharnum(orig);
	i = 0;

	char noneworig[characters + 1];
	while(*orig != '\n'){
		noneworig[i] = *orig;
		++i;
		++orig;
	}

	nonewline[i] = '\0';
	noneworig[i] = '\0';
	
	palindrom(fpOUT, nonewline, noneworig);
}

/**
 *@brief most important function for the palindrom check
 *@param fpOUT 		File where the output is saved, if stated by the option
 *@param word 		word, possibly modified by option functions
 *@param original 	unmodified word, needed for output print
 **/

int palindrom(FILE *fpOUT, char* word, char* original){

	char* c1; 
	char* c2;

	int characters = getcharnum(word);	

	c1 = word;
	c2 = word + characters - 1;	
	
	while(c2 >= c1){
		if(*c1 != *c2){
			// not a palindrome
			if(fpOUT == NULL){
				fprintf(stdout, "%s is not a palindrom \n", original);
			}else{
				fprintf(fpOUT, "%s is not a palindrom \n", original);
			}
			return 0;
			
		}
		c1++;
		c2--;
	}

	if(fpOUT == NULL){
		fprintf(stdout, "%s is a palindrom \n", original);
	}else{
		fprintf(fpOUT, "%s is a palindrom \n", original);
	}
	return 0;
	
}

/**
 *@brief Main function of the module
 *@param argc The argument counter
 *@param argv The argument vector
 *@return Returns EXIT_SUCCESS
 **/

int main(int argc, char* argv[]){
	
	FILE *fpIN = NULL;
	FILE *fpOUT = NULL;

	char* o_arg = NULL;
	char* line = NULL;

	int opt_s = 0;
	int opt_i = 0;
	int c;
	int fileCount = 0;

	size_t length = 0;

	// -s ignore whitespace
	// -i case insensitive (see tolower(3), toupper(3))
	// -o write into given output file else to stdout

	while((c = getopt(argc, argv, "sio:")) != -1){
		switch(c){
			case 's': ++opt_s;
				break;
			case 'i': ++opt_i;
				break;
			case 'o': o_arg = optarg;
				break;
			case '?': usage(argv[0]); // not working?
				break;
			default : assert(0);
		}
	}
	
	
	if(o_arg != NULL){
		// -o did occur, open output file
		if ( (fpOUT = fopen(o_arg, "w")) == NULL){
			fpOUT = fopen(o_arg, "a");			
		}
		if (fpOUT == NULL){		
			fprintf(stderr, "fopen failed: %s\n", strerror(errno));		
			exit(EXIT_FAILURE);		
		}
	}

	if((opt_s > 1)||(opt_i > 1)){
		// -s occurs more than once
		usage(argv[0]);
	}
	
	// reading user input from command line
	if(argv[optind] == NULL){

		while(getline(&line, &length, stdin) != -1){  		
			
			char* uinput = strdup(line);
			if(opt_s == 1){
				remwhitespace(uinput);
			}
			if(opt_i == 1){
				lowercase(uinput);
			}
			
			remnewline(fpOUT, uinput, line);
			free(uinput);
		}		
	}

	// get number of optional files
	fileCount = argc - optind;
	
	// reading user input from one file
	if(fileCount == 1){
		
		fpIN = fopen(argv[optind], "r");
		if (fpIN == NULL){
			fprintf(stderr, "fopen failed: %s\n", strerror(errno));		
			exit(EXIT_FAILURE);	
		}
		readfileinput(fpIN, fpOUT, opt_i, opt_s);	
	
	// reading user input from more files
	} else if(fileCount > 1){
		// assign fpIN to current file, at the end of operation 
		// switch to next file if existing

		for(int i = 0; i < fileCount; i++){			

			if(argv[optind + i] != NULL){
				fpIN = fopen(argv[optind + i], "r");
				if (fpIN == NULL){
					fprintf(stderr, "fopen failed: %s\n", strerror(errno));		
					exit(EXIT_FAILURE);	
				}				
				readfileinput(fpIN, fpOUT, opt_i, opt_s);
			}
		}
	}
	
	// closing files if opened	

	if ( o_arg != NULL ){
		fclose(fpOUT);
	}
	
	if (fpIN != NULL){
		fclose(fpIN);
	}
	
	return EXIT_SUCCESS;
}
