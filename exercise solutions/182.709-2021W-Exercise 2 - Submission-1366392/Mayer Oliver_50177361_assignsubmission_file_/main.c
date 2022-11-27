/**
 * @file main.c
 * @author Mayer Oliver, 12023147
 * @brief Calculate two integers of any size
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "revstr.h"
#include "unrestrictedint.h"

/**
 * @brief Holds the name of this program
 * 
 */
char *g_myprog;

/**
 * @brief Prints the synopsis for the command and exits
 * 
 */
void usage(void);

void usage(void)
{
    fprintf(stderr, "%s takes no arguments!\n", g_myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief close given pipes
 * 
 * @param p1 
 * @param p2 
 * @param p3 
 * @param p4 
 */
void close_pipes(int *p1, int *p2, int *p3, int *p4);
void close_pipes(int *p1, int *p2, int *p3, int *p4)
{
    if (close(p1[0]) == -1)
    {
        fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(p1[1]) == -1)
    {
        fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(p2[0]) == -1)
    {
        fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(p2[1]) == -1)
    {
        fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (p3 != NULL)
    {
        if (close(p3[0]) == -1)
        {
            fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(p3[1]) == -1)
        {
            fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if (p4 != NULL)
    {
        if (close(p4[0]) == -1)
        {
            fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(p4[1]) == -1)
        {
            fprintf(stderr, "Failed to close pipe: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    //--------------------- Argument parsing
    g_myprog = argv[0];
    if (argc > 1)
    {
        usage();
    }

    int inp_size1;
    int inp_size2;
    int signum1;
    int signum2;

    int pipe_write1[2]; //Ah · Bh · 16^n
    int pipe_read1[2];

    int pipe_write2[2]; //Ah · Bl· 16^n/2
    int pipe_read2[2];

    int pipe_write3[2]; //Al · Bh· 16^n/2
    int pipe_read3[2];

    int pipe_write4[2]; //Al · Bl
    int pipe_read4[2];

    //--------------------- open all pipes
    if (pipe(pipe_write1) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_read1) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_write2) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_read2) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_write3) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_read3) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_read4) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipe_write4) == -1)
    {
        fprintf(stderr, "Failed to create pipe!\n");
        exit(EXIT_FAILURE);
    }

    pid_t first_term;
    pid_t second_term;
    pid_t third_term;
    pid_t fourth_term;

    //--------------------- read first input
    unsigned char *temp = NULL;
    temp = parse_Int_from_String(&inp_size1, &signum1);
    if (temp == NULL)
    {
        fprintf(stderr, "Failed reading line!\n");
        exit(EXIT_FAILURE);
    }

    int number_size1 = inp_size1 - 1;
    unsigned char number1[number_size1];
    memcpy(number1, temp, sizeof(unsigned char) * (number_size1));
    free(temp);

    //--------------------- fork if needed
    int all_childs_terminated = 0;
    if (number_size1 > 1)
    {
        fflush(stdout);
        first_term = fork();
        if (first_term == 0)
        {

            if (dup2(pipe_read1[1], STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }

            if (dup2(pipe_write1[0], STDIN_FILENO) == -1)
            {
                fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            close_pipes(pipe_read2, pipe_read3, pipe_read4, pipe_read1);
            close_pipes(pipe_write4, pipe_write2, pipe_write3, pipe_write1);

            if (execlp("./intmul", "./intmul", NULL) == -1)
            {
                fprintf(stderr, "Failed to exec: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else if (first_term == -1)
        {
            fprintf(stderr, "Fork failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        else
        {
            second_term = fork();
            if (second_term == 0)
            {
                if (dup2(pipe_read2[1], STDOUT_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if (dup2(pipe_write2[0], STDIN_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                close_pipes(pipe_read2, pipe_read3, pipe_read4, pipe_read1);
                close_pipes(pipe_write4, pipe_write2, pipe_write3, pipe_write1);
                if (execlp("./intmul", "./intmul", NULL) == -1)
                {
                    fprintf(stderr, "Failed to exec: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            else if (second_term == -1)
            {
                all_childs_terminated++;
                fprintf(stderr, "Fork failed: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else
            {
                third_term = fork();
                if (third_term == 0)
                {
                    if (dup2(pipe_read3[1], STDOUT_FILENO) == -1)
                    {
                        fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(pipe_write3[0], STDIN_FILENO) == -1)
                    {
                        fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    close_pipes(pipe_read2, pipe_read3, pipe_read4, pipe_read1);
                    close_pipes(pipe_write4, pipe_write2, pipe_write3, pipe_write1);
                    if (execlp("./intmul", "./intmul", NULL) == -1)
                    {
                        fprintf(stderr, "Failed to exec: %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else if (third_term == -1)
                {
                    all_childs_terminated = 2;
                    fprintf(stderr, "Fork failed: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                else
                {
                    fourth_term = fork();
                    if (fourth_term == 0)
                    {
                        if (dup2(pipe_read4[1], STDOUT_FILENO) == -1)
                        {
                            fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                            exit(EXIT_FAILURE);
                        }

                        if (dup2(pipe_write4[0], STDIN_FILENO) == -1)
                        {
                            fprintf(stderr, "Failed to dup2 filedescriptors: %s", strerror(errno));
                            exit(EXIT_FAILURE);
                        }

                        close_pipes(pipe_read2, pipe_read3, pipe_read4, pipe_read1);
                        close_pipes(pipe_write4, pipe_write2, pipe_write3, pipe_write1);
                        if (execlp("./intmul", "./intmul", NULL) == -1)
                        {
                            fprintf(stderr, "Failed to exec: %s", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    }
                    else if (fourth_term == -1)
                    {
                        all_childs_terminated = 3;
                        fprintf(stderr, "Fork failed: %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }

    //--------------------- write first input to child-processes
    int h_size = inp_size1 / 2;
    unsigned char h_number1[h_size];
    unsigned char l_number1[h_size];

    memcpy(h_number1, (number1 + h_size), sizeof(unsigned char) * h_size);
    memcpy(l_number1, number1, sizeof(unsigned char) * h_size);

    for (int i = h_size - 1; i >= 0; i--)
    {
        char temp1[9], temp2[9];
        sprintf(temp1, "%x", h_number1[i]);
        sprintf(temp2, "%x", l_number1[i]);
        write(pipe_write1[1], temp1, 1);
        write(pipe_write2[1], temp1, 1);
        write(pipe_write3[1], temp2, 1);
        write(pipe_write4[1], temp2, 1);
    }
    write(pipe_write1[1], "\n", 1);
    write(pipe_write2[1], "\n", 1);
    write(pipe_write3[1], "\n", 1);
    write(pipe_write4[1], "\n", 1);

    //--------------------- read second input
    temp = parse_Int_from_String(&inp_size2, &signum2);
    if (temp == NULL)
    {
        fprintf(stderr, "Failed reading line!\n");
        exit(EXIT_FAILURE);
    }

    int number_size2 = inp_size2 - 1;
    unsigned char number2[number_size2];
    memcpy(number2, temp, sizeof(unsigned char) * (number_size2));
    free(temp);

    //--------------------- check sizes of inputs
    if (((inp_size2 - 1) % 2 != 0) && (inp_size2 != 2))
    {
        fprintf(stderr, "Number of digits is not even!\n");
        if (inp_size1 > 1)
        {
            write(pipe_write1[1], "0\n", 2);
            write(pipe_write2[1], "0\n", 2);
            write(pipe_write3[1], "0\n", 2);
            write(pipe_write4[1], "0\n", 2);
        }

        exit(EXIT_FAILURE);
    }
    if (inp_size2 != inp_size1)
    {
        fprintf(stderr, "Different sizes for numbers not allowed!\n");
        if (inp_size1 > 1)
        {
            write(pipe_write1[1], "0\n", 2);
            write(pipe_write2[1], "0\n", 2);
            write(pipe_write3[1], "0\n", 2);
            write(pipe_write4[1], "0\n", 2);
        }
        exit(EXIT_FAILURE);
    }

    //--------------------- calculate result instant if numbers are one digit
    if (number_size1 == 1)
    {
        unsigned char tmp = number1[0] * number2[0];
        if (tmp > 15)
        {
            unsigned char result[2] = {(tmp & 0x0F), (tmp >> 4)};
            fprintf(stdout, "%x%x\n", result[1], result[0]);
        }
        else
        {
            fprintf(stdout, "0%x\n", tmp);
        }
    }
    //--------------------- calculate using algorithm if numbers have more digits
    else
    {
        //--------------------- close not needed pipes in parent
        close(pipe_write1[0]);
        close(pipe_write2[0]);
        close(pipe_write3[0]);
        close(pipe_write4[0]);

        close(pipe_read1[1]);
        close(pipe_read2[1]);
        close(pipe_read3[1]);
        close(pipe_read4[1]);

        //--------------------- write second input to child processes
        unsigned char h_number2[h_size];
        unsigned char l_number2[h_size];

        memcpy(h_number2, (number2 + h_size), sizeof(unsigned char) * h_size);
        memcpy(l_number2, number2, sizeof(unsigned char) * h_size);

        for (int i = h_size - 1; i >= 0; i--)
        {
            char temp1[9], temp2[9];
            sprintf(temp1, "%x", h_number2[i]);
            sprintf(temp2, "%x", l_number2[i]);
            write(pipe_write1[1], temp1, 1);
            write(pipe_write2[1], temp2, 1);
            write(pipe_write3[1], temp1, 1);
            write(pipe_write4[1], temp2, 1);
        }
        write(pipe_write1[1], "\n", 1);
        write(pipe_write2[1], "\n", 1);
        write(pipe_write3[1], "\n", 1);
        write(pipe_write4[1], "\n", 1);

        //--------------------- value of every term
        unsigned char first_half[number_size1 + number_size1];
        unsigned char second_half[number_size1 + (number_size1 / 2)];
        unsigned char third_half[number_size1 + (number_size1 / 2)];
        unsigned char fourth_half[number_size1];

        int size1;
        int size2;
        int size3;
        int size4;

        pid_t curr_child;
        int curr_status;
        int fork_failed = all_childs_terminated;

        //--------------------- wait for termination of all childs and calculate value for every term
        while ((all_childs_terminated < 4) && (curr_child = wait(&curr_status)))
        {
            if (curr_child == first_term) //Ah · Bh · 16^n
            {
                dup2(pipe_read1[0], STDIN_FILENO);
                close(pipe_read1[0]);

                size1 = number_size1;

                int result_size = size1 * 2;

                for (int i = 0; i < result_size; i++)
                {
                    first_half[i] = 0;
                }

                for (int i = result_size - 1; i >= size1; i--)
                {
                    char tmp = '0';
                    if (read(STDIN_FILENO, &tmp, 1) == -1)
                    {
                        fprintf(stderr, "Failed reading from pipe!");
                        tmp = '0';
                    }
                    char test[] = {tmp, '\0'};
                    first_half[i] = (unsigned char)strtol(test, NULL, 16);
                }

                size1 = result_size;
            }
            else if (curr_child == second_term) //Ah · Bl· 16^n/2
            {
                dup2(pipe_read2[0], STDIN_FILENO);
                close(pipe_read2[0]);

                size2 = number_size1;

                int offset = (number_size1 / 2);
                int result_size = size2 + offset;

                for (int i = 0; i < result_size; i++)
                {
                    second_half[i] = 0;
                }

                for (int i = result_size - 1; i >= size2 - offset; i--)
                {
                    char tmp = '0';
                    if (read(STDIN_FILENO, &tmp, 1) == -1)
                    {
                        fprintf(stderr, "Failed reading from pipe!");
                        tmp = '0';
                    }
                    char test[] = {tmp, '\0'};
                    second_half[i] = (unsigned char)strtol(test, NULL, 16);
                }

                size2 = result_size;
            }
            else if (curr_child == third_term) //Al · Bh· 16^n/2
            {
                dup2(pipe_read3[0], STDIN_FILENO);
                close(pipe_read3[0]);

                size3 = number_size1;

                int offset = (number_size1 / 2);
                int result_size = size3 + offset;

                for (int i = 0; i < result_size; i++)
                {
                    third_half[i] = 0;
                }

                for (int i = result_size - 1; i >= size3 - offset; i--)
                {
                    char tmp = '0';
                    if (read(STDIN_FILENO, &tmp, 1) == -1)
                    {
                        fprintf(stderr, "Failed reading from pipe!");
                        tmp = '0';
                    }
                    char test[] = {tmp, '\0'};
                    third_half[i] = (unsigned char)strtol(test, NULL, 16);
                }
                size3 = result_size;
            }
            else if (curr_child == fourth_term) //Al · Bl
            {
                dup2(pipe_read4[0], STDIN_FILENO);
                close(pipe_read4[0]);

                size4 = number_size1;

                for (int i = 0; i < size4; i++)
                {
                    fourth_half[i] = 0;
                }

                for (int i = size4 - 1; i >= 0; i--)
                {
                    char tmp = '0';
                    if (read(STDIN_FILENO, &tmp, 1) == -1)
                    {
                        fprintf(stderr, "Failed reading from pipe!");
                        tmp = '0';
                    }
                    char test[] = {tmp, '\0'};
                    fourth_half[i] = (unsigned char)strtol(test, NULL, 16);
                }
            }
            all_childs_terminated++;
            if (curr_child == -1)
            {
                fprintf(stderr, "%s", strerror(errno));
            }
            fflush(stderr);
        }
        fflush(stdout);

        if (fork_failed != 0)
        {
            printf("0\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            int size_res1;
            int size_res2;
            unsigned char *res_fs = addIntegers(first_half, second_half, size1, size2, &size_res1);
            unsigned char *res_tf = addIntegers(third_half, fourth_half, size3, size4, &size_res2);
            unsigned char *result = addIntegers(res_fs, res_tf, size_res1, size_res2, &size_res1);

            free(res_fs);
            free(res_tf);

            for (int i = size_res1 - 1; i >= 0; i--)
            {
                printf("%x", result[i]);
            }
            printf("\n");
            free(result);
            fflush(stdout);
        }
    }
    exit(EXIT_SUCCESS);
}