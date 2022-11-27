/***
 * @author: Markovic Predrag 
 * @brief: Helper functions for forksort.c
 * @details: The purpose of this module is to provide useful functions to forksort.c(e.g. error handling, freememory, ...)
 * @date: 8.12.2021
 ***/

#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

/***
 * @brief: Writes stream to stdout
 * @details: It takes two pipe-read descriptors and reads the sorted content of the children
 * @param: FD1 - Read file descriptor of child 1
 * @param: FD2 - Read file descriptor of child 2
 * @return: void
 ***/
void writeStreamsToStdout(const int FD1, const int FD2);
/***
 * @brief: Free Array memory
 * @details: Frees the memory of a array 
 * @param: array - Array of char* 
 * @param: LENGTH - Length of the array
 * @return: void
 ***/
void freeArrayMemory(char* array[], const int LENGTH);
/***
 * @brief: close(fd) check
 * @details: Checks if close file descriptor succeded. If not then EXIT_FAILURE
 * @param: RES - Status
 * @return: void
 ***/
void checkCloseRes(const int RES);
/***
 * @brief: dup(2)(fd) check
 * @details: Checks if dup file descriptor succeded. If not then EXIT_FAILURE
 * @param: RES - Status
 * @return: void
 ***/
void checkDubRes(const int RES);
/***
 * @brief: pipe(fd[]) check
 * @details: Checks if pipe creation succeded. If not then EXIT_FAILURE
 * @param: RES - Status
 * @return: void
 ***/
void checkPipeRes(const int RES);
#endif