
/**
 * @author Ivaylo Bonev, matriculation number: 12025894
 * @date 05-Nov-2021
 * @brief 	Exercise 1A - function "mygrep" that returns all lines in a file or in several files
 *			that contains the specified keyword.
 * @details	Function "mygrep" with usage mygrep [-i] [-o outfile] keyword [file...], where:
 *			- -i disables case-sensitivity,
 *			- -o writes the output to a specified output file as opposed to stdout,
 *			- keyword is the character sequence to search for,
 *			- [file...] is/are the optional file/files where the program should be searching for
 *			  the keyword. If no file/files is/are specified, the input is stdin.
**/

//Standard libraries
#include <stdlib.h>
#include <stdio.h>

//Library for strstr(), strcpy() and strcmp()
#include <string.h>

//Library for getopt(), optarg and optind
#include <unistd.h>

//Library for errno
#include <errno.h>

//Library for tolower()
#include <ctype.h>

//Library for assert()
#include <assert.h>

/* Constants */

//The maximum line length supported by this program.
#define LINE_LENGTH 256

/**
 * @brief 	Prints the program's usage to the user.
 * @details Outputs the program's usage (options and positional parameters)
 *			to the standard error output.
**/
static void printUsage(void) {
	fprintf(stderr, "Usage: mygrep [-i] [-o outfile] keyword [file...]\n");
}

/**
 * @brief 	Prints the program's usage to the user and
 *		  	exits the program with an EXIT_FAILURE status.
 * @details Called when the user attempts to start the program
 *			with invalid parameters. Prints the correct usage and
 *			exits with an EXIT_FAILURE status.
**/
static void invalidUsage(void) {
	printUsage();
	exit(EXIT_FAILURE);
}

/**
 * @brief 	Prints an error message and exits the program if the file is NULL.
 * @details Prints an error message to stderr and exits the program with EXIT_FAILURE if the
 *			file parameter is NULL (e.g. because the file could not be opened). Also prints
 *			the error message via strerror(errno). Does not do anything if the file is not NULL.
 *			argv0 stands for the first element in main()'s argv[] array, i.e. the name of the program.
 *			It is used when printing error messages.
 * @param file		The file whose value is checked.
 * @param fileName	The file's name as a string. Used in the error message, if required.
 * @param argv0		The first element of the array argv[] in main(), i.e. the name of the program.
 *					Used in the error messages.
**/
static void checkIfFileIsNull(FILE* file, char* fileName, char* argv0) {
	if (file == NULL) {
		//File couldn't be opened. Print an error message.
		fprintf(stderr, "%s: File \"%s\" could not be opened. Error: %s\n", argv0, fileName, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief 	Prints the passed line either to a file or to stdout.
 * @details Prints the passed line parameter either to a specified out-file in "a" mode
 *			(append new contents to old ones if file exists or create a new file if it doesn't)
 *			or to stdout if outFile parameter is NULL.
 * 			If the line to be printed is zero (ssize_t equivalent of NULL), the method does nothing.
 *			argv0 stands for the first element in main()'s argv[] array, i.e. the name of the program.
 *			It is used when printing error messages.
 * @param line		The line buffer for the function getline(). Has to be allocated beforehand.
 * @param outFile	The name of the file to output to. If NULL, program outputs to stdout.
 * @param argv0		The first element of the array argv[] in main(), i.e. the name of the program.
 *					Used in the error messages.
**/
static void printLine(char* line, char* outFile, char* argv0) {
	if (line == NULL) {
		return;
	}

	if (outFile == NULL) {
		//If no out-file has been specified, output to the console.
		fprintf(stdout, line);
	}
	else {
		//Output to the out-file.

		//Open the file in "a" mode in order to append changes at the end of the file (i.e. without deleting the old contents),
		//but also create the file if it does not exist.
		FILE* file = fopen(outFile, "a");

		checkIfFileIsNull(file, outFile, argv0);

		fprintf(file, line);
	}
}

/**
 * @brief 	Converts a string to a lower-case string.
 * @details Converts each letter of the passed string to its lower-case version and returns the modified
 *			string in the end. If the string parameter is NULL, NULL is returned.
 * @param string	The string that should be converted to a lower-case string.
 * @return	The passed string parameter, converted to a lower-case string.
**/
static char* strToLower(char* string) {
	if (string == NULL) {
		return NULL;
	}

	for (int i = 0; string[i]; i++) {
  		string[i] = tolower(string[i]);
	}

	return string;
}

/**
 * @brief 	Reads the input stream and writes the lines containing the specified keyword to the output file (or stdout if NULL).
 *			Search can be case-sensitive as well as case-insensitive.
 * @details Reads the input stream (supports all streams that getline() and fprintf() support) line by line
 *			and outputs those lines that contain the specified keyword (case-sensitive or case-insensitive).
 *			The aforementioned lines are written to a file with the specified name, or to stdout if outStream parameter is NULL.
 *			Parameter "line" should be allocated beforehand, and lineSize contains the allowed length of any read line (in characters).
 *			If any parameter apart from outStream is NULL or lineSize is 0, the method returns without doing anything.
 *			argv0 stands for the first element in main()'s argv[] array, i.e. the name of the program.
 *			It is used when printing error messages.
 * @param caseSensitive	Determines if the search for the keyword should be case-sensitive (1)
 *						or case-insensitive (0).
 * @param keyword		The character sequence to search for.
 * @param line			The pre-allocated line buffer, used by getline().
 * @param lineSize		The maximum amount of characters per line supported by this program.
 * @param readStream	The in-stream. Can be any stream type supported by getline().
 * @param outStream		The out-stream. Can be any stream type supported by fprintf().
 * @param argv0			The first element of the array argv[] in main(), i.e. the name of the program.
 *						Used in the error messages.
**/
static void readAndPrintLines(int caseSensitive, const char* keyword, char* line, size_t lineSize, FILE* readStream, char* outStream, char* argv0) {
	if (keyword == NULL || line == NULL || lineSize == 0 || readStream == NULL || argv0 == NULL) {
		return;
	}

	size_t charCount;

	while ((charCount = getline(&line, &lineSize, readStream)) != -1) {
		//Convert the line to a lower-case string if a case-insensitive search is required.
		//Store the conversion to a new line though, as we'll need to print the original line later on.
		char* lineCopy = malloc(sizeof(line));

		if (lineCopy == NULL) {
			fprintf(stderr, "%s: Something went wrong while trying to allocate memory for the line. Error: %s\n", argv0, strerror(errno));
			exit(EXIT_FAILURE);
		}

		strcpy(lineCopy, line);

		if (caseSensitive == 0) {
			lineCopy = strToLower(lineCopy);
		}

		if (strstr(lineCopy, keyword) != NULL) {
			//The keyword was found in this line. Print it.
			printLine(line, outStream, argv0);
		}

		free(lineCopy);
	}
}

int main(int argc, char *argv[]) {
	
	//Program is case-sensitive by default.
	int caseSensitive = 1;

	char* outFile = NULL;

	int option;
	while ((option = getopt(argc, argv, "io:")) != -1) {
		switch (option) {
			case 'i':
				caseSensitive = 0;
				break;
			case 'o':
				outFile = optarg;
				break;
			case '?':
				invalidUsage();
				break;
			default:
				assert(0); //Should never be reached.
				break;
		}
	}

	if ((argc - optind) < 1) {
		//Invalid number of positional arguments!
		invalidUsage();
	}

	//First positional argument is the keyword.
	//In case we want a case-insensitive search, convert the keyword to lower-case.
	char* kwrd = argv[optind];
	if (caseSensitive == 0) {
		kwrd = strToLower(kwrd);
	}

	const char* keyword = kwrd;
	optind++;

	//If there aren't any further positional arguments, then read from the console.
	//Else, read from the in-file(s) (the rest of the positional arguments).

	//getline() requires size_t type for the size.
	size_t lineSize = LINE_LENGTH;
	char* line = (char*) malloc(lineSize * sizeof(char));

	//In case the required memory could not be allocated (should rarely happen), throw an error.
	if (line == NULL) {
		fprintf(stderr, "%s: Could not allocate enough memory for reading the lines. Error: %s\n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((argc - optind) < 1) {
		//No more positional arguments. Read from the console.
		readAndPrintLines(caseSensitive, keyword, line, lineSize, stdin, outFile, argv[0]);
	}
	else {
		//Read from the in-file(s).
		char* fileName;
		FILE* inFile;

		while (optind < argc) {
			fileName = argv[optind];

			if (outFile != NULL) {
				//Make sure the in-file is not the same as the out-file!
				if (strcmp(fileName, outFile) == 0) {
					fprintf(stderr, "%s: File \"%s\" is both an in- and an out-file. This is not allowed.\n", argv[0], fileName);
					exit(EXIT_FAILURE);
				}
			}

			inFile = fopen(fileName, "r");

			checkIfFileIsNull(inFile, fileName, argv[0]);

			//Read the file line by line and print all lines that contain the keyword, if any.
			readAndPrintLines(caseSensitive, keyword, line, lineSize, inFile, outFile, argv[0]);

			fclose(inFile);
			optind++;
		}
	}

	//Free used memory.
	free(line);

	return EXIT_SUCCESS;
}
