/**
 * @file readArgs.h
 * @author Stefan Moser 12025955
 * @date 9.11.2021
 *
 * @brief argument handling
 *
 * @details the module provides a set of functions operating on the datastructure InputArguments_t,
 * e.g initalization, reading, interpreting, storing and error checking of input arguments
 * for details see function documentation below
 **/

#ifndef _READARGS_H /* prevent multiple inclusion */
#define  _READARGS_H

#include <stdio.h>

/**
 *@struct InputArguments_t
 *@brief stores command line inputs
 *
 *@details stores information about options (e.g. case insensitivity or outfile)
 *and the given keyword, input files and all open file handles
 **/
typedef struct argsinfo {
	int caseSensitive;	/**<acts as boolean do determine case sensitivity*/
	char *pOutfileName;	/**<stores outfile name as given in argv,points to argv, not an allocated memory*/
	FILE *outfile;		/**<references file handle for the outfile*/
	char *pKeyword;		/**<points at Keyword given in argv, not an allocated memory*/
	char **pInputFiles;	/**<points at start of input files in argv, not an allocated memory*/
	int countInputFiles; 	/**<number of given input files*/
	FILE **inputFiles;	/**<array stores the file handles, memory of this array has to be managed by mygrep program*/
} InputArguments_t;

/**
 *@brief initArguments initializes all variables of datastructure InputArguments_t with a defined value
 *to support further processing
 *
 *@param[in,out] *pA input arguments
 **/
void initArguments(InputArguments_t *pA);

/**
 *@brief printInputArg prints status of every input argument to stdout
 * used for debugging purposes.
 *
 *@param[in] *pA references input arguments
 **/
void printInputArg(InputArguments_t *pA);

/**
 *@brief analyzeInputArguments interprets input arguments as given in argv and assigns them into *pA's variables
 *
 *@details by using the standard c library function getopt() the options given in argv are analyzed and stored in the InputArguments_t datastructure
 *then the keyword is retrieved from argv
 *possible inputfiles and outfiles are opened for further processing
 *
 *@param[in] argc argument counter
 *@param[in] **argv argument vector
 *@param[in,out] *pA datastructure for input arguments
 **/
void analyzeInputArguments(int argc, char **argv,InputArguments_t *pA);

/**
 *@brief cleanUpInputArgs releases active ressources
 *@details if the outfile is open, it gets closed
 *and all open inputFiles are closed
 *and the array storing the file handles for the open input files is freed
 *@param[in,out] *pA input arguments
 **/
void cleanUpInputArgs(InputArguments_t *pA);

/**
 *@brief calls cleanUpInputArgs and exits
 *@param[in,out] *pA input arguments
 *@param[in] exitStatus exit code
 **/
void cleanUpInputArgsAndExit(InputArguments_t *pA, int exitStatus);

#endif /* _READARGS_H */
