/**
 * @file mygrep.c
 * @author George Tokmaji <e11908523@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief myexpand
 * @details see myexpand
 **/

#include "error.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *program_name; /**< The program name. Needs to be defined by the application. **/

/**
 * @brief myexpand
 * Reads from an input file and writes the read contents to output with tab characters replaced by tab_stop spaces.
 * @param input The input file to read from.
 * @param output The output file to write to.
 * @param tab_stop The amount of spaces to replace a tab character with
 * @return EXIT_SUCCESS on success
 * @return EXIT_FAILURE on failure with an error message written to stderr
 */

__attribute__((nonnull(1, 2))) int myexpand(FILE *const input, FILE *const output, const long long tab_stop)
{
	char *line_buffer = NULL;
	size_t line_buffer_size = 0;
	char *space_buffer;
	if ((space_buffer = malloc(tab_stop + 1)) == NULL)
	{
		error("Allocating space buffer failed");
		return EXIT_FAILURE;
	}

	memset(space_buffer, ' ', tab_stop);
	space_buffer[tab_stop] = '\0';

	int ret = EXIT_SUCCESS;

	line_buffer_size = 0;
	ssize_t line_length;
	while ((line_length = getdelim(&line_buffer, &line_buffer_size, '\t', input)) != -1)
	{
		// don't bother with a write system call for an empty string if it's only a tab character

		if (line_buffer[line_length - 1] == '\t')
		{
			line_buffer[--line_length] = '\0';
			fwrite(line_buffer, sizeof(char), line_length, output);
			fwrite(space_buffer, sizeof(char), tab_stop, output);
		}
		else
		{
			fwrite(line_buffer, sizeof(char), line_length, output);
		}
	}

	if (errno != 0)
	{
		system_error("getline failed");
		ret = EXIT_FAILURE;
	}

	free(line_buffer);
	free(space_buffer);
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

int main(int argc, char **argv)
{
	program_name = argv[0];

	long long tab_stop = 8;
	FILE *output_file = stdout;

	int c;
	while ((c = getopt(argc, argv, "t:o:")) != -1)
	{
		switch (c)
		{
		case 't':
		{
			char *endptr;
			tab_stop = strtoll(optarg, &endptr, 10);
			if (errno != 0)
			{
				system_error("Failed to convert tab stop argument %s", optarg);
				return EXIT_FAILURE;
			}

			else if (endptr == optarg)
			{
				error("Invalid tab stop");
				return EXIT_FAILURE;
			}

			break;
		}

		case 'o':
		{
			FILE *f = fopen(optarg, "w");
			if (f)
			{
				output_file = f;
			}
			else
			{
				error("Cannot open output file %s: %s", optarg, strerror(errno));
				return EXIT_FAILURE;
			}

			break;
		}

		case '?':
		default:
			return EXIT_FAILURE;
		}
	}

	int ret = EXIT_SUCCESS;
	const int diff = argc - optind;

	if (diff > 0)
	{
		for (int pos = optind; pos < argc; ++pos)
		{
			FILE *f = fopen(argv[pos], "r");
			if (f)
			{
				ret = myexpand(f, output_file, tab_stop);
				fclose(f);
			}
			else
			{
				error("Failed to read input file %s", argv[pos]);
				ret = EXIT_FAILURE;
				goto cleanup;
			}
		}
	}
	else if (diff == 0)
	{
		ret = myexpand(stdin, output_file, tab_stop);
	}
	else
	{
		error("optind is bigger than argc. This should not happen. Aborting");
		abort();
	}

cleanup:
	if (output_file != stdout)
	{
		fclose(output_file);
	}

	return ret;
}
