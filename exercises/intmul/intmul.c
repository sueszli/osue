#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define error(...)                               \
  do {                                           \
    fprintf(stderr, "ERROR: " __VA_ARGS__ "\n"); \
    exit(EXIT_FAILURE);                          \
  } while (0)

#define MAXLENGTH 1024

static void read_input(char *firstString, char *secondString) {
  fgets(firstString, MAXLENGTH, stdin);
  fgets(secondString, MAXLENGTH, stdin);
  firstString[strlen(firstString) - 1] = '\0';
  secondString[strlen(secondString) - 1] = '\0';
  if (!(strspn(firstString, "0123456789ABCDEFabcdef") == strlen(firstString)) ||
      !(strspn(secondString, "0123456789ABCDEFabcdef") ==
        strlen(secondString))) {
    error("The input is not a valid HEX-String");
  } else if (strlen(firstString) != strlen(secondString)) {
    error("The length of both strings must be equal");
  } else if ((((strlen(firstString) - 1) / 2) * 2 == strlen(firstString) - 1) &&
             strlen(firstString) != 1) {
    error("The number of digits is not even or 1");
  }
}

static int hex_char_to_int(char character) {
  if (character >= '0' && character <= '9') return character - '0';
  if (character >= 'A' && character <= 'F') return character - 'A' + 10;
  if (character >= 'a' && character <= 'f') return character - 'a' + 10;
  return -1;
}

static char int_to_hex_char(int i) {
  return (char)(i < 10 ? '0' + i : 'a' + i - 10);
}

static void add_hex_char_overflow(char *a, const char *b, char *overflow) {
  int value =
      hex_char_to_int(*a) + hex_char_to_int(*b) + hex_char_to_int(*overflow);
  *a = int_to_hex_char(value % 16);
  *overflow = int_to_hex_char(value / 16);
}

static void add_hex(char *firstHex, const char *secondHex) {
  char overflow = '0';
  int dif = (int)(strlen(firstHex) - strlen(secondHex));
  for (int i = (int)strlen(firstHex) - 1; i >= 0; i--) {
    char second = (i - dif < 0) ? '0' : secondHex[i - dif];
    add_hex_char_overflow(&firstHex[i], &second, &overflow);
  }
  if (overflow != '0') {
    for (int i = (int)strlen(firstHex); i >= 0; i--) {
      firstHex[i + 1] = firstHex[i];
    }
    firstHex[0] = overflow;
  }
}

static void add_X_zeros(char *a, int count) {
  int length = (int)strlen(a);
  for (int i = 0; count > i; i++) {
    a[length] = '0';
    length++;
  }
  a[length] = '\0';
}

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(stderr, "USAGE:\t./intmul\n");
    exit(EXIT_FAILURE);
  }

  // get pair
  int length;
  char firstString[MAXLENGTH];
  char secondString[MAXLENGTH];
  read_input(firstString, secondString);
  length = (int)strlen(firstString) / 2;

  if (strlen(firstString) == 1) {
    int value = (int)strtol(firstString, NULL, 16) *
                (int)strtol(secondString, NULL, 16);
    sprintf(firstString, "%x", value);
    fprintf(stdout, "%s\n", firstString);
    exit(EXIT_SUCCESS);
  }

  // get quad
  char Al[length + 2];
  char Bh[length + 2];
  char Bl[length + 2];
  char Ah[length + 2];
  int i;
  for (i = 0; i < length; i++) {
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

  // --------------------------

  // create pipes
  int p2c[4][2];
  int c2p[4][2];
  for (int i = 0; i < 4; i++) {
    if (pipe(p2c[i]) == -1) {
      error("pipe");
    }
    if (pipe(c2p[i]) == -1) {
      error("pipe");
    }
  }

  enum children { HH, HL, LH, LL };
  enum pipe_end { READ, WRITE };

  // create child processes
  int pid[4];
  for (int i = 0; i < 4; i++) {
    pid[i] = fork();
    if (pid[i] < 0) {
      error("Error at forking");
    } else if (pid[i] == 0) {
      // redirect pipes
      if (dup2(p2c[i][0], STDIN_FILENO) == -1) {
        error("dup2");
      }
      if (dup2(c2p[i][1], STDOUT_FILENO) == -1) {
        error("dup2");
      }
      for (int j = 0; j < 4; j++) {
        close(p2c[j][1]);
        close(c2p[j][0]);
      }

      execlp(argv[0], argv[0], NULL);
      error("execlp");
    }
  }

  // close unnecessary ends
  for (int i = 0; i < 4; i++) {
    close(p2c[i][READ]);
    close(c2p[i][WRITE]);
  }

  // write
  write(p2c[HH][WRITE], Ah, strlen(Ah));
  write(p2c[HH][WRITE], Bh, strlen(Bh));
  write(p2c[HL][WRITE], Ah, strlen(Ah));
  write(p2c[HL][WRITE], Bl, strlen(Bl));
  write(p2c[LH][WRITE], Al, strlen(Al));
  write(p2c[LH][WRITE], Bh, strlen(Bh));
  write(p2c[LL][WRITE], Al, strlen(Al));
  write(p2c[LL][WRITE], Bl, strlen(Bl));
  for (int i = 0; i < 4; i++) {
    close(p2c[i][WRITE]);
  }

  // wait
  for (int i = 0; i < 4; i++) {
    int status;
    waitpid(pid[i], &status, 0);
    if (WEXITSTATUS(status) == 1) {
      error("Error in the childprocess");
    }
  }

  // read
  char retHH[2 * length + length * 2 + 2];
  char retHL[2 * length + length + 2];
  char retLH[2 * length + length + 2];
  char retLL[2 * length + 2];

  int rv;
  rv = (int)read(c2p[HH][READ], retHH, (size_t)length * 2 + 1);
  retHH[rv - 1] = '\0';

  rv = (int)read(c2p[HL][READ], retHL, (size_t)length * 2 + 1);
  retHL[rv - 1] = '\0';

  rv = (int)read(c2p[LH][READ], retLH, (size_t)length * 2 + 1);
  retLH[rv - 1] = '\0';

  rv = (int)read(c2p[LL][READ], retLL, (size_t)length * 2 + 1);
  retLL[rv - 1] = '\0';
  for (int i = 0; i < 4; i++) {
    close(c2p[i][READ]);
  }

  // calculate and print
  add_X_zeros(retHH, length * 2);
  add_X_zeros(retHL, length);
  add_X_zeros(retLH, length);
  add_hex(retHH, retHL);
  add_hex(retHH, retLH);
  add_hex(retHH, retLL);
  fprintf(stdout, "%s\n", retHH);
  exit(EXIT_SUCCESS);
}