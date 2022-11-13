/**
*    	@author = Abdullah TOY
*    	@date = November 12th 2021
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

// Name of the program.
static char* name = "ispalindrom";

// A macro that prints the error messages and terminates afterwards.
#define terminateWithErrorMessage(...)							    	        \
	do{ 														                \
		fprintf(stderr, "%s ERROR:\n", name);                                   \
        exit(EXIT_FAILURE);                                                     \
	} while(0)

// A macro that prints the usage message.
#define usageMessage()															\
	do{																			\
		fprintf(stdout,"USAGE: %s [-s] [-i] [-o outfile] [file...]", name);		\
		exit(EXIT_FAILURE);														\
	} while(0)

// This struct is needed for the "-i" and "-s" options that are added to "./ispalindrom".
typedef struct differentOptions{
    unsigned int i : 1;
	unsigned int s : 1;
	FILE* output;
} differentOptions;

// Check for palindroms.
static int palindromCheck(char* start){	                                            
	char* end = start + strlen(start)-1;                                            // Start at the beginning and the end of the input text. 
	while(end >= start){                                                            // If the two characters are not equal, return 0, else continue.
		if(*end != *start){
			return 0;
        }
		end--;
		start++;
	}
	return 1;
}

// Remove all whitespaces.
static void whitespaceRemove(char* text){
	for(char* checkPosition = text; *checkPosition != '\0'; checkPosition++){       // Go through the entire input text.
		if(*checkPosition != ' '){                                                  // If there is no whitespace, save the current character. If there is one, continue.
			*text = *checkPosition;
			text++;
		}
	}
	*text = '\0';
}

// Convert everything to uppercase.
static void convertToUppercase(char* text){
	for(; *text != '\0'; text++){                                                   // Go through the entire input text and change the current character to its uppercase version.
		*text = toupper(*text);
	}
}


// Check input.
static void stringOutput(const char* text, differentOptions* option){
	char* currentChar = strdup(text);	                                            // Copy the original string for printing later.
    if(option->i){                                                                  // Handle case sensitivity.
		convertToUppercase(currentChar); 
    }
	if(option->s){                                                                  // Handle whitespaces.
		whitespaceRemove(currentChar);
    }
	if(palindromCheck(currentChar)){                                                // Handle different outputs.
		fprintf(option->output, "%s is a palindrom\n", text);                          
    }
	else{
		fprintf(option->output, "%s is not a palindrom\n", text);
    }
	free(currentChar);
}

// Check input file line by line.
static void checkFileLines(FILE* file, differentOptions* option){
	size_t length = 0;
    char* line = NULL;
	while(getline(&line, &length, file) != -1){
		if(strlen(line) == 0){
			continue;
		}
        if(line[strlen(line) - 1] == '\n'){                                         // Remove line breaks.
		    line[strlen(line) - 1] = '\0';
		}
		stringOutput(line, option);
	}
    free(line);
}

// Command lines
static int checkCommandLineArgs(differentOptions * option, int argc, char ** argv){
	int opt;
	while((opt=getopt(argc, argv, "iso:"))!=-1){
		switch(opt){                                                                // Switch the options.
			case 'i':
				option->i = 1;
				break;
			case 's':
				option->s = 1;
				break;
			case 'o':
				if(option->output != stdout){
					fclose(option->output);
				}
				option->output = fopen(optarg, "w");
				break;
			default:
				usageMessage();
		}
	}
	if(option->output == NULL){
	    terminateWithErrorMessage("Output file not found.");
	}
	return optind;
}

// Main
int main(int argc,char ** argv){
	differentOptions option = {0,0,stdout};
	int i = checkCommandLineArgs(&option, argc, argv);
	if(i >= argc){											
		checkFileLines(stdin, &option);
	}
	else{
		for(; i<argc; i++){
			FILE* input = fopen(argv[i], "r");
			if(input == NULL){
				terminateWithErrorMessage("File %s could not be opened.", argv[i]);
				continue;
			}
			checkFileLines(input, &option);
			fclose(input);
		}
	}
	exit(EXIT_SUCCESS);
}