/**
 * @file smgrep.h
 * @author Stefan Moser 12025955
 * @date 9.11.2021
 *
 * @brief implements grepping
 *
 * @details smgrep provides functions for either reading from stdin or given input files
 * grepFiles reads from input Files, if there are none grepStdin is called
 * For further details of the functions, please read documentation below!
 **/

#ifndef _SMGREP_H
#define _SMGREP_H

#include "readArgs.h"


/**
 *@brief search keyword from stdin
 *
 *@details grepStdin reads lines from stdin and searches for the specified keyword
 * stored in InputArguments_t *pA
 * The function differentiates between case sensitive/insensitive search
 * The function either writes the lines containing the keyword
 * to stdout or to a specified output file.
 *
 *@param[in,out] *pA input arguments
 **/
void grepStdin(InputArguments_t *pA);

/**
 *@brief search keyword from all input files
 *
 *@details grepFiles reads all lines in given files and searches for the specified keyword
 * stored in InputArguments_t *pA
 * The function differentiates between case sensitive/insensitive search.
 * The function either writes the lines containing the keyword
 * to stdout or to a specified output file.
 *
 *@param[in,out] *pA input arguments
 **/
void grepFiles(InputArguments_t *pA);


#endif //_SMGREP_H
