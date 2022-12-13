#include "validate.h"

#include <assert.h>
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
  printf("input: %s\n", iban);
  if (strlen(iban) > 999) {  // <--- test to get min size that is legal works
    usage();
  }

  // place first 4 chars at the end
  char four[5];
  for (int i = 0; i < 4; i++) {  // <--- simplify these parts to memncpy
    four[i] = iban[i];
  }
  four[4] = '\0';
  printf("first four chars: %s\n", four);

  // copy remaining chars
  const size_t remainingLen = strlen(iban) - 4;
  char *remaining = malloc((remainingLen + 1) * sizeof(char));
  if (remaining == NULL) {
    error_exit("malloc");
  }
  size_t rCounter = 0;
  for (size_t i = 4; i <= strlen(iban); i++) {
    remaining[rCounter++] = iban[i];
  }
  printf("remaining chars: %s\n", remaining);

  // write into tmp in flipped order
  char *tmp = malloc((strlen(iban) + 1) * sizeof(char));
  if (tmp == NULL) {
    error_exit("malloc");
  }
  size_t tCounter = 0;
  for (size_t i = 0; i < strlen(remaining); i++) {
    tmp[tCounter++] = remaining[i];
  }
  for (size_t i = 0; i < 4; i++) {
    tmp[tCounter++] = four[i];
  }
  tmp[tCounter] = '\0';
  free(remaining);
  printf("tmp: %s\n", tmp);

  // convert chars to digits, write into expr
  size_t eCounter = 0;
  for (size_t i = 0; i < strlen(tmp); i++) {
    if (isdigit(tmp[i])) {
      printf("already digit: %c\n", tmp[i]);
      expr[eCounter++] = tmp[i];

    } else if (isalpha(tmp[i])) {
      printf("not digit: %c\n", tmp[i]);
      int val = tmp[i] - 55;

      int fstInt = val / 10;
      assert(fstInt < 10);
      char fstChar[2] = {'\0', '\0'};
      sprintf(fstChar, "%d", fstInt);
      printf("fst: %c\n", fstChar[0]);
      expr[eCounter++] = fstChar[0];

      int sndInt = val % 10;
      assert(sndInt < 10);
      char sndChar[2] = {'\0', '\0'};
      sprintf(sndChar, "%d", sndInt);
      printf("snd: %c\n", sndChar[0]);
      expr[eCounter++] = sndChar[0];

    } else {
      usage();
    }
  }
  free(tmp);
  expr[eCounter++] = ' ';
  expr[eCounter++] = '%';
  expr[eCounter++] = ' ';
  expr[eCounter++] = '9';
  expr[eCounter++] = '7';
  expr[eCounter] = '\0';
  printf("expr: %s\n", expr);

  // task_1_DEMO(iban, expr);
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
  /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
  task_2_DEMO(fd, expr, result);
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
  /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
  task_3_DEMO(fd, expr);
}
