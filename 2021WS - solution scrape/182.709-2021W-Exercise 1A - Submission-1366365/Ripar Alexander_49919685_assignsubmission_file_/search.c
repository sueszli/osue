/**
 * @file search.c
 * @author Alexander Ripar <e12022494@student.tuwien.ac.at>
 * @date 9.11.2021
 *
 * @brief Contains logic responsible for searching files for the given keyword.
 *
 * @details Contains the definitions of the function run_search, as well as the 
 * static functions check_file, check_line, init_sized_buffer, destroy_sized_buffer
 * and grow_sized_buffer. Also defines the sized_buffer struct.
**/

#include "search.h"

#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

// Minimum initial size for the sized_buffer used in run_search.
#define INITIAL_BUFFER_SZ 4096


/**
 * @brief A pointer to a heap buffer, paired with the buffer's current size.
**/
typedef struct sized_buffer_
{
	char* buf;		/**Points to the heap buffer managed by sized_buffer.**/
	size_t sz;		/**The size of the buffer pointed to by buf.**/
} sized_buffer;

/**
 * @brief Doubles the size of the passed sized buffer.
 *
 * @details buf is grown to twice its size using realloc. If realloc fails,
 * buf remains unchanged. Otherwise buf->buf and buf->sz are updated accordingly.
 *
 * @param buf Pointer to sized_buffer to be resized.
 *
 * @return true if buf could be resized, false otherwise.
**/
static bool grow_sized_buffer(sized_buffer* buf)
{
	char* tmp = realloc(buf->buf, buf->sz << 1);

	if (tmp == NULL)
		return false;

	buf->buf = tmp;
	buf->sz <<= 1;

	return true;
}

/**
 * @brief Initializes a sized_buffer.
 *
 * @details After init_sized_buffer returns, buf->buf will point to a heap buffer
 * allocated with malloc and buf->sz will hold the buffer's size. 
 * If the allocation fails, an assertion is triggered.
 * 
 * @param buf Pointer to sized_buffer to be initialized.
 * 
 * @param initial_sz The initial size of buf.
**/
static void init_sized_buffer(sized_buffer* buf, size_t initial_sz)
{
	buf->buf = malloc(initial_sz);
	
	assert(buf->buf != NULL);

	buf->sz = initial_sz;
}

/**
 * @brief Frees the resources held by a sized_buffer.
 *
 * @details Frees buf->buf.
 *
 * @param buf Pointer to sized_buffer to be freed.
**/
void destroy_sized_buffer(sized_buffer* buf)
{
	if (buf->buf != NULL)
		free(buf->buf);
}

/**
 * @brief Searches buffer->buf for ctx->keyword.
 *
 * @details Checks if buffer->buf contains ctx->keyword. 
 * If ctx->is_case_sensitive is true, this is resolved by a call to strstr.
 * Otherwise, buffer->buf is first copied to a temporary buffer, where it is
 * changed to lowercase. This is necessary, since buffer->buf should not be
 * mutated, as it is also used as the source for outputting lines containing
 * the keyword.
 *
 * @param ctx The proc_context holding the search configuration.
 *
 * @param buffer A sized_buffer containing the search line to be searched.
 * 
 * @return true if the keyword is was found, false otherwise.
**/
static bool contains_keyword(const proc_context* ctx, const sized_buffer* buffer)
{
	if (ctx->is_case_sensitive)
	{
		return strstr(buffer->buf, ctx->keyword);
	}
	else
	{
		// Calling malloc for every line is a bit ugly, 
		// but that shouldn't matter too much for this example project.
		// And it makes the code simpler.
		char* lowercase_buf = malloc(buffer->sz);

		assert(lowercase_buf != NULL);

		size_t i = 0;

		for (; buffer->buf[i] != '\0'; ++i)
			lowercase_buf[i] = tolower(buffer->buf[i]);

		lowercase_buf[i] = '\0';

		bool found = strstr(lowercase_buf, ctx->keyword);

		free(lowercase_buf);

		return found;
	}
}

/**
 * @brief Searches the current line of file for ctx->keyword.
 *
 * @details Reads a line from file into buffer. If buffer is too small, 
 * grow_sized_buffer is called, the file is rewound and reread until the current
 * line fits into the buffer. Afterwards, contains_keyword is called,
 * and if a match was found buffer is written to ctx->output_file.
 *
 * @param ctx The proc_context holding the search configuration.
 *
 * @param file Handle to the file to be searched.
 *
 * @param buffer A sized_buffer containing the search line to be searched.
 *
 * @return true if the line was successfully checked, false if an error occurred
 * or the end of file was reached.
**/
static bool check_line(proc_context* ctx, FILE* file, sized_buffer* buffer)
{
	int read_chars = 0;

	do {
		if (fgets(buffer->buf + read_chars, buffer->sz - read_chars, file) == NULL)
		{
			// Handle the last line (which is not terminated by a \n).
			// First check if fgets returned NULL because the end of file has been reached though.
			// Also check if there have been characters already read, because otherwise this is useless.
			if (read_chars && feof(file) != 0 && contains_keyword(ctx, buffer))
			{
				if (fwrite(buffer->buf, 1, read_chars, ctx->output_file) != read_chars)
					return false;
			}

			return false;
		}

		read_chars += strlen(buffer->buf + read_chars);

		if (buffer->buf[read_chars - 1] != '\n')
			if (!grow_sized_buffer(buffer))
				return false;
	}
	while (buffer->buf[read_chars - 1] != '\n');

	if (contains_keyword(ctx, buffer))
		if (fwrite(buffer->buf, 1, read_chars, ctx->output_file) != read_chars)
			return false;

	return true;
}

/**
 * @brief Searches all lines of file for ctx->keyword.
 *
 * @details Repeatedly calls check_line until it returns false. This way, all
 * lines in file are checked for ctx->keyword. The return value is determined
 * by checking if file was read to its end using a call to feof.
 *
 * @param ctx The proc_context holding the search configuration.
 *
 * @param file Handle to the file to be searched.
 *
 * @param buffer A sized_buffer containing the search line to be searched.
 *
 * @return true if the file was completely read, false otherwise.
**/
static bool check_file(proc_context* ctx, FILE* file, sized_buffer* buffer)
{
	while (check_line(ctx, file, buffer));
	
	return feof(file) != 0; // Check whether file has been completely processed.
}

bool run_search(proc_context* ctx)
{
	sized_buffer buffer;

	init_sized_buffer(&buffer, INITIAL_BUFFER_SZ > ctx->keyword_len * 2 ? INITIAL_BUFFER_SZ : ctx->keyword_len * 2);

	// If there are no input files we default to stdin.
	if (ctx->input_file_cnt == 0)
	{
		if (!check_file(ctx, stdin, &buffer))
			goto FAILURE;
	}

	for (int i = 0; i != ctx->input_file_cnt; ++i)
	{
		FILE* in = fopen(ctx->input_filenames[i], "r");

		if (in == NULL)
		{
			fprintf(stderr, "%s: Could not open the file '%s'.\n", ctx->program_name, ctx->input_filenames[i]);

			goto FAILURE;
		}

		if (!check_file(ctx, in, &buffer))
		{
			fprintf(stderr, "%s: An error occurred while working on the file '%s'\n.", ctx->program_name, ctx->input_filenames[i]);

			fclose(in);

			goto FAILURE;
		}

		fclose(in);
	}

	destroy_sized_buffer(&buffer);

	return true;

FAILURE: // Makes cleanup cleaner.

	destroy_sized_buffer(&buffer);

	return false;
}
