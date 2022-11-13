#ifndef _FORKSORT_H_
#define _FORKSORT_H_

#include <stdio.h>
#include <sys/types.h>


/* ----- debug ----- */

#ifdef DEBUG
#define debug(fmt, ...) \
    (void) fprintf(stderr, "[%s:%d] " fmt "\n", \
                    __FILE__, __LINE__, \
                    ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif


/* ----- constants ----- */

#define READ 0      // Read end of pipe
#define WRITE 1     // Write end of pipe


/* ----- structs ----- */

struct fork_process  
{  
    FILE *write;
    FILE *read;
    pid_t pid;
}; 


/* ----- prototypes ----- */

/**
 * @brief define usage function
**/
static void usage(void);

/**
 * @brief error exit method. It print's the given message and exit the program with EXIT_FAILURE
 * @param msg the message which should be printed
**/
static void error_exit(const char *msg);

/**
 * @brief forks the child. The child executes recursive the program
 * @param program_name the program name. Is needed for recursive execusion
 * @return the pid, read and write FILEs from the child
**/
static struct fork_process fork_child(char* program_name);

/**
 * @brief reads from the childs line by line, merges and write the output to stdout
 * @param first_child the file reader from first child
 * @param second_child the file reader from second child
**/
static void read_and_merge(FILE *first_child, FILE *second_child);

#endif  // _FORKSORT_H_
