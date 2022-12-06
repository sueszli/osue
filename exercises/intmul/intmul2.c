#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define USAGE()                                  \
  do {                                           \
    fprintf(stderr, "USAGE:\t%s\n", "./intmul"); \
    exit(EXIT_FAILURE);                          \
  } while (0)

#define ERROR_EXIT(...)                                         \
  do {                                                          \
    fprintf(stderr, "%s ERROR: " __VA_ARGS__ "\n", "./intmul"); \
    exit(EXIT_FAILURE);                                         \
  } while (0)

#define SUCCESS_EXIT()  \
  do {                  \
    exit(EXIT_SUCCESS); \
  } while (0)

#define MAXLENGTH 1024
#define HEXDIGITS "0123456789ABCDEFabcdef"

#define WRITE 1
#define READ 0

static void read_input(char *firstString, char *secondString) {
  fgets(firstString, MAXLENGTH, stdin);
  fgets(secondString, MAXLENGTH, stdin);
  firstString[strlen(firstString) - 1] = '\0';
  secondString[strlen(secondString) - 1] = '\0';

  if ((strspn(secondString, HEXDIGITS) != strlen(secondString)) ||
      (strspn(firstString, HEXDIGITS) != strlen(firstString))) {
    ERROR_EXIT("The input is not a valid HEX-String");
  } else if (strlen(firstString) != strlen(secondString)) {
    ERROR_EXIT("The length of both strings must be equal");
  } else if ((((strlen(firstString) - 1) / 2) * 2 == strlen(firstString) - 1) &&
             strlen(firstString) != 1) {
    ERROR_EXIT("The number of digits is not even or 1");
  }
}

static int hex_char_to_int(char character) {
  if (character >= '0' && character <= '9') return character - '0';
  if (character >= 'A' && character <= 'F') return character - 'A' + 10;
  if (character >= 'a' && character <= 'f') return character - 'a' + 10;
  return -1;
}

static char int_to_hex_char(int i) {
  if (i < 10) {
    return '0' + i;
  } else {
    return 'a' + i - 10;
  }
}

/**
 *@brief adds two hex-numbers and adds an overflow
 *@param the numbers to add together, value returned in first, overflow in last
 *one
 */
static void add_hex_char_overflow(char *a, const char *b, char *overflow) {
  int value =
      hex_char_to_int(*a) + hex_char_to_int(*b) + hex_char_to_int(*overflow);
  // functions like strtol would need a \0 at the
  // end - custom function since we only process 1 char
  *a = int_to_hex_char(value % 16);
  *overflow = int_to_hex_char(value / 16);
}

/**
 *@brief adds two hex-strings together
 *@param the numbers to add together, the second string has to be the shorter
 *number, value returned in first parameter
 */
static void add_hex(char *firstHex, const char *secondHex) {
  char overflow = '0';
  int dif = strlen(firstHex) - strlen(secondHex);

  for (int i = strlen(firstHex) - 1; i >= 0; i--) {
    char second = (i - dif < 0) ? '0' : secondHex[i - dif];
    add_hex_char_overflow(&firstHex[i], &second, &overflow);
  }

  if (overflow != '0') {  // if overflow exists shift the entire hexstring by 1
                          // and add the overflow to the beginning
    for (int i = strlen(firstHex); i >= 0; i--) {
      firstHex[i + 1] = firstHex[i];
    }
    firstHex[0] = overflow;
  }
}

/**
 *@brief adds 0 to the end of a string
 *@param pointer to the string and amount of 0 to add
 */
static void add_X_zeros(char *a, int count) {
  int length = strlen(a);
  for (int i = 0; count > i; i++) {
    a[length] = '0';
    length++;
  }
  a[length] = '\0';
}

// pipes, i * 2, i * 2 + 1
static void dup_needed_pipes(int pipes[8][2], int neededReadPipe,
                             int neededWritePipe) {
  for (int i = 0; i < 8; i++) {
    if (i == neededReadPipe) {
      if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
        ERROR_EXIT("dup-Error");
      }

    } else if (i == neededWritePipe) {
      if (dup2(pipes[i][0], STDIN_FILENO) == -1) {
        ERROR_EXIT("dup-Error");
      }
    }

    close(pipes[i][1]);
    close(pipes[i][0]);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 1) {
    USAGE();
  }

#pragma region region
  // read in input -> get hex pair
  int length;
  char firstString[MAXLENGTH];
  char secondString[MAXLENGTH];
  read_input(firstString, secondString);
  length = strlen(firstString) / 2;

  // base case
  if (strlen(firstString) == 1) {
    int value = (int)strtol(firstString, NULL, 16) *
                (int)strtol(secondString, NULL, 16);
    sprintf(firstString, "%x", value);
    fprintf(stdout, "%s\n", firstString);
    SUCCESS_EXIT();
  }

  // get hex quad
  char Al[length + 2];
  char Bh[length + 2];
  char Bl[length + 2];
  char Ah[length + 2];
  int i = 0;
  for (; i < length; i++) {
    Ah[i] = firstString[i];
    Bh[i] = secondString[i];
    Al[i] = firstString[length + i];
    Bl[i] = secondString[length + i];
  }
  Ah[i] = '\n';
  Bh[i] = '\n';
  Al[i] = '\n';
  Bl[i] = '\n';
  Ah[i + 1] = '\0';
  Bh[i + 1] = '\0';
  Al[i + 1] = '\0';
  Bl[i + 1] = '\0';

  // open pipes
  const int READ_CHILD_HH = 0;
  const int WRITE_CHILD_HH = 1;
  const int READ_CHILD_LH = 2;
  const int WRITE_CHILD_LH = 3;
  const int READ_CHILD_HL = 4;
  const int WRITE_CHILD_HL = 5;
  const int READ_CHILD_LL = 6;
  const int WRITE_CHILD_LL = 7;
  int pipes[8][2];

  if (pipe(pipes[READ_CHILD_HH]) == -1 || pipe(pipes[WRITE_CHILD_HH]) == -1 ||
      pipe(pipes[READ_CHILD_HL]) == -1 || pipe(pipes[WRITE_CHILD_HL]) == -1 ||
      pipe(pipes[READ_CHILD_LH]) == -1 || pipe(pipes[WRITE_CHILD_LH]) == -1 ||
      pipe(pipes[READ_CHILD_LL]) == -1 || pipe(pipes[WRITE_CHILD_LL]) == -1) {
    ERROR_EXIT("Error when opening pipes");
  }

#pragma endregion region

  // create child processes
  int pid[4];
  for (int i = 0; i < 4; i++) {
    pid[i] = fork();
    if (pid[i] < 0) {
      ERROR_EXIT("Error at forking");
    } else if (pid[i] == 0) {
      dup_needed_pipes(pipes, i * 2, i * 2 + 1);

      if (execlp(argv[0], argv[0], NULL) == -1) {
        ERROR_EXIT("Error on execlp");
      }
    }
  }

  {
    // close all unnecessary ends
    close(pipes[WRITE_CHILD_HH][READ]);
    close(pipes[READ_CHILD_HH][WRITE]);
    close(pipes[WRITE_CHILD_HL][READ]);
    close(pipes[READ_CHILD_HL][WRITE]);
    close(pipes[WRITE_CHILD_LH][READ]);
    close(pipes[READ_CHILD_LH][WRITE]);
    close(pipes[WRITE_CHILD_LL][READ]);
    close(pipes[READ_CHILD_LL][WRITE]);

    // writing
    write(pipes[WRITE_CHILD_HH][WRITE], Ah, strlen(Ah));
    write(pipes[WRITE_CHILD_HH][WRITE], Bh, strlen(Bh));
    close(pipes[WRITE_CHILD_HH][WRITE]);

    write(pipes[WRITE_CHILD_HL][WRITE], Ah, strlen(Ah));
    write(pipes[WRITE_CHILD_HL][WRITE], Bl, strlen(Bl));
    close(pipes[WRITE_CHILD_HL][WRITE]);

    write(pipes[WRITE_CHILD_LH][WRITE], Al, strlen(Al));
    write(pipes[WRITE_CHILD_LH][WRITE], Bh, strlen(Bh));
    close(pipes[WRITE_CHILD_LH][WRITE]);

    write(pipes[WRITE_CHILD_LL][WRITE], Al, strlen(Al));
    write(pipes[WRITE_CHILD_LL][WRITE], Bl, strlen(Bl));
    close(pipes[WRITE_CHILD_LL][WRITE]);

    // Wait for child
    int status[4];
    waitpid(pid[0], &status[0], 0);
    waitpid(pid[1], &status[1], 0);
    waitpid(pid[2], &status[2], 0);
    waitpid(pid[3], &status[3], 0);

    if (WEXITSTATUS(status[0]) == 1 || WEXITSTATUS(status[1]) == 1 ||
        WEXITSTATUS(status[2]) == 1 || WEXITSTATUS(status[3]) == 1) {
      ERROR_EXIT("Error in the childprocess");
      exit(EXIT_FAILURE);
    }
  }

  // Read string from child and close reading end.
  char returnChildHH[2 * length + length * 2 + 2];
  char returnChildHL[2 * length + length + 2];
  char returnChildLH[2 * length + length + 2];
  char returnChildLL[2 * length + 2];

  {
    int rv;
    rv = read(pipes[READ_CHILD_HH][READ], returnChildHH, length * 2 + 1);
    returnChildHH[rv - 1] = '\0';
    close(pipes[READ_CHILD_HH][READ]);

    rv = read(pipes[READ_CHILD_HL][READ], returnChildHL, length * 2 + 1);
    returnChildHL[rv - 1] = '\0';
    close(pipes[READ_CHILD_HL][READ]);

    rv = read(pipes[READ_CHILD_LH][READ], returnChildLH, length * 2 + 1);
    returnChildLH[rv - 1] = '\0';
    close(pipes[READ_CHILD_LH][READ]);

    rv = read(pipes[READ_CHILD_LL][READ], returnChildLL, length * 2 + 1);
    returnChildLL[rv - 1] = '\0';
    close(pipes[READ_CHILD_LL][READ]);
  }

  // calculation

  add_X_zeros(returnChildHH, length * 2);
  add_X_zeros(returnChildHL, length);
  add_X_zeros(returnChildLH, length);

  add_hex(returnChildHH, returnChildHL);
  add_hex(returnChildHH, returnChildLH);
  add_hex(returnChildHH, returnChildLL);

  fprintf(stdout, "%s\n", returnChildHH);
  SUCCESS_EXIT();
}