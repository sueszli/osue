#ifndef __VALIDATE_H__
#define __VALIDATE_H__

#define READ_END        0
#define WRITE_END       1

#define MAX_TEXTLEN     80

#define EXIT_EFORK      5

/******************************************************************************
 * Function declarations (all functions below are already implemented)
 *****************************************************************************/

/**
 * @brief Reads the argument(s).
 * @param argc Number of command line arguments (including argv[0]).
 * @param argv Array of command line arguments.
 * @param iban Pointer to the IBAN (C-String; IBAN without spaces).
 */
void read_arguments(int argc, char** argv, char **iban);

/** Print an error message to stderr and exit with EXIT_FAILURE. */
void error_exit(const char *msg);

/** Print usage message to stderr and exit with EXIT_FAILURE. */
void usage(void);

/** Waits for the child process to quit. */
void wait_for_child(void);

/** Demo solutions. */
void task_1_DEMO(char *iban, char expr[MAX_TEXTLEN]);
void task_2_DEMO(int fd[2], char expr[MAX_TEXTLEN], char result[MAX_TEXTLEN]);
void task_3_DEMO(int fd[2], char expr[MAX_TEXTLEN]);

#endif
