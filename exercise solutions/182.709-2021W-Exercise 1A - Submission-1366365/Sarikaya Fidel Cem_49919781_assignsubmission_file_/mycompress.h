/**     
*@file mycompress.h
*@author Fidel Cem Sarikaya <e11941488@student.tuwien.ac.at>
*@date 05.11.2021
*
*@brief Provides the compressor() function to the program.
*
* The mycompress module. Contains a function related to compressing a text using run-length coding algorithm.
**/

#ifndef MYCOMPRESS_H   /* #IncludeGuards that prevent the double      */
#define MYCOMPRESS_H   /* declaration of the function 'compressor()'. */

/**
 * Compresses some text in run-length form.
 * @brief This function writes the run-length coded from of the paramter 'in' to the 'out' parameter 'out'
 * @details The function does not check whether the 'out' variable is allocated enough memory to contain
 * the worst-case compression of 'in' variable. The function works correctly only when 'out' has a buffer 
 * size that more than twice the size of 'in'.
 * @param in This string gets compressed using run-length coding.
 * @param out This empty string contains the compressed form of 'in'.
 */
extern void *compressor(const char *in, char *out);

#endif                 //* MYCOMPRESS_H