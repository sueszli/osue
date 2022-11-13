/**
 * @file cpair.c
 * @author MAYERL Thomas, 52004289 <thomas.mayerl@tuwien.ac.at>
 * @date 09.11.2021
 * 
 * @brief Program cpair: This program looks for the closest pair of points
 * 
 * @details This program reads from its standard input and recursively try to find the closest pair of points by splitting the points
 * into different parts that are checked by child processes.
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define MAX_LINE_LEN 100 /* max line length of input */

typedef struct {
    float x;
    float y;
} point_t; /* point struct to save the coordinates of one point */

char *prog_name; /**< The program name */
point_t *points = NULL; /**< The memory to save the read points */

/**
 * Print error function
 * @brief This function takes an error message as a parameter and writes it to stderr. Additionally, it takes a detail message.
 * @details Before termianting, this function will free the memory of all points (if possible)
 * Global variables: prog_name - The program name; points - The memory where all points are stored.
 * @param error_msg The error message
 * @param detail_msg More details of the error - often obtained with strerror and errno
 * @param file1 If set, this file will also be closed
 * @param file2 If set, this file will also be closed
 **/
static void print_error_and_quit(char *error_msg, char *detail_msg, FILE* file1, FILE* file2) 
{
    fprintf(stderr, "[%s] %s: %s\n", prog_name, error_msg, detail_msg);
    if (file1 != NULL)
    {
        fclose(file1);
    }
    if (file2 != NULL)
    {
        fclose(file2);
    }
    free(points);
    exit(EXIT_FAILURE);
}

/**
 * Usage function
 * @brief This function prints information on how to use the program.
 * @details Global variables: prog_name - The program name
 **/
static void usage(void)
{
    fprintf(stderr,"Usage: %s\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Get distance function
 * @brief Calculates the distance between two points.
 * @details Takes two points as parameters and calculates the distance
 * @param point1 The first point
 * @param point2 The second point
 * @returns The distance
 **/
static float get_distance_between_points(point_t point1, point_t point2)
{
    float x1 = point1.x;
    float x2 = point2.x;
    float y1 = point1.y;
    float y2 = point2.y;
    float difference_squared_x = (x2 - x1) * (x2 - x1);
    float difference_squared_y = (y2 - y1) * (y2 - y1);
    return sqrt(difference_squared_x + difference_squared_y);
}

/**
 * Calculate shortest pair between parts function
 * @brief This function calculates what the shortest pair between the two parts is
 * @details This function checks all pairs between the two parts (that is skipps all others) and calculates the distance.
 * If the distance is the smallest one found so far, the distance and the pair are saved.
 * Global variables: points - The memory where all points are stored.
 * @param avg The average of all points - the border between the two parts
 * @param num_points The number of points 
 * @param result The pointer, where the result pair will be stored to
 * @returns The distance of the smallest pair
 **/
static float calculate_shortest_pair_between_parts(point_t *left_part[], point_t *right_part[], int len_left_part, int len_right_part,point_t *result)
{
    float minDistance = -1;

    for(int i = 0; i < len_left_part; i++) {
        for(int j = 0; j < len_right_part; j++) {
            float distance = get_distance_between_points(*(left_part[i]), *(right_part[j]));
            if (minDistance == -1 || distance < minDistance)
            {
                minDistance = distance;
                result[0] = *(left_part[i]);
                result[1] = *(right_part[j]);
            }
        }
    }
    return minDistance;
}

/**
 * Calculate mean of points function
 * @brief This function calculates the mean of all x coordinates
 * @details The x coordinates of the points are summed up. Afterwards the sum is divided by the number of points.
 * Global variables: points - The memory where all points are stored.
 * @param num_points The number of points
 * @returns The mean
 **/
static float calculate_mean_of_points(int num_points)
{
    float sum = 0;
    for(int i = 0; i < num_points; i++)
    {
        sum += (points+i) -> x;
    }
    return sum / num_points;
}

/**
 * Write points and quit function
 * @brief Write two points and quits
 * @details The points are written to stdout in the form x y where x is the x coordinate and y the y coordinate.
 * Afterwards the points storage is freed and EXIT_SUCCESS is returned on exit.
 * Global variables: points - The memory where all points are stored.
 * @param point1 The first point
 * @param point2 The second point
 **/
static void write_points_and_quit(point_t point1, point_t point2)
{
    printf("%f %f\n", point1.x, point1.y);
    printf("%f %f\n", point2.x, point2.y);
    free(points);
    exit(EXIT_SUCCESS);
}

/**
 * Main function
 * @brief Program entry and function to find closest pair
 * @details Tries to find the closest pair by splitting are in two halves and generating child process that check those.
 * Global variables: prog_name - The program name; points - The memory where all points are stored.
 * @param argc The number of program arguments
 * @param argv The program arguments
 * @returns The exit code
 **/
int main(int argc, char *argv[])
{
    size_t max_line_length = MAX_LINE_LEN;
    prog_name = argv[0];
    if (argc != 1)
    {
        usage();
    }

    int num_points = 0;
    char lineArr[max_line_length];
    char *line = lineArr;

    while(getline(&line, &max_line_length, stdin) != EOF) 
    {
        points = realloc(points, ++num_points * sizeof(points));
        if (points == NULL)
        {
            print_error_and_quit("Could not allocate memory", strerror(errno), NULL, NULL);
        }
        char tmp;
        if(sscanf(line, "%f %f %[\001-\377]", &((points + num_points - 1)->x), &((points + num_points - 1)->y), &tmp) != 2)
        {
            print_error_and_quit("Could not parse points", "Invalid number of point coordinates found or coordinate is not a number.", NULL, NULL);
        }
    }
    if(num_points == 0)
    {
        print_error_and_quit("Could not parse points", "No points have been found.", NULL, NULL);
    }

    if(num_points == 1)
    {
        free(points);
        exit(EXIT_SUCCESS);
    }

    if(num_points == 2)
    {
        write_points_and_quit(*points, *(points+1));
    }

    float avg = calculate_mean_of_points(num_points);

    int pipefd1r[2];
    int pipefd1w[2];
    int pipefd2r[2];
    int pipefd2w[2];

    if(pipe(pipefd1r) != 0 || pipe(pipefd2r) != 0 || pipe(pipefd1w) != 0 || pipe(pipefd2w) != 0)
    {
        print_error_and_quit("Could not create pipes", strerror(errno), NULL, NULL);
    }

    pid_t pid1 = fork();
    switch(pid1)
    {
        case -1:
            print_error_and_quit("Could not fork", strerror(errno), NULL, NULL);
        case 0:
            close(pipefd2r[0]);
            close(pipefd2r[1]);
            close(pipefd2w[0]);
            close(pipefd2w[1]);

            close(pipefd1r[0]);
            close(pipefd1w[1]);
            
            if(dup2(pipefd1r[1], STDOUT_FILENO) == -1)
            {
                print_error_and_quit("Could not redirect child stdout to pipe", strerror(errno), NULL, NULL);
            }
            if(dup2(pipefd1w[0], STDIN_FILENO) == -1)
            {
                print_error_and_quit("Could not redirect child stdin to pipe", strerror(errno), NULL, NULL);
            }

            close(pipefd1r[1]);
            close(pipefd1w[0]);

            execlp(prog_name, prog_name, NULL);
            print_error_and_quit("Could not execute child process", strerror(errno), NULL, NULL);
            // No default here: because default is: just continue!
    }
    pid_t pid2 = fork();
    switch(pid2)
    {
        case -1:
            print_error_and_quit("Could not fork", strerror(errno), NULL, NULL);
        case 0:
            close(pipefd1r[0]);
            close(pipefd1r[1]);
            close(pipefd1w[0]);
            close(pipefd1w[1]);

            close(pipefd2r[0]);
            close(pipefd2w[1]);
            
            if(dup2(pipefd2r[1], STDOUT_FILENO) == -1)
            {
                print_error_and_quit("Could not redirect child stdout to pipe", strerror(errno), NULL, NULL);
            }
            if(dup2(pipefd2w[0], STDIN_FILENO) == -1)
            {
                print_error_and_quit("Could not redirect child stdin to pipe", strerror(errno), NULL, NULL);
            }

            close(pipefd2r[1]);
            close(pipefd2w[0]);

            execlp(prog_name, prog_name, NULL);
            print_error_and_quit("Could not execute child process", strerror(errno), NULL, NULL);
            // No default here: because default is: just continue!
    }

    close(pipefd1r[1]);
    close(pipefd2r[1]);
    close(pipefd1w[0]);
    close(pipefd2w[0]);

    int len_left_part = 0;
    int len_right_part = 0;
    point_t *left_part[num_points];
    point_t *right_part[num_points];
    
    FILE *inputFilePipe1 = fdopen(pipefd1w[1], "a");
    FILE *inputFilePipe2 = fdopen(pipefd2w[1], "a");

    if (inputFilePipe1 == NULL || inputFilePipe2 == NULL)
    {
        print_error_and_quit("Error whilst opening pipe for writing to child processes", strerror(errno), inputFilePipe1, inputFilePipe2);
    }

    int nextWhenAverage = 1;
    for(int i = 0; i < num_points; i++)
    {
        float x = (points+i) -> x;
        float y = (points+i) -> y;
        if (x < avg || (x == avg && nextWhenAverage))
        {
            if (x == avg)
            {
                nextWhenAverage = 0;
            }
            left_part[len_left_part] = points+i;
            len_left_part++;
            if(fprintf(inputFilePipe1, "%f %f\n", x, y) == -1)
            {
                print_error_and_quit("Could not write to pipe", "fprintf failed.", inputFilePipe1, inputFilePipe2);
            }
        }
        else
        {
            if (x == avg)
            {
                nextWhenAverage = 1;
            }
            right_part[len_right_part] = points+i;
            len_right_part++;
            if(fprintf(inputFilePipe2, "%f %f\n", x, y) == -1)
            {
                print_error_and_quit("Could not write to pipe", "fprintf failed.", inputFilePipe1, inputFilePipe2);
            }
        }
        if (fflush(inputFilePipe1) == -1 || fflush(inputFilePipe2) == -1)
        {
            print_error_and_quit("Could not flush pipes", strerror(errno), inputFilePipe1, inputFilePipe2);
        }
    }
    if (fclose(inputFilePipe1) == EOF)
    {
        print_error_and_quit("Could not close pipe end towards children, so they stop reading at EOF", strerror(errno), NULL, inputFilePipe2);
    }
    if (fclose(inputFilePipe2) == EOF)
    {
        print_error_and_quit("Could not close pipe end towards children, so they stop reading at EOF", strerror(errno), NULL, inputFilePipe1);
    }


    point_t points_res[4];
    int i = 0;
    FILE *resultFilePipe1 = fdopen(pipefd1r[0], "r");
    FILE *resultFilePipe2 = fdopen(pipefd2r[0], "r");
    if(resultFilePipe1 == NULL || resultFilePipe2 == NULL)
    {
        print_error_and_quit("Error whilst opening pipe for reading", strerror(errno), resultFilePipe1, resultFilePipe2);
    }
    max_line_length = MAX_LINE_LEN; // Reassign because getline might have changed max length
    while((getline(&line, &max_line_length, resultFilePipe1)) != EOF)
    {
        if (i > 1)
        {
            assert(0); // one child process should only output one pair (= two points)
        }
        sscanf(line, "%f %f", &(points_res[i].x), &(points_res[i].y));
        i++;
    }
    if (ferror(resultFilePipe1))
    {
        print_error_and_quit("Could not read from pipe", strerror(errno), resultFilePipe1, resultFilePipe2);
    }
    max_line_length = MAX_LINE_LEN;
    while((getline(&line, &max_line_length, resultFilePipe2)) != EOF)
    {
        if (i > 3)
        {
            assert(0); // one child process should only output one pair (= two points)
        }
        sscanf(line, "%f %f", &(points_res[i].x), &(points_res[i].y));
        i++;
    }
    if (ferror(resultFilePipe2))
    {
        print_error_and_quit("Could not read from pipe", strerror(errno), resultFilePipe1, resultFilePipe2);
    }

    fclose(resultFilePipe1);
    fclose(resultFilePipe2);
    int status1;
    if(waitpid(pid1, &status1, 0) != pid1)
    {
        print_error_and_quit("Error whilst waiting for child process", strerror(errno), NULL, NULL);
    }
    if(status1 != EXIT_SUCCESS)
    {
        free(points); // No error message needed - already printed by terminating child
        exit(EXIT_FAILURE);
    }

    int status2;
    if(waitpid(pid2, &status2, 0) != pid2)
    {
        print_error_and_quit("Error whilst waiting for child process", strerror(errno), NULL, NULL);
    }
    if(status2 != EXIT_SUCCESS)
    {
        free(points); // No error message needed - already printed by terminating child
        exit(EXIT_FAILURE);
    }

    point_t between[2];
    float distance_between = calculate_shortest_pair_between_parts(left_part, right_part, len_left_part, len_right_part, between);
    float distance_within_1 = get_distance_between_points(points_res[0], points_res[1]);
    if (i == 2)
    {
        // Only one client returned a result (can happen when there is a odd number of points)
        if (distance_within_1 <= distance_between)
        {
            write_points_and_quit(points_res[0], points_res[1]);
        }
        else
        {
            write_points_and_quit(between[0], between[1]);
        }
        free(points);
        exit(EXIT_SUCCESS);
    }

    if (i == 4)
    {
        float distance_within_2 = get_distance_between_points(points_res[2], points_res[3]);
        if (distance_within_1 <= distance_between)
        {
            if (distance_within_1 <= distance_within_2)
            {
                write_points_and_quit(points_res[0], points_res[1]);
            }
            else
            {
                write_points_and_quit(points_res[2], points_res[3]);
            }
        }
        else
        {
            if (distance_within_2 <= distance_between)
            {
                write_points_and_quit(points_res[2], points_res[3]);
            }
            else
            {
                write_points_and_quit(between[0], between[1]);
            }
        }
    }
    free(points);
    assert(0); // Another possibility should not exist at this point
}
