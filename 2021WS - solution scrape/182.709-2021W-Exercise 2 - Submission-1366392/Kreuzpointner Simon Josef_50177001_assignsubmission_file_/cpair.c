/**
 * @file cpair.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 14 November 2021
 * 
 * @brief this is the main module
 * 
 * @details This program returns the closes pair of a set of points by 
 * searching through the set in a recursive way.
 * 
 * In order to achieve this goal the process forks itself and passes a 
 * smaller version of the problem to its children until the problem is trivial.
 * It then processes the resulting values and returns the correct result.
 * 
 * This program uses a specified algorithm to get to the desired goal:
 * 
 * - 1. If the input read consists of only one point the program terminates with
 * EXIT_SUCCESS.
 * 
 * - 2. If the input read consists of two points the points are returned and the 
 * program exits with EXIT_SUCCESS.
 * 
 * - 3. If the input read consits of more than two points this process forks. At
 * first the input list will be split based on the x-mean @see computeXMean().
 * Then the process calls the children and passes in the divided input parts, one 
 * each. These processes than handle the input like written here.
 * 
 * The parent communicates with the child processes with the help of pipes, two
 * for each children to allow for bidirectional communication. The input will be 
 * bend to the stdin of the child, and stdout will be bend to the parent in pipe.
 * That results in the behaviour, that everything the parents passes to the child
 * via the parent-out pipe will pop up in the stdin stream of the child and therefore
 * enableing a seamless communication. Also, everything that will be written to stdout 
 * in the child will be put into the write end of the parent-in pipe, and can
 * therefore be read from the parent from the corresponding file descriptor.
 * 
 * If any of the children exit with EXIT_FAILURE this process also exits with 
 * EXIT_FAILURE.
 * 
 * - 4. If all of the children exited with EXIT_SUCCESS, the result will be fetched
 * from the corresponding file descriptores. Each child will return a pair of
 * the closest points in it's input list. No pair is returned if the input list
 * only contained one point.
 * 
 * The next step is to compare the two resulting pair of points. A new pair of points
 * will be defined as the closest point between the input list of the first child
 * and the input list of the second child.
 * 
 * The pair that is eventually returned is the smallest of these three pairs. If this
 * program succseeds in finding such a pair it exits with EXIT_SUCCESS after 
 * writing the result to stdout.
 * 
 * This program only takes use of the single floating point type.
 * 
 * If an error is encountered at any point in this program the process exits with
 * EXIT_FAILURE which will result in an exit for any given parents of this process.
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>

#include "debugUtility.h"
#include "pointActions.h"
#include "pipeActions.h"

/**
 * @brief subdivides the problem of searching for the closes pair
 * 
 * @details This function calculates the decision value to divide the point
 * array into two parts.
 * 
 * @see computeXMean()
 * @see dividePointArray()
 * 
 * @param points list of points
 * @param pointCount number of points in the list
 * @param left destination for the lower half of the points array
 * @param leftCount number of points in the lower half list
 * @param right destination for the upper half of the points array
 * @param rightCount number of points in the upper half list
 * @return 0 on success -1 otherwise
 */
static int subdivideProblem(point_t **points, int pointCount, point_t **left, int *leftCount, point_t **right, int *rightCount);

/**
 * @brief sets up all the child processes and works with them
 * 
 * @details This function utilizes the splitted array from subdivideProblem()
 * and recursively calls this process with the two new lists.
 * 
 * At first the function flushes the standard streams before creating enough
 * pipes to communitcate with the children in both directions using createAllPipes().
 * 
 * It then forks this process and adjusts the children and their pipes using
 * closeUnnecessaryPipesOnChildren() and bendPipeOnChildren() before replacing
 * the process image using execlp(). 
 * 
 * After the children are created the controll is handed over to the parent 
 * and doParentWork() is called. That means this function will return after the 
 * parent finished waiting for the children to finished and wrapped up the work
 * after that.
 * 
 * @param left list of points for the first child
 * @param leftCount number of points in left
 * @param right list of points for the second child
 * @param rightCount number of points in right
 * @return 0 on success and -1 on error
 */
static int callChildProcesses(point_t **left, int leftCount, point_t **right, int rightCount);

/**
 * @brief waits for each children to finish
 * 
 * @details This function waits for each children and counts the number of
 * children that finished with EXIT_FAILURE. This function sits in wait()
 * until every child finished.
 * 
 * @return the number of children that finished with EXIT_FAILURE, or
 * -1 on error
 */
static int waitForChildren(void);

/**
 * @brief all the work for the parent after forking the process
 * 
 * @details This function does all the work the parent has to do, after 
 * forking the process.
 * 
 * At first the unnecessary pipes on the parent are closed via closeUnnecessaryPipesOnParent().
 * After that the children are supplied with the necessary input using 
 * writeToChildren().
 * 
 * The next step for the parent is to wait for the children using waitForChildren()
 * and deciding what to do next based on the failure count of the children.
 * 
 * If any child exits with an error this function immediately returns -1,
 * else the parent fetches the result of each child.
 * 
 * Based on the fetched result a result of the function is computed and printed
 * to stdout.
 * 
 * @param left points list for the first child
 * @param leftCount number of points in left
 * @param right points list for the second child
 * @param rightCount number of points in right
 * @param pipefds file descriptores of the process
 * @return 0 on success and -1 otherwise
 */
static int doParentWork(point_t **left, int leftCount, point_t **right, int rightCount, int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

/**
 * @brief cleans up all the accumulated memory
 * 
 * @details This function frees any allocated memory, aswell as flushing
 * the standard file streams before closing them.
 * 
 * This function should not be called twice.
 * 
 * @param points point list
 * @param left point list for the first child
 * @param right point list for the second child
 */
static void cleanup(point_t *points, point_t *left, point_t *right);

/**
 * @brief cleans up all the accumulated memory and exits with EXIT_FAILURE
 * 
 * @details This function internally calls cleanup() before exiting with
 * EXIT_FAILURE.
 * 
 * @param points point list
 * @param left point list for the first child
 * @param right point list for the second child
 */
static void cleanupExit(point_t *points, point_t *left, point_t *right);

/**
 * @brief cleans up all the accumulated memory in the parent
 * 
 * @details This function frees all the memory that was allocated inside of
 * the parent aswell as closing the open file descriptors of the parent.
 * 
 * This function must not be called twice.
 * 
 * @param pairLeft result of the first child
 * @param pairRight result of the second child
 * @param pipefds file descriptores of the process
 */
static void cleanupParent(point_t **pairLeft, point_t **pairRight, int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

/**
 * @brief displays the synopsis
 *
 * @details This function displays the synopsios of the program and
 * will exit afterwards with an exit code of EXIT_FAILURE if exitAfterwards is 1.
 * This function also uses the global variable programName.
 * @see programName
 * 
 * @param exitAfterwards If set to 1 the program will exit with EXIT_FAILURE
 * after displaying the synopsis, otherwise it will just return without exiting.
 */
void usage(int exitAfterwards);

/**
 * @brief the program name
 * 
 * @details This is equal to the first argument of the argv array
 * passed to the main() function.
*/
char *programName;

/**
 * @brief this is the main entry point of cpair
 * 
 * @details This function takes no arguments. This function
 * reads in the input on stdin and parses it to a point list.
 * 
 * Based on the number of points the program either forks or returns immediately to
 * avoid an infinite loop. If only one point is passed in, the program terminates with 
 * EXIT_SUCCESS. If two points are passed in the program returns these two points and
 * terminates with EXIT_SUCCESS.
 * 
 * Otherwise this function forks and divides the problem into two smaller halfs 
 * and solves it recursively using child processes and pipes to communicate
 * with them.
 * 
 * @param argc argument counter
 * @param argv argument vector
 * @return Returns EXIT_SUCCESS on success and EXIT_FAILURE on error.
 */
int main(int argc, char *argv[])
{
    debug("main");

    programName = argv[0];

    point_t *points = NULL;
    int pointCount = 0;

    point_t *left = NULL;
    int leftCount = 0;

    point_t *right = NULL;
    int rightCount = 0;

    int temp = STDIN_FILENO;
    if (getPoints(&points, &pointCount, &temp) < 0)
    {
        (void)cleanupExit(points, left, right);
    }

    if (pointCount == 1)
    {
        (void)cleanup(points, left, right);
        return EXIT_SUCCESS;
    }
    else if (pointCount == 2)
    {
        (void)printPoints(&points, pointCount, STDOUT_FILENO);
        (void)cleanup(points, left, right);
        return EXIT_SUCCESS;
    }
    else if (pointCount > 2)
    {
        if (subdivideProblem(&points, pointCount, &left, &leftCount, &right, &rightCount) < 0)
        {
            (void)cleanupExit(points, left, right);
        }

        // after the divide the original list is no longer needed
        if (points != NULL)
        {
            (void)free(points);
            points = NULL;
        }

        if (callChildProcesses(&left, leftCount, &right, rightCount) < 0)
        {
            (void)cleanupExit(points, left, right);
        }
    }
    else
    {
        (void)printError("main", "The point count was negative.");
        (void)cleanupExit(points, left, right);
    }

    (void)cleanup(points, left, right);
    return EXIT_SUCCESS;
}

static int subdivideProblem(point_t **points, int pointCount, point_t **left, int *leftCount, point_t **right, int *rightCount)
{
    debug("subdivideProblem()");

    float xm = 0;
    if (computeXMean(points, pointCount, &xm) < 0)
    {
        return -1;
    }

    if (dividePointArray(points, pointCount, left, leftCount, right, rightCount, xm) < 0)
    {
        return -1;
    }

    debug("X mean = %f", xm);

    return 0;
}

static int callChildProcesses(point_t **left, int leftCount, point_t **right, int rightCount)
{
    debug("callChildProcesses()");

    pid_t pids[CHILDPROCESS_COUNT];

    // for every child we need two pipes (parent-in and parent-out)
    // [...][0] = parent-in fds
    // [...][1] = parent-out fds
    int pipefds[CHILDPROCESS_COUNT][2][2];

    if (fflush(stdin) == EOF)
    {
        (void)printError("fflush", "An error ocurred while trying to flush stdin");
        return -1;
    }

    if (fflush(stdout) == EOF)
    {
        (void)printError("fflush", "An error ocurred while trying to flush stdout.");
        return -1;
    }

    if (fflush(stderr) == EOF)
    {
        (void)printError("fflush", "An error ocurred while trying to flush stderr.");
        return -1;
    }

    // create all the necessary pipes
    if (createAllPipes(&pipefds) < 0)
    {
        return -1;
    }

    for (int i = 0; i < CHILDPROCESS_COUNT; i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            (void)printError("fork", "An error ocurred while trying to fork the process.");
            return -1;
        }
        else if (pids[i] == 0)
        {
            // in children

            if (closeUnnecessaryPipesOnChildren(i, &pipefds) < 0)
            {
                return -1;
            }

            debug("bending the pipes");

            // stdin should be the ouput of the parent-out read end
            if (bendPipeOnChildren(pipefds[i][1][0], STDIN_FILENO) < 0)
            {
                return -1;
            }

            // stdout should be the input of the parent-in write end
            if (bendPipeOnChildren(pipefds[i][0][1], STDOUT_FILENO) < 0)
            {
                return -1;
            }

            debug("replacing process image");

            (void)execlp(programName, programName, NULL);

            (void)printError("execlp", "An error ocurred while trying to replace the process image.");
            return -1;
        }
    }

    // in parent
    if (doParentWork(left, leftCount, right, rightCount, &pipefds) < 0)
    {
        return -1;
    }

    return 0;
}

static int doParentWork(point_t **left, int leftCount, point_t **right, int rightCount, int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("doParentWork()");

    point_t *pairLeft = NULL;
    int pairLeftCount = 0;

    point_t *pairRight = NULL;
    int pairRightCount = 0;

    pointPair_t pair1;
    pointPair_t pair2;
    pointPair_t pair3;
    pointPair_t result;

    int failureCount = 0;

    // close the pipes
    if (closeUnnecessaryPipesOnParent(pipefds) < 0)
    {
        (void)cleanupParent(&pairLeft, &pairRight, pipefds);
        return -1;
    }

    // write the infos to the children
    if (writeToChildren(left, leftCount, right, rightCount, pipefds) < 0)
    {
        (void)cleanupParent(&pairLeft, &pairRight, pipefds);
        return -1;
    }

    // wait for children
    if ((failureCount = waitForChildren()) < 0)
    {
        (void)cleanupParent(&pairLeft, &pairRight, pipefds);
        return -1;
    }

    if (failureCount > 0)
    {
        debug("At least 1 child process exited with EXIT_FAILURE.");
        (void)cleanupParent(&pairLeft, &pairRight, pipefds);
        return -1;
    }
    else
    {
        debug("Step 6. in instructions.");

        if (getPoints(&pairLeft, &pairLeftCount, &((*pipefds)[0][0][0])) < 0)
        {
            (void)printError("getPoints", "An error ocurred while trying to read the returning values of a child from the parent-in pipe read end.");
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        if (getPoints(&pairRight, &pairRightCount, &((*pipefds)[1][0][0])) < 0)
        {
            (void)printError("getPoints", "An error ocurred while trying to read the returning values of a child from the parent-in pipe read end.");
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        debug("Step 7. in instructions.");

        if (convertPointsToPointPair(&pairLeft, pairLeftCount, &pair1) < 0)
        {
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        if (convertPointsToPointPair(&pairRight, pairRightCount, &pair2) < 0)
        {
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        if (getClosestPair(left, leftCount, right, rightCount, &pair3) < 0)
        {
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        debug("Step 8. in instructions.");

        if (getMinDistancePointPair(pair1, pair2, pair3, &result) < 0)
        {
            (void)cleanupParent(&pairLeft, &pairRight, pipefds);
            return -1;
        }

        (void)printPointPair(&result, STDOUT_FILENO);
    }

    // cleanup
    (void)cleanupParent(&pairLeft, &pairRight, pipefds);

    return 0;
}

static int waitForChildren(void)
{
    debug("waitForChildren()");

    pid_t pid;
    int status = 0;
    int failureCount = 0;
    int n = CHILDPROCESS_COUNT;

    while (n > 0)
    {
        pid = wait(&status);
        if (pid < 0)
        {
            (void)printError("wait", "An error ocurred while trying to wait on child process.");
            return -1;
        }
        if (errno == EINTR)
        {
            debug("Child interrupt");
            continue;
        }

        debug("A child exited.");
        n--;

        if (WEXITSTATUS(status) == EXIT_FAILURE)
        {
            debug("The exited child exited with EXIT_FAILURE.");
            failureCount++;
        }
    }

    return failureCount;
}

static void cleanup(point_t *points, point_t *left, point_t *right)
{
    debug("cleanup()");

    if (points != NULL)
    {
        (void)free(points);
        points = NULL;
    }

    if (left != NULL)
    {
        (void)free(left);
        left = NULL;
    }

    if (right != NULL)
    {
        (void)free(right);
        right = NULL;
    }

    (void)fflush(stdin);
    (void)fflush(stdout);
    (void)fflush(stderr);

    (void)close(STDIN_FILENO);
    (void)close(STDOUT_FILENO);
    (void)close(STDERR_FILENO);
}

static void cleanupExit(point_t *points, point_t *left, point_t *right)
{
    debug("cleanupExit()");

    (void)cleanup(points, left, right);
    (void)exit(EXIT_FAILURE);
}

static void cleanupParent(point_t **pairLeft, point_t **pairRight, int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("cleanupParent()");

    if (*pairLeft != NULL)
    {
        (void)free(*pairLeft);
    }

    if (*pairRight != NULL)
    {
        (void)free(*pairRight);
    }

    // closing the parent out fds
    (void)close((*pipefds)[0][1][1]);
    (void)close((*pipefds)[1][1][1]);
}

void usage(int exitAfterwards)
{
    extern char *programName;

    (void)fprintf(stderr, "Usage: %s\n", programName);

    if (exitAfterwards == 1)
    {
        (void)exit(EXIT_FAILURE);
    }
}