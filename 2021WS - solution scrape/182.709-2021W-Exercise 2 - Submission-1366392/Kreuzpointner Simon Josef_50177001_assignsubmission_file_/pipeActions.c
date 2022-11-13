/**
 * @file pipeACtions.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 17 November 2021
 * 
 * @brief implementation of the pipe actions module
 */

#include <unistd.h>

#include "pipeActions.h"
#include "pointActions.h"

int createAllPipes(int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("createAllPipes()");

    for (int i = 0; i < CHILDPROCESS_COUNT; i++)
    {
        // parent-in and parent-out pipe
        for (int j = 0; j < 2; j++)
        {
            if (pipe((*pipefds)[i][j]) < 0)
            {
                (void)printError("pipe", "An error ocurred while trying to create a pipe.");
                return -1;
            }
        }
    }

    return 0;
}

int closeUnnecessaryPipesOnChildren(int childIndex, int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("closeUnnecessaryPipesOnChildren()");
    debug("Closing every child write file descriptor that is not needed.");
    // close every child write file descriptor
    for (int j = 0; j < CHILDPROCESS_COUNT; j++)
    {
        // close every parent-out pipe write end because these are only used by the parent and not the children
        if (close((*pipefds)[j][1][1]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the write end of the pipe.");
            return -1;
        }

        if (j == childIndex)
        {
            continue;
        }

        // close every parent-in pipe write end
        if (close((*pipefds)[j][0][1]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the write end of the pipe.");
            return -1;
        }
    }

    debug("Closing every child read file descriptor that is not needed.");
    // close every child read file descriptor
    for (int j = 0; j < CHILDPROCESS_COUNT; j++)
    {
        // close every parent-in pipe read end
        if (close((*pipefds)[j][0][0]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the read end of the pipe.");
            return -1;
        }

        if (j == childIndex)
        {
            continue;
        }

        // close every parent-out pipe read end
        if (close((*pipefds)[j][1][0]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the read end of the pipe.");
            return -1;
        }
    }

    return 0;
}

int bendPipeOnChildren(int fromfd, int tofd)
{
    debug("bendPipeOnChildren()");

    if (tofd == fromfd)
    {
        (void)printWarning("bendPipeOnChildren", "from-fd and to-fd was equal.");
    }

    if (close(tofd) < 0)
    {
        (void)printError("close", "An error ocurred while trying to close the to-fd.");
        return -1;
    }

    if (dup2(fromfd, tofd) < 0)
    {
        (void)printError("dup2", "An error ocurred while trying to bend from-fd to to-fd.");
        return -1;
    }

    if (close(fromfd) < 0)
    {
        (void)printError("close", "An error ocurred while trying to close from-fd.");
        return -1;
    }

    return 0;
}

int closeUnnecessaryPipesOnParent(int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("closeUnnecessaryPipesOnParent()");
    debug("Closing all the pipe ends that are not needed in the parent.");
    for (int j = 0; j < CHILDPROCESS_COUNT; j++)
    {
        if (close((*pipefds)[j][0][1]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the write end of the pipe.");
            return -1;
        }

        if (close((*pipefds)[j][1][0]) < 0)
        {
            (void)printError("close", "An error ocurred while trying to close the read end of the pipe.");
            return -1;
        }
    }

    return 0;
}

int writeToChildren(point_t **left, int leftCount, point_t **right, int rightCount, int (*pipefds)[CHILDPROCESS_COUNT][2][2])
{
    debug("writeToChildren()");

    // NOTE: this is not scaled. This does expect only 2 children
    (void)printPoints(left, leftCount, (*pipefds)[0][1][1]);
    (void)printPoints(right, rightCount, (*pipefds)[1][1][1]);

    // close the pipes so it will read EOF
    if (close((*pipefds)[0][1][1]) < 0)
    {
        (void)printError("close", "An error ocurred while trying to close the read end of the pipe");
        return -1;
    }

    if (close((*pipefds)[1][1][1]) < 0)
    {
        (void)printError("close", "An error ocurred while trying to close the read end of the pipe");
        return -1;
    }

    return 0;
}