/**
 * @author Markus Böck 12020632
 * @date 22.11.2021
 * @brief forksort implementation
 * @details Implements a parallel mergesort via multiple processes.
 * Every process that receives more than 1 line, splits them into two
 * and calls a subprocess of itself to sort both halfs. It then merges
 * both halfs together to get a single sorted list.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>

/**
 * @brief points to argv[0], the executable name of the program
 */
static char* executableName = NULL;

#ifdef __GNUC__
#define FORMAT_FUNC __attribute__((format(printf, 1, 2)))
#else
#define FORMAT_FUNC
#endif

/**
 * @brief utility function for outputting errors
 * 
 * This function is used as a utility throughout the program to ease outputting errors.
 * All output is done to stderr with the value of the global 'executableName' preprended to the output.
 * It's signature is identical to printf, and its format strings + varargs allow the same values and options as printf.
 * @param format Format string as defined by printf & friends
 */
FORMAT_FUNC static void error(const char* format,...) 
{
    (void)fprintf(stderr, "%s: ", executableName);
    va_list list;
    va_start(list, format);
    (void)vfprintf(stderr, format, list);
    va_end(list);
    fprintf(stderr, "\n");
}

/**
 * @brief Datastructure used to hold a list of lines
 */
typedef struct LineBuffer
{
    size_t lineCount;
    char** lines;
} LineBuffer;

/**
 * @brief deallocates ALL memory referenced by the LineBuffer 
 * 
 * This function deallocates both the array of lines as well as 
 * every line.
 * 
 * @param lineBuffer LineBuffer to deallocate
 */
static void destroyLineBuffer(LineBuffer* lineBuffer)
{
    for (size_t i = 0; i < lineBuffer->lineCount; i++)
    {
        free(lineBuffer->lines[i]);
    }
    free(lineBuffer->lines);
}

/**
 * @brief struct holding everything important about a child process
 */
struct Child
{
    pid_t pid;
    int readPipe;
    int writePipe;
    size_t linesWritten;
};

/**
 * @brief creates a new child and sets it up for bidirectional 
 * communication
 * 
 * @param child Child datastructure in which pipes and pid are inserted
 * @return true upon success, false otherwise
 */
static bool setupChild(struct Child* child)
{
    int stdinPipe[2];
    int stdoutPipe[2];
    if(pipe(stdinPipe) < 0)
    {
        error("stdin pipe failed with errno: %s", strerror(errno));
        return false;
    }
    if(pipe(stdoutPipe) < 0)
    {
        error("stdout pipe failed with errno: %s", strerror(errno));
        return false;
    }
    child->pid = fork();
    if (child->pid == -1)
    {
        error("Error creating a new process");
        return false;
    }
    if (child->pid != 0)
    {
        // Parent code
        close(stdinPipe[0]);
        child->readPipe = stdoutPipe[0];
        close(stdoutPipe[1]);
        child->writePipe = stdinPipe[1];
        return true;
    }
    
    // Child code
    child->readPipe = stdinPipe[0];
    close(stdinPipe[1]);
    if (dup2(child->readPipe, STDIN_FILENO) < 0)
    {
        error("Failed to replace stdin");
        return false;
    }
    close(child->readPipe);
    
    close(stdoutPipe[0]);
    if (dup2(stdoutPipe[1], STDOUT_FILENO) < 0)
    {
        error("Failed to replace stdout");
        return false;
    }
    close(stdoutPipe[1]);
    
    if (execlp(executableName, executableName, (char*)NULL) < 0)
    {
        error("exec error starting %s: %s", executableName, strerror(errno));
        return false;
    }
    // actually unreachable
    return true;
}

/**
 * @brief enum indicating the various scenarios based on stdin input
 */
typedef enum HandleStdinStatus
{
    HandleStdinStatusError, ///< An error occured and the application should exit with error
    HandleStdinStatusTrivial, ///< It was a trivial case that was handled and the application can exit successfully
    HandleStdinStatusRecursive, ///< It was a recursive case and child processes have been started and already written too
} HandleStdinStatus;

/**
 * @brief Reads input from stdin and handles the three possible cases
 * 
 * @param left Left child in the tree that will be created and written to in the recursive case
 * @param right Right child in the tree that will be created and written to in the recursive case
 * @return enum indicating the scenario
 */
static HandleStdinStatus handleStdin(struct Child* left, struct Child* right)
{
    HandleStdinStatus result = HandleStdinStatusError;
    char* trivialLine = NULL;
    char* currentLine = NULL;
    bool outputToLeft = false;
    size_t n = 0;
    do
    {
        if (getline(&currentLine, &n, stdin) < 0)
        {
            break;
        }
        switch (result)
        {
        case HandleStdinStatusError:
            // State transition from error (no input) -> trivial
            trivialLine = currentLine;
            currentLine = NULL;
            result = HandleStdinStatusTrivial;
            continue;
        case HandleStdinStatusTrivial:
            // State transition from trivial -> recursive
            result = HandleStdinStatusRecursive;
            if (!setupChild(left))
            {
                free(trivialLine);
                free(currentLine);
                return HandleStdinStatusError;
            }
            if (!setupChild(right))
            {
                // Close left childs stdin to force its termination
                // and wait for that termination to occur
                (void)close(left->writePipe);
                waitpid(left->pid, NULL, 0);
                free(trivialLine);
                free(currentLine);
                return HandleStdinStatusError;
            }
            if (write(left->writePipe, trivialLine, strlen(trivialLine)) < 0)
            {
                goto error;
            }
            free(trivialLine);
            trivialLine = NULL;
            left->linesWritten++;
            break;
        default:break;
        }
        struct Child* childToWriteTo = outputToLeft ? left : right;
        if (write(childToWriteTo->writePipe, currentLine, strlen(currentLine)) < 0)
        {
            goto error;
        }
        childToWriteTo->linesWritten++;
        outputToLeft = !outputToLeft;
    }
    while (true);
    
    switch (result)
    {
    case HandleStdinStatusError:
        error("Got no text :(");
        break;
    case HandleStdinStatusTrivial:
        printf("%s", trivialLine);
        printf("%s", trivialLine);
        break;
    case HandleStdinStatusRecursive:
        (void)close(left->writePipe);
        (void)close(right->writePipe);
        break;
    default: assert(0);
    }
    free(trivialLine);
    free(currentLine);
    return result;
    
error:
    free(trivialLine);
    free(currentLine);
    (void)close(left->writePipe);
    (void)close(right->writePipe);
    waitpid(left->pid, NULL, 0);
    waitpid(right->pid, NULL, 0);
    return HandleStdinStatusError;
}


/**
 * @brief struct used to hold the image of a tree graph
 * 
 * This struct contains the image of a tree graph as is output by
 * children. It has height amount of lines and the largest line is
 * width characters long. 
 */
typedef struct GraphBlock
{
    int height, width;
    char** lines;
} GraphBlock;

/**
 * @brief deallocates ALL memory referenced by the GraphBlock
 * 
 * This function deallocates both the array of lines as well as 
 * every line.
 * 
 * @param graphBlock GraphBlock to deallocate
 */
static void destroyGraphBlock(GraphBlock* graphBlock)
{
    for (size_t i = 0; i < graphBlock->height; i++)
    {
        free(graphBlock->lines[i]);
    }
    free(graphBlock->lines);
}

/**
 * @brief Receives the lines from the child as well as the tree
 *        graph
 * 
 * @param child Child to get the update lines from
 * @param lineBuffer LineBuffer to append the new lines to
 * @param block Destination graph block to insert the childs graph into
 * @return true upon success, false otherwise
 */
static bool readLines(struct Child* child, LineBuffer* lineBuffer, GraphBlock* block)
{
    size_t lineCapacity = lineBuffer->lineCount;
    for (size_t i = 0; i < child->linesWritten; i++)
    {
        lineBuffer->lineCount++;
        if (lineBuffer->lineCount > lineCapacity)
        {
            lineCapacity *= 1.5;
            if (lineCapacity < lineBuffer->lineCount)
            {
                lineCapacity = lineBuffer->lineCount;
            }
            char** realloced = realloc(lineBuffer->lines, lineCapacity * sizeof(*lineBuffer->lines));
            if (!realloced)
            {
                error("Out of memory");
                return false;
            }
            lineBuffer->lines = realloced;
        }
        lineBuffer->lines[lineBuffer->lineCount-1] = NULL;
        size_t currentCapacity = 0;
        size_t currentSize = 0;
        char character;
        while (read(child->readPipe, &character, sizeof character))
        {
            currentSize++;
            if (currentSize > currentCapacity)
            {
                currentCapacity *= 1.5;
                if (currentCapacity < currentSize)
                {
                    currentCapacity = currentSize;
                }
                char* newBuffer = realloc(lineBuffer->lines[lineBuffer->lineCount-1], sizeof(char) * currentCapacity);
                if (!newBuffer)
                {
                    error("Out of memory");
                    return false;
                }
                lineBuffer->lines[lineBuffer->lineCount-1] = newBuffer;
            }
            if (character == '\n')
            {
                lineBuffer->lines[lineBuffer->lineCount-1][currentSize-1] = '\0';
                break;
            }
            lineBuffer->lines[lineBuffer->lineCount-1][currentSize-1] = character;
        }
    }
    
    size_t currentSize = 0;
    size_t currentCapacity = 0;
    char character;
    while (read(child->readPipe, &character, sizeof character))
    {
        if (currentSize == 0)
        {
            block->height++;
            char** lines = realloc(block->lines, sizeof(char*) * block->height);
            if (!lines)
            {
                error("Out of memory");
                destroyGraphBlock(block);
                return false;
            }
            block->lines = lines;
            block->lines[block->height - 1] = NULL;
        }
        currentSize++;
        if (currentSize > currentCapacity)
        {
            currentCapacity *= 1.5;
            if (currentCapacity < currentSize)
            {
                currentCapacity = currentSize;
            }
            char* newBuffer = realloc(block->lines[block->height-1], sizeof(char) * currentCapacity);
            if (!newBuffer)
            {
                error("Out of memory");
                destroyGraphBlock(block);
                return false;
            }
            block->lines[block->height-1] = newBuffer;
        }
        if (character == '\n')
        {
            block->lines[block->height-1][currentSize-1] = '\0';
            if (currentSize - 1 > block->width)
            {
                block->width = currentSize - 1;
            }
            currentSize = 0;
            currentCapacity = 0;
            continue;
        }
        block->lines[block->height-1][currentSize-1] = character;
    }
    
    return true;
}

/**
 * @brief sorts the two halves of the LineBuffer inplace. Assumes both
 * halfs themselves are already sorted
 * 
 * @param lineBuffer lineBuffer to sort inplace
 * @param middle Index of where the second halve begins
 * @return true upon success, false otherwise
 */
static bool merge(struct LineBuffer* lineBuffer, size_t middle)
{
    struct LineBuffer temp;
    temp.lineCount = lineBuffer->lineCount;
    temp.lines = malloc(sizeof(*temp.lines) * temp.lineCount);
    if (!temp.lines)
    {
        error("Out of memory");
        return false;
    }
    
    char** dest = temp.lines;
    char** lhs = lineBuffer->lines;
    char** rhs = lineBuffer->lines + middle;
    while (lhs != lineBuffer->lines + middle
            && rhs != lineBuffer->lines + lineBuffer->lineCount)
    {
        if (strcmp(*lhs, *rhs) <= 0)
        {
            *dest = *lhs;
            dest++;
            lhs++;
        }
        else
        {
            *dest = *rhs;
            dest++;
            rhs++;
        }
    }
    for (;lhs != lineBuffer->lines + middle; lhs++, dest++)
    {
        *dest = *lhs;
    }
    for (;rhs != lineBuffer->lines + lineBuffer->lineCount; rhs++, dest++)
    {
        *dest = *rhs;
    }
    
    free(lineBuffer->lines);
    *lineBuffer = temp;
    return true;
}

/**
 * @brief renders the new tree graph
 * 
 * This function takes the trees returned by the child processes
 * as well as the line buffer containing the result and renders
 * a new properly formatted tree
 * 
 * @param result Destination graphblock that will be inserted into
 * @param left left child that will be appearing on the left branch
 * @param right right child that will be appearing on the right
 * @param lineBuffer LineBuffer containing the results of this process
 * @return true upon success, false otherewise
 */
static bool 
renderNewGraphBlock(GraphBlock* result,const GraphBlock* left, 
            const GraphBlock* right,const struct LineBuffer* lineBuffer)
{
    if (left->height > right->height)
    {
        result->height = left->height;
    }
    else
    {
        result->height = right->height;
    }
    result->height += 2; // SORT header + / and \\ line
    result->lines = calloc(sizeof(char*), result->height);
    if (!result->lines)
    {
        error("Out of memory");
        return false;
    }
    result->width = left->width + 2 + right->width;
    int resultLineWidth = sizeof("SORT()") - 1; // Null terminator excluded
    for (size_t i = 0; i < lineBuffer->lineCount; i++)
    {
        resultLineWidth += strlen(lineBuffer->lines[i]);
    }
    resultLineWidth += lineBuffer->lineCount - 1; // add commas
    if (resultLineWidth > result->width)
    {
        result->width = resultLineWidth;
    }
    
    for (size_t i = 0; i < result->height; i++)
    {
        result->lines[i] = malloc(sizeof(char) * result->width + 1); // + 1 for Null terminator
        if (!result->lines[i])
        {
            destroyGraphBlock(result);
            return false;
        }
    }
    
    size_t offset = snprintf(result->lines[0], result->width + 1, "%*sSORT(", (result->width - resultLineWidth) / 2, "");
    offset += snprintf(result->lines[0] + offset, result->width - offset + 1, "%s", lineBuffer->lines[0]);
    for (size_t i = 1; i < lineBuffer->lineCount; i++)
    {
        offset += snprintf(result->lines[0] + offset, result->width - offset + 1, ",%s", lineBuffer->lines[i]);
    }
    snprintf(result->lines[0] + offset, result->width - offset + 1,")%*s", (result->width - resultLineWidth) / 2, "");
    
    int leftMiddle = left->width / 2;
    int rightMiddle = right->width / 2 + result->width - right->width;
    snprintf(result->lines[1], result->width + 1, "%*s%*s%-*s", leftMiddle,"/", 
                                                rightMiddle - 1 - leftMiddle,"",
                                                result->width - rightMiddle, "\\");
    for (size_t i = 2; i < result->height; i++)
    {
        if (left->height <= i - 2)
        {
            snprintf(result->lines[i], result->width + 1, "%*s%*s",
                        result->width - right->width, "", 
                        right->width, right->lines[i-2]);
        }
        else if (right->height <= i - 2)
        {
            snprintf(result->lines[i], result->width + 1, "%-*s", 
                        result->width, left->lines[i-2]);
        }
        else
        {
            snprintf(result->lines[i], result->width + 1, "%-*s%*s",
                        result->width - right->width, left->lines[i-2], 
                        right->width, right->lines[i-2]);
        }
    }
    return true;
}

/**
 * @brief Mainprogram of forksort
 * 
 * @param argc argument count
 * @param argv arguments
 * @return exit code
 */
int main(int argc, char* argv[])
{
    executableName = argv[0];
    if (argc > 1)
    {
        error("No command line arguments expected");
        printf("SYNOPSIS\n\tforksort\n");
        return EXIT_FAILURE;
    }
    
    struct Child left = {0};
    struct Child right = {0};
    switch (handleStdin(&left,&right))
    {
    case HandleStdinStatusTrivial:
        return EXIT_SUCCESS;
    case HandleStdinStatusRecursive:
        break;
    case HandleStdinStatusError:
        return EXIT_FAILURE;
    default: assert(0);
    }
    
    LineBuffer lineBuffer = {0};
    GraphBlock leftGraph = {0};
    if (!readLines(&left, &lineBuffer, &leftGraph))
    {
        goto error_read;
    }
    size_t middle = lineBuffer.lineCount;
    GraphBlock rightGraph = {0};
    if (!readLines(&right, &lineBuffer, &rightGraph))
    {
        goto error_read_right;
    }
    
    bool failure = false;
    int status;
    (void)waitpid(left.pid, &status, 0);
    failure = failure || status != 0;
    (void)waitpid(right.pid, &status, 0);
    failure = failure || status != 0;
    if (failure)
    {
        destroyGraphBlock(&leftGraph);
        destroyGraphBlock(&rightGraph);
        goto error;
    }
    
    if (!merge(&lineBuffer, middle))
    {
        destroyGraphBlock(&leftGraph);
        destroyGraphBlock(&rightGraph);
        goto error;
    }
    for (size_t i = 0; i < lineBuffer.lineCount; i++)
    {
        printf("%s\n", lineBuffer.lines[i]);
    }
    
    GraphBlock result = {0};
    if (!renderNewGraphBlock(&result, &leftGraph, &rightGraph, &lineBuffer))
    {
        destroyGraphBlock(&leftGraph);
        destroyGraphBlock(&rightGraph);
        goto error;
    }
    for (size_t i = 0; i < result.height; i++)
    {
        printf("%s\n", result.lines[i]);
    }
    
    destroyGraphBlock(&result);
    destroyGraphBlock(&leftGraph);
    destroyGraphBlock(&rightGraph);
    destroyLineBuffer(&lineBuffer);
    return EXIT_SUCCESS;
    
error_read_right:
    destroyGraphBlock(&leftGraph);
error_read:
    (void)waitpid(right.pid, &status, 0);
    (void)waitpid(left.pid, &status, 0);
error:
    destroyLineBuffer(&lineBuffer);
    return EXIT_FAILURE;
}
