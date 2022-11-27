/**
 * @file main.c
 * @author Markus Paoli 01417212
 * @date 2021/11/10
 *
 * @brief The main program module.
 *
 * @details This module handles the arguments passed to main, the overall
 *          program flow and errors.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "util.h"

static char *prog_name; // the program name

/**
 * The print_usage function
 * @brief Prints the usage message.
 *
 * @details This function prints the program name followed by the valid command line
 *          options and positional arguments to stderror. Afterwards it exits with
 *          EXIT_FAILURE.
 * global variable prog_name The program name.
 */
static void print_usage(void) {
	fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", prog_name);
	exit(EXIT_FAILURE);
}

/**
 * The print_error function
 * @brief Prints error messages.
 *
 * @details This function prints custom error messages. 
 *          Afterwards it exits with EXIT_FAILURE.
 * global variable prog_name The program name.
 * @param err_msg The custom error message.
 */
static void print_error(char *err_msg) {
	fprintf(stderr, "[%s] ERROR: %s; %s.\n", prog_name, err_msg, strerror(errno));
	exit(EXIT_FAILURE);
}

/**
 * The close_streams function
 * @brief Closes the in and out streams
 *
 * @details This function closes the streams in and out if they are not NULL.
 * @param in The in file stream
 * @param out The out file stream
 */
static void close_streams(FILE *in, FILE *out) {
    if (in != NULL) {
        fclose(in);
    }
    if (out != NULL) {
        fclose(out);
    }
}

/**
 * The process_text function
 *@brief This function reads from in processes what was read and writes
 *       it to out.
 *
 * @details This function reads text from a stream, replaces every occurrence of
 *          '\t' by a number of spaces so that the next character is at position
 *          p = tabstop * ((x / tabstop) + 1) and writes it to an other stream.
 * @param in The pointer to the input stream.
 * @param out The pointer to the output stream.
 * @param tabstop The column width.
 */
static void process_text(FILE *in, FILE *out, long tabstop) {
	char *tmp;
	while (feof(in) == 0) {
		if ((tmp = read_line(in)) == NULL) {
			close_streams(in, out);
			print_error("allocating memory failed");
		}
		if (ferror(in) != 0) {
			close_streams(in, out);
			print_error("reading of file failed");
		}
		if ((tmp = replace_tabs(tmp, tabstop)) == NULL) {
			close_streams(in, out);
			print_error("replacing tabs failed");
		}
		if (fputs(tmp, out) == EOF) {
			close_streams(in, out);
			print_error("writing to file failed");
		}
        free(tmp);
    }
	fclose(in);
}

/**
 * The main function
 * @brief Handles command line arguments and program flow.
 *
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
	prog_name = argv[0];
	int opt_t = 0;
	int opt_o = 0;
	char *t_arg = NULL;
	char *o_arg = NULL;
	int c;
	
	while ((c = getopt(argc, argv, "t:o:")) != -1) {
		switch (c) {
			case 't':
				++opt_t;
				t_arg = optarg;
				break;
			case 'o':
				++opt_o;
				o_arg = optarg;
				break;
			case '?':
				print_usage();
				break;
			default:
				assert(0);
		}
	}
	// options -t or -o occurred too often -> exit
	if ((opt_t > 1) || (opt_o > 1)) {
		print_usage();
	}
	// set tabstop to user input or exit with error if no integer or out of bounds
	long tabstop;
	if (t_arg == NULL) {
		// default tabstop
        tabstop = 8;
	} else {
		// set tabstop to user input or exit with error if no positive integer was provided
        tabstop = strtol(t_arg, NULL, 0);
		if (errno == ERANGE || tabstop < 1) {
			print_error("tabstop must be a positive integer");
		}
	}

	FILE *in = NULL;
    FILE *out = NULL;
	if (o_arg == NULL) {
        // no outfile was provided, output goes to stdout
		out = stdout;
	} else {
		// set outfile
        if ((out = fopen(o_arg, "a")) == NULL) {
			close_streams(in, out);
            print_error("fopen of oufile failed");
        }
	}
    // no input files were provided, program reads from stdin
	if ((argc - optind) == 0) {
		in = stdin;
		process_text(in, out, tabstop);
	} else {
		// while we have files try to process them one after the other
		while (argv[optind] != NULL) {
            if ((in = fopen(argv[optind], "r")) == NULL) {
				close_streams(in, out);
                print_error("fopen of infile failed");
            }
			process_text(in, out, tabstop);
            ++optind;
		}
	}
	fclose(out);
    return EXIT_SUCCESS;
}
