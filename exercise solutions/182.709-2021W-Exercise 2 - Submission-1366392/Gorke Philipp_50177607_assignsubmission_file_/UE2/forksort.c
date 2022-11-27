/**
 * @file   forksort.c
 * @author Philipp Gorke (e12022511)
 * @date   06.12.2021
 *
 * @brief Programm reads in Strings line by line and sorts them alphabetically
 *
 * @details The programm first reads in a stdin line by line until an EOR occurs. If
 * there is just one line of input the line is simply returned. If not, the programm
 * forks two times and exec the same programm again to call it recursively. The
 * input is split into two parts until both of them finally become one line and can be
 * returned. After that the algorithm compares all those Strings together and merges them.
 * All communication is done via, Stdin, Stdout and pipes. The programm takes no arguments
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

// region GLOBAL VARIABLES
/** The program name as specified in argv[0] */
static char *programmName;

// region TYPES
/**
 * Represents all pipes used in the programm in one struct
 * Each pipe has a read[0] and a write[1] end.
 */
typedef struct
{
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];
    int pipe4[2];
} pipes;

// region FUNCTION DECLARATIONS
static void Usage(void);
static void error(char *message);
static pipes initPipes(void);
static pid_t forkChild1(pipes ps);
static pid_t forkChild2(pipes ps);
static void writeToChildren(pipes ps, int capPid1, int capPid2, char **inputStrings);
static void waitForChildren(pid_t pid1, pid_t pid2);
static void readFromChildren(pipes ps, char **outputChild1, char **outputChild2, int capPid1, int capPid2);
static void compare(char **outputChild1, char **outputChild2, int capPid1, int capPid2);

/**
 * @brief Main function of the programm.
 * @details This function executes the whole program by calling upon other functions.
 *
 * global variables used: programmName
 *
 * @param argumentCounter The argument counter.
 * @param argumentValues  The argument values.
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[])
{
    programmName = argv[0];
    if (argc != 1)
        Usage();

    pipes ps = initPipes();

    //CREATES ARRAY OF STRINGS HERE
    int lineAmount = 1;
    char **inputStrings = malloc(lineAmount * sizeof(char *));
    char *line = NULL;
    int linesread = 0;
    size_t size = 0;
    ssize_t bytesread;

    while ((bytesread = getline(&line, &size, stdin)) != EOF)
    {
        inputStrings[linesread] = malloc(bytesread+1);
        inputStrings[linesread] = line; 
        line = 0;

        if (++linesread >= lineAmount)
        {
            ++lineAmount;
            inputStrings = realloc(inputStrings, (lineAmount) * sizeof(char *));
        }
    }
    free(line);
    int numberOfLines = lineAmount-1;

    //Numbers in which array is split in half for child processes
    int capPid1 = numberOfLines / 2;
    int capPid2 = numberOfLines % 2 == 0 ? numberOfLines / 2 : numberOfLines / 2 + 1;

    if (numberOfLines == 1)
    {
        fprintf(stdout, "%s\n", inputStrings[0]);
        exit(EXIT_SUCCESS);
    }

    pid_t pid1 = forkChild1(ps);
    pid_t pid2 = forkChild2(ps);

    // PARENT PROCESS
    writeToChildren(ps, capPid1, capPid2, inputStrings);
    waitForChildren(pid1, pid2);
    char **outputChild1 = malloc(sizeof(char *) * capPid1);
    char **outputChild2 = malloc(sizeof(char *) * capPid2);
    readFromChildren(ps, outputChild1, outputChild2, capPid1, capPid2);
    compare(outputChild1, outputChild2, capPid1, capPid2);

    // CLEANUP RESOURCES
    free(outputChild1);
    free(outputChild2);
    for (int i = 0; i < numberOfLines; i++){
        free(inputStrings[i]);
    }
    free(inputStrings);
    exit(EXIT_SUCCESS);
}

/**
 * @brief Function that's called when the programm has the wrong usage
 * @details Returns the correct usage, doesn't take parameters
 * global variables used: programName
 */
void Usage()
{
    printf("Usage: %s", programmName);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints the error message and exits the programm with EXIT_FAILURE
 *
 * global variables used: programName
 *
 * @param message The custom message to print along with the program name and errno contents.
 */
void error(char *message)
{
    fprintf(stderr, "ERROR in %s: %s", programmName, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief sets up the pipes
 * @details initialized all pipes and returns the struct which contains all 4 pipes, no parameters
 *
 **/
pipes initPipes()
{
    pipes ps;
    if (pipe(ps.pipe1) == -1)
    {
        error("opening pipe1 Failed");
    }
    if (pipe(ps.pipe2) == -1)
    {
        error("opening pipe2 failed");
    }
    if (pipe(ps.pipe3) == -1)
    {
        error("opening pipe1 Failed");
    }
    if (pipe(ps.pipe4) == -1)
    {
        error("opening pipe2 failed");
    }
    return ps;
}



/**
 * @brief Forks the main programm and calls exec recursively
 * @details Main Programm is forked. In the Child Process Stdin and Stdout
 * fds are changed to the given pipes. All other pipes are closed.
 * The function then calls exec to recursively call the programm.
 * Also returns the pid.
 *
 * @param ps Container of pipes which is used for the children Process
 */
static pid_t forkChild1(pipes ps)
{
    pid_t pid1 = fork();
    if (pid1 == -1)
    {
        error("Forking failed");
    }
    else if (pid1 == 0)
    {
        // CHILD PROCESS
        // STDIN
        close(ps.pipe1[1]);
        if (dup2(ps.pipe1[0], STDIN_FILENO) == -1)
        {
            error("redirecting fd failed");
        }
        close(ps.pipe1[0]);
        // STDOUT
        close(ps.pipe2[0]);
        if (dup2(ps.pipe2[1], STDOUT_FILENO) == -1)
        {
            error("redirecting fd failed");
        }
        close(ps.pipe2[1]);

        close(ps.pipe3[0]);
        close(ps.pipe3[1]);
        close(ps.pipe4[0]);
        close(ps.pipe4[1]);

        execlp(programmName, programmName, NULL);
        error("Shouldn't reach that line");
    }
    else
        return pid1;
    return -1;
}

/**
 * @brief Forks the main programm and calls exec recursively
 * @details Main Programm is forked. In the Child Process Stdin and Stdout
 * fds are changed to the given pipes. All other pipes are closed.
 * The function then calls exec to recursively call the programm.
 * Also returns the pid.
 *
 * @param ps Container of pipes which is used for the children Process
 */
static pid_t forkChild2(pipes ps)
{
    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        error("Forking failed");
    }
    else if (pid2 == 0)
    {
        // CHILD PROCESS
        // STDIN
        close(ps.pipe3[1]);
        if (dup2(ps.pipe3[0], STDIN_FILENO) == -1)
        {
            error("redirecting fd failed");
        }
        close(ps.pipe3[0]);
        // STDOUT
        close(ps.pipe4[0]);
        if (dup2(ps.pipe4[1], STDOUT_FILENO) == -1)
        {
            error("redirecting fd failed");
        }
        close(ps.pipe4[1]);

        close(ps.pipe1[0]);
        close(ps.pipe1[1]);
        close(ps.pipe2[0]);
        close(ps.pipe2[1]);

        execlp(programmName, programmName, NULL);
        error("Shouldn't reach that line");
    }
    else
        return pid2;
    return -1;
}

/**
 * @brief Writes to the children Process
 * @details Parent process writes through pipes to the children process
 * which is interpreted as stdin and then read later.
 *
 * @param ps Container of pipes which is used for the children Process
 * @param capPid1 the maximum number of elements for the first branch
 * @param capPid2 the maximum number of elements for the second branch
 * @param inputStrings the String array of all inputs
 */
static void writeToChildren(pipes ps, int capPid1, int capPid2, char **inputStrings)
{
    close(ps.pipe1[0]);
    close(ps.pipe3[0]);
    for (int i = 0; i < capPid1; i++)
    {
        if (write(ps.pipe1[1], inputStrings[i], (strlen(inputStrings[i]))) == -1)
        {
            error("writing to pipe failed");
        }
    }
    close(ps.pipe1[1]);

    for (int i = capPid1; i < capPid2 + capPid1; i++)
    {
        if (write(ps.pipe3[1], inputStrings[i], (strlen(inputStrings[i]))) == -1)
        {
            error("writing to pipe failed");
        }
    }
    close(ps.pipe3[1]);
}

/**
 * @brief Waits for children until programm can continue
 * @details Waits for the children to push something to stdout
 * so it can be read later
 *
 * @param pid1 pid of the first Child
 * @param pid2 pid of the second Child

 */
static void waitForChildren(pid_t pid1, pid_t pid2)
{
    int status, status2;
    if (waitpid(pid1, &status, 0) == -1)
    {
        error("problem waiting for children");
    };
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        error("Problem in child process");
    }

    if (waitpid(pid2, &status2, 0) == -1)
    {
        error("problem waiting for children");
    }

    if (WEXITSTATUS(status2) != EXIT_SUCCESS)
    {
        error("Problem in child process");
    }
}

/**
 * @brief Reads from the children processes
 * @details Parent process reads stdout from their children processes.
 * Communication via pipes. Also slightly changes the String so it can
 * be interpretated later.
 *
 * @param ps Container of pipes which is used for the children Process
 * @param outputChild1 char ** which is changed and strings are stored here
 * @param outputChild2 char ** which is changed and strings are stored here
 * @param capPid1 the maximum number of elements for the first branch
 * @param capPid2 the maximum number of elements for the second branch
 */
static void readFromChildren(pipes ps, char **outputChild1, char **outputChild2, int capPid1, int capPid2)
{
    close(ps.pipe2[1]);
    close(ps.pipe4[1]);
    char *line1 = NULL;
    size_t size = 0;
    FILE *temp1 = fdopen(ps.pipe2[0], "r");

    for (int i = 0; i < capPid1; i++)
    {
        line1 = NULL;
        getline(&line1, &size, temp1);
        outputChild1[i] = line1;
        outputChild1[i][strlen(outputChild1[i]) - 1] = '\0';
    }
    close(ps.pipe2[0]);

    FILE *temp2 = fdopen(ps.pipe4[0], "r");

    for (int i = 0; i < capPid2; i++)
    {
        line1 = NULL;
        getline(&line1, &size, temp2);
        outputChild2[i] = line1;
        outputChild2[i][strlen(outputChild2[i]) - 1] = '\0';
    }
    close(ps.pipe4[0]);
}

/**
 * @brief Compares two char ** and sorts them.
 * @details Two char ** are given which are already sorted due to
 * recursion. After that both Pointers are iterated and therefore sorted
 * result is printed to stdout
 *
 * @param outputChild1 char ** strings are stored here, will be compared to outputChild2
 * @param outputChild2 char ** strings are stored here, will be compared to outputChild2
 * @param capPid1 the maximum number of elements for the first branch
 * @param capPid2 the maximum number of elements for the second branch
 */
static void compare(char **outputChild1, char **outputChild2, int capPid1, int capPid2)
{
    int counter1 = 0;
    int counter2 = 0;
    while (counter1 != (capPid1) || counter2 != (capPid2))
    {
        if (counter1 != (capPid1) && counter2 != (capPid2))
        {
            if (strcmp(outputChild1[counter1], outputChild2[counter2]) < 0)
            {
                fprintf(stdout, "%s\n", outputChild1[counter1]);
                counter1++;
            }
            else
            {
                fprintf(stdout, "%s\n", outputChild2[counter2]);
                counter2++;
            }
        }
        else if (counter1 != capPid1)
        {
            fprintf(stdout, "%s\n", outputChild1[counter1]);
            counter1++;
        }
        else
        {
            fprintf(stdout, "%s\n", outputChild2[counter2]);
            counter2++;
        }
    }
}
