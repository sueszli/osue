/**
 * @author Sunny SINGH 00271187
 * @date 2021-11-01
 * @brief OSUE Exercise 1A - Assignment ispalindrom
 * @details The program ispalindrom reads files line by line and for each line checks
 * whether it is a palindrom, i.e. whether the text read backwards is identical to itself.
 * Each line will be printed followed by the text "is a palindrom" if the line is a palindrom
 * or "is not a palindrom" if the line is not a palindrom.
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char* PROGRAM_NAME;

/*
 * @brief Print the usage and exit.
 * @details Print the usage of the program and exit the program with EXIT_FAILURE.
 */
void usage_and_exit(void)
{
  fprintf(stderr, "USAGE: %s [-s] [-i] [-o OUTFILE] [FILE...]\n", PROGRAM_NAME);
  exit(EXIT_FAILURE);
}

/*
 * @brief Checks if the given line is a palindrom.
 * @details Checks the given line whether it is a palindrom or not. 
 * If so the line with "is a palindrom" will be written to the output.
 * Otherwise the line with "is not a palindrom" will be written to the output.
 * NOTICE: An empty line will be handled as palindrom. 
 * @param line Pointer to the line.
 * @param flag_s If integer > 0 than whitespaces will be ignored.
 * @param flag_i If integer > 0 than lower/upper cases will be ignored.
 * @param FILE* Pointer to the output file. 
 */
void palindrom(char *line, int flag_s, int flag_i, FILE* output)
{
  size_t len = strlen(line);
  line[len-1] = '\0';

  int l,r;
  char a,b;
  for(l = 0, r = len-2; l <= r; l++, r--)
  {
    // ignore whitespace
    if (flag_s > 0)
    {
      while(isspace(line[l]) != 0)
      {
        if (l >= r) break;
        l++;
      }

      while(isspace(line[r]) != 0)
      {
        if (l >= r) break;
        r--;
      }
    }

    a = line[l];
    b = line[r];

    if (flag_i > 0)
    {
      a = tolower(a);
      b = tolower(b);
    }

    if (a != b)
    {
      fprintf(output, "%s is not a palindrom\n", line);
      return;
    }
  }

  fprintf(output, "%s is a palindrom\n", line);
}

/*
 * @bief Starting point of the program.
 * @details Starting point of the program ispalindrom.It reads files line by line and for each line checks
 * whether it is a palindrom, i.e. whether the text read backwards is identical to itself.
 * Each line will be printed followed by the text "is a palindrom" if the line is a palindrom
 * or "is not a palindrom" if the line is not a palindrom.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
  PROGRAM_NAME = argv[0];

  int flag_s = 0, flag_i = 0, flag_o = 0;
  int inp = 0;
  int opt;

  char *line;

  int nRet;
  size_t *t = malloc(0);
  char **gptr = malloc(sizeof(char*));
  *gptr = NULL;

  size_t len;

  FILE* output = stdout;
  FILE* input = stdin;

  while((opt = getopt(argc, argv, "sio:")) != -1)
  {
    switch(opt)
    {
      case 's': // ignore whitespaces
        if (flag_s >= 1) usage_and_exit();
        flag_s++;
        break;

      case 'i': // case insensitive
        if (flag_i >= 1) usage_and_exit();
        flag_i++;
        break;

      case 'o': // specify output file instead of stdout
        if (flag_o >= 1) usage_and_exit();
        output = fopen(optarg, "w+");
        if (output == NULL)
        {
          fprintf(stderr, "%s: error fopen.\n%s\n", PROGRAM_NAME, strerror(errno));
          exit(EXIT_FAILURE);
        }
        flag_o++;
        break;

      default:
        usage_and_exit();
    }

  }

  inp = argc - optind;

  do{
    if (inp > 0)
    {
      input = fopen(argv[optind], "r+");
      if (input == NULL)
      {
        fprintf(stderr, "%s: error fopen.\n%s\n", PROGRAM_NAME, strerror(errno));
        continue;
      }
      inp--;
    }

    while((nRet = getline(gptr, t, input)) > 0)
    {
      palindrom(*gptr, flag_s, flag_i, output);
    }
  } while(inp > 0);
  
  exit(EXIT_SUCCESS);
}


