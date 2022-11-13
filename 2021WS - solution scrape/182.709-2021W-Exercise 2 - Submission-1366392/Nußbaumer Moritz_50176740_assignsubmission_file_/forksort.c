/**
 * @file forksort.c
 * @author Nußbaumer Moritz (12026652)
 * @brief reads in multiple lines, sorts them alphabetically and prints them to stdout
 * @details reads in lines from stdin, forks and then redirects lines to child processes; after redirecting every line
 * reads the lines from the children's stdout, merges them and prints them in alphabetical order to stdout
 * @date 2021-12-09
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//global pointer so that err() can access the program name
char *myprog = NULL;

/**
 * @brief struct to save parameters of a child pipe
 * @param in stdin of child process
 * @param out stdout of child process
 * @param ID id of the child process
 */
typedef struct
{
    FILE *in, *out;
    pid_t ID; 
} pipe_;

//arrays to store file descriptors of pipes
int fst_child_w[2], fst_child_r[2], snd_child_w[2], snd_child_r[2];

/**
 * @brief closes file descriptors of the pipes
 */
static void closePipes(void)
{
    int i = 0;
    for (; i < 2; i++)
    {
        close(fst_child_w[i]);
        close(fst_child_r[i]);
        close(snd_child_w[i]);
        close(snd_child_r[i]);
    }
}

/**
 * @brief prints the occured error, exits the program and returns EXIT_FAILURE
 * @param msg the additonal message to be printed in the error message
 */
static void err(char *msg)
{
    fprintf(stderr, "%s: %s %s", myprog, msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief forks the current process
 * @details duplicates the file descriptors if fork succeeded, since we now have 2 instead of 1 process; otherwise terminates
 * @param write write file descriptors of child process
 * @param read read file descriptors of child process
 * @return int returns the id of the child process
 */
static int fork_children(int write[2], int read[2])
{
    int id = fork();
    switch (id)
    {
    case -1:
        closePipes();
        err("couldn't fork");
        break;
    case 0:
        if (((dup2(write[0], STDIN_FILENO)) == -1) || ((dup2(read[1], STDOUT_FILENO)) == -1))
        {
            closePipes();
            err("couldn't duplicate FD");
        }
        closePipes();
        if ((execlp(myprog, myprog, NULL)) == -1)
        {
            err("execlp failed");
        }
        break;
    default:
        break;
    }
    return id;
}

/**
 * @brief reads in lines, sorts them alphabetically and prints them to stdout
 * @details reads lines and hands them to child processes if more than 1 line is read; returns read line if it was only 1;
 * reads the childs lines from their output, merges them and then sorts them alphabetically and prints it to stdout
 * @param argc the number of provided arguments + 1;
 * @param argv a char array filled with the provided arguments; argv[0] is the program name
 * @return int returns EXIT_SUCCESS if the program completed its task successfully, othersie EXIT_FAILURE
 */
int main(int argc, char **argv)
{
    //not expecting any arguments
    if (argc != 1)
    {
        err("too many arguments provided");
    }

    myprog = argv[0];
    ssize_t read = 0;
    size_t len = 0;
    char *line = NULL, *fstLine = NULL;
    int lineCnt = 0;

    //save the first 2 lines to know whether we have 0, 1 or at least 2 lines
    if ((read = getline(&line, &len, stdin)) != EOF)
    {
        lineCnt++;
    }
    if ((read = getline(&fstLine, &len, stdin)) != EOF)
    {
        lineCnt++;
    }

    if (lineCnt < 1)
    {
        err("no lines to sort");
    }
    else if (lineCnt == 1)
    {
        fprintf(stdout, "%s", line);
        free(line);
        free(fstLine);
        exit(EXIT_SUCCESS);
    }

    //create pipes
    if (pipe(fst_child_w) == -1 || pipe(fst_child_r) == -1 || pipe(snd_child_w) == -1 || pipe(snd_child_r) == -1)
    {
        err("couldn't create children");
    }

    //create a sctruct to store child's parameters and sets the ID
    pipe_ fstChld, sndChld;
    fstChld.ID = fork_children(fst_child_w, fst_child_r);
    sndChld.ID = fork_children(snd_child_w, snd_child_r);

    //closes unused file descriptors
    close(fst_child_w[0]);
    close(fst_child_r[1]);
    close(snd_child_w[0]);
    close(snd_child_r[1]);

    //sets children's stdin
    if ((fstChld.in = fdopen(fst_child_w[1], "w")) == NULL || (sndChld.in = fdopen(snd_child_w[1], "w")) == NULL)
    {
        closePipes();
        err("couldn't open FD");
    }

    //hand the saved lines to the children
    fprintf(fstChld.in, "%s", fstLine);
    fprintf(sndChld.in, "%s", line);

    free(fstLine);

    //hand children the rest of the input lines
    while ((read = getline(&line, &len, stdin)) != EOF)
    {
        fprintf((++lineCnt % 2 == 1) ? fstChld.in : sndChld.in, "%s%s", line, (line[read - 1] != '\n') ? "\n" : "");
    }

    fclose(fstChld.in);
    fclose(sndChld.in);

    //sets children's stdout
    if ((fstChld.out = fdopen(fst_child_r[0], "r")) == NULL || (sndChld.out = fdopen(snd_child_r[0], "r")) == NULL)
    {
        closePipes();
        err("couldn't open FD");
    }

    ssize_t read2 = 0;
    size_t len2 = 0;
    char *line2 = NULL;

    read = getline(&line, &len, fstChld.out);
    read2 = getline(&line2, &len2, sndChld.out);

    //merge and sort the returned lines
    while (read != EOF || read2 != EOF)
    {
        if (read != EOF && read2 != EOF)
        {
            if (strcmp(line, line2) <= 0)
            {
                fprintf(stdout, "%s", line);
                read = getline(&line, &len, fstChld.out);
            }
            else
            {
                fprintf(stdout, "%s", line2);
                read2 = getline(&line2, &len2, sndChld.out);
            }
        }
        else if (read != EOF)
        {
            fprintf(stdout, "%s", line);
            read = getline(&line, &len, fstChld.out);
        }
        else
        {
            fprintf(stdout, "%s", line2);
            read2 = getline(&line2, &len2, sndChld.out);
        }
    }

    fclose(fstChld.out);
    fclose(sndChld.out);

    int statusFst, statusSnd;

    waitpid(fstChld.ID, &statusFst, 0);
    waitpid(sndChld.ID, &statusSnd, 0);

    //wait and receive child's status
    if (statusFst != EXIT_SUCCESS || statusSnd != EXIT_SUCCESS)
    {
        closePipes();
        err("waitpid failed");
    }

    free(line);
    free(line2);

    exit(EXIT_SUCCESS);
}