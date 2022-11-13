#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *PROGRAM_NAME;
static void readAndPrint(FILE *input, FILE *output, char *keyword, bool isCaseSensitive);
static void toLowerStr(char *text)
{
  for (int i = 0; text[i] != '\0'; i++)
  {
    text[i] = tolower(text[i]);
  }
}

int main(int argc, char *argv[])
{
  PROGRAM_NAME = argv[0];
  bool isCaseSensitive = true;
  char *outFilename = NULL;
  {
    int c;
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
      switch (c)
      {
      case 'i':
      {
        isCaseSensitive = false;
        break;
      }
      case 'o':
      {
        outFilename = optarg;
        break;
      }
      case '?':
        break;
      }
    }
  }

  FILE *output = NULL;
  if (outFilename != NULL)
  {
    output = fopen(outFilename, "w");
    if (output == NULL)
    {
      fprintf(stderr, "%s failed to open the output file", PROGRAM_NAME);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc)
  {
    fprintf(stderr, "%s missing keyword", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  char *keyword = argv[optind];
  optind += 1;
  if (!isCaseSensitive)
  {
    toLowerStr(keyword);
  }

  // If we have one or more files
  bool hasFile = optind < argc;
  if (hasFile)
  {
    for (int i = optind; i < argc; i++)
    {
      char *inputFilename = argv[optind];
      FILE *input = fopen(inputFilename, "r");
      readAndPrint(input, output == NULL ? stdout : output, keyword, isCaseSensitive);
      fclose(input);
    }
  }
  else
  {
    readAndPrint(stdin, output == NULL ? stdout : output, keyword, isCaseSensitive);
  }

  if (output != NULL)
  {
    fclose(output);
  }
  return 0;
}

static void readAndPrint(FILE *input, FILE *output, char *keyword, bool isCaseSensitive)
{
  size_t length = 0;
  ssize_t readCount = 0;
  char *line = NULL;
  // Getline is defined in stdio.h, just like fputs
  while ((readCount = getline(&line, &length, input)) != -1)
  {
    char *lineToCompare = line;
    if (!isCaseSensitive)
    {
      lineToCompare = strdup(line);
      toLowerStr(lineToCompare);
    }

    if (strstr(lineToCompare, keyword) != NULL)
    {
      fputs(line, output);
    }
    if (!isCaseSensitive)
    {
      free(lineToCompare);
    }
  }

  if (ferror(input))
  {
    fprintf(stderr, "%s error reading input ", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }
  if (ferror(output))
  {
    fprintf(stderr, "%s error writing to output", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }
}