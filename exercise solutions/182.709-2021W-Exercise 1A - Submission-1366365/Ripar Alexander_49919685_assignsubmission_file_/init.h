/**
 * @file init.h
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 9.11.2021
 * 
 * @brief Header for logic responsible for parsing mygrep's command line and reporting errors.
 * 
 * @details Contains the declarations of the functions init_proc_context, 
 * destroy_proc_context, print_usage_and_exit, and print_error_and_exit, 
 * as well as the definition for the proc_context struct, which is used to store
 * the process's command line in a more convenient form.
**/

#ifndef INIT_INCLUDE_GUARD
#define INIT_INCLUDE_GUARD

#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Stores the process's command line in a convenient format for other function
 * to access.
**/
typedef struct proc_context_
{
	const char* program_name;		/**Holds the argv[0].**/
	char* keyword;					/**Contains the search keyword.**/
	FILE* output_file				/**The file all output should be written to (stdout if no file was specified using the -o option).**/;
	const char** input_filenames;	/**The names of the files to be searched.**/
	int input_file_cnt;				/**The number of elements in input_filenames.**/
	int keyword_len;				/**The length of keyword in bytes**/
	bool is_case_sensitive;			/**indicates whether -i was passed on the command line.**/
} proc_context;

/**
 * @brief Initializes a proc_context.
 * 
 * @details Initializes the proc_context pointed to by out_ctx using the command line passed in argc and argv.
 * Since this function uses getopt, the contents of argv will be permuted after the init_prc_context returns.
 * As such, init_proc_context should only be called once to parse given command line into a proc_context.
 * This function may call exit(EXIT_FAILURE) if an error is encountered. Globals used: optarg, optind.
 * 
 * @param out_ctx Pointer to the proc_context struct to be initialized.
 * 
 * @param argc The argument counter passed to main.
 * 
 * @param argv The argument vector passed to main.
 * 
 * @return true if out_ctx is fully initialized, false otherwise.
**/
bool init_proc_context(proc_context* out_ctx, int argc, char** argv);

/**
 * @brief Frees the resources held by a proc_context
 *
 * @details After destroy_proc_context returns, all resources allocated by a previous call to init_proc_context
 * are freed.
 *
 * @param ctx Pointer to the proc_context struct to be freed.
**/
void destroy_proc_context(proc_context* ctx);

#endif // INIT_INCLUDE_GUARD
