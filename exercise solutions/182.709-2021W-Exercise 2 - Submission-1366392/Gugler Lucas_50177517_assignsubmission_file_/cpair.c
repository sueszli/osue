/**
 * @file cpair.c
 * @author Lucas Gugler 12019849
 * @date 8.12.2021
 *
 * @brief Module implements functionality of cpair
 **/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <sys/wait.h>
#include <regex.h>

#define BUF_SIZE 1024

typedef struct
{
    float x;
    float y;
} point;

static point *points = NULL;
static int numpoints = 0;

/**
 * Usage function.
 * @brief Prints an usage message to stderr and ends the program
 * @param prog_name Name of the program.
 **/
static void usage(const char prog_name[])
{
    fprintf(stderr, "[%s] Usage: %s \n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/** Cleanup function 
 * @brief Frees allocated memory 
 * @details Before termination the cleanup function frees all allocated memories 
 */
static void cleanup(void)
{
    if (points != NULL)
    {
        free(points);
    }
}

/**
 * Main function
 * @brief This function is the entrypoint of the cpair program. It implements
 * the full functionality of the cpair program. This includes argument handling, file handling,
 * compression of data and printing of statistics
 * @param argc
 * @param argv
 * @return Upon success EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 **/
int main(int argc, char *argv[])
{

    const char *const PROGNAME = argv[0];
    if (argc != 1)
    {
        usage(PROGNAME);
    }

    if (atexit(cleanup) < 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "setting up cleanup function failed");
        exit(EXIT_FAILURE);
    }

    //reading input
    char x[4];
    char y[4];
    points = malloc(1 * sizeof(*points));
    if (points == NULL)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "memory allocation");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, stdin)) != -1)
    {
        regex_t regex;
        int result;
        if (regcomp(&regex, "^[-+]?[0-9]*.?[0-9]+ [-+]?[0-9]*.?[0-9]+(\n)?$", REG_EXTENDED) != 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "regex setup failed");
            exit(EXIT_FAILURE);
        }
        result = regexec(&regex, line, 0, 0, 0);
        if (result != 0)
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "wrong input format");
            exit(EXIT_FAILURE);
        }
        regfree(&regex);
        if (sscanf(line, "%s %s\n", x, y) == 2)
        {
            float xn = strtof(x, NULL);
            if (errno != 0)
            {
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "wrong input format");
                exit(EXIT_FAILURE);
            }
            float yn = strtof(y, NULL);
            if (errno != 0)
            {
                fprintf(stderr, "[%s]: %s\n", PROGNAME, "wrong input format");
                exit(EXIT_FAILURE);
            }

            point p = {.x = xn, .y = yn};
            numpoints++;
            points = realloc(points, numpoints * sizeof(*points));
            points[numpoints - 1] = p;
        }
        else
        {
            fprintf(stderr, "[%s]: %s\n", PROGNAME, "wrong input format");
            exit(EXIT_FAILURE);
        }
    }
    free(line);
    //output
    if (numpoints == 0)
    {
        fprintf(stderr, "[%s]: %s\n", PROGNAME, "got 0 points as input");
        exit(EXIT_FAILURE);
    }
    if (numpoints == 1)
    {
        fprintf(stdout, "({%.1f, %.1f})\n", points[0].x, points[0].y);
        exit(EXIT_SUCCESS);
    }
    if (numpoints == 2)
    {
        fprintf(stdout, "%.1f, %.1f\n", points[0].x, points[0].y);
        fprintf(stdout, "%.1f, %.1f\n", points[1].x, points[1].y);
        fprintf(stdout, "({%.1f, %.1f}, {%.1f, %.1f})\n", points[0].x, points[0].y, points[1].x, points[1].y);
        exit(EXIT_SUCCESS);
    }

    //calc mean and arrays
    int xmean = 0;
    for (int i = 0; i < numpoints; i++)
    {
        xmean += points[i].x;
    }
    xmean /= numpoints;
    int numless = 0;
    int numgreater = 0;
    for (int i = 0; i < numpoints; i++)
    {
        if (points[i].x <= xmean)
        {
            numless++;
        }
        else
        {
            numgreater++;
        }
    }
    point smaller[numless];
    point bigger[numgreater];
    int indexsmall = 0;
    int indexbig = 0;
    for (int i = 0; i < numpoints; i++)
    {
        if (points[i].x <= xmean)
        {
            smaller[indexsmall++] = points[i];
        }
        else
        {
            bigger[indexbig++] = points[i];
        }
    }

    //setting up pipes
    int pipefdchild1read[2];
    if (pipe(pipefdchild1read) == -1)
    {
        fprintf(stderr, "Error setting up pipes\n");
        exit(EXIT_FAILURE);
    }
    int pipefdchild1write[2];
    if (pipe(pipefdchild1write) == -1)
    {
        fprintf(stderr, "Error setting up pipes\n");
        exit(EXIT_FAILURE);
    }
    int pipefdchild2read[2];
    if (pipe(pipefdchild2read) == -1)
    {
        fprintf(stderr, "Error setting up pipes\n");
        exit(EXIT_FAILURE);
    }
    int pipefdchild2write[2];
    if (pipe(pipefdchild2write) == -1)
    {
        fprintf(stderr, "Error setting up pipes\n");
        exit(EXIT_FAILURE);
    }

    //forking
    pid_t pid1, pid2;
    switch (pid1 = fork())
    {
    case -1:
        fprintf(stderr, "Cannot fork!\n");
        exit(EXIT_FAILURE);
    case 0:
        // child 1 tasks
        // closing unused pipes
        if (close(pipefdchild1read[1]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild2read[1]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild2read[0]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild1write[0]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild2write[0]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild2write[1]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if ((dup2(pipefdchild1read[0], STDIN_FILENO)) < 0)
        {
            fprintf(stderr, "Error rerouting pipes\n");
            exit(EXIT_FAILURE);
        }
        if ((dup2(pipefdchild1write[1], STDOUT_FILENO)) < 0)
        {
            fprintf(stderr, "Error rerouting pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild1read[0]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }
        if (close(pipefdchild1write[1]) != 0)
        {
            fprintf(stderr, "Error closing pipes\n");
            exit(EXIT_FAILURE);
        }

        //executing program
        if (execlp(PROGNAME, PROGNAME, NULL) == -1)
        {
            fprintf(stderr, "Cannot execute!\n");
            exit(EXIT_FAILURE);
        }

        break;
    default:
        switch (pid2 = fork())
        {
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            // child 2 tasks
            // closing unused pipes
            if (close(pipefdchild1read[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2read[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild1read[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild1write[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2write[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild1write[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if ((dup2(pipefdchild2read[0], STDIN_FILENO)) < 0)
            {
                fprintf(stderr, "Error rerouting pipes\n");
                exit(EXIT_FAILURE);
            }
            if ((dup2(pipefdchild2write[1], STDOUT_FILENO)) < 0)
            {
                fprintf(stderr, "Error rerouting pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2read[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2write[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }

            //executing program
            if (execlp(PROGNAME, PROGNAME, NULL) == -1)
            {
                fprintf(stderr, "Cannot execute!\n");
                exit(EXIT_FAILURE);
            }

            break;
        default:
            // parent tasks
            // closing unused pipes

            if (close(pipefdchild1read[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild1write[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2read[0]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }
            if (close(pipefdchild2write[1]) != 0)
            {
                fprintf(stderr, "Error closing pipes\n");
                exit(EXIT_FAILURE);
            }

            //writing to child processes
            FILE *filechild1 = fdopen(pipefdchild1read[1], "a");
            FILE *filechild2 = fdopen(pipefdchild2read[1], "a");

            for (int i = 0; i < numless; i++)
            {
                int size = snprintf(NULL, 0, "%f %f\n", smaller[i].x, smaller[i].y) + 1;
                char *buffer = malloc(size);
                if (buffer == NULL)
                {
                    fprintf(stderr, "memory allocation failed\n");
                    exit(EXIT_FAILURE);
                }
                snprintf(buffer, size, "%f %f\n", smaller[i].x, smaller[i].y);
                fprintf(filechild1, "%s", buffer);
                free(buffer);
            }

            for (int i = 0; i < numgreater; i++)
            {
                int size = snprintf(NULL, 0, "%f %f\n", bigger[i].x, bigger[i].y) + 1;
                char *buffer = malloc(size);
                if (buffer == NULL)
                {
                    fprintf(stderr, "memory allocation failed\n");
                    exit(EXIT_FAILURE);
                }
                snprintf(buffer, size, "%f %f\n", bigger[i].x, bigger[i].y);
                fprintf(filechild2, "%s", buffer);
                free(buffer);
            }
            if (fclose(filechild1) < 0 || fclose(filechild2) < 0) // closing write end of pipes
            {
                fprintf(stderr, "serror closing files\n");
                exit(EXIT_FAILURE);
            }

            //waiting for child processes
            int state;
            while (waitpid(pid1, &state, 0) < 0)
            {
                if (errno != EINTR)
                {
                    fprintf(stderr, "error waiting for child process\n");
                    exit(EXIT_FAILURE);
                }
            }

            if (WEXITSTATUS(state) != EXIT_SUCCESS) // first child failed
            {
                fprintf(stderr, "error waiting for child process\n");
                exit(EXIT_FAILURE);
            }

            while (waitpid(pid2, &state, 0) < 0)
            {
                if (errno != EINTR)
                {
                    fprintf(stderr, "error waiting for child process\n");
                    exit(EXIT_FAILURE);
                }
            }
            if (WEXITSTATUS(state) != EXIT_SUCCESS) // second child failed
            {
                fprintf(stderr, "error waiting for child process\n");
                exit(EXIT_FAILURE);
            }

            FILE *fchild1 = fdopen(pipefdchild1write[0], "r");
            FILE *fchild2 = fdopen(pipefdchild2write[0], "r");

            //reading input child 1
            char x1[4];
            char y1[4];
            char x2[4];
            char y2[4];
            point points1[2];
            point points2[2];
            int size1 = 0;
            int size2 = 0;

            char *line1 = NULL;
            char *line2 = NULL;
            size_t len1 = 0;
            size_t len2 = 0;

            if (numless >= 2)
            {
                while (size1 < 2)
                {
                    getline(&line1, &len1, fchild1);
                    sscanf(line1, "%s %s\n", x1, y1);
                    float xn = strtof(x1, NULL);
                    float yn = strtof(y1, NULL);
                    point p = {.x = xn, .y = yn};
                    size1++;
                    points1[size1 - 1] = p;
                }
            }
            free(line1);
            if (numgreater >= 2)
            {
                while (size2 < 2)
                {
                    getline(&line2, &len2, fchild2);
                    sscanf(line2, "%s %s\n", x2, y2);
                    float xn = strtof(x2, NULL);
                    float yn = strtof(y2, NULL);
                    point p = {.x = xn, .y = yn};
                    size2++;
                    points2[size2 - 1] = p;
                }
            }
            free(line2);

            if (size2 == 0)
            {
                fprintf(stdout, "%f, %f\n", points1[0].x, points1[0].y);
                fprintf(stdout, "%f, %f\n", points1[1].x, points1[1].y);
            }
            else if (size1 == 0)
            {
                fprintf(stdout, "%f, %f\n", points2[0].x, points2[0].y);
                fprintf(stdout, "%f, %f\n", points2[1].x, points2[1].y);
            }
            else
            {
                float distance = -1;
                point p31 = {.x = 0, .y = 0};
                point p32 = {.x = 0, .y = 0};
                for (int i = 0; i < 2; i++)
                {
                    point p1 = points1[i];
                    for (int j = 0; j < 2; j++)
                    {
                        point p2 = points2[j];
                        float dist = sqrt((pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)));
                        if (dist < distance || distance == -1)
                        {
                            distance = dist;
                            p31 = p1;
                            p32 = p2;
                        }
                    }
                }
                float distance1 = sqrt((pow(points1[1].x - points1[0].x, 2) + pow(points1[1].y - points1[0].y, 2)));
                float distance2 = sqrt((pow(points2[1].x - points2[0].x, 2) + pow(points2[1].y - points2[0].y, 2)));

                if (distance1 < distance2)
                {
                    if (distance1 < distance)
                    {
                        fprintf(stdout, "%f, %f\n", points1[0].x, points1[0].y);
                        fprintf(stdout, "%f, %f\n", points1[1].x, points1[1].y);
                    }
                    else
                    {
                        fprintf(stdout, "%f, %f\n", p31.x, p31.y);
                        fprintf(stdout, "%f, %f\n", p32.x, p32.y);
                    }
                }
                else
                {
                    if (distance2 < distance)
                    {
                        fprintf(stdout, "%f, %f\n", points2[0].x, points2[0].y);
                        fprintf(stdout, "%f, %f\n", points2[1].x, points2[1].y);
                    }
                    else
                    {
                        fprintf(stdout, "%f, %f\n", p31.x, p31.y);
                        fprintf(stdout, "%f, %f\n", p32.x, p32.y);
                    }
                }
            }
            //printing tree
            char *linel = NULL;
            char *liner = NULL;

            //reading first line of children
            getline(&linel, &len1, fchild1);
            getline(&liner, &len2, fchild2);

            if (linel[strlen(linel) - 1] == '\n')
            {
                linel[strlen(linel) - 1] = '\0';
            }
            if (liner[strlen(liner) - 1] == '\n')
            {
                liner[strlen(liner) - 1] = '\0';
            }

            long lengthl = strlen(linel);
            long lengthr = strlen(liner);

            //creating first line
            char *linie = malloc(3);
            strcpy(linie, "(");
            for (size_t i = 0; i < numpoints; i++)
            {
                int pointlength = snprintf(NULL, 0, "{%.1f, %.1f}", points[i].x, points[i].y);
                char *pointstring = malloc(pointlength + 1);
                linie = realloc(linie, strlen(linie) + pointlength + 3);
                sprintf(pointstring, "{%.1f, %.1f}", points[i].x, points[i].y);
                strcat(linie, pointstring);
                free(pointstring);
                if (i + 1 != numpoints)
                {
                    strcat(linie, ", ");
                }
            }
            strcat(linie, ")");
            long len = strlen(linie);
            int padding = (lengthl + lengthr + 2) / 2 - (len / 2);

            //printing first line
            for (long i = 0; i < padding; i++)
            {
                putchar(' ');
            }
            printf("%s", linie);
            for (long i = 0; i < ((lengthl + lengthr + 2) - padding - len); i++)
            {
                putchar(' ');
            }
            putchar('\n');
            free(linie);

            //printing second line
            for (long i = 0; i < lengthl + lengthr + 2; i++)
            {
                if (i == lengthl / 2)
                {
                    putchar('/');
                }
                else if (i == (lengthr / 2) + lengthl + 2)
                {
                    putchar('\\');
                }
                else
                {
                    putchar(' ');
                }
            }
            putchar('\n');

            int r = 1;
            int l = 1;

            while (r == 1 || l == 1)
            {

                if (l == 1)
                {
                    if (linel[strlen(linel) - 1] == '\n')
                    {
                        linel[strlen(linel) - 1] = '\0';
                    }
                    printf("%s", linel);
                    putchar(' ');
                    putchar(' ');
                }
                else
                {
                    for (long i = 0; i < lengthl + 2; i++)
                    {
                        putchar(' ');
                    }
                }
                if (r == 1)
                {
                    if (liner[strlen(liner) - 1] == '\n')
                    {
                        liner[strlen(liner) - 1] = '\0';
                    }
                    printf("%s", liner);
                }
                else
                {
                    for (long i = 0; i < lengthr; i++)
                    {
                        putchar(' ');
                    }
                }
                putchar('\n');
                if (getline(&linel, &len1, fchild1) == -1)
                {
                    l = 0;
                }
                if (getline(&liner, &len2, fchild2) == -1)
                {
                    r = 0;
                }
            }
            free(linel);
            free(liner);

            if (fclose(fchild1) < 0 || fclose(fchild2) < 0) // closing write end of pipes
            {
                fprintf(stderr, "serror closing files\n");
                exit(EXIT_FAILURE);
            }
            fflush(stdout);
            break;
        }
        break;
    }

    return EXIT_SUCCESS;
}
