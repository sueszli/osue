/**
 * @file myexpand.c
 * @author Tobias Hohendanner <e11941112@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Main and only program module.
 * 
 * This program mimics the command "expand" linux. It takes a file or User Input as input. You can call this program from the commandline.
 **/
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

void openFile(void);
void usage(void);
void replaceTab(char *str);

char *myprog;
char *path;
int tabstop_dist;
char *outfile;
#define CHUNK_SIZE 1000

/**
    * @brief Displays usage information.
    * @details 
    * @param myprog Holds absolute path to the program.
    * @return exit(EXIT_FAILURE) if there are faulty arguments.
* */

void usage(void) {
	fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n", myprog);
	exit(EXIT_FAILURE);
}

/**
    * @brief This function takes the InputFile if given and writes the content to singleLine.
    * @details 
    * @param new_str new text, which will be written to stdout or the given output file.
    * @param size Takes size of input text.
	* @param input Tries to open file at given path.
	* @param path Path to InputFile.
    * @return exit(EXIT_FAILURE) if file could not be opened/read/found.
* */

void openFile(void) {

	FILE *input = fopen(path, "r");

	if (input == NULL) {
		fprintf(stderr, "%s: Input file could not be opened/read/found: (Please try to input the absolute path)\n", strerror(errno));
	exit(EXIT_FAILURE);
	}

	char *singleLine = malloc(CHUNK_SIZE);
	int ch;
	int size = CHUNK_SIZE;

  	int i = 0;
  	while ((ch = fgetc(input)) != EOF)						// read one char until EOF 
  	{
    	singleLine[i++] = (char)ch;							// add char into buffer

    	if (i == size)										// if buffer full ...
    	{
      		size += CHUNK_SIZE;								// increase buffer size
     		singleLine = realloc(singleLine, size);			// reallocate new size
    	}
  	}

	singleLine[i] = '\0'; 

	replaceTab(singleLine);

  	free(singleLine);

	fclose(input);

}




/**
    * @brief This function replaces all tabs with the given amount of whitespaces and writes the result to an output file if given or to stdout.
    * @details 
    * @param new_str new text, which will be written to stdout or the given output file.
    * @param size Takes size of input text.
    * @param tabstop replace tab-character with a minimum of 1 tabstop spaces, default is 8
    * @param *str Pointer to the given Input text.
    * @param output Takes name from outfile, creates the file and writes new_str to the new file.
    * @return exit(EXIT_FAILURE) if file could not be created.
* */

void replaceTab(char *str) {


	char *new_str = malloc(sizeof(str));
	int size = sizeof(str);

	int j = 0;
	for(int i = 0; str[i] != '\0'; i++) {

		if (j+tabstop_dist >= size)									// if buffer full ...
		{
     		size += CHUNK_SIZE;										// increase buffer size
     		new_str = realloc(new_str, size);						// reallocate new size
    	}

        if(str[i] == '\t') {										
			for (int x = 0; x<tabstop_dist; x++) {					
    		new_str[j] = ' ';
			j++;
			}
		} else {
			new_str[j] = str[i];
			j++;
		}
		
	}

	new_str[j] = '\0';

	if(outfile == NULL) {
		printf("%s", new_str);
	} else {
		FILE *output = fopen(outfile, "w");

		if (output == NULL) {
			fprintf(stderr, "%s: Output file could not be created:\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		fputs(new_str, output);

		fclose(output);
	}

	free(new_str);
}




/**
    * @brief This function checks argv[] for the options -t <int> and -o <output_file> and if there is an InputFile or UserInput, and handles faulty input regarding options
    * @details 
    * @param argc how many args in argv
    * @param argv all of the arguments
    * @param tabstop replace tab-character with the given amount of 1 whitespaces. Minimum: 1 , Default: 8 (no -t option).
    * @param myprog takes program absolute path from argv[0]
    * @param outfile pointer to the pointer of outfile
    * @param input if there is no file input, stdin will be saved in input.
	* @param path points to input file if given.
    * @return exit(EXIT_FAILURE) if there are faulty arguments.
* */

int main(int argc, char *argv[])
{
	myprog = argv[0];
	tabstop_dist = 8;
	outfile = NULL;

	int option_index;
	while (( option_index = getopt(argc, argv, "t:o:")) != -1){
		switch (option_index) {
			case 't':
				tabstop_dist = strtol(optarg, NULL, 10);			// converts input to int
				if (tabstop_dist < 1) {
				fprintf(stderr, "%s: tabstop must be bigger or equal 1!\n", strerror(errno));
				exit(EXIT_FAILURE);	
				}
				break;
			case 'o':
				outfile = optarg;
				break;
			case '?': 												// invalid option
				usage();
				break;
			default:
				assert(0);
		}
	}
	
	if(argv[optind] == NULL) {										// if there is no input file, take stdin

		char *input = malloc(100000);
		printf("Please enter the String:\n");
		fgets(input, 100000, stdin);
		if(input[0] != '\n') {
		replaceTab(input);
		free(input);

		} else {
			fprintf(stderr, "%s: There was no input by user/input by file!\n", strerror(errno));
			usage();

		}	
	} else {

	path = argv[optind];
	openFile();
	
	}

	exit(EXIT_SUCCESS);
}
