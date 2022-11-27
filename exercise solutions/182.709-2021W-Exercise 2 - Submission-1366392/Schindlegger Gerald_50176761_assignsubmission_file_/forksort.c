/**
 * @file forksort.c
 * @author Gerald Schindlegger
 * @date 22.11.2021
 *
 * @brief Forksort program.
 * 
 * This program is the forksort.
 **/


#include "forksort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE if failures occur
**/
int main(int argc, char *argv[])
{	
    if (argc > 1)
    {
        // arguments were passed
        (void)usage();
    }

    char *first_line = NULL;
    size_t len = 0;
    ssize_t first_line_size = 0, second_line_size = 0;

    // Reading first line
    first_line_size = getline(&first_line, &len, stdin);
    if (first_line_size == -1)
    {
        free(first_line);
        return EXIT_SUCCESS;
    }

    // Reading second line
    char *second_line = NULL;
    second_line_size = getline(&second_line, &len, stdin);
    if (second_line_size == -1) 
    {
        debug("%s", "Readed only one line.");
        (void)fprintf(stdout, "%s", first_line);
        free(first_line);
        free(second_line);
        return EXIT_SUCCESS;
    }

    // Two lines were read successfully. Start forking and send the readed lines to the childs
    debug("%s", "Readed two lines successfully, start forking");
    struct fork_process first_child = fork_child(argv[0]);
    struct fork_process second_child = fork_child(argv[0]);
    (void)fprintf(first_child.write, "%s", first_line);
    (void)fprintf(second_child.write, "%s", second_line);

    debug("%s", "Start reading and sending lines");
    int count = 2;
    char *line = NULL;
    while (getline(&line, &len, stdin) != -1)
    {
        if ((count % 2) == 0)
        {
            debug("Send line %d to first child", count);
            (void)fprintf(first_child.write, "%s", line);
        }
        else
        {
            debug("Send line %d to second child", count);
            (void)fprintf(second_child.write, "%s", line);
        }
        count++;
    }
    fclose(first_child.write);
    fclose(second_child.write);
    free(first_line);
    free(second_line);
    free(line);
    debug("Sent %d lines to both childs", count);

    debug("%s", "Start reading from both childs");
    read_and_merge(first_child.read, second_child.read);
    debug("%s", "Finished reading from childs");

    fclose(first_child.read);
    fclose(second_child.read);

    int status_first_child, status_second_child;
    if (waitpid(first_child.pid, &status_first_child, 0) == -1)
        error_exit("Waiting failed for child1");
    if (waitpid(second_child.pid, &status_second_child, 0) == -1)
        error_exit("Waiting failed for child2");

    if (WEXITSTATUS(status_first_child) != EXIT_SUCCESS || WEXITSTATUS(status_second_child) != EXIT_SUCCESS)
        error_exit("Childs didn't finished successfully");

    debug("Childs with %d and %d pid finished. Parent can be terminated successfully.", first_child.pid, second_child.pid);   
	return EXIT_SUCCESS;
}


/* ----- implementations ----- */

static void usage(void)	
{
	(void)fprintf(stderr, "SYNOPSIS: forksort\n");
	exit(EXIT_FAILURE);
}

static void error_exit(const char *msg)
{
    (void)fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static struct fork_process fork_child(char* program_name)
{
    int par_child[2], child_par[2];  

    if (pipe(par_child) == -1)
        error_exit("Pipe failed.");
    if (pipe(child_par) == -1)
        error_exit("Pipe failed.");

    pid_t child_pid = fork();
    struct fork_process process = {0};
    switch (child_pid)
    {
        case -1:
            error_exit("Fork failed");
            break;

        case 0:
            // Child
            close(par_child[WRITE]);
            close(child_par[READ]);

            if (dup2(par_child[READ], STDIN_FILENO) == -1)
                error_exit("dup for read end failed");
            if (dup2(child_par[WRITE], STDOUT_FILENO) == -1)
                error_exit("dup for write end failed");

            debug("Child execute programm %s", program_name);
            if (execlp(program_name, program_name, NULL) == -1)
                error_exit("executing failed");
            break;

        default:
            // Parent
            close(par_child[READ]);
            close(child_par[WRITE]);

            FILE *write = fdopen(par_child[WRITE], "w");
            if (write == NULL)
                error_exit("failed to open write end");

            FILE *read = fdopen(child_par[READ], "r");
            if (read == NULL)
                error_exit("failed to open read end");

            process.write = write;
            process.read = read;
            process.pid = child_pid;
    }  

    return process;      
}

static void read_and_merge(FILE *first_child, FILE *second_child)
{
    char *first_child_line = NULL, *second_child_line = NULL;
    size_t len_first = 0, len_second = 0;
    ssize_t first_child_line_size = 0, second_child_line_size = 0;
    int readed_first_line = 0, readed_second_line = 0;

    /* Read line by line from the both childs, compare each and write the "smaller" line to stdout
        e.g. if the line from the first child is smaller, write it to stdout and set the var readed_first_line to 0.
             Then in the next iteration it will try to read the next line from the first child and compare it again with the line from the second child and so on...
    */
    while (first_child_line_size != -1 || second_child_line_size != -1)
    {
        // check if the line from first child can be readed
        if (readed_first_line == 0)
        {
            first_child_line_size = getline(&first_child_line, &len_first, first_child);
            readed_first_line = 1;
        }

        // check if the line from second child can be readed
        if (readed_second_line == 0)
        {
            second_child_line_size = getline(&second_child_line, &len_second, second_child);
            readed_second_line = 1;
        }

        if (first_child_line_size == -1 && second_child_line_size == -1)
            continue;
        else if (first_child_line_size == -1)
        {
            // got from the first child EOF, so write the readed line from the second child to stdout
            (void)fprintf(stdout, "%s", second_child_line);
            readed_second_line = 0;
        }
        else if (second_child_line_size == -1)
        {
            // got from the second child EOF, so write the readed line from the first child to stdout
            (void)fprintf(stdout, "%s", first_child_line);
            readed_first_line = 0;
        }
        else
        {   
            // compare both lines and write the "smaller" to stdout
            int result = strcmp(first_child_line, second_child_line);
            if (result > 0)
            {
                (void)fprintf(stdout, "%s", second_child_line);
                readed_second_line = 0;
            }
            else
            { 
                (void)fprintf(stdout, "%s", first_child_line);
                readed_first_line = 0;
            }
        }
    }

    free(first_child_line);
    free(second_child_line);
}
