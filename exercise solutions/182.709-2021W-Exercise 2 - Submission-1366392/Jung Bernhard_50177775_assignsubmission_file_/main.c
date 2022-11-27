#include "myline.h"

#include <ctype.h>

#include <errno.h>

#include <getopt.h>

#include <math.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <regex.h>

#include <unistd.h>

#include <sys/wait.h>

/**
 * @file main.c
 * @author Bernhard Jung 12023965
 * @brief My implementation of the second Exercise: forkFFT
 * @details In this implementation fork and exec are being used recursivly. That means that each parent starts 2 child forks and hands them even or odd parameters.
 *          That is being done until the parent output is only one line long. Than the second part of the recursion jumps in and calculates the results within the child
 *          and hands it back to the parent. 
 * @date 2021.12.01
 */

int main(int argc, char ** argv) {
  size_t len = 0;
  ssize_t line_size = 0;

  regex_t regex_pattern;
  input_t input;
  output_t output;

  input.even = NULL;
  input.odd = NULL;
  input.length = 0;
  input.length_even = 0;
  input.length_odd = 0;
  input.number = NULL;

  output.length_even = 0;
  output.length_odd = 0;
  output.even = NULL;
  output.odd = NULL;
  output.even_imag = NULL;
  output.odd_imag = NULL;

  char * line = NULL;
  char * token = NULL;

  char key_real[2] = " ";
  char key_imag[2] = "*";

  if (regcomp( & regex_pattern, "(-|\\+)?([0-9]+)(.[0-9]*)$", REG_EXTENDED) != 0) {
    fprintf(stderr, "Error: %s!\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  line_size = getline( & line, & len, stdin);

  while (line_size > 0) //checking every line if it matches specified
  {
    if (regexec( & regex_pattern, line, 0, NULL, 0) != 0) {
      fprintf(stderr, "Error: RegEx Pattern not matching!\n");
      exit(EXIT_FAILURE);
    } else {
      input.number = (float * ) realloc(input.number, (input.length + 1) * sizeof(float));
      input.number[input.length] = strtof(line, NULL);
      input.length += 1;
    }
    line_size = getline( & line, & len, stdin);
  }

  if ((input.length % 2) != 0 && input.length > 1) {
    regfree(&regex_pattern);
    fprintf(stderr, "Error: uneven number of arguments!\n");
    exit(EXIT_FAILURE);
  }

  if (input.length >= 2) // Begin of of main input logic
  {
    for (int i = 0; i < input.length; i++) // Loop threw input
    {
      if ((i % 2) == 0) // even
      {
        input.even = (float * ) realloc(input.even, (input.length_even + 1) * sizeof(float));
        input.even[input.length_even] = input.number[i];
        input.length_even += 1;
      } else //odd
      {
        input.odd = (float * ) realloc(input.odd, (input.length_odd + 1) * sizeof(float));
        input.odd[input.length_odd] = input.number[i];
        input.length_odd += 1;
      }
    }

    //setting up pipes for even child
    int fd_parent_to_child_even[2];
    int fd_child_even_to_parent[2];

    if (pipe(fd_parent_to_child_even) == -1) {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (pipe(fd_child_even_to_parent) == -1) {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    pid_t pid_even = fork();
    switch (pid_even) //child even main fork switch
    {
    case ERROR: {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    case CHILD: //childs stdin = parents stdout
    {
      close(fd_parent_to_child_even[PIPE_WRITE]);
      close(fd_child_even_to_parent[PIPE_READ]);

      if (dup2(fd_parent_to_child_even[PIPE_READ], STDIN_FILENO) == -1) {
        fprintf(stderr, "Error: %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (dup2(fd_child_even_to_parent[PIPE_WRITE], STDOUT_FILENO) == -1) {
        fprintf(stderr, "Error: %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      close(fd_parent_to_child_even[PIPE_READ]);
      close(fd_child_even_to_parent[PIPE_WRITE]);
      execlp(argv[0], argv[0], NULL);
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    default: //PARENT   write to fd_parent_to_child_even
    {
      close(fd_parent_to_child_even[PIPE_READ]);
      close(fd_child_even_to_parent[PIPE_WRITE]);

      for (int i = 0; i < input.length_even; i++) {
        dprintf(fd_parent_to_child_even[PIPE_WRITE], "%f\n", input.even[i]);
      }
      close(fd_parent_to_child_even[PIPE_WRITE]);
      break;
    }
    }

    //setting up pipes for even odd
    int fd_parent_to_child_odd[2];
    int fd_child_odd_to_parent[2];

    if (pipe(fd_parent_to_child_odd) == -1) {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (pipe(fd_child_odd_to_parent) == -1) {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    pid_t pid_odd = fork();
    switch (pid_odd) //child odd main fork switch
    {
    case ERROR: {
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    case CHILD: //childs stdin = parents stdout
    {
      close(fd_parent_to_child_odd[PIPE_WRITE]);
      close(fd_child_odd_to_parent[PIPE_READ]);

      if (dup2(fd_parent_to_child_odd[PIPE_READ], STDIN_FILENO) == -1) {
        fprintf(stderr, "Error: %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (dup2(fd_child_odd_to_parent[PIPE_WRITE], STDOUT_FILENO) == -1) {
        fprintf(stderr, "Error: %s!\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      close(fd_parent_to_child_odd[PIPE_READ]);
      close(fd_child_odd_to_parent[PIPE_WRITE]);

      execlp(argv[0], argv[0], NULL);
      fprintf(stderr, "Error: %s!\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    default: //PARENT write to fd_parent_to_child_odd
    {
      close(fd_parent_to_child_odd[PIPE_READ]);
      close(fd_child_odd_to_parent[PIPE_WRITE]);

      for (int i = 0; i < input.length_odd; i++) {
        dprintf(fd_parent_to_child_odd[PIPE_WRITE], "%f\n", input.odd[i]);
      }
      close(fd_parent_to_child_odd[PIPE_WRITE]);
      break;
    }
    }

    int wait_var = 0;
    while (wait_var < 2) //waiting for the for childs to terminate
    {
      pid_t pid_child = wait(NULL);
      if (pid_child == pid_odd) //odd Child
      {
        if (dup2(fd_child_even_to_parent[0], STDIN_FILENO) == -1) {
          fprintf(stderr, "Error: %s!\n", strerror(errno));
          exit(EXIT_FAILURE);
        }
        close(fd_child_even_to_parent[0]);
        while (getline( & line, & len, stdin) > 0) //parsing results from childs
        {
          output.even = (float * ) realloc(output.even, (output.length_even + 1) * sizeof(float));
          output.even_imag = (float * ) realloc(output.even_imag, (output.length_even + 1) * sizeof(float));
          token = strtok(line, key_real);
          output.even[output.length_even] = strtof(token, NULL);
          token = strtok(NULL, key_imag);
          if (token != NULL) {
            output.even_imag[output.length_even] = strtof(token, NULL);
          } else {
            output.even_imag[output.length_even] = 0;
          }
          output.length_even += 1;
        }
        wait_var += 1;
      }
      if (pid_child == pid_even) //even Child
      {
        if (dup2(fd_child_odd_to_parent[PIPE_READ], STDIN_FILENO) == -1) {
          fprintf(stderr, "Error: %s!\n", strerror(errno));
          exit(EXIT_FAILURE);
        }
        close(fd_child_odd_to_parent[PIPE_READ]);
        while (getline( & line, & len, stdin) > 0) //parsing results from childs
        {
          output.odd = (float * ) realloc(output.odd, (output.length_odd + 1) * sizeof(float));
          output.odd_imag = (float * ) realloc(output.odd_imag, (output.length_odd + 1) * sizeof(float));
          token = strtok(line, key_real);
          output.odd[output.length_odd] = strtof(token, NULL);
          token = strtok(NULL, key_imag);
          if (token != NULL) {
            output.odd_imag[output.length_odd] = strtof(token, NULL);
          } else {
            output.odd_imag[output.length_odd] = 0;
          }
          output.length_odd += 1;
        }
        wait_var += 1;
      }
    }
  } else if (input.length == 1) //recursion tree -> leaf
  {
    //printf("%f 0.0*i\n", input.number[0]);
    dprintf(STDOUT_FILENO, "%f 0.0*i\n", input.number[0]);
  } else if (input.length == 0) {
    fprintf(stderr, "Error: input.length == 0 !\n");
    exit(EXIT_FAILURE);
  }

  //writing the result
  float result_real;
  float result_imag;
  for (int k = 0; k < output.length_even; k++) {
    result_real = ((cosf(-1 * ((2 * PI) / input.length) * (k)) * output.odd[k]) - (sinf(-1 * ((2 * PI) / input.length) * (k)) * output.odd_imag[k]));
    result_imag = ((cosf(-1 * ((2 * PI) / input.length) * (k)) * output.odd_imag[k]) + (sinf(-1 * ((2 * PI) / input.length) * (k)) * output.odd[k]));

    result_real = output.even[k] + result_real;
    result_imag = output.even_imag[k] + result_imag;
    //printf("%f %f*i\n", result_real, result_imag);
    dprintf(STDOUT_FILENO, "%f %f*i\n", result_real, result_imag);
  }

  for (int k = 0; k < output.length_odd; k++) {
    result_real = ((cosf(-1 * ((2 * PI) / input.length) * (k)) * output.odd[k]) - (sinf(-1 * ((2 * PI) / input.length) * (k)) * output.odd_imag[k]));
    result_imag = ((cosf(-1 * ((2 * PI) / input.length) * (k)) * output.odd_imag[k]) + (sinf(-1 * ((2 * PI) / input.length) * (k)) * output.odd[k]));

    result_real = output.even[k] - result_real;
    result_imag = output.even_imag[k] - result_imag;
    //printf("%f %f*i\n", result_real, result_imag);
    dprintf(STDOUT_FILENO, "%f %f*i\n", result_real, result_imag);
  }

  //free resources
  free(input.number);
  free(input.even);
  free(input.odd);
  free(output.odd);
  free(output.odd_imag);
  free(output.even);
  free(output.even_imag);
  free(line);
  regfree( & regex_pattern);
  return EXIT_SUCCESS;
}