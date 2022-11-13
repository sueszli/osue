#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

static void testForMoreThanOneLine(void);
static void initializeProcesses(void);
static void shareInput();
static void tryWaitForChildCompletion(pid_t processId);
static void compareLines();
//static void compareSolutions();
//static int checkForMultipleLines(FILE *fp, char **line, char **line2);

pid_t processId_1;
pid_t processId_2;

int pipefd_1[2];
int pipefd_2[2];
int pipefd_3[2];
int pipefd_4[2];

char *buf_2 = NULL;
char *buf_1 = NULL;
const char *PATH = NULL;
char *bufGlobal;
char *line1;
char *line2;
char *inputLine, *inputLine2;
int globalCounter = 0;
int lineInteger1 = 0;
int lineInteger2 = 0;
FILE *fp1, *fp2;

int main(int argc, char const *argv[])
{

    PATH = argv[0];

    if (argc > 1)
    {
        fprintf(stderr, "%s", "USAGE: ...");
        exit(EXIT_FAILURE);
    }

    testForMoreThanOneLine();
    /*
    buf_1 = malloc(sizeof(char*));
    buf_2 = malloc(sizeof(char*));
    int lineCount;
    lineCount = checkForMultipleLines(stdin, &buf_1, &buf_2);

    if (lineCount == 1)
    {
        fprintf(stdout, "%s", buf_1);
        free(buf_1);
        free(buf_2);
        exit(EXIT_SUCCESS);
    }
    */

    initializeProcesses();
    shareInput();

    tryWaitForChildCompletion(processId_1);
    tryWaitForChildCompletion(processId_2);

    //line1 = malloc(1000);
    //line2 = malloc(1000);

    compareLines();

    //compareSolutions();
    close(pipefd_2[0]);
    close(pipefd_4[0]);
    exit(EXIT_SUCCESS);
}

/*static int checkForMultipleLines(FILE *fp, char **line, char **line2)
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
            *line2 = malloc(sizeof(char *));
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
}*/

static void testForMoreThanOneLine(void)
{
    int counter = 0;
    size_t bufsize1 = 0;
    size_t bufsize2 = 0;
   // int result = 0;
    /*
    while (((result = getline(&buf_1, &bufsize, stdin)) != EOF))
    {
        counter++;
        if (counter > 1)
        {
            break;
        }
        // buf_2 = realloc(buf_2, sizeof(buf_1));
        strcpy(buf_2, buf_1);
    }
    */
    if ((getline(&buf_1, &bufsize1, stdin)) != -1)
    {
        counter++;
    }
    if ((getline(&buf_2, &bufsize2, stdin)) != -1)
    {
        counter++;
    }
    //if (((strlen(buf_1) - 1) == 0) || ((strlen(buf_2) - 1) == 0))
    if (counter == 1)
    {
        fprintf(stdout, "%s", buf_1);
        free(buf_2);
        free(buf_1);
        exit(EXIT_SUCCESS);
    }

    /*
    if((*buf_1 == '/') && (*buf_2 == '/')){
        fprintf(stderr, "%s\n", "You need to write at least one word and press ENTER!");
        free(buf_1);
        free(buf_2);
        exit(EXIT_FAILURE);

    }
    if((*buf_1 == '/'))
    {
        fprintf(stdout, "%s\n", buf_2);
        free(buf_2);
        free(buf_1);
        exit(EXIT_SUCCESS);
    }
    if((*buf_2 == '/')){
        fprintf(stdout, "%s\n", buf_1);
        free(buf_2);
        free(buf_1);
        exit(EXIT_SUCCESS);
    }
    */

    if (counter == 0)
    {
        fprintf(stderr, "%s\n", "You need to write at least one word and press ENTER!");
        free(buf_1);
        free(buf_2);
        exit(EXIT_FAILURE);
    }

    // free(buf_1);
}

static void initializeProcesses(void)
{
    pipe(pipefd_1);
    pipe(pipefd_2);

    // int test;
    // int test2;
    //printf("%s\n", "Initalize Child!");

    processId_1 = fork();

    switch (processId_1)
    {
    case -1:

        //  printf("%s", "Process initialization FAILED!");
        exit(EXIT_FAILURE);
        break;
    case 0:
        //printf("%s", "Child Process SUCCESSFULLY initialized!");

        close(pipefd_1[1]);
        if (dup2(pipefd_1[0], STDIN_FILENO) < 0)
        {
            exit(EXIT_FAILURE);
        }
        //close(pipefd_1[0]);

        close(pipefd_2[0]);
        dup2(pipefd_2[1], STDOUT_FILENO); // out

        close(pipefd_1[0]);
        close(pipefd_2[1]);

        execlp(PATH, PATH, NULL);

        break;

    default:

        close(pipefd_1[0]);
        //buf_1 = malloc(strlen(inputLine));
        //strcpy(buf_1,inputLine);
        //fp1 = fdopen(pipefd_1[1], "w");
        //fprintf(fp1, "%s", buf_1);
        //fflush(fp1);
        //test = write(pipefd_1[1], buf_1, strlen(buf_1));

        dprintf(pipefd_1[1], "%s", buf_1);

        //close(pipefd_1[1]); // DIESES CLOSE MUSS NOCH GEFIXT WERDEN (RICHTIGE STELLE!)

        free(buf_1);
        //buf_1 = NULL;
        //fprintf(stdout, "%s", "Parent process1!");

        //write(pipefd_1[1], buf_2, strlen(buf_2));

        break;
    }

    pipe(pipefd_3);
    pipe(pipefd_4);

    processId_2 = fork();

    switch (processId_2)
    {
    case -1:

        //  printf("%s", "Process initialization FAILED!");
        exit(EXIT_FAILURE);
        break;

    case 0:
        // printf("%s", "Child Process SUCCESSFULLY initialized!");
        //  fprintf(stdout, "%s", "Parent process!");
        close(pipefd_3[1]);
        dup2(pipefd_3[0], STDIN_FILENO);

        close(pipefd_4[0]);
        dup2(pipefd_4[1], STDOUT_FILENO);

        close(pipefd_3[0]);
        close(pipefd_4[1]);

        execlp(PATH, PATH, NULL);
        break;

    default:

        //    write(pipefd_3[1], buf_1, strlen(buf_1));
        close(pipefd_3[0]);
        //buf_2 = malloc(strlen(inputLine2));
        //strcpy(buf_2,inputLine2);
        //fp2 = fdopen(pipefd_3[1], "w");
        //fprintf(fp2, "%s", buf_2);
        //fflush(fp2);
        //test2 = write(pipefd_3[1], buf_2, strlen(buf_2));
        //close(pipefd_3[1]); // DIESES CLOSE MUSS NOCH GEFIXT WERDEN (RICHTIGE STELLE!)
        dprintf(pipefd_3[1], "%s", buf_2);
        free(buf_2);
        //buf_2 = NULL;
        //     fprintf(stdout, "%s", "Parent process2!");
        break;
    }
}

static void shareInput()
{

    int counter = 0;
    char *buf_3 = NULL;
    size_t bufsize = 0;

    /*
    close(pipefd_1[0]);
    close(pipefd_3[0]);
    */

    int test;
    int test2;
    while ((getline(&buf_3, &bufsize, stdin)) != -1)
    {
        /*
        if (strlen(buf_3) == 1)
        {
            dprintf(pipefd_1[1], "%s", buf_3);
            dprintf(pipefd_3[1], "%s", buf_3);
            break;
        }
        */
        if (counter % 2 == 0)
        {
            //test = write(pipefd_1[1], buf_3, strlen(buf_3));
            //test = fprintf(fp1, "%s", buf_3);
            dprintf(pipefd_1[1], "%s", buf_3);
            //fflush(fp1);
        }
        else
        {
            //test2 = write(pipefd_3[1], buf_3, strlen(buf_3));
            dprintf(pipefd_3[1], "%s", buf_3);
            //test = fprintf(fp2, "%s", buf_3);
            //fflush(fp2);
        }
        counter++;
    }
    /*
    write(pipefd_1[1], "\0", sizeof(char));
    write(pipefd_3[1], "\0", sizeof(char));
    dprintf(pipefd_1[1], "%c", '\n');
    dprintf(pipefd_3[1], "%c", '\n');
    */
    free(buf_3);
    test = close(pipefd_1[1]);
    test2 = close(pipefd_3[1]);
    //fclose(fp1);
    //fclose(fp2);
}

static void compareLines()
{
    // printf("%s%s\n", "LINE1:", *line1);
    // printf("%s%s\n", "LINE2:", *line2);
    close(pipefd_2[1]);
    close(pipefd_4[1]);
    FILE *fp1 = fdopen(pipefd_2[0], "r");
    if (fp1 == NULL)
    {
        exit(EXIT_FAILURE);
    }

    FILE *fp2 = fdopen(pipefd_4[0], "r");
    if (fp2 == NULL)
    {
        exit(EXIT_FAILURE);
    }

    size_t size1 = 0;
    size_t size2 = 0;

    // int exit = 0;
    int linePos1 = 0;
    int linePos2 = 0;
    int x = 0;
    // char a;
    // char b;
    // char c;
    // char d;
    char compare;
    /*
    char *input1, *input2;
    size_t size1 = 0;
    size_t size2 = 0;
    getline(&input1, &size1, fp1);
    getline(&input2, &size2, fp2);
    fprintf(stderr, "%s", input1);
    fprintf(stderr, "%s", input2);

    return 1;
    */
    // printf("%s\n", "HALLOOO");
    while (1)
    {
        int eofline1;
        int eofline2;
        if (lineInteger1 == 0)
        {
            eofline1 = getline(&line1, &size1, fp1);
        }
        if (lineInteger2 == 0)
        {
            eofline2 = getline(&line2, &size2, fp2);
        }

        if ((lineInteger1 == 0) && (lineInteger2 == 0))
        {
            if (eofline1 == EOF)
            {
                if (eofline2 == EOF)
                {
                    break;
                }
                //ungetc(c, fp1);
            }
            //ungetc(a, fp2);
        }

        // printf("%s%s\n", "LINE2.2:", *line2);

        if ((eofline1 == EOF) && (lineInteger1 == 0))
        {
            //printf("%s\n", *line2);
            //ungetc(c, fp1);
            fprintf(stdout, "%s", line2);
            while ((getline(&line2, &size2, fp2)) != EOF)
            {
                fprintf(stdout, "%s", line2);
            }
            break;
        }
        else if ((eofline2 == EOF) && (lineInteger2 == 0))
        {
            //ungetc(a, fp2);
            //ungetc(c, fp1);
            //printf("%s\n", *line1);
            fprintf(stdout, "%s", line1);
            while ((getline(&line1, &size1, fp1)) != EOF)
            {
                // printf("%d", 10000);

                fprintf(stdout, "%s", line1);
            }
            break;
        }
        /*
        ungetc(c, fp1);
        ungetc(a, fp2);

        // ungetc(c, fp1);
        //  ungetc(a, fp2);

        // fseek(fp1, -1, SEEK_CUR);
        // fseek(fp2, -1, SEEK_CUR);
        while ((getline(&input1, &size1, fp1)) != -1)
        {

            fprintf(stderr, "%s", input1);
        }
        while ((getline(&input2, &size2, fp2)) != -1)
        {
            fprintf(stderr, "%s", input2);
        }
        return;
        */
       /*
        if ((lineInteger1 == 0))
        {
            getline(line1, &size1, fp1);
            
            line1 = malloc(sizeof(char *));
            while ((b = fgetc(fp1)) != EOF)
            {
                *line1 = realloc(*line1, (linePos1 + 1) * sizeof(char));
                (*line1)[linePos1] = b;
                //fprintf(stderr, "%c%c", b, '_');

                if (b != '\n')
                {
                    linePos1++;
                }
                else
                {
                    lineInteger1 = 1;
                    break;
                }
            }
            
        }
        */

        // printf("%c", '\n');
        //  printf("%s", *line1);
        // exit(EXIT_SUCCESS);
        /*
        if ((lineInteger2 == 0))
        {
            line2 = malloc(sizeof(char *));
            while ((d = fgetc(fp2)) != EOF)
            {
                //  printf("%c%c", d, '_');
                *line2 = realloc(*line2, (linePos2 + 1) * sizeof(char));
                (*line2)[linePos2] = d;
                //fprintf(stderr, "%c%c", d, '_');

                if (d != '\n')
                {
                    linePos2++;
                }
                else
                {
                    lineInteger2 = 1;
                    // printf("%s\n", "HALLOOO");
                    break;
                }
            }
        }
        */
        //fprintf(stderr, "Line1: %s", line1);
        //fprintf(stderr, "Line2: %s", line2);

        lineInteger1 = 1;
        lineInteger2 = 1;

        if ((strcmp(line1, line2)) == 0)
        {
            // printf("%s\n", *line1);
            // printf("%s\n", *line2);

            fprintf(stdout, "%s", line1);
            fprintf(stdout, "%s", line2);
            line1 = NULL;
            line2 = NULL;
            //free(*line1);
            //free(*line2);
            lineInteger1 = 0;
            lineInteger2 = 0;
        }
        else
        {

            // exit(EXIT_SUCCESS);

            if (strlen(line1) < strlen(line2))
            {
                x = strlen(line1);
            }
            else
            {
                x = strlen(line2);
            }

            // BIS HIERHER KORREKT!

            // printf("%s", "HAAALLOOO2");
            for (size_t i = 0; i < x; i++)
            {
                if (((line1)[i] < (line2)[i]))
                {
                    if (((line1)[i] <= 90) && ((line2)[i] <= 90))
                    {
                        fprintf(stdout, "%s", line1);

                        line1 = NULL;
                        //free(*line1);
                        lineInteger1 = 0;

                        // fseek(fp2, -(linePos2 + 1), SEEK_CUR);

                        // while (linePos2 >= 0)
                        // {
                        //     c = (int)(*line2)[linePos2];
                        //     ungetc(c, fp2);
                        //     linePos2--;
                        // }
                        break;
                    }
                    else
                    {
                        if ((line1)[i] <= 90)
                        {
                            compare = (line1)[i] + 32;

                            if (compare <= (line2)[i])
                            {
                                // write(stdout, line1, strlen(line1));
                                fprintf(stdout, "%s", line1);
                                line1 = NULL;
                                //free(*line1);
                                lineInteger1 = 0;

                                // fseek(fp2, -(linePos2 + 1), SEEK_CUR);

                                //  while (linePos2 >= 0)
                                //  {
                                //      c = (int)(*line2)[linePos2];
                                //      ungetc(c, fp2);
                                //      linePos2--;
                                //  }
                                break;
                            }
                            else
                            {
                                // write(stdout, line2, strlen(line2));
                                fprintf(stdout, "%s", line2);
                                line2 = NULL;

                                //free(*line2);
                                lineInteger2 = 0;

                                // fseek(fp1, -(linePos1 + 1), SEEK_CUR);
                                //  while (linePos1 >= 0)
                                //  {
                                //      c = (int)(*line1)[linePos1];
                                //      ungetc(c, fp1);
                                //      linePos1--;
                                //  }
                                break;
                            }
                        }
                        else
                        {
                            // write(stdout, line1, strlen(line1));
                            fprintf(stdout, "%s", line1);
                            line1 = NULL;

                            //free(*line1);
                            lineInteger1 = 0;

                            // fseek(fp2, -(linePos2 + 1), SEEK_CUR);

                            // while (linePos2 >= 0)
                            // {
                            //     c = (int)(*line2)[linePos2];
                            //     ungetc(c, fp2);
                            //     linePos2--;
                            // }
                            break;
                        }
                    }
                }
                else if (((line2)[i] < (line1)[i]))
                {
                    if (((line2)[i] <= 90) && ((line1)[i] <= 90))
                    {
                        fprintf(stdout, "%s", line2);
                        line2 = NULL;

                        //free(*line2);
                        lineInteger2 = 0;

                        // fseek(fp1, -(linePos1 + 1), SEEK_CUR);

                        //  while (linePos1 >= 0)
                        //  {
                        //      c = (int)(*line1)[linePos1];
                        //      ungetc(c, fp1);
                        //      linePos1--;
                        //  }
                        break;
                    }
                    else
                    {
                        if ((line2)[i] <= 90)
                        {
                            compare = (line2)[i] + 32;
                            if (compare <= (line1)[i])
                            {
                                // write(stdout, line2, strlen(line2));
                                fprintf(stdout, "%s", line2);
                                line2 = NULL;

                                //free(*line2);
                                lineInteger2 = 0;

                                // fseek(fp1, -(linePos1 + 1), SEEK_CUR);

                                //  while (linePos1 >= 0)
                                //  {
                                //      c = (int)(*line1)[linePos1];
                                //      ungetc(c, fp1);
                                //      linePos1--;
                                //  }
                                break;
                            }
                            else
                            {
                                // write(stdout, line1, strlen(line1));
                                fprintf(stdout, "%s", line1);
                                line1 = NULL;

                                //free(*line1);
                                lineInteger1 = 0;
                                // fseek(fp2, -(linePos2 + 1), SEEK_CUR);

                                //  while (linePos2 >= 0)
                                //  {
                                //      c = (int)(*line2)[linePos2];
                                //      ungetc(c, fp2);
                                //      linePos2--;
                                //  }
                                break;
                            }
                        }
                        else
                        {
                            // write(stdout, line2, strlen(line2));
                            fprintf(stdout, "%s", line2);
                            line2 = NULL;

                            //free(*line2);
                            lineInteger2 = 0;
                            // fseek(fp1, -(linePos1 + 1), SEEK_CUR);

                            //  while (linePos1 >= 0)
                            //  {
                            //      c = (int)(*line1)[linePos1];
                            //      ungetc(c, fp1);
                            //      linePos1--;
                            //  }
                            break;
                        }
                    }
                }
                else
                {

                    if ((i + 1) == x)
                    {
                        if (linePos1 < linePos2)
                        {
                            fprintf(stdout, "%s", line1);
                            line1 = NULL;

                            //free(*line1);
                            lineInteger1 = 0;

                            // fseek(fp2, -(linePos2 + 1), SEEK_CUR);
                            //   while (linePos2 >= 0)
                            //   {
                            //       c = (int)(*line2)[linePos2];
                            //       ungetc(c, fp2);
                            //       linePos2--;
                            //   }
                            break;
                        }
                        else
                        {
                            fprintf(stdout, "%s", line2);
                            line2 = NULL;

                            //free(*line2);
                            lineInteger2 = 0;
                            // fseek(fp1, -(linePos1 + 1), SEEK_CUR);
                            //   while (linePos1 >= 0)
                            //   {
                            //       c = (int)(*line1)[linePos1];
                            //       ungetc(c, fp1);
                            //       linePos1--;
                            //   }
                            break;
                        }
                    }
                }
            }
        }
    }
}

/*static void compareSolutions()
{
    close(pipefd_2[1]);
    close(pipefd_4[1]);
    FILE *readFirstChild = fdopen(pipefd_2[0], "r");
    FILE *readSecondChild = fdopen(pipefd_4[0], "r");
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
        while ((encounterEOFC1 = getline(&childOutput, &size1, readFirstChild)) != -1)
        {
            fprintf(stdout, "%s", childOutput);
        }
    }

    free(childOutput);
    free(childOutput2);
}*/

static void tryWaitForChildCompletion(pid_t processId)
{
    int status;
    while (waitpid(processId, &status, 0) == -1)
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
