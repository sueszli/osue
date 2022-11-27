/**
 * @author
 * Yoan Ivanov
 * e12024745@student.tuwien.ac.at
 * 
 * @brief
 * Does the processing of the grep implementation
 * 
 * @date
 * 10.11.2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "mygrep.h"



char *prog_name;
bool ignore_case;
FILE *output_file;
char *keyword;


/**
 * @brief
 * Main Processing Function
 * 
 * @details
 * Parses commands from console and 
 * calls the necessary functions to execute
 * 
 * @return
 * Returns EXIT_SUCCESS if everything goes well
 * or calls usage() if something goes wrong
 */
int main(int argc, char **argv)
{
  prog_name = argv[0];
  if (argc < 2)
    usage();

  
  output_file = stdout;

  int c;
  while((c = getopt(argc, argv, "io:")) != -1)
  {
    switch(c)
    {
      case 'i': ignore_case = true;
        break;

      case 'o':
        output_file = fopen(optarg, "w");
        break;

      default: /* undefined argument */
        usage();
    }
  }

  keyword = malloc((strlen(argv[optind]) + 1) * sizeof(char));
  if (keyword == NULL)
    usage();
  strcpy(keyword, argv[optind]);


  int num_files = argc - (optind + 1);
  if (num_files == 0)
    console_compare();
  else
  {
    char **args = malloc(num_files * sizeof(char*));
    for (int i = 0; i < num_files; ++i)
      args[i] = argv[optind + 1 + i];

    file_compare(num_files, args);

    free(args);
  }

  free(keyword);

  exit(EXIT_SUCCESS);
}


/**
 * @brief
 * Exits the program
 * 
 * @details
 * Exits the program with a failure and
 * prints out the correct way to use it
 * 
 * @return
 * Returns EXIT_FAILURE
 */
void usage(void)
{
  fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...] \n", prog_name);
  exit(EXIT_FAILURE);
}

/**
 * @brief
 * Doing grep with input from console
 * 
 * @details
 * Setting up strings to operate on and checking for 
 * word occurrences in the input
 * 
 * @return
 * Returns nothing
 */
void console_compare(void)
{
  // setting up strings with a max length MAX_CONSOLE_INPUT
  char input[MAX_CONSOLE_INPUT], processed[MAX_CONSOLE_INPUT];

  // if the input is larger than what our string can hold
  // print a notice that they should put in something shorter
  if (fgets(input, MAX_CONSOLE_INPUT, stdin) == NULL)
  {
    fprintf(stderr, "More than %i characters entered; cannot process", MAX_CONSOLE_INPUT);
    return;
  }

  // are we ignoring cases
  if(ignore_case)
  {
    for (int i = 0; i < strlen(keyword); ++i)
      keyword[i] = tolower(keyword[i]);

    for (int i = 0; i < MAX_CONSOLE_INPUT; ++i)
      processed[i] = tolower(input[i]);

    // checking for occurrences
    if (strstr(processed, keyword) == NULL)
      return;
  }
  else
  {
    // checking for occurrences
    if (strstr(input, keyword) == NULL)
      return;
  }



  // if no output file is specified, print to console
  if (output_file == NULL)
  {
    fprintf(stdout,"%s", input);
    return;
  }

  fputs(input, output_file);
  fclose(output_file);
}

/**
 * @brief
 * Doing grep with input from files
 * 
 * @details
 * Setting up strings to operate on, looping through all the files
 * and checking for word occurrences in their contents
 * 
 * @return
 * Returns nothing
 */
void file_compare(int num_files, char **input_files)
{
  // setting up strings with a max length MAX_CONSOLE_INPUT
  char input[MAX_CONSOLE_INPUT];

  // are we ignoring cases
  if(ignore_case)
  {
    char processed[MAX_CONSOLE_INPUT];

    // processing keyword
    for (int i = 0; i < strlen(keyword); ++i)
      keyword[i] = tolower(keyword[i]);

    // looping through input files
    for (int i = 0; i < num_files; ++i)
    {
      // if the input file doesn't exist, skip it
      FILE *fp = fopen(input_files[i], "r");
      if (fp == NULL)
        continue;

      while(fgets(input, MAX_CONSOLE_INPUT, fp) != NULL)
      {
        if (strlen(input) <= 1)
          continue;
        // processing line
        for (int j = 0; j < strlen(input); ++j)
          processed[j] = tolower(input[j]);
        //processed[line_length + 1] = '\0';

        // if processed keyword exists in processed line, we write the original line to output
        if (strstr(processed, keyword) != NULL)
          fprintf(output_file, "%s", input);
      }

      // closing input file
      fclose(fp);
    }
  }
  else
  {
    // looping through input files
    for (int i = 0; i < num_files; ++i)
    {
      // if the input file doesn't exist, skip it
      FILE *fp = fopen(input_files[i], "r");
      if (fp == NULL)
        continue;

      while(fgets(input, MAX_CONSOLE_INPUT, fp) != NULL)
      {
        if (strlen(input) <= 1)
          continue;
        // if keyword exists in line, we write to output
        if (strstr(input, keyword) != NULL)
          fprintf(output_file, "%s", input);
      }
      
      // closing input file
      fclose(fp);
    }
  }

  fclose(output_file);
}