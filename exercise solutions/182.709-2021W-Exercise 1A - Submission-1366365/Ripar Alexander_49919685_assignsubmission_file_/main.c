/**
 * @file main.c
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 9.11.2021
 * 
 * @brief Contains the entry point of the mygrep program.
 *
 * @details main.c contains the entry point of mygrep. From here, 
 * init_proc_context is called to parse the command line into a proc_context struct. 
 * If this succeeds, run_search is called to perform the actual work of 
 * searching through the given files.
**/

#include <stdbool.h>
#include <stdlib.h>

#include "init.h"
#include "search.h"

/**
 * @brief mygrep's entry point.
 * 
 * @details The initial function called from the CRT on process start. 
 * init_proc_context is called to parse the given command line.
 * print_usage_and_exit is called if init_proc_context fails.
 * run_search is called to search through the given files.
 * 
 * @param argc Contains the number of char* pointed to by argv.
 * 
 * @param argv Pointer to an array of char*, 
 * containing the command line arguments for the process.
 * 
 * @return EXIT_FAILURE on process failure and EXIT_SUCCESS on successful completion.
**/
int main(int argc, char** argv)
{
	proc_context ctx;

	if (!init_proc_context(&ctx, argc, argv))
	{
		fprintf(stderr, "%s: Usage: %s [-i] [-o outfile] keyword [file...]\n", ctx.program_name, ctx.program_name);

		goto FAILURE;
	}

	if (!run_search(&ctx))
		goto FAILURE;

	destroy_proc_context(&ctx);

	return EXIT_SUCCESS;

FAILURE: // This makes cleanup a bit cleaner.

	destroy_proc_context(&ctx);

	return EXIT_FAILURE;
}
