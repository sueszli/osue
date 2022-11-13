
/**
* \file ispalindrom.c
* \author Sepehr Shirali, 01528967
* \date 14.11.2021
* The module determines wether the lines of the input files are palindrom.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define IS_WHITE_SPACE(c) ((c == ' ') || (c == '\t'))

int termination; //< A flag that is set after the SIGINT or SIGTERM are recived.

/**
* \brief The function writes an error masseage.
* \param[in] msg The message to be displayed.
* \details It writes the message given as the argument and also the corresponding string to the errno,
*         which is set by the function that has caused the error
*/
void error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(EXIT_FAILURE);
}

/**
* \brief It deletes the whitespaces.
*
* \param[in, out] str The string whose whitespaces are to be deleted.
*
* \details The tabs and spaces in the given string are eliminated. The string itself will be
*           containing the new reduced content.
*
*/
void del_whitespaces(char *str) {
  int i;
  for(i = 0; i < strlen(str); i++) {
      if(IS_WHITE_SPACE(str[i])) {
        memmove(&str[i], &str[i+1], strlen(str)-i);
        i--;
      }
  }
}

/**
* \brief tells whether the lines of the given stream are palindrom.
*
* \param[in] in The input stream
* \param[in] flag_i The flag that shows whether the -i option is set.
* \param[in] flag_s The flag that shows whether the -s option is set.
* \param[out] out the stream to write the output into.
*
* \details The function reads the input stream line by line and checks whether it is
*             palindrom, then it writes the result in output stream given as argument.
*/
void f_is_palindrom(FILE *in, int flag_i, int flag_s, FILE *out) {
  int i, j, k;
  char *line = NULL;
  char *line_o;
  size_t line_size = 0;

  while(getline(&line, &line_size, in) != -1 && !termination) {
    if(line[strlen(line)-1] == '\n')
      line[strlen(line)-1] = '\0';
    line_o = malloc((strlen(line)+1) * sizeof(char));
    strcpy(line_o, line);
    if(flag_i) {
      for(i = 0; i < strlen(line); i++)
        line_o[i] = tolower(line[i]);
    }
    if(flag_s) {
      del_whitespaces(line_o);
    }

    i = 0, j = strlen(line_o)-1, k = 0;
    while(i < j) {
      k += (line_o[i] != line_o[j]);
      i++, j--;
    }

    if(!k)
      fprintf(out, "%s is palindrom\n", line);
    else
      fprintf(out, "%s is not palindrom\n", line);

    fflush(out);
    free(line_o);
  }
  free(line);
}

void handle_signal(int signal) {
    termination = 1;
}

int main(int argc, char *argv[]) {
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = handle_signal;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  termination = 0;

  int flag_i = 0, flag_s = 0;
  FILE *in;
  FILE *out = stdout;
  int c;

  while((c = getopt(argc, argv, "iso:")) != -1) {
    switch (c) {
      case 'i':
        flag_i = 1;
        break;
      case 's':
        flag_s = 1;
        break;
      case 'o':
        if((out = fopen(optarg, "w")) == NULL)
          error("fopen failed");
        break;
      default:
        fprintf(stderr, "usage: ispalindrom [-i] [-s] [-o outfile] [file...]\n");
        return EXIT_FAILURE;
    }
  }
  argc -= optind;
  argv += optind;

  if(!argc) {
    f_is_palindrom(stdin, flag_i, flag_s, out);
    return 0;
  }

  while(argc > 0) {
    if(!(in = fopen(argv[0], "r")))
      error("fopen failed");
    f_is_palindrom(in, flag_i, flag_s, out);
    if(fclose(in) == EOF)
      error("fclose failed");
    argc--, argv++;
  }

  if(fclose(out) == EOF)
    error("fclose failed");

  return 0;
}
