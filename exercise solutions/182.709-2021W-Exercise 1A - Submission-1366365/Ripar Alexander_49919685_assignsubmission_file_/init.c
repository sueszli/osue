/**
 * @file init.c
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 9.11.2021
 *
 * @brief Contains logic responsible for parsing mygrep's command line
 *
 * @details Contains the definitions of the functions init_proc_context and
 * destroy_proc_context, as well as the definition for the proc_context struct, 
 * which is used to store the process's command line in a more convenient form.
 * Also defines the static functions parse_options, parse_keywords and 
 * parse_input_files, which are helpers for init_proc_context.
**/

#include "init.h"

#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

/**
 * @brief Called by init_proc_context to parse the command line's options.
 * 
 * @details Calls getopt to parse the process's command line options.
 * Valid options are -o and -i. Globals used: optarg
 * 
 * @param out_ctx The proc context that receives the parsed options.
 * Mutated members: output_file, is_case_sensitive
 *
 * @param argc The argument counter passed to main.
 * 
 * @param argv The argument vector passed to main.
 *
 * @return true if the options have benn successfully stored in out_ctx, false otherwise.
**/
static bool parse_options(proc_context* out_ctx, int argc, char** argv)
{
	// If there is no -o option, default to stdout.
	// Otherwise, this gets overwritten in the getopt-loop
	out_ctx->output_file = stdout;
	
	char opt_char;

	while ((opt_char = getopt(argc, argv, "o:i")) != -1)
		switch (opt_char)
		{
		case 'i':

			out_ctx->is_case_sensitive = false;

			break;

		case 'o':

			if (optarg == NULL)
			{
				fprintf(stderr, "%s: The Option '-o' must be followed by a filename\n", out_ctx->program_name);

				return false;
			}
			else
			{
				out_ctx->output_file = fopen(optarg, "w");

				if (out_ctx->output_file == NULL)
				{
					fprintf(stderr, "%s: Could not open the file '%s' for writing.\n", out_ctx->program_name, optarg);

					return false;
				}
			}

			break;

		case '?':

			return false; // In case an invalid option was encountered, getopt already handled the error message, so we can simply return false.

		default:

			assert(0);
		}

	return true;
}

/**
 * @brief Stores the keyword passed on the command line into out_ctx.
 *
 * @details This function is called from init_proc_context after parse_options.
 * This is important, as it means that argv[optind] will be the first non-option parameter,
 * i.e. keyword. Instead of simply storing the relevant element of argv, the keyword is duplicated
 * in order to set it to lower case if is_case_sensitive is false. Globals used: optind.
 *
 * @param out_ctx The proc_context that receives the parsed keyword.
 * Mutated members: keyword, keyword_len
 *
 * @param argc The argument counter passed to main.
 * 
 * @param argv The argument vector passed to main.
 *
 * @return true if the keyword was successfully stored in out_ctx, false otherwise.
**/
static bool parse_keyword(proc_context* out_ctx, int argc, char** argv)
{
	// If optind is already at least as big as argc, then all elements of argv must be option.
	// Hence keyword is not present, and we print an according error message.
	// optind should never be strictly greater than argc, but just to be safe...)
	if (optind >= argc)
	{
		fprintf(stderr, "%s: Missing keyword.\n", out_ctx->program_name);

		return false;
	}

	size_t keyword_len = strlen(argv[optind]);

	out_ctx->keyword_len = keyword_len;

	// A new heap buffer is allocated instead of just using argv[optind], in order to handle
	// is_case_sensitive a bit more graciously.

	out_ctx->keyword = malloc(keyword_len);

	assert(out_ctx->keyword != NULL);

	if (out_ctx->is_case_sensitive)
		memcpy(out_ctx->keyword, argv[optind], keyword_len);
	else
		for (int i = 0; i != keyword_len; ++i)
			out_ctx->keyword[i] = tolower(argv[optind][i]);

	// Increment optind so it points to the first argument after keyword, for use by parse_input_files.
	++optind;

	return true;
}

/**
 * @brief Parses the positional input_file parameters from the process's command line
 *
 * @details Called by init_prc_context after parse_keyword. This is important, 
 * as it means that optind will be pointing to the first element of argv after keyword,
 * i.e. the first input file. For all remaining elements of argv beginning at optind, 
 * access is called to determine if they are existing files and can be opened for reading.
 * Globals used: optind.
 *
 * @param out_ctx The proc_context that receives the input filenames.
 * Mutated members: input_filenames, input_file_cnt
 *
 * @param argc The argument counter passed to main.
 * 
 * @param argv The argument vector passed to main.
 *
 * @return true if the input filenames have been successfully stored in out_ctx, false otherwise.
**/
static bool parse_input_files(proc_context* out_ctx, int argc, char** argv)
{
	int input_file_cnt = argc - optind;

	// If there are no input files, stdin is used instead. Hence, this is not treated as an error.
	// out_ctx->input_file_cnt is already set to zero at the beginning of init_proc_context.
	if (input_file_cnt <= 0)
	{
		return true;
	}

	for (int i = optind; i != argc; ++i)
	{
		// Check if we have an existing filename
		if (access(argv[i], F_OK) != 0)
		{
			fprintf(stderr, "%s: Could not find the file '%s'.\n", out_ctx->program_name, argv[i]);

			return false;
		}

		// Check if we can read the file.
		if (access(argv[i], R_OK) != 0)
		{
			fprintf(stderr, "%s: Could not open the file '%s' for reading.\n", out_ctx->program_name, argv[i]);

			return false;
		}
	}

	// Since this is the last function that actually touches argv, we can simply reuse it instead of allocating a new buffer.
	out_ctx->input_filenames = (const char**) (argv + optind);

	out_ctx->input_file_cnt = input_file_cnt;
	
	return true;
}

bool init_proc_context(proc_context* out_ctx, int argc, char** argv)
{
	out_ctx->program_name = argv[0];
	out_ctx->is_case_sensitive = true;
	out_ctx->input_file_cnt = 0;
	out_ctx->input_filenames = NULL;
	out_ctx->keyword_len = 0;
	out_ctx->keyword = NULL;
	out_ctx->output_file = NULL;

	if (!parse_options(out_ctx, argc, argv))
		return false;

	if (!parse_keyword(out_ctx, argc, argv))
		return false;

	return parse_input_files(out_ctx, argc, argv);
}

void destroy_proc_context(proc_context* ctx)
{
	if (ctx->output_file)
		fclose(ctx->output_file);

	if (ctx->keyword)
		free(ctx->keyword);
}
