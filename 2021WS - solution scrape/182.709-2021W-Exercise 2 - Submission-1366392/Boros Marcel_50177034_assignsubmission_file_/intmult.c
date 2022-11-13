/**
 * @file intmult.c
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 05.12.2021
 *
 * @brief Main program module.
 * 
 * This program is the main program for reading in and computing hexadecimal numbers from stdin. It mainly uses functions
 * defined in mult.c respectively readIn.c.
 **/


#include "readIn.h"
#include "mult.h"

/**
 * @brief Calculate the product of two hexadecimal numbers.
 * @details firstLine and secondLine are the input strings for the two hexadecimal numbers from stdin. SolutionArray holds
 * the not computed parts of the final solution in its 4 elements. The functions readLines, checkInputVAlues and multiply
 * are used to compute the product.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv) {
    
    char* firstLine = NULL, *secondLine = NULL;
    char* solutionArray[4];

    
    
    //allocate memory for the first line (first hex number)
    if ((firstLine = malloc(sizeof(char) * BYTES)) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    //allocate memory for the second line (second hex number)
    if ((secondLine = malloc(sizeof(char) * BYTES)) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        exit(EXIT_FAILURE);
    }
    
    
    
    //read in first and second line from stdin
    readLines(firstLine, secondLine, argv);
    
    
    //check input values if they have equal length and are hexadecimal numbers
    checkInputValues(firstLine,secondLine,argv);
    
    //multiply hexadecimal numbers and print the result to stdout
    multiply(firstLine, secondLine, argv, solutionArray);
    
    for(int i=0; i<4; ++i) {
        free(solutionArray[i]);
    }
    
    
    
    free(firstLine);
    free(secondLine);
    exit(EXIT_SUCCESS);
        
}