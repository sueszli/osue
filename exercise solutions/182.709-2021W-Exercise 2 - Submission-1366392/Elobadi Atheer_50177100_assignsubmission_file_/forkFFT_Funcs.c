/**
 * @file forkFFT_Funcs.c
 * @author Atheer Elobadi (e01049225@student.tuwien.ac.at)
 * @brief Contains the function definitions which are being used in the program. 
 * @date 11.12.2021
 * 
 */
#include "forkFFT.h"

void usage(char *program_name)
{
    fprintf(stderr, "USAGE: %s\n", program_name);
    free(program_name);
    exit(EXIT_FAILURE);
}

void strtoc(complex_t *c, char *str)
{
    char *endptr = NULL;

    c->real = strtof(str, &endptr);
      if (c->real == HUGE_VALF || c->real == HUGE_VALL )
      {
        exit(EXIT_FAILURE);
      }

    c->img = strtof(endptr, &endptr);
          if (c->img == HUGE_VALF || c->img == HUGE_VALL)
          {
        exit(EXIT_FAILURE);   
    }
}

void butterfly(complex_t *result, complex_t *c_even, complex_t *c_odd, int count, FILE *fd_up[])
{
    float re, im;
    int i;
    size_t len;
    for (i = 0; i < count / 2; i++)
    {
        re = cos(-2*PI*i/count);
        im = sin(-2*PI*i/count);


            
        char *result_odd = NULL;
        if (getline(&result_odd, &len, fd_up[CHILD_EVEN]) == -1)
        {
            exit(EXIT_FAILURE);
        }

        strtoc(c_odd, result_odd);
        free(result_odd);
        char *result_even = NULL;
        if (getline(&result_even, &len, fd_up[CHILD_ODD]) == -1)
        {
            exit(EXIT_FAILURE);
        }
        strtoc(c_even, result_even);
        free(result_even);
        
        result[i].real = c_even->real + re * c_odd->real - im * c_odd->img;
        result[i].img = c_even->img + re * c_odd->img + im * c_odd->real;

        result[i + count / 2].real = c_even->real - re * c_odd->real + im * c_odd->img;
        result[i + count / 2].img = c_even->img - re * c_odd->img - im * c_odd->real;
    }

}

int init_pipes(int down[2], int up[2])
{
    if (close(down[PIPE_WRITE_END]) == -1)
        return PIPE_FAILURE;
    if (dup2(down[PIPE_READ_END], STDIN_FILENO) == -1)
        return PIPE_FAILURE;
    if (close(down[PIPE_READ_END]) == -1)
        return PIPE_FAILURE;


    if (close(up[PIPE_READ_END]) == -1)
        return PIPE_FAILURE;
    if (dup2(up[PIPE_WRITE_END], STDOUT_FILENO) == -1)
        return PIPE_FAILURE;
    if (close(up[PIPE_WRITE_END]) == -1)
        return PIPE_FAILURE;

    return PIPE_SUCCESS;
}
