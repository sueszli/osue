/**
 * @file ispalindrom.c
 * @author Michael Stasek 1177727
 * @date 13.11.2021
 *
 * @brief A program to check if a line is a palindrom
 **/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <getopt.h>


#define NAME "ispalindrom"

/**
 * Usage function.
 * @brief The function writes a error description to stderr and then exists the program
 * @details Exit the program with EXIT_FAILURE. The error message contains the program name
 * and the cause of error
 * @param errormsg description of the error
 **/
static void usage(const char *errormsg){																	
		fprintf(stderr,"USAGE: %s [-s] [-i] [-o outfile] [file...], %s", NAME, errormsg);				
		exit(EXIT_FAILURE);
}

/**
 * @brief removes whitespace from a string and converts it characters to a lower case
 * @details If white_space is 1, then every space in the string is removed.
 * After that if low_case is value 1, then all characters get casted to lower
 * case. If both parameters are true, the string str will stay unchanged.
 *
 * @param str the to be modified string
 * @param low_case if 1 then lower case all characters
 * @param white_space if 1 then remove all white_spaces
 */
static void handleSpaceAndCase(char * str, int low_case, int white_space)
{
	int len = strlen(str);

	int i = 0;
	int j = 0;

	if(white_space){
	for (j = 0; j < len; j++) {
		if (str[j]!=' ') {
			str[i] = str[j];
			i++;
		}
	}
	int k;
	for (k=i; k < len; k++) {
		str[k] = '\0';
	}
	}
	if(low_case){
        int m;
        for (m=0; m < len; m++) {
		str[m] = tolower(str[m]);
	}
	}
}

/**
 *
 * @brief checks if a string is a palindrom
 * @details This function checks if the given line is a palindrom. At first the
 * handleSpaceandCase function is called to remove if needed the spaces between
 * chars and converts the characters of the string to a lower case. After that
 * it get checked if the string is a palindrom.
 *
 *
 * @param out the file in which the program writes to
 * @param line the string that get checked whether it is a palindrom
 * @param low_case indicates whether the palindrom check is case insensitiv
 * @param white_space indicates whether the palindrom check ignores white spaces
 **/
static void isPalindrom(FILE * out, char * line, int low_case, int white_space){

    char * str = strdup(line);
    handleSpaceAndCase(str, low_case, white_space);
	int length = strlen(str);
	int is_palindrom = 1;
	int first = 0;
	int last= length - 1;

	while (first < last) {
		if (str[first] != str[last]) {
			is_palindrom=0;
		}
		first++;
		last--;
	}
	if(is_palindrom){
		fprintf(out, "%s is a palindrom\n", line);
	}
	else{
		fprintf(out, "%s is not a palindrom\n", line);
	}
	free(str);
}

/**
 *
 * @brief reads lines from file and then checks for each line if they are a palindrom
 * @details This function reads line after line from the input file in. If the line ends with
 * '\n', it gets replaced with a null terminator. For each line a palindrom check is called.
 *
 *
 * @param out the file in which the program writes to
 * @param in the file from where the program reads the string, lines
 * @param low_case indicates whether the palindrom check is case insensitiv
 * @param white_space indicates whether the palindrom check ignores white spaces
 **/
static void readFileByLine(FILE * out, FILE * in, int low_case, int white_space){

	char * line = NULL;
	size_t length = 0;
	while(getline(&line, &length, in) != -1){
       if(line[strlen(line)-1] == '\n'){
			line[strlen(line)-1] = '\0';
		}
		if(strlen(line) == 0){
			continue;
		}
		isPalindrom(out, line, low_case, white_space);
	}
	free(line);
}


/**
 *
 * @brief main function of ispalindrom
 * @details This function executes the program. The output and input files get opened
 * and the function calls the readFileByLine function.
 *
 *
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if the program is successful, EXIT_FAILURE if an error occurs
 **/
int main(int argc, char ** argv){

	int opt;
	int low_case=0;
	int white_space=0;

	FILE * out = stdout;
	FILE * in = stdin;

	while((opt=getopt(argc,argv,"sio:"))!=-1) {
        switch (opt)
        {
        case 's':
				white_space = 1;
				break;
        case 'i':
				low_case = 1;
				break;
        case 'o':
            if (out != stdout)
            {
                usage("ERROR: only 1 output file allowed\n" );
            }
            out = fopen(optarg, "w");
            break;
        case '?':
            usage("ERR0R: Invalid option\n");
            break;
        default:
            assert(0);
        }
    }

	if(out == NULL){
		exit(EXIT_FAILURE);
	}

	if(optind==argc){
		readFileByLine(out,stdin, low_case, white_space);
		fclose(in);
	}
	else{
		for(int i = optind;i<argc;i++){

            in = fopen(argv[i],"r");
			if(in == NULL){
                usage("ERROR: File does not exist\n");
			}

			readFileByLine(out,in, low_case, white_space);
			fclose(in);

		}
	}

	fclose(out);
	exit(EXIT_SUCCESS);
}
