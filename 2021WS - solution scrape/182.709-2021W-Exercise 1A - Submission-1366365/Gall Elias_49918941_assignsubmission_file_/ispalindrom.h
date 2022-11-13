#ifndef ISPALINDROM
#define ISPALINDROM
/**
 * @file ispalindrom.h
 * @author 	Elias GALL - 12019857
 * @brief 	header file for ispalindrom.c
 * @details Exports functions form ispalindrom.c for use in other files.
 * @date 	2021-10-29
 */

/**
 * @brief   prints error message from other module to stderr
 * @details Called by 'checkpalindrom.c' if an error occured there. Prints the specified error message with the program name to stderr.
 *          Uses 'program_name' to fill in the name of the program in the error message.
 * @param   msg     message to be printed to stderr
 * @return  void
 */
void external_error(const char* error);

#endif