/**
 * @file mygrep.c
 * @author Tarik Rešidbegović <e12038064@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief mygrep
 *
 **/
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *program_name; /**< The program name. Needs to be defined by the application. **/

/**
 * @brief Prints an error message to stderr. The error message is prefixed with the program name (program_name) enclosed in square brackets.
 * @param format_string The format string
 * @param ... Format arguments
 */

__attribute__((format (printf, 1, 2))) void error(const char *const restrict format_string, ...){

	va_list args;
	va_start(args, format_string);
	fprintf(stderr, "[%s] ", program_name);
	vfprintf(stderr, format_string, args);
	va_end(args);

	fprintf(stderr, "\n");
}


/**
 * @brief Writes up to length lowerCase equivalents of the chars in input to output
 * @param input The input buffer
 * @param output The output buffer
 * @param length The number of chars to write
 */
static void lowerCase(const char *const restrict input, char *const restrict output, int length){

	for (int i = 0; i < length; ++i){
		output[i] = (char) toLow((unsigned char) input[i]);
	}
}

/**
 * @brief Greps a file for a keyword and outputs the matching lines to stdout.
 * @param input An input file opened for reading
 * @param keyword The keyword to search for
 * @param output An output file opened for writing to write the matching lines to
 * @param case_insensitive Whether the search should occur without regard to case sensitivity
 * @return EXIT_SUCCESS on success
 * @return EXIT_FAILURE on failure. An error message has been written to stderr
 */
int mygrep(FILE *input, const char *const key, FILE *output, bool case_insensitive){

	int ret = EXIT_SUCCESS;
	
	char *key_buff;
	char *haystack = NULL;
	int length;
	char *haystack_buff = NULL;
	
	if (case_insensitive == true){

		length = strlen(key);
		
		key_buff = malloc(length * sizeof(char));
		if (key_buff == NULL){

			error("malloc failed");
			ret = EXIT_FAILURE;
			goto cleanup;
		}
		lowerCase(key, key_buff, length);
	}
	else{

		key_buff = (char *const) key;
	}

	length = 0;
	while ((getline(&haystack, &length, input)) != -1){

		if (case_insensitive){

			if (!(haystack_buff = realloc(haystack_buff, length * sizeof(char)))){

				error("realloc failed: %s", strerror(errno));
				ret = EXIT_FAILURE;
				goto cleanup;
			}
			lowerCase(haystack, haystack_buff, length);
		}
		else{

			haystack_buff = haystack;
		}
		
		if (strstr(haystack_buff, key_buff)){

			fprintf(output, "%s", haystack);
		}
	}

cleanup:
	if (case_insensitive){

		free(key_buff);
		free(haystack_buff);
	
	}

	free(haystack);

	return ret;
}

/**
 * @brief main
 * The main program. It first reads all options via getopt and complains about missing ones.
 * It then scans the remaining arguments for input files to search in if none are passed, stdin is used.
 * @param argc The argument count
 * @param argv The arguments
 * @return EXIT_SUCCESS on success
 * @return EXIT_FAILURE on failure with an error message written to stderr
 */

int main(int argc, char **argv){

	program_name = argv[0];

	bool case_insensitive = false;
	FILE *output_file = stdout;
	
	int c;
	while ((c = getopt(argc, argv, "i::o:")) != -1) {//if it has something to process
		if(c == '?'){
			return EXIT_FAILURE;
		}
		else if(c == 'o'){

			FILE *f = fopen(optarg, "w");
			if (f != NULL){
			
				output_file = f;
			}
			else{

				error("Cannot open the file");
				return EXIT_FAILURE;
			}
		}
		else if(c == 'i'){
			case_insensitive = true;
		}
	}
	
	int r = EXIT_SUCCESS;
	int difference;

	char *key;
	if (optind < argc && strlen(argv[optind]) > 0)
	{
		key = argv[optind++];
	}
	else
	{
		error("Keyword required!");

		fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", argv[0]);
		r = EXIT_FAILURE;
		goto cleanup;
	}

	difference = argc - optind;

	if (difference == 0)
	{
		r = mygrep(stdin, key, output_file, case_insensitive);
	}
	else if (difference > 0)
	{
		for (int position = optind; position < argc; ++position)
		{
			FILE *f = fopen(argv[position], "r");
			if (f)
			{
				r = mygrep(f, key, output_file, case_insensitive);
				fclose(f);
			}
			else
			{
				error("Could not read input file %s", argv[position]);
				r = EXIT_FAILURE;
			}
		}
	}
	else
	{
		error("optind cannot bigger than argc. Aborting");
		abort();
	}

cleanup:
	if (output_file != stdout)
	{
		fclose(output_file);
	}

	return r;
}