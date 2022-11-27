/**
 * @file isapalindrome.c
 * @author Benedikt Monarth-Hauser, 01501767 <benedikt.monarth-hauser@tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Implements the requested function of exercise 1a.
 * @details The program takes input either from a file specified or from stdin and
 * checks line by line if the line is a palindrome. If desired the check is not
 * case-sensitive and ignores whitespaces.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @param name The name of the program.
 */
static void usage(char *name) {
	
	fprintf(stderr,"Usage: %s [-s] [-i] [-o outputfile] file\n", name);
	
	exit(EXIT_FAILURE);
}

/**
 * Converts string to lowercase version of the same string.
 * @brief This function takes a string and converts it to lower case.
 * @details The function converts each letter of the string to lower case until
 * the end of the string is reached.
 * @param input The string to compress.
 */
static void input_to_lower_case(char* input) {

	while(*input != '\0') {

		*input = tolower(*input);
		++input;
	}
}

/**
 * Removes whitespaces from string.
 * @brief This function takes a string and removes every space.
 * @details The function looks at each letter of a copy of the string and checks whether it is
 * a space or not. If the value is not a space, the the character gets written in the original
 * string, otherwise it is skipped. At the end the original string is terminated to avoid errors.
 * @param input The string to remove whitespaces from.
 */
static void remove_spaces(char* input) {

	for(char* tmp = input; *tmp != '\0'; ++tmp){

		if(*tmp != ' '){

			*input = *tmp;
			++input;
		}
	}
	*input = '\0';
}

/**
 * Checks if string is a palindrome.
 * @brief This function iterates a string and its reverse string to check for a palindrome.
 * @details The function iterates over the input string and its reverse to check if the
 * characters match.
 * @param input The string to check for a palindrome.
 * @return If it is a palindrome 1 is returned, otherwise 0 is returned.
 */
static int is_palindrom(char* input) {	

	char* last = input + (strlen(input)-1);

	while(last >= input){

		if(*last != *input) return 0;

		--last;
		++input;
	}
	return 1;
}

/**
 * Handles input and output.
 * @brief This function takes the input and checks whether ist is a palindrome or not. The
 * result is printed to the parameter output.
 * @details The function iterates of the input line by line. for each line a copy is made for
 * the output. According to the set options when calling the program the input is either converted
 * to lower string or stripped of spaces or both.
 * @param input The input to check for a palindrome.
 * @param output Where to print the result.
 * @param s Whether to remove spaces (1) or not (0).
 * @param s Whether to convert the input to lower case (1) or not (0).
 */
static void input_handler(FILE* input, FILE* output, int s, int i) {
	
	char* line = NULL;
	size_t size = 0;

	while(getline(&line, &size, input) != -1) {

		if(line[strlen(line) - 1] == '\n') line[ strlen(line) - 1] = '\0';

		char* word = strdup(line);
		
		if(s == 1) remove_spaces(word);

		if(i == 1) input_to_lower_case(word);

		if(is_palindrom(word) == 1) {

			fprintf(output, "%s is a palindrom\n", line);

		} else {

			fprintf(output, "%s is not a palindrom\n", line);
		}
		free(word);
		
	}
	free(line);
	fclose(input);

}

/**
 * Main function.
 * @brief This function implements the argument handling and file operations.
 * @details The function iterates the arguments and sets the corresponding bits for case-sensitivity
 * and whitespace-removal. If an output file is specified it is opened here. Furthermore this
 * function checks whether there is an input file or if stdin is used as input and passes the
 * correct one to the input_handler function.
 * @param argc
 * @param argv
 * @return On success EXIT_SUCCESS and on failure EXIT_FAILURE is returned.
 */
int main(int argc, char *argv[]) {

    int s = 0;
    int i = 0;
	FILE* output = stdout;

	int tmp;
	int index = 1;

	while((tmp = getopt(argc, argv, "sio:")) != -1) {

		switch(tmp) {

			case 's':
				s=1;
				break;
			
			case 'i':
				i=1;
				break;

			case 'o':
				output = fopen(optarg, "w");
				
				if (output == NULL) {
				
					fprintf(stderr,"Outputfile could not be opened.\n");
					exit(EXIT_FAILURE);
				}
				break;

			default:
				usage(argv[0]);
		}
		index = optind;
	}

	if(index >= argc) input_handler(stdin, output, s, i);

	else {

		FILE* input = fopen(argv[argc - 1], "r");

		if(input == NULL) {
			
			fprintf(stderr,"Inputfile could not be opened.\n");
			exit(EXIT_FAILURE);
		}

		input_handler(input, output, s, i);
	}

	exit(EXIT_SUCCESS);
}