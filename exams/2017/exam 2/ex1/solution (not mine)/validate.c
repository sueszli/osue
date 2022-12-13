#include "validate.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/******************************************************************************
 * Function declarations
 *****************************************************************************/

// tasks you shall implement (located below main function)
void task_1(char *iban, char expr[MAX_TEXTLEN]);
void task_2(int fd[2], char expr[MAX_TEXTLEN], char result[MAX_TEXTLEN]);
void task_3(int fd[2], char expr[MAX_TEXTLEN]);

/******************************************************************************/

int main(int argc, char *argv[]) {
  // reads the arguments
  char *iban; /**< Pointer to the IBAN given as positional argument. */
  read_arguments(argc, argv, &iban);

  // prepare expression (convert IBAN to integer)
  char expr[MAX_TEXTLEN]; /**< Expression for `./calc`. */
  task_1(iban, expr);

  // setup pipe to communicate with the child process
  int fd[2];
  if (pipe(fd) < 0) {
    error_exit("Creating pipe failed.");
  }
  // fork a child process (calls task_3)
  char result[MAX_TEXTLEN]; /**< Result from the child process. */
  task_2(fd, expr, result);

  wait_for_child();

  // print result
  result[2] = '\0';
  if ((strcmp(result, "1\n") == 0) || (strcmp(result, "1") == 0)) {
    printf("valid\n");
    exit(EXIT_SUCCESS);
  }

  printf("invalid\n");
  exit(EXIT_FAILURE);
}

/******************************************************************************/

/***************************************************************************
 * Task 1
 * ------
 * Prepare the expression calculating the remainder of the IBAN as integer
 * mod 97.
 *
 * - Move the country code and the check digits (first 4 characters of the
 *   IBAN) to the end.
 * - Replace all alphabetic characters with numbers ('A' -> "10", 'B' -> "11",
 *   .., 'Z' -> "35").
 * - Prepare the expression for the child process `./calc`, i.e., the variable
 *   'expr' shall contain "<iban-as-integer> % 97".
 *
 * See also: snprintf(3), strncpy(3), isalpha(3), isupper(3), isdigit(3)
 **************************************************************************/

/**
 * @brief Task 1: Prepare the expression, convert the IBAN to an integer.
 * @param iban The pointer to the IBAN.
 * @param expr The expression for `./calc`.
 */
void task_1(char *iban, char expr[MAX_TEXTLEN]) {
  // step 1
  long iban_length = strlen(iban);
  char temp[iban_length + 1];
  strncpy(temp, iban + 4, iban_length - 4);
  strncpy(temp + iban_length - 4, iban, 4);
  temp[iban_length] = '\0';

  // step 2
  char temp2[(iban_length * 2) + 3];
  int index = 0;
  for (int i = 0; i < iban_length; ++i) {
    if (isupper(temp[i])) {
      sprintf(temp2 + index, "%d", temp[i] - 55);
      index += 2;
    } else if (isdigit(temp[i])) {
      sprintf(temp2 + index, "%c", temp[i]);
      index += 1;
    }
  }

  char *end = "%97";
  strncpy(temp2 + index, end, 3);
  temp2[index + 3] = '\0';
  // step 3
  strncpy(expr, temp2, index + 4);
}

/***************************************************************************
 * Task 2
 * ------
 * Fork a child process.
 *
 * - In the child process, call task_3(fd, expr).
 * - In the parent process, read the result from the pipe. Save the result
 *   to the variable `result`.
 * - If fork(2) fails, quit the program with exit code EXIT_EFORK.
 *
 * See also: fork(2), close(2), read(2), fdopen(3), fgets(3), fclose(3)
 **************************************************************************/

/**
 * @brief Task 2: Fork a child process and communicate via the pipe.
 * @param fd Pipe to communicate with the child.
 * @param expr The expression for the child `./calc`.
 * @param result The result received from the child `./calc`.
 */
void task_2(int fd[2], char expr[MAX_TEXTLEN], char result[MAX_TEXTLEN]) {
  pid_t pid = fork();

  if (pid == 0) {
    // child process
    task_3(fd, expr);
    write(fd[WRITE_END], expr, MAX_TEXTLEN);
    close(fd[READ_END]);
  } else if (pid > 0) {
    // parent process
    read(fd[READ_END], result, MAX_TEXTLEN);
    close(fd[WRITE_END]);
  } else {
    // error
    exit(EXIT_EFORK);
  }
}

/****************************************************************************
 * Task 3
 * ------
 * Execute: ./calc expression.
 *
 * - Close the read end of the pipe.
 * - Redirect the write end of the pipe to stdout.
 * - Then execute './calc' with the given expression, using exec*(3). The
 *   path is "./calc".
 * - If executing './calc expression' fails, terminate the process.
 *
 * See also: execl(3), execv(3), execlp(3), execvp(3), close(2), dup2(2)
 ***************************************************************************/

/**
 * @brief Task 3: Setup the pipe and execute the child program.
 * @param fd Pipe for parent/child communication.
 * @param expr Prepared expression for `./calc`, the child program.
 */
void task_3(int fd[2], char expr[MAX_TEXTLEN]) {
  close(fd[READ_END]);
  dup2(fd[WRITE_END], STDOUT_FILENO);
  close(fd[WRITE_END]);
  int exec_err = 0;
  exec_err = execl("./calc", "./calc", expr, (char *)NULL);
  if (exec_err == -1) {
    error_exit("execl failed.");
  }
}
