/**
 * @file mygrep.c
 * @author Nursultan Imanov 01528474
 * @date 01.11.2021
 *
 * @brief Main program module
 * @details The program mygrep reads files line by line 
 * and for each line checks whether it contains the search term keyword.
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

const char *progname = "mygrep";

/**
 * @brief structure to save program options
 * @details 
 *  i_flag - to store "ignore case sensitivity" option
 * 
 *  keyword - to store search term
 * 
 *  output_file - to store pointer to the output stream (can be stdout or output file)
 */
typedef struct poptions_s
{
  int i_flag;
  char *keyword;
  FILE *output_file;
} poptions_t;

/**
 * @brief documents correct calling interface
 * @details only -i -o options are allowed,
 * otherwise the program has to terminate
 * with the usage message containing the program name and the correct calling syntax.
 */
static void usage(void)
{
  fprintf(stderr, "USAGE: %s [-i] [-o outfile] keyword [file...]", progname);
  exit(EXIT_FAILURE);
}

/**
 * @brief saves programm arguments to the structure
 * @details 
 *  only 2 options are supported by the program(-i -o),
 *  otherwise the program has to terminate with the usage message,
 *  containing the program name and the correct calling syntax
 * @param argc The argument counter
 * @param argv The argument vector
 * @param popt program options, which provide program flags and output stream
 * @return the index of the first positional argument
 */
static int process_arguments(int argc, char **argv, poptions_t *popt)
{
  int c;
  while ((c = getopt(argc, argv, "io:")) != -1)
  {
    switch (c)
    {
    case 'i':
      popt->i_flag = 1;
      break;
    case 'o':
      popt->output_file = fopen(optarg, "w");
      break;
    default:
      usage();
    }
  }

  return optind;
}

/**
 * @brief converts string to lower case
 * @details 
 * @param str string to be changed
 */
static void string_tolower(char *str)
{
  while (*str)
  {
    *str = tolower(*str);
    ++str;
  }
}

/**
 * @brief read stream(stdin or input file) and find lines with the search term
 * @details 
 *  reads input stream line by line and
 *  for each line checks whether it contains the search term keyword.
 * @param stream input stream to read from
 * @param popt program options, which provide program flags and output stream
 */
static void read_stream(FILE *stream, poptions_t *popt)
{
  char *line = NULL;
  size_t size = 0;
  while (getline(&line, &size, stream) != -1)
  {
    char *copy = malloc(sizeof(char) * strlen(line));
    if (copy == NULL)
    {
      fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", progname, strerror(errno));
      exit(EXIT_FAILURE);
    }

    strcpy(copy, line);
    if (popt->i_flag)
    {
      string_tolower(copy);
    }

    if (strstr(copy, popt->keyword))
    {
      fprintf(popt->output_file, "%s", line);
    }

    free(copy);
  }
  fprintf(popt->output_file, "\n");
  free(line);
  fclose(stream);
}

/**
 * @brief The main block of the program
 * @details 
 *  program should process arguments at first and throw an error,
 *  if provided options are not allowed.
 *  Then it should read input stream(stdin or input file) and find lines with the search term.
 * @param argc The argument counter
 * @param argv The argument vector
 */
int main(int argc, char **argv)
{
  poptions_t poptions = {0, "", stdout};
  int i = process_arguments(argc, argv, &poptions);
  if (i == argc)
  {
    usage();
  }
  else
  {
    poptions.keyword = argv[i];
    i++;
  }
  if (i == argc)
  {
    read_stream(stdin, &poptions);
  }
  else
  {
    while (i < argc)
    {
      FILE *input_file;
      if ((input_file = fopen(argv[i], "r")) == NULL)
      {
        fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", progname, strerror(errno));
        exit(EXIT_FAILURE);
      }
      read_stream(input_file, &poptions);
      i++;
    }
  }

  if (poptions.output_file != NULL)
  {
    fclose(poptions.output_file);
  }

  return 0;
}
