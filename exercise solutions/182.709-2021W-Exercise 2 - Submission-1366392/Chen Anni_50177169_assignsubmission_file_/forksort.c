/**
 * @file   forksort.c
 * @author Anni Chen (11902050)
 * @date   10.12.2021
 *
 * @brief This program reads lines line by line from stdin and sorts them by using a recursive variant
 * of merge sort
 *
 * @details This program reads lines from stdin until an EOF is encountered.
 * If only one line is read then this line is immediately returned. Otherwise the process forks twice and
 * sends half the amount of lines to the one and the other half of lines to the other child. The children
 * processes each call the program again.
 * The parent process reads the output of its children line by line, compares them and outputs the line that
 * comes alphabetically before the other line.
 * The program takes no arguments.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

//the name of the program
static char *progname = "<not yet set>";

static void error_exit(const char *msg)
{
    fprintf(stderr, "[%s] [ERROR] %s \n", progname, msg);
    exit(EXIT_FAILURE);
}

static void usage(void)
{
    fprintf(stderr, "[%s] [USAGE]: %s \n", progname, progname);
    exit(EXIT_FAILURE);
}

/**
 * @brief closes pipe ends that the child processes do not need and redirects the used pipe ends to stdin and stdout
 * @details For one pipe, the write end is closed since the child reads from the parent, here the read end is redirected
 * to stdin, so that the child reads the results from stdin. For the other pipe, the read end is closed, so that the child
 * can write results to the parent, here the write end is redirected to stdout
 * @param pipefd_w[2], the pipe that enables children to read from the parent as well es parents to write to children
 * @param pipefd_r[2], the pipe that enables children to write to the parent as well es parents to read from children
 */
static void close_and_dub_pipes(int pipefd_w[2], int pipefd_r[2])
{
    if (close(pipefd_w[1]) == -1)
    {
        error_exit("failed to close pipe end");
    }

    if (close(pipefd_r[0]) == -1)
    {
        error_exit("failed to close pipe end");
    }

    //redirect stdin to pipe
    if ((dup2(pipefd_w[0], STDIN_FILENO)) == -1)
    {
        error_exit("failed to redirect stdin to pipe");
    }

    if (close(pipefd_w[0]) == -1)
    {
        error_exit("failed to close pipe end");
    }

    //redirect stdout to pipe
    if ((dup2(pipefd_r[1], STDOUT_FILENO)) == -1)
    {
        error_exit("failed to redirect stdout to pipe");
    }

    if (close(pipefd_r[1]) == -1)
    {
        error_exit("failed to close pipe end");
    }
}

/**
 * @brief The entry point of the program.
 * @details This function executes the whole program. It checks if there are no arguments passed. After that, the first line
 * is read, followed by the second line. If there is no second line, the program prints the first line to stdout and terminates
 * successfully. If there is more than one line, the pipes needed for the first child are initialised and their unused pipe ends
 * closed. After that, the child process is forked and calls the program with execlp(). Analogous for the second child. The parent
 * process distributes the lines evenly to both childs by writing them to the corresponding stdouts, if the number of lines are even,
 * otherwise one child will have one more line. The parent process then waits for its children to finish. After the children terminate,
 * it reads the lines line by line, compares them and prints the lines in an alphabetically order.

 * @param argc argument counter
 * @param argv argument values
 *
 */
int main(int argc, char *argv[])
{
    progname = argv[0];

    if (argc != 1)
    {
        usage();
    }

    char *line1 = NULL;
    char *line2 = NULL;
    size_t len = 0;

    //try to read first line
    if ((getline(&line1, &len, stdin)) == -1)
    {
        error_exit("failed to read line");
    }

    //try to read second line, if there is no line print first line
    if ((getline(&line2, &len, stdin)) == -1)
    {
        fprintf(stdout, "%s\n", line1);
        exit(EXIT_SUCCESS);
    }

    int pipefd_1_w[2]; //pipe fd from parent to child1 (parent writes to child)
    int pipefd_1_r[2]; //pipe fd from child 1 to parent (parent reads from child)

    if ((pipe(pipefd_1_w)) == -1 || (pipe(pipefd_1_r)) == -1)
    {
        error_exit("failed to create pipes");
    }

    pid_t pid1 = fork();

    if (pid1 == -1)
    {
        error_exit("fork failed");
    }

    if (pid1 == 0) //first child even Pe
    {

        close_and_dub_pipes(pipefd_1_w, pipefd_1_r);

        if (execlp("./forksort", "forksort", NULL) == -1)
        {
            error_exit("exec failed");
        }
    }

        int pipefd_2_w[2]; //pipe fd from parent to child 2 (parent writes to child)
        int pipefd_2_r[2]; //pipe fd from child 2 to parent (parent reads from child)

        if ((pipe(pipefd_2_w)) == -1 || (pipe(pipefd_2_r)) == -1)
        {
            error_exit("failed to create pipes");
        }

        pid_t pid2 = fork();

        if (pid2 == -1)
        {
            error_exit("fork failed");
        }

        if (pid2 == 0)
        {
            // close pipe ends of child 1
            if (close(pipefd_1_w[0]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_1_w[1]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_1_r[0]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_1_r[1]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            close_and_dub_pipes(pipefd_2_w, pipefd_2_r);

            if (execlp("./forksort", "forksort", NULL) == -1)
            {
                error_exit("exec failed");
            }
        }

            //close pipe ends that parent does not need
            if (close(pipefd_1_w[0]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_2_w[0]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_1_r[1]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            if (close(pipefd_2_r[1]) == -1)
            {
                error_exit("failed to close pipe end");
            }

            FILE *out1 = fdopen(pipefd_1_w[1], "w");
            FILE *out2 = fdopen(pipefd_2_w[1], "w");

            if (out1 == NULL)
            {
                error_exit("failed to open output file");
            }

            if (out2 == NULL)
            {
                error_exit("failed to open output file");
            }

            int written1 = 0;
            int written2 = 0;

            if ((fputs(line1, out1)) == EOF)
            {
                error_exit("failed to write line to output file");
            }
            written1++;
            if ((fputs(line2, out2)) == EOF)
            {
                error_exit("failed to write line to output file");
            }
            free(line1);
            written2++;

            int count = 0;

            //distribute lines evenly to the two child processes
            while ((getline(&line2, &len, stdin)) != -1)
            {
                if (count % 2 == 0)
                {
                    if ((fputs(line2, out1)) == EOF)
                    {
                        error_exit("failed to write line to output file");
                    }
                    written1++;
                }
                else
                {
                    if ((fputs(line2, out2)) == EOF)
                    {
                        error_exit("failed to write line to output file");
                    }
                    written2++;
                }

                count++;
            }

            free(line2);

            if (fclose(out2) == EOF)
            {
                error_exit("failed to close output file");
            }

            if (fclose(out1) == EOF)
            {
                error_exit("failed to close output file");
            }

            int status1;
            int status2;

            waitpid(pid1, &status1, 0);
            if (WEXITSTATUS(status1))
            {
                error_exit("an error happend in the child process");
            }

            waitpid(pid2, &status2, 0);
            if (WEXITSTATUS(status2))
            {
                error_exit("an error happend in the child process");
            }

            //read result from 1. and 2.child
            FILE *in1 = fdopen(pipefd_1_r[0], "r");
            FILE *in2 = fdopen(pipefd_2_r[0], "r");

            if (in1 == NULL || in2 == NULL)
            {
                error_exit("failed to open input file");
            }

            char *next_line1 = NULL;
            char *read_lines_1[written1];
            int k = 0;

            //store lines from input file in an array
            while ((getline(&next_line1, &len, in1)) != -1)
            {
                read_lines_1[k] = malloc(sizeof(char) * (strlen(next_line1) + 1));
                strcpy(read_lines_1[k], next_line1);
                k++;
            }

            free(next_line1);

            char *next_line2 = NULL;
            char *read_lines_2[written2];
            k = 0;

            //store lines from input file in an array
            while ((getline(&next_line2, &len, in2)) != -1)
            {
                read_lines_2[k] = malloc(sizeof(char) * (strlen(next_line2) + 1));
                strcpy(read_lines_2[k], next_line2);
                k++;
            }

            free(next_line2);

            int f = 0;
            int s = 0;

            //compare lines from both sets and print the line that precedes other lines first
            while (f != written1 && s != written2)
            {
                if (strcmp(read_lines_1[f], read_lines_2[s]) < 0)
                {
                    fprintf(stdout, "%s", read_lines_1[f]);
                    free(read_lines_1[f]);
                    f++;
                }
                else
                {
                    fprintf(stdout, "%s", read_lines_2[s]);
                    free(read_lines_2[s]);
                    s++;
                }
            }

            //print remaining lines
            while (f != written1 || s != written2)
            {
                if (f != written1)
                {
                    fprintf(stdout, "%s", read_lines_1[f]);
                    free(read_lines_1[f]);
                    f++;
                }
                else
                {
                    fprintf(stdout, "%s", read_lines_2[s]);
                    free(read_lines_2[s]);
                    s++;
                }
            }

            if (fclose(in1) == EOF)
            {
                error_exit("failed to close input file");
            }

            if (fclose(in2) == EOF)
            {
                error_exit("failed to close input file");
            }

            exit(EXIT_SUCCESS);


}
