/**
 * @file cpair.c
 * @author Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
 * @date 10.12.2021
 *
 * @brief Return closest pair of points.
 * 
 * @details This programm reads points with x, y coordinates from stdin
 * and determins the point pair with the shortest distance between them.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <float.h>

char *progName;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details After writing the usage information the program will close with the EXIT_FAILURE value.
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s\n", progName);
    exit(EXIT_FAILURE);
}

/**
 * Error helper function
 * @brief This function writes a message to stderr.
 * @details Given a message this function will write the message and the corresponding error
 * message to stderr. Afterwards it will exit the program with the EXIT_FAILURE value.
 * @param message A custom error message
 */
static void error(char *message)
{
    fprintf(stderr, "%s: %s\n", progName, message);
    exit(EXIT_FAILURE);
}

struct Point {
    float x;
    float y;
};

/**
 * Program entry point.
 * @brief The main function returns the pair of points with the smalles distance between them.
 * @details The main function expects only one argument (program name). The function will read all lines
 * in stdin. If there are more then 2 lines (one line = one point) given, they will be grouped according to the average mean of their x coordinates.
 * The programm will then fork into 2 child processes. These processes are 
 * connected to their parent using pipes that replace stdin and stdout. 
 * The parent sends one group to each child and waits for the response.
 * Upon recieving the response through the pipe the parent will compare the 2 pairs and will output the closest pair amongst them.
 * 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
    progName = argv[0];
    int status;

    if (argc > 1)
    {
        usage();
    }

    char *line = NULL;
    size_t len = 0;

    struct Point *pointArray = NULL;
    size_t pointArraySize = 0;
    int arrayCounter = 0;

    pointArray = malloc(0);

    while (getline(&line, &len, stdin) > 0) {
        char *seperator;
        struct Point p;
        p.x = strtof(line, &seperator);
        if (p.x == 0 && line == seperator) {
            free(line);
            error("Input malformed.");
        }
        char *seperator2;
        p.y = strtof(seperator, &seperator2);
        if (p.y == 0 && seperator == seperator2) {
            free(line);
            error("Input malformed.");
        }

        //Dynamic growing array
        pointArraySize += sizeof p;
        struct Point *tmp = realloc(pointArray, pointArraySize);
        if (!tmp) {
            error("realloc - virtual memory exhausted");
        }

        pointArray = tmp;
        pointArray[arrayCounter] = p;
        arrayCounter++;
    }
    
    free(line);

    switch (arrayCounter)
    {
    case 0:
        free(pointArray);
        error("No point given through stdin");
        break;
    case 1:
        free(pointArray);
        break;
    case 2:
        fprintf(stdout, "%f %f\n", pointArray[0].x, pointArray[0].y);
        fprintf(stdout, "%f %f\n", pointArray[1].x, pointArray[1].y);
        free(pointArray);
        break;
    default:
        ;
        float sum = 0;
        for (int i = 0; i < arrayCounter; i++)
        {
            sum += pointArray[i].x;
        }
        float xmean = sum / arrayCounter;

        struct Point pointArray1[arrayCounter];
        int pointArray1Counter = 0;
        struct Point pointArray2[arrayCounter];
        int pointArray2Counter = 0;

        for (int i = 0; i < arrayCounter; i++)
        {
            if (pointArray[i].x < xmean) {
                pointArray1[pointArray1Counter] = pointArray[i];
                pointArray1Counter++;
            } else if (pointArray[i].x == xmean) { //Avoid putting all points in the mean into the same fork
                if (pointArray1Counter < pointArray2Counter) {
                    pointArray1[pointArray1Counter] = pointArray[i];
                    pointArray1Counter++;
                } else {
                    pointArray2[pointArray2Counter] = pointArray[i];
                    pointArray2Counter++;
                }
            } else {
                pointArray2[pointArray2Counter] = pointArray[i];
                pointArray2Counter++;
            }
        }

        free(pointArray);
        
        
        int p1In[2], p1Out[2], p2In[2], p2Out[2];
        if (pipe(p1In) < 0)
        {
            error("Creating pipe1 failed.");
        }
        if (pipe(p1Out) < 0)
        {
            close(p1In[0]);
            close(p1In[1]);
            error("Creating pipe1 failed.");
        }

        pid_t pid1 = fork();

        switch (pid1)
        {
        case -1: //ERROR
            close(p1In[0]);
            close(p1In[1]);
            close(p1Out[0]);
            close(p1Out[1]);
            error("Could not fork.");
        case 0: //CHILD
            close(p1In[1]);
            if (dup2(p1In[0], STDIN_FILENO) < 0) {
                close(p1In[0]);
                close(p1Out[0]);
                close(p1Out[1]);
                error("Could not dup2 the pipe fd.");
            }
            close(p1In[0]);

            close(p1Out[0]);
            if (dup2(p1Out[1], STDOUT_FILENO) < 0) {
                close(p1Out[1]);
                error("Could not dup2 the pipe fd.");
            }
            close(p1Out[1]);

            if (execlp("./cpair", strcat(progName, "/1"), NULL) < 0) {
                error("Failed to execute ./cpair in fork1");
            }
            exit(EXIT_SUCCESS);
        default: //Parent
            close(p1In[0]);
            close(p1Out[1]);
            if (pipe(p2In) < 0)
            {
                close(p1In[1]);
                close(p1Out[0]);
                error("Creating pipe2 failed.");
            }

            if (pipe(p2Out) < 0)
            {
                close(p2In[0]);
                close(p2In[1]);
                close(p1In[1]);
                close(p1Out[0]);
                error("Creating pipe2 failed.");
            }

            pid_t pid2 = fork();

            switch (pid2)
            {
            case -1: //ERROR
                close(p2In[0]);
                close(p2In[1]);
                close(p2Out[0]);
                close(p2Out[1]);
                close(p1In[1]);
                close(p1Out[0]);
                error("Could not fork.");
            case 0: //Child
                close(p1In[1]);
                close(p1Out[0]);

                close(p2In[1]);
                if (dup2(p2In[0], STDIN_FILENO) < 0) {
                    close(p2In[0]);
                    close(p2Out[0]);
                    close(p2Out[1]);
                    error("Could not dup2 the pipe fd.");
                }
                close(p2In[0]);

                close(p2Out[0]);
                if (dup2(p2Out[1], STDOUT_FILENO) < 0) {
                    close(p2Out[1]);
                    error("Could not dup2 the pipe fd.");
                }
                close(p2Out[1]);

                if (execlp("./cpair", strcat(progName, "/2"), NULL) < 0) {
                    error("Failed to execute ./cpair in fork2");
                }
                exit(EXIT_SUCCESS);
            default: //PARENT
                close(p2In[0]);
                close(p2Out[1]);
                break;
            }
            break;
        }

        for (int i = 0; i < pointArray1Counter; i++)
        {
            char buffer[50];
            int count;
            count = sprintf(buffer, "%f %f\n", pointArray1[i].x, pointArray1[i].y);
            write(p1In[1], buffer, count);
        }
        for (int i = 0; i < pointArray2Counter; i++)
        {
            char buffer[50];
            int count;
            count = sprintf(buffer, "%f %f\n", pointArray2[i].x, pointArray2[i].y);
            write(p2In[1], buffer, count);
        }

        //Sending EOF
        close(p1In[1]);
        close(p2In[1]);

        pid_t wpid;
        while ((wpid = wait(&status)) > 0)
        {
            //fprintf(stderr, "%s: Exit status of %d was %d\n", progName, (int) wpid, status);
            if (status != EXIT_SUCCESS) {
                close(p1Out[0]);
                close(p2Out[0]);
                error("There was a error closing a childtask.");
            }
        }

        FILE *out1 = fdopen(p1Out[0], "r");
        if (out1 == NULL)
        {
            close(p1Out[0]);
            close(p2Out[0]);
            error("There was a error opening the stdout of child.");
        }
        FILE *out2 = fdopen(p2Out[0], "r");
        if (out2 == NULL)
        {
            fclose(out1);
            close(p1Out[0]);
            close(p2Out[0]);
            error("There was a error opening the stdout of child.");
        }

        line = NULL;
        len = 0;

        struct Point p1[2];
        struct Point p2[2];
        int p1Single = 0;
        int p2Single = 0;

        if (getline(&line, &len, out1) > 0) {
            char *seperator;
            p1[0].x = strtof(line, &seperator);
            p1[0].y = strtof(seperator, NULL);
            if (getline(&line, &len, out1) > 0) {
                p1[1].x = strtof(line, &seperator);
                p1[1].y = strtof(seperator, NULL);
            }
        } else { //Check if one of the tasks only had a single point (no output)
            p1[0] = pointArray1[0];
            p1Single = 1;
        }

        if (getline(&line, &len, out2) > 0) {
            char *seperator;
            p2[0].x = strtof(line, &seperator);
            p2[0].y = strtof(seperator, NULL);
            if (getline(&line, &len, out2) > 0) {
                p2[1].x = strtof(line, &seperator);
                p2[1].y = strtof(seperator, NULL);
            }
        } else {
            p2[0] = pointArray2[0];
            p2Single = 1;
        }

        free(line);
        fclose(out1);
        fclose(out2);
        close(p1Out[0]);
        close(p2Out[0]);

        float p1d, p2d, p3d;

        if (p1Single) {
            p1d = FLT_MAX;
        } else {
            p1d = fabsf(sqrtf(powf(p1[1].x - p1[0].x, 2) + powf(p1[1].y - p1[0].y, 2)));
        }
        if (p2Single) {
            p2d = FLT_MAX;
        } else {
            p2d = fabsf(sqrtf(powf(p2[1].x - p2[0].x, 2) + powf(p2[1].y - p2[0].y, 2)));
        }

        float p3d00 = fabsf(sqrtf(powf(p2[0].x - p1[0].x, 2) + powf(p2[0].y - p1[0].y, 2)));
        float p3d01 = fabsf(sqrtf(powf(p2[1].x - p1[0].x, 2) + powf(p2[1].y - p1[0].y, 2)));
        float p3d10 = fabsf(sqrtf(powf(p2[0].x - p1[1].x, 2) + powf(p2[0].y - p1[1].y, 2)));
        float p3d11 = fabsf(sqrtf(powf(p2[1].x - p1[1].x, 2) + powf(p2[1].y - p1[1].y, 2)));

        if (p1Single) {
            p3d10 = FLT_MAX;
            p3d11 = FLT_MAX;
        }
        if (p2Single) {
            p3d01 = FLT_MAX;
            p3d11 = FLT_MAX;
        }

        p3d = p3d00;
        int p3index[2] = {0, 0};
        if (p3d01 < p3d) {
            p3d = p3d01;
            p3index[0] = 0;
            p3index[1] = 1;
        }
        if (p3d10 < p3d) {
            p3d = p3d10;
            p3index[0] = 1;
            p3index[1] = 0;
        }
        if (p3d11 < p3d) {
            p3d = p3d11;
            p3index[0] = 1;
            p3index[1] = 1;
        }

        if (p1d <= p2d) {
            if (p1d <= p3d) {
                fprintf(stdout, "%f %f\n", p1[0].x, p1[0].y);
                fprintf(stdout, "%f %f\n", p1[1].x, p1[1].y);
            } else {
                fprintf(stdout, "%f %f\n", p1[p3index[0]].x, p1[p3index[0]].y);
                fprintf(stdout, "%f %f\n", p2[p3index[1]].x, p2[p3index[1]].y);
            }
        } else {
            if (p2d <= p3d) {
                fprintf(stdout, "%f %f\n", p2[0].x, p2[0].y);
                fprintf(stdout, "%f %f\n", p2[1].x, p2[1].y);
            } else {
                fprintf(stdout, "%f %f\n", p1[p3index[0]].x, p1[p3index[0]].y);
                fprintf(stdout, "%f %f\n", p2[p3index[1]].x, p2[p3index[1]].y);
            }
        }

        break;
    }
    exit(EXIT_SUCCESS);
}