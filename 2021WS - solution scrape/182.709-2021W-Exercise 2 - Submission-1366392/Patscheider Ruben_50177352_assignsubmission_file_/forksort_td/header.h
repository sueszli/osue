/**
*   Author: @author Ruben Patscheider
*   MatrikelNummer: 01627951
*   Date: @date 11.12.2021
*   Name of Project: forksort
*   Name of module: header
*   Purpose of module: 
*   @brief header.h contains all signatures of functions used in functions.c as well as
*   definitions
*/

#define INCREMENT 5 ///< size of each increment in which the buffer is made bigger each increment
#define POS_READ 0 ///< Reading position of a pipe
#define POS_WRITE 1 ///< Write Position of a pipe

/**
 * @brief the input read from a file (pipe or stdin) will be handeled and each line
 * will be transfered as a line into the char** array (string array).
 * 
 * @param buffer a pointer to a string array, where the input is stored after it was handeled by the function
 * @param linecount number of read lines will be stored here, after it was calculated in function
 * @param file the file where the function will read from
 */

void handleInput(char*** buffer, int* linecount, FILE* file);

/**
 * @brief the forked child will use this function to create a new process with execlp
 * 
 */

void execProcess(void);

/**
 * @brief merges two sorted arrays into a resultarray, the result is sorted too
 * 
 * @details by comparing the elements of two sorted arrays, a 3rd sorted array
 * is generated. If one array runs out of elements, the other array will add
 * its missing elements in the same order as it was ordered in the initial array,
 * since its already ordered. By using this function recursively, mergesort is 
 * implemented via forks.
 * 
 * @param child1 first sorted subarray to merge into result
 * @param child2 second sorted subarray to merge into result
 * @param child1len number of elements in child1
 * @param child2len number of elements in child2
 * @param sorted result array, where the final merged array is saved
 * @param len number of over all elements, aka number of elements in sorted
 */

void merge(char** child1, char** child2, int child1len, int child2len, char** sorted, int len);

/**
 * @brief function to free an array, by freeing all its lines (strings) and then
 * freeing the pointer to those lines
 * 
 * @param array array to be freed
 * @param n lenghts of the array that should be freed
 */

void freeArray(char** array, int n);

/**
 * @brief closes both ends of a pipe
 * 
 * @param pipe pipe that should be closed
 */

void closePipe(int pipe[2]);