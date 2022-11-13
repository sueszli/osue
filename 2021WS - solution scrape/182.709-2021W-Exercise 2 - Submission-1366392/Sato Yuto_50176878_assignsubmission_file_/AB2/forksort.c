#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

typedef struct
{
    pid_t pid;
    int pipefd_read[2];
    int pipefd_write[2];
} child_struct;

char *inputLine, *inputLine2;
char *childLine, *childLine2;
int lineCount = 0;
FILE *fp;
const char *programName;
child_struct child1, child2;

static int checkForMultipleLines(FILE *fp, char **line, char **line2);
static void splitLinesForChildren();
//static int readLine(FILE *stream, char **input);
static void initChild(child_struct *child, char **sendFirstLine);
static void initPipes(child_struct *child);
static void compareSolutions();
static void waitForChild(child_struct child);

int main(int argc, const char *argv[])
{
    programName = argv[0];
    fp = stdin;
    inputLine = malloc(sizeof(char*));
    inputLine2 = malloc(sizeof(char*));
    lineCount = checkForMultipleLines(fp, &inputLine, &inputLine2);

    if (lineCount == 1)
    {
        fprintf(stdout, "%s", inputLine);
        free(inputLine);
        free(inputLine2);
        fclose(fp);
        exit(EXIT_SUCCESS);
    }

    initChild(&child1, &inputLine);
    initChild(&child2, &inputLine2);
    free(inputLine);
    free(inputLine2);

    splitLinesForChildren();

    waitForChild(child1);
    waitForChild(child2);
    //childLine = malloc(sizeof(char*));
    //childLine2 = malloc(sizeof(char*));
    compareSolutions();
    close(child1.pipefd_write[0]);
    close(child2.pipefd_write[0]);

    fclose(fp);

    exit(EXIT_SUCCESS);
}

static int checkForMultipleLines(FILE *fp, char **line, char **line2)
{
    char c;
    int linePos = 0;
    int inputLines = 0;
    while ((c = fgetc(fp)) != EOF)
    {

        *line2 = realloc(*line2, ((linePos + 1) * sizeof(char)));
        (*line2)[linePos] = c;
        linePos++;
        if (c == '\n')
        {
            inputLines++;
            if (inputLines == 2)
            {
                break;
            }
            *line = realloc(*line, strlen((*line2)));
            strcpy((*line), (*line2));
            free(*line2);
            *line2 = malloc(sizeof(char*));
            linePos = 0;
        }
    }
    if (inputLines == 0)
    {
        free(*line);
        free(*line2);
        exit(EXIT_FAILURE);
    }
    return inputLines;
}

/*
static int readLine(FILE *stream, char **input)
{

    int linePos = 0;
    char c;
    
    c = getline(input, 0, stream);
    //fprintf(stderr, "--%s", (*input));
    if((c = getline(input, 0, stream)) != -1)
    {
        return 0;
    }else{
        return 1;
    }
    

    while ((c = fgetc(stream)) != EOF)
    {
        if (c == '\n')
        {
            return 0;
        }
        *input = realloc(*input, ((linePos + 1) * sizeof(char)));
        (*input)[linePos] = c;
        linePos++;
    }

    return 1;
}
*/

static void splitLinesForChildren(void)
{

    int counter = 0;
    int eofFound = 0;
    char *sendLine = NULL;
    size_t size = 0;

    while ((eofFound = getline(&sendLine, &size, stdin)) != -1)
    {
        
        //eofFound = readLine(stdin, &sendLine);

        if (counter % 2 == 0)
        {

            dprintf(child1.pipefd_read[1], "%s", sendLine);
            //fprintf(stderr, "%d", success);
        }
        else
        {
            dprintf(child2.pipefd_read[1], "%s", sendLine);
            //fprintf(stderr, "%d", success);
        }
        counter++;
    }
    free(sendLine);
    close(child1.pipefd_read[1]);
    close(child2.pipefd_read[1]);
}

static void initChild(child_struct *child, char **sendFirstLine)
{
    initPipes(child);

    child->pid = fork();
    switch (child->pid)
    {
    case -1:
        fprintf(stderr, "%s child creation failed!", strerror(errno));
        exit(EXIT_FAILURE);
        break;
    case 0:
        close(child->pipefd_read[1]);
        dup2(child->pipefd_read[0], STDIN_FILENO);

        close(child->pipefd_write[0]);
        dup2(child->pipefd_write[1], STDOUT_FILENO);

        close(child->pipefd_read[0]);
        close(child->pipefd_write[1]);

        execlp(programName, programName, NULL);

        break;
    default:
        close(child->pipefd_read[0]);
        dprintf(child->pipefd_read[1], "%s", (*sendFirstLine));
        //close(child->pipefd_read[1]);
    }
}

static void initPipes(child_struct *child)
{
    pipe(child->pipefd_read);
    pipe(child->pipefd_write);
}

static void compareSolutions()
{
    close(child1.pipefd_write[1]);
    close(child2.pipefd_write[1]);
    FILE *readFirstChild = fdopen(child1.pipefd_write[0], "r");
    FILE *readSecondChild = fdopen(child2.pipefd_write[0], "r");
    int firstChildEmpty = 0;
    int secondChildEmpty = 0;
    int chooseFirst = 1;
    int chooseSecond = 1;
    int encounterEOFC1 = 0;
    int encounterEOFC2 = 0;
    char *childOutput = NULL;
    char *childOutput2 = NULL;
    size_t size1 = 0;
    size_t size2 = 0;

    while ((firstChildEmpty == 0) || (secondChildEmpty == 0))
    {
        if (chooseFirst == 1)
        {
            encounterEOFC1 = getline(&childOutput, &size1, readFirstChild);
        }
        if (chooseSecond == 1)
        {
            encounterEOFC2 = getline(&childOutput2, &size2, readSecondChild);
        }

        if (encounterEOFC1 == -1)
        {
            firstChildEmpty = 1;
            break;
        }
        else if (encounterEOFC2 == -1)
        {
            secondChildEmpty = 1;
            break;
        }
        if ((strcmp(childOutput, childOutput2)) < 0)
        {
            fprintf(stdout, "%s", childOutput);
            chooseSecond = 0;
            chooseFirst = 1;
        }
        else
        {
            fprintf(stdout, "%s", childOutput2);
            chooseSecond = 1;
            chooseFirst = 0;
        }
    }

    if (firstChildEmpty == 1)
    {
        fprintf(stdout, "%s", childOutput2);
        while ((encounterEOFC2 = getline(&childOutput2, &size2, readSecondChild)) != -1)
        {
            fprintf(stdout, "%s", childOutput2);
        }
    }
    else if (secondChildEmpty == 1)
    {
        fprintf(stdout, "%s", childOutput);
        while ((encounterEOFC1= getline(&childOutput, &size1, readFirstChild)) != -1)
        {
            fprintf(stdout, "%s", childOutput);
        }
    }

    free(childOutput);
    free(childOutput2);
}

static void waitForChild(child_struct child)
{

    int status;
    while (waitpid(child.pid, &status, 0) == -1)
    {
        if (errno == EINTR)
            continue;
        exit(EXIT_FAILURE);
    }

    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        exit(EXIT_FAILURE);
    }
}
