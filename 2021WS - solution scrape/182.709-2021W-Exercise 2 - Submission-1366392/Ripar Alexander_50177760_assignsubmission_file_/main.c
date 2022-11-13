/**
 * @file main.c
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 11.12.2021
 *
 * @brief Contains the entire code for the forkFFT program.
 *
 * @details Contains the definition of the program's entry point main,
 * as well as structs representing complex numbers and the program's 
 * context. Also contains functions for error messaging, simple arithmetic
 *  with complex numbers, and manipulating the program context.
**/

#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdarg.h>

/** The mathematical constant pi. **/
#define PI 3.141592654F

/** Global variable to make argv[0] more easily accessible outside main. **/
static const char* g_proc_name = "{Unknown}";

/**
 * @brief Writes the given format and varargs to stderr, alongside addition 
 * error information.
 * 
 * @details Prints the program's invocation name and pid, followed by 
 * format and any additional arguments, which are forwarded to vfprintf.
 * If errno is not zero, the current error value and the result of strerror
 * are also printed.
 * 
 * @param format The format to be used for printing. Forwarded to vfprintf.
**/
static void errmsg(const char* format, ...)
{
	fprintf(stderr, "[%s@%d]: ", g_proc_name, getpid());

	if(errno != 0)
		fprintf(stderr, "%s (0x%X): ", strerror(errno), errno);

	va_list vargs;

	va_start(vargs, format);

	vfprintf(stderr, format, vargs);

	va_end(vargs);
}

/**
 * @brief Represents a complex number.
 *
 * @details re holds the complex number's real part, while im holds the
 * imaginary part.
**/
typedef struct complex_
{
	/** Real part. **/
	float re;
	
	/** Imaginary part. **/
	float im;
} complex;

/**
 * @brief Multiplies two complex numbers.
 * 
 * @details The returned complex number contains the result of multiplying
 * a and b with each other. More formally, the result is 
 * { a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re }.
 * 
 * @param a First multiplicand.
 * 
 * @param b Second multiplicand.
 * 
 * @return Value of a * b.
**/
static complex complex_mul(complex a, complex b)
{
	complex rst;
	rst.re = a.re * b.re - a.im * b.im;
	rst.im = a.re * b.im + a.im * b.re;

	return rst;
}

/**
 * @brief Adds two complex numbers.
 * 
 * @details The returned complex number contains the result of adding
 * up a and b. More formally, the result is { a.re + b.re, a.im + b.im }.
 * 
 * @param a First summand.
 * 
 * @param b Second summand.
 * 
 * @return Value of a + b.
**/
static complex complex_add(complex a, complex b)
{
	complex rst;
	rst.re = a.re + b.re;
	rst.im = a.im + b.im;

	return rst;
}

/**
 * @brief Adds two complex numbers.
 * 
 * @details The returned complex number contains the result of subtracting
 * b from a. More formally, the result is { a.re - b.re, a.im - b.im }.
 * 
 * @param a First summand.
 * 
 * @param b Second summand.
 * 
 * @return Value of a + b.
**/
static complex complex_sub(complex a, complex b)
{
	complex rst;
	rst.re = a.re - b.re;
	rst.im = a.im - b.im;

	return rst;
}

/**
 * @brief Closes a file descriptor if it isn't -1 and sets it to -1.
 * 
 * @details if *fd is not equal to -1 it is closed. If close fails, -1
 * is returned. Otherwise *fd is set to -1 to avoid double closing of
 * file descriptors. On failure, *fd is left unchanged.
 * 
 * @param fd Pointer to file descriptor to be closed and invalidated.
 * 
 * @return -1 on failure, 0 on success.
**/
static int safeclose(int* fd)
{
	if (*fd != -1)
		if (close(*fd) == -1)
			return -1; // Don't overwrite the fd in case of an error to allow it to be inspected for debugging.

	*fd = -1;

	return 0;
}

/**
 * @brief Holds all necessary data related to a child process.
 *
 * @details Contains a child process's pid, as well as the pipes to and from
 * that process and a FILE* on top of pipe_from[0], which is used by 
 * read_child_results.
**/
typedef struct child_ctx_
{
	/** Process ID of the child. **/
	pid_t pid;

	/** Pipe which is read by the child process and written to by the parent. **/
	int pipe_to[2];

	/** Pipe which is written to by the child process and read from by the parent. **/
	int pipe_from[2];

	/** File stream on top of pipe_from[0], which is used in read_child_results to make getline usable. **/
	FILE* stream_from;

} child_ctx;

/**
 * @brief Holds all the data required by the process.
 *
 * @details Contains data on child processes and related pipes, as well as pointers
 * to all allocated heap memory which is used across functions. This simplifies 
 * cleanup and argument passing.
**/
typedef struct context_
{
	/** Data on child processes **/
	child_ctx children[2];

	/** Results of child processes, represented as complex numbers.
	 * The first half of the array contains the results of children[0], i.e.
	 * the even input indices, while the second holds those of children[1],
	 * i.e. the odd input indices.
	 * **/
	complex* child_results;

	/** Contains the first input line, which is read before forking to test if more than one input is present. **/
	char* first_line;

	/** Contains the second input line, which is read before forking to test if more than one input is present. **/
	char* second_line;

	/** Contains the total number of input lines. **/
	int lines_read;
} context;

/**
 * @brief Sets all members of a context struct to invalid values.
 * 
 * @details Sets all file descriptors in *ctx to -1, while setting all other
 * members (i.e. pointers) to 0. This avoids accidentally freeing or closing
 * handles more than once.
 * 
 * @param ctx Pointer to context to be invalidated.
**/
static void invalidate_context(context* ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	
	for(int i = 0; i != 2; ++i)
		for(int j = 0; j != 4; ++j) // The layout of child_ctx allows the two pipes to be treated as an array of four fds.
			ctx->children[i].pipe_to[j] = -1;
}

/**
 * @brief Closes all the pipes held by a context and sets their file 
 * descriptors to -1.
 * 
 * @details Tries closing all pipes in *ctx. File descriptors with value -1
 * are ignored. In case of close generating an error, the function proceeds
 * trying to close all remaining file descriptors. All descriptors that are
 * successfully closed are set to -1, to avoid double closing.
 * 
 * @param ctx Pointer to context which should have its pipes closed.
 * 
 * @return -1 on failure, 0 on success.
**/
static int close_context_pipes(context* ctx)
{
	int retval = 0;

	for(int i = 0; i != 2; ++i)
		for(int j = 0; j != 4; ++j) // The layout of child_ctx allows the two pipes to be treated as an array of four fds.
			if(ctx->children[i].pipe_to[j] != -1)
			{
				if(close(ctx->children[i].pipe_to[j]) == -1)
				{
					errmsg("close failed.\n");

					retval = -1;
				}
				else
				{
					// Only invalidate fds if they were closed successfully.
					ctx->children[i].pipe_to[j] = -1;
				}
			}
	return retval;
}

/**
 * @brief Frees all resources held by a context.
 * 
 * @details Tries freeing all file descriptors and pointers held by *ctx.
 * NULL pointers and file descriptors with a value of -1 are ignored. In
 * case of an error, the function generates an error message and proceeds
 * with cleanup. As this function is only intended to be called once the
 * process has finished all its work, returning an error status is not
 * necessary. At the end of the function invalidate_context is called to
 * avoid any tampering with the handles held by *ctx.
 * 
 * @param ctx Pointer to context to have its resources freed.
 * **/
static void free_context(context* ctx)
{
	for(int i = 0; i != 2; ++i)
		if(ctx->children[i].stream_from != NULL)
			if(fclose(ctx->children[i].stream_from) != 0)
				errmsg("fclose failed.\n");

	// Since we're in the middle of cleanup, we can ignore the result. 
	// Error messages are already handled by the callee.
	close_context_pipes(ctx); 

	if(ctx->child_results != NULL)
		free(ctx->child_results);

	if(ctx->first_line != NULL)
		free(ctx->first_line);

	if(ctx->second_line != NULL)
		free(ctx->second_line);

	// Invalidate everything just to really make sure.
	invalidate_context(ctx);
}

/**
 * @brief Initializes a context.
 * 
 * @details Creates all necessary pipes and invalidates all other members of
 * *ctx.
 * 
 * @param ctx Pointer to context to be initialized.
 * 
 * @return -1 on failure, 0 on success.
**/
static int init_context(context* ctx)
{
	invalidate_context(ctx);

	for(int i = 0; i != 2; ++i)
		for(int j = 0; j != 4; j += 2) // The layout of child_ctx allows for the two pipes to be trated as an array of two int[2].
			if(pipe(ctx->children[i].pipe_to + j) == -1)
			{
				errmsg("Could not create pipe.\n");

				goto FAILURE;
			}

	return 0;

 FAILURE:

	free_context(ctx);

	return -1;
}

/**
 * @brief Checks if stdin only contains a single line. This is necessary to
 * avoid infinite forking.
 * 
 * @details Tries reading the first and second lines of stdin. If there is no
 * first line, or getline fails due to an error condition other than eof, the
 * the has not received any input. If reading the first line succeeds, but 
 * reading the second line fails, the current process is a leaf. It should thus
 * write its input back as its result, and terminate without forking.
 * 
 * @param ctx Pointer to process context.
 * 
 * @return -1 on failure, 0 on success.
**/
static int check_for_oneliner(context* ctx)
{
	size_t getline_size;

	if(getline(&ctx->first_line, &getline_size, stdin) == -1)
	{
		errmsg("No input found.\n");

		return -1;
	}

	// We have to get another line, to actually get to the EOF. 
	// Just getting a character and putting it back with a seek
	// doesn't work as pipes apparently cannot be seek'd. 
	if(getline(&ctx->second_line, &getline_size, stdin) == -1)
	{
		if(feof(stdin) != 0)
		{
			int first_line_len = strlen(ctx->first_line);

			// Remove the trailing newline to allow formatting as a complex number.
			if(ctx->first_line[first_line_len - 1] == '\n')
				ctx->first_line[first_line_len - 1] = '\0';

			if(printf("%s %f*i\n", ctx->first_line, 0.0F) < 0)
			{
				errmsg("printf failed.\n");

				return -1;
			}

			return 1;
		}
		else
		{
			errmsg("getline failed.\n");

			return -1;
		}
	}

	return 0;
}

/**
 * @brief Creates a child process.
 * 
 * @details Forks the current process and writes all the relevant data about
 * the child to the parent's context. In the generated child process, execlp
 * is used to restart execution of the same program. stdin and stdout are
 * redirected to the relevant pipes in *ctx, and all irrelevant file descriptors
 * in *ctx are closed. 
 * 
 * @param ctx Pointer to program context.
 * 
 * @param index The child index which the generated child takes in *ctx. must be
 * either 0 or 1.
 * 
 * @return -1 on failure, 0 on success.
**/
static int create_child(context* ctx, int index)
{
	if(index != 0 && index != 1)
	{
		errmsg("Invalid child index.\n");

		return -1;
	}

	switch (ctx->children[index].pid = fork())
	{
	case -1:

		errmsg("fork failed.\n");
		
		return -1;
	
	case 0:

		if(dup2(ctx->children[index].pipe_to[0], STDIN_FILENO) == -1)
		{
			errmsg("dup2 failed.\n");

			return -1;
		}
		
		if(dup2(ctx->children[index].pipe_from[1], STDOUT_FILENO) == -1)
		{
			errmsg("dup2 failed.\n");

			return -1;
		}

		// We can close all the context's fds, as all relevant fds are 
		// duplicated into stdin and stdout anyways.
		if(close_context_pipes(ctx) == -1)
			return -1;
			
		execlp(g_proc_name, g_proc_name, NULL);

		errmsg("execlp failed.\n");

		return -1;

	default:
		break;
	}
	
	return 0;
}

/**
 * @brief Reads input from stdin and forwards directly forwards it to the child processes.
 * 
 * @details Reads lines from stdin until the end of file is reached or an error is encountered.
 * The read lines are directly forwarded to the relevant child process. ctx->first_line and
 * ctx->second_line are also forwarded. The number of lines read is stored in *ctx, and
 * an error is raised if the number of lines read was odd. Finally, the write ends of the
 * pipes used for communicating to the children are closed. 
 * 
 * @param ctx Pointer to program context.
 * 
 * @return -1 on failure, 0 on success.
**/
static int read_and_forward_stdin(context* ctx)
{
	// Write out the lines that were read n by check_for_oneliner.

	if(write(ctx->children[0].pipe_to[1], ctx->first_line, strlen(ctx->first_line)) == -1)
	{
		errmsg("write failed\n");

		return -1;
	}

	if(write(ctx->children[1].pipe_to[1], ctx->second_line, strlen(ctx->second_line)) == -1)
	{
		errmsg("write failed\n");

		return -1;
	}

	int lines_read;

	for(lines_read = 2;; ++lines_read)
	{
		char* line = NULL;

		ssize_t line_bytes;

		size_t getline_size;

		if((line_bytes = getline(&line, &getline_size, stdin)) == -1)
		{
			free(line);

			if(feof(stdin) == 0)
			{
				errmsg("getline failed.\n");

				return -1;
			}
			else
			{
				break;
			}
		}

		if(write(ctx->children[(lines_read & 1)].pipe_to[1], line, strlen(line)) == -1)
		{
			free(line);

			errmsg("write failed\n");

			return -1;
		}

		free(line);
	}

	// We can close the pipes to our child processes now, as there is no further information to be sent to them.
	if (safeclose(&ctx->children[0].pipe_to[1]) == -1 || safeclose(&ctx->children[1].pipe_to[1]) == -1)
	{
		errmsg("close failed.\n");

		return -1;
	}

	if ((lines_read & 1) != 0)
	{
		errmsg("UNEVEN number of lines: %d\n", lines_read);

		return -1;
	}

	ctx->lines_read = lines_read;

	return 0;
}

/**
 * @brief Reads the results of the generated child processes.
 * 
 * @details Reads lines from the pipes connected to child processes as soon as
 * the relevant processes have exited. The reading is done using getline, with
 * FILE*s generated using fdopen on file descriptors obtained by duplicating the
 * relevant pipe read ends. The read results are stored in ctx->child_results,
 * which is allocated by the function. This is necessary, as further processing
 * of results requires data from both halves of the resulting array.
 * 
 * @param ctx Pointer to program context.
 * 
 * @return -1 on failure, 0 on success.
**/
static int read_child_results(context* ctx)
{
	for(int i = 0; i != 2; ++i)
	{
		// Generate FILE*s on duplicated fds to simplify cleanup.
		int dup_from;

		if((dup_from = dup(ctx->children[i].pipe_from[0])) == -1)
		{
			errmsg("dup failed.\n");

			return -1;
		}

		if((ctx->children[i].stream_from = fdopen(dup_from, "r")) == NULL)
		{
			errmsg("fdopen failed.\n");

			return -1;
		}
	}

	if((ctx->child_results = malloc(ctx->lines_read * sizeof(complex))) == NULL)
	{
		errmsg("malloc failed.\n");

		return -1;
	}
	
	for(int i = 0; i != 2; ++i)
	{
		int child_status;

		pid_t pid = wait(&child_status);

		// Just in case the child was terminated abnormally, also check 
		// WIFEXITED and not just WEXITSTATUS.
		if(WEXITSTATUS(child_status) != EXIT_SUCCESS || WIFEXITED(child_status) == 0)
		{
			errmsg("Child failed.\n");

			return -1;
		}

		// Determine which child actually exited.

		FILE* curr_in = NULL;

		int rst_offset;

		for(int i = 0; i != 2; ++i)
			if(pid == ctx->children[i].pid)
			{
				curr_in = ctx->children[i].stream_from;

				rst_offset = i * (ctx->lines_read >> 1);

				break;
			}
		
		if(curr_in == NULL)
		{
			errmsg("Unknown child pid.\n");

			return -1;
		}

		size_t getline_size;

		for(int i = 0; i != ctx->lines_read >> 1; ++i)
		{
			char* line = NULL;

			if(getline(&line, &getline_size, curr_in) == -1)
			{
				free(line);

				errmsg("getline failed.\n");

				return -1;
			}

			char* endptr;

			ctx->child_results[rst_offset + i].re = strtof(line, &endptr);

			ctx->child_results[rst_offset + i].im = strtof(endptr + 1, NULL);

			free(line);
		}
	}

	return 0;
}

/**
 * @brief Processes results from children and writes the results to stdout.
 * 
 * @details The previously read child results are processed according to the
 * Cooley-Turkey FFT algorithm and written to stdout. The processing is done in
 * two steps, one for the first half of the results, and another for the second 
 * one. This leads to some redundancy in the computation of the involved 
 * factors, but allows all results to be written to stdout directly without
 * buffering in an array.
 * 
 * @param ctx Pointer to program context.
 * 
 * @return -1 on failure, 0 on success.
**/
static int write_results(context* ctx)
{
	for(int i = 0; i != ctx->lines_read >> 1; ++i)
	{
		complex factor;
		factor.re = cos((-2.0F * PI * i) / ctx->lines_read);
		factor.im = sin((-2.0F * PI * i) / ctx->lines_read);

		complex result = complex_add(ctx->child_results[i], complex_mul(factor, ctx->child_results[i + (ctx->lines_read >> 1)]));
		
		if(printf("%f %f*i\n", result.re, result.im) < 0)
		{
			errmsg("fprintf failed.\n");

			return -1;
		}

	}
	
	for(int i = 0; i != ctx->lines_read >> 1; ++i)
	{
		complex factor;
		factor.re = cos((-2.0F * PI * i) / ctx->lines_read);
		factor.im = sin((-2.0F * PI * i) / ctx->lines_read);

		complex result = complex_sub(ctx->child_results[i], complex_mul(factor, ctx->child_results[i + (ctx->lines_read >> 1)]));
		
		if(printf("%f %f*i\n", result.re, result.im) < 0)
		{
			errmsg("fprintf failed.\n");

			return -1;
		}
	}
	
	return 0;
}

/**
 * @brief Entry point of the forkFFT program.
 * 
 * @details Checks command line arguments, creates context, and calls all 
 * functions necessary for computing the resulting output. Cleaup is performed
 * by calling free_context in both the FAILURE and SUCCESS labels.
 * 
 * @param argc Argument counter.
 * 
 * @param argv Argument vector.
 * 
 * @return EXIT_SUCCESS on successful completion, EXIT_FAILURE otherwise.
**/
int main(int argc, char** argv)
{
	g_proc_name = argv[0];

	context ctx;

	invalidate_context(&ctx);

	if(argc > 1)
	{
		fprintf(stderr, "[%s]: Unexpected number of arguments. Usage: %s\n", g_proc_name, g_proc_name);

		goto FAILURE;
	}

	if(init_context(&ctx) == -1)
		goto FAILURE;

	int rst = check_for_oneliner(&ctx);

	if(rst == -1)
		goto FAILURE;
	else if(rst == 1)
		goto SUCCESS;

	for(int i = 0; i != 2; ++i)
		if(create_child(&ctx, i) == -1)
			goto FAILURE;

	for(int i = 0; i != 2; ++i)
		if(safeclose(&ctx.children[i].pipe_to[0]) == -1 || safeclose(&ctx.children[i].pipe_from[1]) == -1)
		{
			errmsg("close failed.\n");

			goto FAILURE;
		}

	if(read_and_forward_stdin(&ctx) == -1)
		goto FAILURE;

	if(read_child_results(&ctx) == -1)
		goto FAILURE;

	if(write_results(&ctx) == -1)
		goto FAILURE;

 SUCCESS:

	free_context(&ctx);

	return EXIT_SUCCESS;

 FAILURE:

	free_context(&ctx);

	return EXIT_FAILURE;
}
