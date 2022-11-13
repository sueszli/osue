/**
 * @file main.c
 * @author Jakob Guttmann 11810289 <e11810289@student.twuien.ac.at>
 * @brief module with main Method for program ./cpair, forks children processes to compute closest pair of points
 * @date 04.12.2021
 * 
 * 
 * 
 */
#include "points.h"

/**
 * Program entry point
 * 
 * @brief at the beginning the program reads points from stdin in the format "x y",
 *  where x and y are float numbers, and between is just one whitespace allowed
 *  then computes the arithmetic mean of the x-values and the program is rearranging the array
 *  that on the lower half are the points with x-values smaller and other part x-values higher
 *  if x-value equals the arithmetic mean, it is either on the left or right side of array (is changing)
 *  forking two children processes to compute closest pair for each half of the array using again cpair
 *  parent process writes the points via unnamed pipes to the children processes
 *  meanwhile the parent process computes the closest pair between the two halfes
 *  each process (parent and two children) have computed their closest pair, write closest pair to stdout
 *  so that either parent process can read this result or this result is back again at the user
 * 
 * @param argc argument counter 
 * @param argv argument which are passed to the program: program accepts only zero arguments
 * @return int returns EXIT_SUCCESS if everthing went fine and EXIT_FAILURE if an error occered anywhere
 */
int main(int argc, char *argv[])
{
    int status1 = 0;
    int status2 = 0;

    if (argc != 1)
    {
        fprintf(stderr, "[%s] ERROR: wrong usage of cpair. Usage: ./cpair\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sizeArray = INIT_SIZE_ARRAY;

    Point *points = malloc(sizeArray * sizeof(Point));
    if (points == NULL)
        ERRORHANDLING("allocate points array")

    int pointCounter = 0;
    int numRead;
    size_t len = 0;
    char *line = NULL;

    FILE *input = stdin;

    while ((numRead = getline(&line, &len, input)) != -1)
    {
        status1 = parsePoint(line, &points[pointCounter]);

        if (status1 == -1)
        {
            free(points);
            free(line);
            ERRORHANDLING("parse point: wrong format")
        }

        pointCounter++;
        if (pointCounter >= sizeArray)
        {
            sizeArray *= 2;

            points = realloc(points, sizeArray * sizeof(Point));
            if (points == NULL)
            {
                status1 = 1;
                break;
            }
        }
    }
    free(line);

    points = realloc(points, pointCounter * sizeof(Point));

    if (status1 || (points == NULL && pointCounter > 0))
    {
        free(points);
        ERRORHANDLING("realloc array")
    }

    if (pointCounter <= 1)
    {
    }
    else if (pointCounter == 2)
    {
        if(printPoints(points, pointCounter, stdout) == -1)
        {
            free(points);
            ERRORHANDLING("print to stdout")
        }
    }
    else if (pointCounter > 2)
    {
        //index in points
        // 0 <= i < middle: smaller than mean
        // middle <= i < pointCounter: greater than mean
        // exactly mean: on both sides, are swapped
        int middle = rearrangeArray(points, pointCounter);
        pid_t child_low, child_high;

        FILE *writeLow, *writeHigh;

        // aus Parent Sicht
        int pipe_low_write[2];
        int pipe_low_read[2];

        status1 = pipe(pipe_low_write);
        status2 = pipe(pipe_low_read);

        if (status1 == -1 || status2 == -1)
        {
            free(points);
            ERRORHANDLING("cannot pipe")
        }

        child_low = fork();
        switch (child_low)
        {
        case -1:
            free(points);
            close(pipe_low_write[0]);
            close(pipe_low_write[1]);
            close(pipe_low_read[0]);
            close(pipe_low_read[1]);
            ERRORHANDLING("fork child low")
            break;
        case 0:
            //write end can be closed, because parent is writing
            close(pipe_low_write[1]);
            //read end can be closed, because parent is writing
            close(pipe_low_read[0]);

            if (dup2(pipe_low_write[0], STDIN_FILENO) == -1 || dup2(pipe_low_read[1], STDOUT_FILENO) == -1)
            {
                free(points);
                close(pipe_low_write[0]);
                close(pipe_low_read[1]);
                ERRORHANDLING("redirect to low stdin")
            }

            close(pipe_low_write[0]);
            close(pipe_low_read[1]);

            execlp(argv[0], argv[0], NULL);

            free(points);
            ERRORHANDLING("children cannot execute recursively")

            break;
        default:
            //read end can be closed, because parent writes
            close(pipe_low_write[0]);
            //write end can be closed, parent reads
            close(pipe_low_read[1]);

            writeLow = fdopen(pipe_low_write[1], "w");

            if (writeLow == NULL)
            {
                status2 = -1;
            }
            else
            {
                status1 = printPoints(points, middle, writeLow);
                fflush(writeLow);
                fclose(writeLow);
            }
            close(pipe_low_write[1]);
            break;
        }
        if (status1 == -1 || status2 == -1)
        {
            close(pipe_low_read[0]);
            free(points);
            ERRORHANDLING("write to children process")
        }

        int pipe_high_write[2];
        int pipe_high_read[2];

        status1 = pipe(pipe_high_write);
        status2 = pipe(pipe_high_read);

        if (status1 == -1 || status2 == -1)
        {
            free(points);
            ERRORHANDLING("cannot pipe")
        }

        child_high = fork();
        switch (child_high)
        {
        case -1:
            free(points);
            close(pipe_high_write[0]);
            close(pipe_high_write[1]);
            close(pipe_high_read[0]);
            close(pipe_high_read[1]);
            close(pipe_low_read[0]);
            ERRORHANDLING("fork child high")
            break;
        case 0:
            // Read End für Elternprozess kann geschlossen werden
            close(pipe_low_read[0]);

            // diese braucht Kind nicht
            close(pipe_high_write[1]);
            close(pipe_high_read[0]);

            if (dup2(pipe_high_write[0], STDIN_FILENO) == -1 || dup2(pipe_high_read[1], STDOUT_FILENO) == -1)
            {
                free(points);
                close(pipe_high_write[0]);
                close(pipe_high_read[1]);
                ERRORHANDLING("redirect to stdin/stdout")
            }

            close(pipe_high_write[0]);
            close(pipe_high_read[1]);

            execlp("./cpair", "./cpair", NULL);

            free(points);
            ERRORHANDLING("children cannot execute recursively")

            break;
        default:
            close(pipe_high_read[1]);
            close(pipe_high_write[0]);

            writeHigh = fdopen(pipe_high_write[1], "w");

            if (writeHigh == NULL)
            {
                status2 = -1;
            }
            else
            {
                status1 = printPoints(points + middle, pointCounter - middle, writeHigh);
                fflush(writeHigh);
                fclose(writeHigh);
            }
            close(pipe_high_write[1]);
            break;
        }

        if (status2 == -1 || status1 == -1)
        {
            close(pipe_low_read[0]);
            close(pipe_high_read[0]);
            free(points);
            ERRORHANDLING("write to children process")
        }

        //calculate minimum distance between one group and the other group

        Point p_min[2];

        float minimum_p = 1.0 / 0.0;
        int p_min1;
        int p_min2;
        Point p3[2];
        float distance;
        for (int i = 0; i < middle; i++)
        {
            p3[0] = points[i];
            for (int j = middle; j < pointCounter; j++)
            {
                p3[1] = points[j];
                distance = distancePoints(p3[0], p3[1]);
                if (distance < minimum_p)
                {
                    p_min1 = i;
                    p_min2 = j;
                    minimum_p = distance;
                }
            }
        }

        p3[0] = points[p_min1];
        p3[1] = points[p_min2];
        p_min[0] = p3[0];
        p_min[1] = p3[1];

        status2 = waitpid(child_low, &status1, 0);

        FILE *fromLowChild = fdopen(pipe_low_read[0], "r");

        if (status2 == -1 || !WIFEXITED(status1) || fromLowChild == NULL)
        {
            close(pipe_low_read[0]);
            close(pipe_high_read[0]);
            free(points);
            ERRORHANDLING("wait for low children or open pipe")
        }

        len = 0;
        line = NULL;
        pointCounter = 0;
        Point p1[2];
        status1 = 0;

        while ((numRead = getline(&line, &len, fromLowChild)) != -1)
        {
            if (pointCounter == 2 || status1 == -1)
            {
                status1 = -1;
                break;
            }
            status1 = parsePoint(line, &p1[pointCounter]);
            pointCounter++;
        }
        free(line);
        fclose(fromLowChild);
        close(pipe_low_read[0]);

        if (status1 == -1)
        {
            free(points);
            close(pipe_high_read[0]);
            ERRORHANDLING("read output from child low")
        }

        if (pointCounter == 2)
        {
            distance = distancePoints(p1[0], p1[1]);

            if (distance < minimum_p)
            {
                p_min[0] = p1[0];
                p_min[1] = p1[1];
                minimum_p = distance;
            }
        }

        status2 = waitpid(child_high, &status1, 0);

        FILE *fromHighChild = fdopen(pipe_high_read[0], "r");

        if (status2 == -1 || !WIFEXITED(status1) || fromHighChild == NULL)
        {
            close(pipe_low_read[0]);
            close(pipe_high_read[0]);
            free(points);
            ERRORHANDLING("wait for high children or open pipe")
        }

        len = 0;
        line = NULL;
        pointCounter = 0;
        Point p2[2];
        status1 = 0;

        while ((numRead = getline(&line, &len, fromHighChild)) != -1)
        {
            if (pointCounter == 2 || status1 == -1)
            {
                status1 = -1;
                break;
            }
            status1 = parsePoint(line, &p2[pointCounter]);
            pointCounter++;
        }
        free(line);
        close(pipe_high_read[0]);
        fclose(fromHighChild);

        if (status1 == -1)
        {
            free(points);
            ERRORHANDLING("read output from child high")
        }

        if (pointCounter == 2)
        {
            distance = distancePoints(p2[0], p2[1]);

            if (distance < minimum_p)
            {
                p_min[0] = p2[0];
                p_min[1] = p2[1];
                minimum_p = distance;
            }
        }

        if(printPoints(p_min, 2, stdout) == -1)
        {
            free(points);
            ERRORHANDLING("write minimum points to stdout")
        }
    }
    free(points);
    exit(EXIT_SUCCESS);
}