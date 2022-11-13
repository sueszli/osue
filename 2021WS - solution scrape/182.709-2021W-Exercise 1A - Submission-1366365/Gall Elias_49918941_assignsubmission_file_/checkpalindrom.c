#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "checkpalindrom.h"
#include "ispalindrom.h"

/**
 * @file checkpalindrom.c
 * @author  Elias GALL - 12019857
 * @brief   checks files and stdin for palindroms
 * @details Reads lines from files or from stdin an check whether they are palindroms according to specified
 *          criteria. Writes the results either to stdout or to a file.
 * @date    2021-10-29
 */

static int is_line_palindrom(const char*, int, int);
static int check_line(const char*, int);
static int remove_whitespaces(char**);
static int string_to_lower(char*);
static int write_to_file(char*);
static void close_if_not_null(FILE*);
static int check_stream_for_palindroms(FILE*, int, int);
static int write_output(char*);
static int get_line_from_stream(char**, size_t*, FILE*);

// return codes
// 0 ... success
// 1 ... unspecified error
// 2 ... input is NULL and not expected
// 3 ... could not allocate memory
// 4 ... could not open file

/**
 *  stores the file context of the output file
 */
static FILE *output = NULL;
/**
 * stores the number of files already processed, used to decide whether the ouput file has to be opened in mode 'write' or 'append'
 */
static int files_processed = 0;

int check_file_for_palindroms(const char* filename, int ignore_whitespaces, int ignore_case, const char* output_filename) {
    if (filename == NULL) {
        external_error("%s: argument is NULL");
        return 2;
    }
    if (output_filename != NULL) {
        output = fopen(output_filename, (files_processed == 0) ? "w" : "a");
        if (output == NULL) {
            external_error("%s: could not allocate memory");
            return 0;
        }
    }
    files_processed ++;
    FILE *input = NULL;
    input = fopen(filename, "r");
    if (input == NULL) {
        close_if_not_null(output);
        external_error("%s: could not open file");
        return 4;
    }
    int code = check_stream_for_palindroms(input, ignore_whitespaces, ignore_case);
    close_if_not_null(input);
    close_if_not_null(output);
    return code;
}

/**
 * @brief   checks specified stream for palindroms using specified settings
 * @details Checks 'stream' line-wise for palindroms using 'ignore_whitespaces' and 'ignore_case' to
 *          determine what counts as a palindrom. Loops over all lines in the stream until EOF.
 * @param   stream              the stream to be checked for palindroms
 * @param   ignore_whitespace   logically interpreted int specifying whether to ignore whitespaces or not
 * @param   ignore_case         logically interpreted int specifying whether or not to ignore case
 * @return  0 ... success
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 * @return  4 ... could not open file
 */
static int check_stream_for_palindroms(FILE *stream, int ignore_whitespaces, int ignore_case) {
    size_t read_length = 0;
    char *read_line = NULL;
    int code = get_line_from_stream(&read_line, &read_length, stream);
	while (code != -1) {
        int line_checked = is_line_palindrom(read_line, ignore_whitespaces, ignore_case);
        char *line_end_str = NULL;
        if (line_checked == -1) {
            line_end_str = " is a palindrom\n";
        } else if (line_checked == 0) {
            line_end_str = " is not a palindrom\n";
        }
        write_output(read_line);
        write_output(line_end_str);
        code = get_line_from_stream(&read_line, &read_length, stream);
    }
    free(read_line);
    if (code != -1) return code;
    return 0;
}

/**
 * @brief   closes file if not NULL
 * @details Closes the specified file if it is not NULL.
 * @param   file    file to be closed
 * @return  void
 */
static void close_if_not_null(FILE *file) {
    if (file != NULL) {
        fclose(file);
    }
}
/**
 * @brief   writes to either stdout or the opened file
 * @details If a file is currently open ('output'), the string is written to it, otherwise it is written to stdout.
 * @param   text    string to be written
 * @return  0 ... success
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 * @return  4 ... could not open file
 */
static int write_output(char *text) {
    if (output == NULL) {
        printf(text);
        return 0;
    }
    return write_to_file(text);
}
/**
 * @brief   reads line from stream
 * @details Reads a line from the specified stream and points the char array referenced by 'line' to it. Uses 'getline' to
 *          read from the stream.
 * @param   line    reference to a char pointer which will be pointed to the line read
 * @param   length  reference to a size_t which will be set to the number of bytes read
 * @param   context stream from which to read
 * @return -1 ... EOF
 * @return  0 ... success
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 */
static int get_line_from_stream(char **line, size_t *length, FILE *context) {
    if (line == NULL) {
        return 2;
    }
	char *read_line = NULL;
	int read_length = getline(&read_line, length, context);
	if (read_length == -1) {
        free(read_line);
		return -1;
	}
    char *output_line = NULL;
    int output_line_len = 0;
    if (read_line[read_length - 1] == '\n') {
        output_line_len = read_length - 1;
    } else {
        output_line_len = read_length;
    }
    if (*line != NULL) {
        free(*line);
    }
    output_line = malloc((output_line_len + 1) * sizeof(char));
    if (output_line == NULL) {
        free(line);
        external_error("%s: could not allocate memory");
        return 3;
    }
    strncpy(output_line, read_line, output_line_len);
    free(read_line);
    output_line[output_line_len] = '\0';
	*line = output_line;
	return 0;
}

int check_std_in_for_palindroms(const char* output_filename, int ignore_whitespaces, int ignore_case) {
    if (output_filename != NULL) {
        output = fopen(output_filename, "w");
        if (output == NULL) {
            return 4;
            external_error("%s: could not open file");
        }
    }
    
    int code = check_stream_for_palindroms(stdin, ignore_whitespaces, ignore_case);
    close_if_not_null(output);
    return code;
}

/**
 * @brief   checks whether a line is a palindrom or not according to the settings specified
 * @details Copies 'line' and modifies the copy according to the settings specified. Then checks whether the resulting line
 *          is a palindrom or not and returns the verdict as a logical int.
 * @param   line                string to be checked
 * @param   ignore_whitespace   logically interpreted int specifying whether to ignore whitespaces or not
 * @param   ignore_case         logically interpreted int specifying whether or not to ignore case
 * @return -0 ... palindrom
 * @return  0 ... not a palindrom
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 * @return  4 ... could not open file
 */
static int is_line_palindrom(const char* line, int ignore_whitespace, int ignore_case) {
    if (line == NULL) {
        return 2;
    }
    int line_length = strlen(line);
    char *local_line = malloc(line_length + 1);
    if (local_line == NULL) {
        return 3;
        external_error("%s: could not allocate memory");
    }
    strcpy(local_line, line);

    if (ignore_case) {
        string_to_lower(local_line);
    }
    if (ignore_whitespace) {
        remove_whitespaces(&local_line);
        line_length = strlen(local_line);
    }
    int code = check_line(local_line, line_length);
    free(local_line);
    return code;
}
/**
 * @brief   removes whitespaces from a string
 * @details Takes a pointer to a char pointer (string) and removes all the whitespaces, sets the pointer at the given address
 *          to a new string without whitespaces.
 * @param   line    pointer to the pointer of the line to be changed
 * @return  0 ... success
 * @return  1 ... no spaces found, string unchanged
 * @return  2 ... input is NULL
 * @return  3 ... could not allocate memory
 */
static int remove_whitespaces(char** line) {
    if (*line == NULL) {
        external_error("%s: argument is NULL");
        return 2;
    }
    int len = strlen(*line);
    int i = 0;
    int whitespaces = 0;
    for (i = 0; i < len; i++) {
        if (isspace((*line)[i])) {
            whitespaces++;
        }
    }
    if (whitespaces == 0) {
        return 1;
    }
    char *out_line = malloc(len + 1 - whitespaces);
    if (out_line == NULL) {
        external_error("%s: could not allocate memory");
        return 3;
    }
    int char_counter = 0;
    for (i = 0; i < len; i++) {
        if (isspace((*line)[i])) {
            continue;
        }
        out_line[char_counter] = (*line)[i];
        char_counter++;
    }
    out_line[char_counter] = '\0';
    free(*line);
    *line = out_line;
    return 0;
}

/**
 * @brief   checks a if a string is a palindrom
 * @details Iterates over the string from the front and back and checks whether all characters match. If so, then the string is a palindrom.
 * @param   line    string to check
 * @param   len     length of the string to check
 * @return -1 ... palindrom
 * @return  0 ... not a palindrom
 * @return  2 ... argument is NULL
 */
static int check_line(const char* line, int len) {
    if (line == NULL) {
        external_error("%s: argument is NULL");
        return 2;
    }
    int no_cr_len = len;
    if (line[len - 1] == '\n') {
        no_cr_len--;
    }
    int steps = no_cr_len / 2 + no_cr_len % 2;
    int i = 0;
    for (i = 0; i < steps; i++) {
        if (line[i] != line[no_cr_len - i - 1]) {
            return 0;
        }
    }
    return -1;
}
/**
 * @brief   changes a string to lower case
 * @details Iterates over a string and sets all characters to lower case.
 * @param   line    string to modify
 * @return  0 ... success
 * @return  2 ... argument is NULL
 */
static int string_to_lower(char* line) {
    if (line == NULL) {
        return 2;
    }
    int i = 0;
    for (i = 0; line[i] != '\0'; i++) {
        line[i] = tolower(line[i]);
    }
    return 0;
}
/**
 * @brief   writes a string to the opened output file
 * @details Writes the specified 'line' to the output file, if it is open. Global variables used: output
 * @param   line    string to be written
 * @return  0 ... success
 * @return  1 ... output was not open
 */
static int write_to_file(char *line) {
    if (output != NULL) {
	    fwrite(line, sizeof(char), strlen(line), output);
        return 0;
    }
	return 1;
}