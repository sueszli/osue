/**
 * @file forkFFT.c
 * @author Gabriel Kitzberger (e12024014@student.tuwien.ac.at)
 * @brief Implements the Cooley-Tukey Fast Fourier Transform algorithm
 * @version 0.1
 * @date 2021-12-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  float r;
  float i;
} Complex;

/**
 * @brief check if the string is a float, if so return the float, otherwise exit
 *
 * @param str the string which should be convertable to float via sscanf
 * @return float parsed from the string
 */
static float to_float(char *str) {
  float value;
  int err;
  err = sscanf(str, "%f", &value);
  if (err <= 0) {
    fprintf(stderr, "Invalid argument\n");
    _exit(EXIT_FAILURE);
  }
  return value;
}

int main(void) {
  size_t len = 0;
  char *first_line, *second_line;
  getline(&first_line, &len, stdin);

  int c;
  if ((c = getline(&second_line, &len, stdin)) == EOF) {
    // Complex number with: 'real imaginary'
    fprintf(stdout, "%f %f*i\n", to_float(first_line), 0.0);
    fclose(stdout);
    _exit(EXIT_SUCCESS);
  }

  // Pipes
  int pipe_to_child1[2];
  int pipe_to_child2[2];
  int pipe_from_child1[2];
  int pipe_from_child2[2];
  if (pipe(pipe_to_child1) != 0 || pipe(pipe_to_child2) != 0 ||
      pipe(pipe_from_child1) != 0 || pipe(pipe_from_child2) != 0) {
    fprintf(stderr, "Pipe Error\n");
    _exit(EXIT_FAILURE);
  }

  // Create two children
  pid_t c1_pid, c2_pid;
  c1_pid = fork();
  if (c1_pid) c2_pid = fork();  // If parent
  if (c1_pid == -1 || c2_pid == -1) {
    fprintf(stderr, "Fork Error\n");
    _exit(EXIT_FAILURE);
  }

  if (c1_pid == 0) {
    // Child 1
    // Close all pipes from other child
    close(pipe_to_child2[0]);
    close(pipe_to_child2[1]);
    close(pipe_from_child2[0]);
    close(pipe_from_child2[1]);

    close(pipe_to_child1[1]);
    close(pipe_from_child1[0]);

    // Change pipes to stdin/out and then close them
    dup2(pipe_to_child1[0], STDIN_FILENO);
    dup2(pipe_from_child1[1], STDOUT_FILENO);
    close(pipe_to_child1[0]);
    close(pipe_from_child1[1]);

    execlp("./forkFFT", "forkFFT", NULL);
  } else if (c2_pid == 0) {
    // Child 2
    // Close all pipes from other child
    close(pipe_to_child1[0]);
    close(pipe_to_child1[1]);
    close(pipe_from_child1[0]);
    close(pipe_from_child1[1]);

    close(pipe_to_child2[1]);
    close(pipe_from_child2[0]);

    // Change pipes to stdin/out and then close them
    dup2(pipe_to_child2[0], STDIN_FILENO);
    dup2(pipe_from_child2[1], STDOUT_FILENO);
    close(pipe_to_child2[0]);
    close(pipe_from_child2[1]);

    execlp("./forkFFT", "forkFFT", NULL);
  } else {
    // Parent
    close(pipe_to_child1[0]);
    close(pipe_to_child2[0]);
    close(pipe_from_child1[1]);
    close(pipe_from_child2[1]);

    // Send to child
    write(pipe_to_child1[1], first_line, strlen(first_line));
    write(pipe_to_child2[1], second_line, strlen(second_line));
    free(first_line);
    free(second_line);

    int c;
    int lines = 2;
    char *other_lines;
    while ((c = getline(&other_lines, &len, stdin)) != EOF) {
      if (lines % 2 == 0) {
        write(pipe_to_child1[1], other_lines, strlen(other_lines));
      } else {
        write(pipe_to_child2[1], other_lines, strlen(other_lines));
      }
      lines++;
    }
    close(pipe_to_child1[1]);
    close(pipe_to_child2[1]);
    free(other_lines);

    // exit if odd nums
    if (lines % 2 != 0) {
      fprintf(stderr, "Array has odd number of elements\n");
      _exit(EXIT_FAILURE);
    }

    int status;
    while (wait(&status) > 0) {
      if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Error in child\n");
        _exit(EXIT_FAILURE);
      }
    }

    FILE *from_child1 = fdopen(pipe_from_child1[0], "r");
    FILE *from_child2 = fdopen(pipe_from_child2[0], "r");
    Complex *R_E = malloc(sizeof(Complex) * lines / 2);
    Complex *R_O = malloc(sizeof(Complex) * lines / 2);
    char *p_E, *p_O, *pend1, *pend2;
    for (size_t i = 0; i < lines / 2; i++) {
      getline(&p_E, &len, from_child1);
      getline(&p_O, &len, from_child2);

      // Parse to floats
      Complex c1 = {.r = strtof(p_E, &pend1), .i = strtof(pend1, NULL)};
      Complex c2 = {.r = strtof(p_O, &pend2), .i = strtof(pend2, NULL)};

      R_E[i] = c1;
      R_O[i] = c2;
    }
    fclose(from_child1);
    fclose(from_child2);
    close(pipe_from_child1[0]);
    close(pipe_from_child2[0]);
    free(p_E);
    free(p_O);

    size_t k = 0;
    for (size_t i = 0; i < lines; i++) {
      k = i < lines / 2 ? i : i - lines / 2;
      float a = cosf(-((2.0 * M_PI) / lines) * k);
      float b = sinf(-((2.0 * M_PI) / lines) * k);
      float c = R_O[k].r;
      float d = R_O[k].i;

      float real = (a * c - b * d);
      float imaginary = (a * d + b * c);
      if (i < lines / 2) {
        fprintf(stdout, "%f %f*i\n", R_E[k].r + real, R_E[k].i + imaginary);
      } else {
        fprintf(stdout, "%f %f*i\n", R_E[k].r - real, R_E[k].i - imaginary);
      }
    }
    free(R_E);
    free(R_O);

    fclose(stdout);
    _exit(EXIT_SUCCESS);
  }
}