/**
 * @file mycompress.c
 * @author Munir Yousif Elagabani <e12022518@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Small program that takes strings as input and outputs the run-length
 * encoding of the string.
 **/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 100

static char *pgm_name;

/** Mandatory usage function.
 *  @brief This function writes helpful usage information about the program to
 *stderr.
 *  @details global variables: pgm_name
 **/
static void usage(void) {
  fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", pgm_name);
  exit(EXIT_FAILURE);
}

static void compress(FILE *out, FILE *in);

/** Program entry point.
 *  @brief The program starts here. This function takes care of the options and
 *  sets up the input and output streams and finally calls the compress function
 *  @param argc the number of arguments
 *  @param argv the array of arguments
 *  @return Returns exit code EXIT_SUCCESS OR EXIT_FAILURE
 **/
int main(int argc, char **argv) {
  pgm_name = argv[0];

  char *outfile = NULL;
  char c;
  while ((c = getopt(argc, argv, "o:")) != -1) {
    switch (c) {
      case 'o':
        if (outfile == NULL) {
          outfile = optarg;
        } else {
          fprintf(stderr, "%s: There should be only one -o option!\n",
                  pgm_name);
          usage();
        }
        break;
      default:
        usage();
        break;
    }
  }

  FILE *out = NULL;
  if (outfile != NULL) {
    out = fopen(outfile, "w");
    if (!out) {
      fprintf(stderr, "%s: fopen failed: %s\n", pgm_name, strerror(errno));
      exit(EXIT_FAILURE);
    }
  } else {
    out = stdout;
  }

  int numInFiles = argc - optind;
  if (numInFiles != 0) {
    for (int i = optind; i < argc; i++) {
      char *inFile = argv[i];
      FILE *in = fopen(inFile, "r");
      if (!in) {
        fprintf(stderr, "%s: fopen failed: %s\n", pgm_name, strerror(errno));
      }
      compress(out, in);
      fclose(in);
    }
  } else {
    compress(out, stdin);
  }

  if (outfile != NULL && fclose(out)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/** Compress function.
 * @brief This function takes one input and output file, and writes the
 * compressed input onto the output file.
 * @details out also accepts stdout and in also accepts stdin
 **/
static void compress(FILE *out, FILE *in) {
  char buffer[BUFFER_SIZE];
  int totalRead = 0;
  int totalWritten = 0;
  int currentChar = -1;
  int currentAmount = 0;

  while (fgets(buffer, sizeof(buffer), in) != NULL) {
    for (int i = 0; i < strlen(buffer); i++) {
      int readChar = buffer[i];
      if (readChar == '\0') {
        break;
      }

      if (currentChar == -1) {
        currentChar = readChar;
        currentAmount = 1;
        continue;
      }

      if (currentChar != readChar) {
        fprintf(out, "%c%d", currentChar, currentAmount);
        totalWritten += 2;
        currentChar = readChar;
        currentAmount = 1;
      } else {
        currentAmount++;
      }
    }
    totalRead += strlen(buffer);
  }
  if (currentAmount != 0 && currentChar != EOF) {
    fprintf(out, "%c%d", currentChar, currentAmount);
    totalWritten += 2;
    currentAmount = 0;
  }
  fprintf(out, "\n");

  if (ferror(in)) {
    fprintf(stderr, "%s: fopen failed: %s\n", pgm_name, strerror(errno));
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Read: \t\t%d characters\n", totalRead);
  fprintf(stderr, "Written: \t%d characters\n", totalWritten);
  fprintf(stderr, "Compression ratio: %.1f%% \n",
          ((float)totalWritten / totalRead) * 100);
  fflush(stderr);
}