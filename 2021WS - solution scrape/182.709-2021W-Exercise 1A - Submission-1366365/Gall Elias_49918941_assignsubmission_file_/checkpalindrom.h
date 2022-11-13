#ifndef CHECKPALINDROM
#define CHECKPALINDROM

/**
 * @file checkpalindrom.h
 * @author 	Elias GALL - 12019857
 * @brief 	header file for checkpalindrom.c
 * @details Exports functions form checkpalindrom.c for use in other files.
 * @date 	2021-10-29
 */

/**
 * @brief   checks a specified file for palindroms and writes the result either to stdout or a file
 * @details Checks the lines in the file specified by 'filename' for whether they are a palindrom or not.
 *          Whitespaces and case can be ignored using arguments. The result is either written to 'output_filename'
 *          or to stdout if the argument is NULL. Global variables used: output, files_processed
 * @param   filename            name of the file to be read
 * @param   ignore_whitespace   logically interpreted int specifying whether to ignore whitespaces or not
 * @param   ignore_case         logically interpreted int specifying whether or not to ignore case
 * @param   output_filename     name of the file to write the results to
 * @return  0 ... success
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 * @return  4 ... could not open file
 */
int check_file_for_palindroms(const char* file_to_check, int ignore_whitespaces, int ignore_case, const char* output_filename);
/**
 * @brief   checks stdin for palindroms and writes the result either to a file or to stdout
 * @details Reads lines from stdin an checks whether they are palindroms or not according to the
 *          specified criteria. Writes the results to 'output_filename' if it is not NULL, otherwise
 *          to stdout. Global variables used: output
 * @param   output_filename     the destination to write results to, if NULL written to stdout
 * @param   ignore_whitespace   logically interpreted int specifying whether to ignore whitespaces or not
 * @param   ignore_case         logically interpreted int specifying whether or not to ignore case
 * @return  0 ... success
 * @return  1 ... unspecified error
 * @return  2 ... input is NULL and not expected
 * @return  3 ... could not allocate memory
 * @return  4 ... could not open file
 */
int check_std_in_for_palindroms(const char* output_filename, int ignore_whitespaces, int ignore_case);

#endif