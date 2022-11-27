#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file main.c
 * @author Bernhard Jung 12023965
 * @brief My implementation of the first exercise, 1A: MyGrep - A Program to check if certain keyword are within a line.
 * @details My Programm replicates on a small scale the functions of the unix Terminl progam "grep".
 *          It achieves that by performing a few key steps:
 *          1. At first the inputarguments are parsed, if all needed parameters are present.
 *          2. Flags are being set for further execution.
 *          3. The input files or the console is read, and it is being checked of they contain the specified keyword.
 *          4. Matching lines are being written to the console or output file if specified.
 * 
 *          The Syntax of inputarguments got interpreted as follows / Assumptions were made:
 *          1. The Order in which -i and -o are used is irrelevant
 *          2. Only the first use of -o is being considered. Secondary uses are being ignored.
 *          3. If -o is used an Outputfile must be specified.
 *          4. The first argument not related to -o or -i is taken as Keyword
 *          5. All later arguments not related to -o or -i are taken as an Inputfile(s).
 * @date 2021.11.14
 */

int main(int argc, char ** argv) {
  char c;

  char * output_file = NULL;
  char * keyword = NULL;

  int input_from = -1;
  int param_cnt = 0;
  int o_flag = 0;
  int i_flag = 0;

  char * needle;
  FILE * output_fp;

  while ((c = getopt(argc, argv, "io:")) != -1) { // Parsing Input arguments
    switch (c) {
    case 'i': {
      param_cnt++;
      i_flag = 1;
      break;
    }
    case 'o': {
      param_cnt += 2;
      if (o_flag == 0) {
        output_file = argv[param_cnt];
      }
      o_flag = 1;
      break;
    }
    default: {
      fprintf(stderr, "%s -o missing Argument for Output-file!\n", argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
    }
  }

  if (argc > (param_cnt + 1)) { //check if a Keyword is given
    keyword = argv[param_cnt + 1];
  } else {
    fprintf(stderr, "%s No Keyword found!\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argc > (param_cnt + 2)) { //check if an Inputfile is given
    input_from = param_cnt + 2;
  }

  if (o_flag) {
    output_fp = fopen(output_file, "a");
    if (output_file == NULL) {
      fprintf(stderr, "%s Error @ fopen(output_file) ! Errno: %s\n", argv[0], strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if (input_from != -1) { // TRUE if input files are specified, FALSE if input from console
    for (int i = input_from; i < argc; i++) { // loop threw all specified input files
      FILE * input_fp = fopen(argv[i], "r");
      if (input_fp != NULL) { //check if the specified input file even exists
        size_t buff_size = 0;
        char * file_string = NULL;
        while (getline( & file_string, & buff_size, input_fp) != -1) { // reading the inputfile line by line
          if (i_flag == 1) { // Check if the filesearch should be case sensitive
            char * i_file_string = (char * ) malloc(strlen(file_string) * sizeof(char * ));
            strcpy(i_file_string, file_string);

            char * i_keyword = (char * ) malloc(strlen(keyword) * sizeof(char * ));
            strcpy(i_keyword, keyword);

            for (int i = 0; i < strlen(file_string); i++) {
              i_file_string[i] = tolower(file_string[i]);
            }
            for (int i = 0; i < strlen(keyword); i++) {
              i_keyword[i] = tolower(keyword[i]);
            }

            needle = strstr(i_file_string, i_keyword);
            free(i_file_string);
            free(i_keyword);
          } else {
            needle = strstr(file_string, keyword);
          }
          if (needle != 0) { //If keyword was found
            if (o_flag == 1) {
              if (fputs(file_string, output_fp) < 0) {
                fprintf(stderr, "%s Error @ puts(file_string, output_fp) ! Errno: %s\n", argv[0], strerror(errno));
              }
            } else {
              printf("%s", file_string);
            }
          }
        }
        fclose(input_fp);
        free(file_string);
      } else {
        fprintf(stderr, "%s Error @ fopen(input_fp) ! Errno: %s: %s\n", argv[0], strerror(errno), argv[i]);
      }
    }
  } else { // Console input Path
    size_t buff_size = 0;
    char * file_string = NULL;
    getline( & file_string, & buff_size, stdin);
    needle = strstr(file_string, keyword);
    if (i_flag == 1) { // Check if the search should be case sensitive
      char * i_file_string = (char * ) malloc(strlen(file_string) * sizeof(char * ));
      strcpy(i_file_string, file_string);

      char * i_keyword = (char * ) malloc(strlen(keyword) * sizeof(char * ));
      strcpy(i_keyword, keyword);

      for (int i = 0; i < strlen(file_string); i++) {
        i_file_string[i] = tolower(file_string[i]);
      }
      for (int i = 0; i < strlen(keyword); i++) {
        i_keyword[i] = tolower(keyword[i]);
      }

      needle = strstr(i_file_string, i_keyword);
      free(i_file_string);
      free(i_keyword);
    } else {
      needle = strstr(file_string, keyword);
    }
    if (needle != 0) { //If keyword was found
      if (o_flag == 1) {
        if (fputs(file_string, output_fp) < 0) {
          fprintf(stderr, "%s Error @ puts(file_string, output_fp) ! Errno: %s\n", argv[0], strerror(errno));
        }
      } else {
        printf("%s", file_string);
      }
    }
    free(file_string);
  }

  if (o_flag) {
    fclose(output_fp);
  }
  return EXIT_SUCCESS;
}