/**
 * @file cpair.c
 * @author Manuel Hochreiter e0671428@student.tuwien.ac.at
 * @brief Main program module.
 * 
 * This program reads the coordinates and creates child processes for evaluating a solution.
 * 
 * @version 0.1
 * @date 2021-08-12
 * 
 * @copyright Copyright (c) 2021
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <sys/wait.h>

static char *program_name = "cpair";
size_t lineLength = 0;
char *buffer = NULL;
int ret;

/**
 * @brief This function provides information about calling it correctly to stderr
 * @details global variables: program_name
 */
static void usage()
{
    fprintf(stderr, "Usage: ./%s \n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Swaps two items - helper function for selection_sort
 * 
 * @param item_1 first item to swap
 * @param item_2 first item to swap
 */
static void swap_items(float *item_1, float *item_2)
{
    float temp = *item_1;
    *item_1 = *item_2;
    *item_2 = temp;
}

/**
 * @brief Sorts an array in ascending order
 * 
 * @param array array to sort
 * @param size size of the array
 */
static void selection_sort(float array[], int size)
{
    int min_index = 0;
    for (int m = 0; m < size; m++)
    {
        min_index = m;
        for (int n = m + 1; n < size; n++)
        {
            if (array[n] < array[min_index])
            {
                min_index = n;
            }
        }
        swap_items(&array[min_index], &array[m]);
    }
}

struct point
{
    float x_coordinate;
    float y_coordinate;
};

/**
 * @brief The main function is the program entry point.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        usage();
    }

    struct point *points = malloc(sizeof(struct point) * 10);
    int current_pointer_size = 10;
    int point_count = 0;

    /////////////////////////////////// read points from stdin ///////////////////////////////////

    while ((ret = getline(&buffer, &lineLength, stdin)) != -1)
    {
        errno = 0;
        char *endpoint;
        float coord_x = strtof(buffer, &endpoint);
        float coord_y = strtof(endpoint + 1, NULL);

        if ((coord_x == 0) || (coord_y == 0))
        {
            if (errno != 0)
            {
                errno = 22;
                fprintf(stderr, "strtof failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        if ((coord_x == HUGE_VAL) || (coord_y == HUGE_VAL))
        {
            fprintf(stderr, "strtof error: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if ((point_count + 1) == current_pointer_size)
        {
            points = (struct point *)realloc(points, 2 * sizeof(struct point) * current_pointer_size);
            current_pointer_size = 2 * current_pointer_size;
        }

        points[point_count].x_coordinate = coord_x;
        points[point_count].y_coordinate = coord_y;

        point_count++;
    }

    //////////////////////////////////////// recursive decision ///////////////////////
    if (point_count < 1)
    {
        free(buffer);
        exit(EXIT_FAILURE);
    }

    if (point_count == 1)
    {
        free(buffer);
        exit(EXIT_SUCCESS);
    }

    if (point_count == 2)
    {
        printf("%f ", points->x_coordinate);
        printf("%f\n", points->y_coordinate);

        points++;

        printf("%f ", points->x_coordinate);
        printf("%f\n", points->y_coordinate);

        points = points - 1;
        free(buffer);
        exit(EXIT_SUCCESS);
    }

    if (point_count > 2)
    {
        //////////////////////////////////// splitting array in two halfes ///////////////////////////
        double x_median = 0;
        float x_coordinates[point_count];

        for (int k = 0; k < point_count; k++)
        {
            x_coordinates[k] = points[k].x_coordinate;
        }

        selection_sort(x_coordinates, point_count);

        int index_1 = (point_count / 2) - 1;
        int index_2 = (point_count / 2);
        x_median = (x_coordinates[index_1] + x_coordinates[index_2]) / 2;

        int size_lower_array = 0;
        int size_higher_array = 0;

        struct point *lower_points = malloc(point_count * sizeof(struct point));
        struct point *higher_points = malloc(point_count * sizeof(struct point));

        for (int j = 0; j < point_count; j++)
        {
            points = points + j;
            if (points->x_coordinate <= x_median)
            {
                lower_points->x_coordinate = points->x_coordinate;
                lower_points->y_coordinate = points->y_coordinate;
                lower_points++;
                size_lower_array++;
            }

            if (points->x_coordinate > x_median)
            {
                higher_points->x_coordinate = points->x_coordinate;
                higher_points->y_coordinate = points->y_coordinate;
                higher_points++;
                size_higher_array++;
            }

            points = points - j;
        }

        lower_points = lower_points - size_lower_array;
        higher_points = higher_points - size_higher_array;

        free(points);

        /////////////////////// preparing for fork() and exev() /////////////////////////////
        int cid_1 = -2;
        int cid_2 = -2;
        char *cmd[] = {"cpair", (char *)0};

        int pipe_1_task[2];
        int pipe_1_solution[2];
        int pipe_2_task[2];
        int pipe_2_solution[2];

        if (pipe(pipe_1_task) == -1)
        {
            fprintf(stderr, "pipe failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (pipe(pipe_1_solution) == -1)
        {
            fprintf(stderr, "pipe failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (size_lower_array > 1)
        {
            cid_1 = fork();
        }

        switch (cid_1)
        {
        case -1:
            fprintf(stderr, "fork failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case 0:
            dup2(pipe_1_task[0], STDIN_FILENO);
            dup2(pipe_1_solution[1], STDOUT_FILENO);
            close(pipe_1_solution[0]);
            close(pipe_1_task[1]);
            close(pipe_1_solution[1]);
            close(pipe_1_task[0]);
            execv("./cpair", cmd);
            fprintf(stderr, "execv failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case -2:
            break;
        default:
            break;
        }

        if (pipe(pipe_2_task) == -1)
        {
            fprintf(stderr, "pipe failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (pipe(pipe_2_solution) == -1)
        {
            fprintf(stderr, "pipe failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (size_higher_array > 1)
        {
            cid_2 = fork();
        }

        switch (cid_2)
        {
        case -1:
            fprintf(stderr, "fork failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case 0:
            dup2(pipe_2_task[0], STDIN_FILENO);
            dup2(pipe_2_solution[1], STDOUT_FILENO);
            close(pipe_2_solution[0]);
            close(pipe_2_task[1]);
            close(pipe_2_solution[1]);
            close(pipe_2_task[0]);
            close(pipe_1_solution[0]);
            close(pipe_1_task[1]);
            close(pipe_1_solution[1]);
            close(pipe_1_task[0]);
            execv("./cpair", cmd);
            fprintf(stderr, "execv failed: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case -2:
            break;
        default:
            break;
        }

        close(pipe_1_solution[1]);
        close(pipe_1_task[0]);
        close(pipe_2_solution[1]);
        close(pipe_2_task[0]);

        /////////////////////////////////// sending tasks to children ////////////////////////////
        FILE *task_lower = fdopen(pipe_1_task[1], "w");
        FILE *task_higher = fdopen(pipe_2_task[1], "w");

        if (task_lower == NULL)
        {
            fprintf(stderr, "fdopen failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (task_higher == NULL)
        {
            fprintf(stderr, "fdopen failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (size_lower_array >= 1)
        {
            for (int m = 0; m < size_lower_array; m++)
            {
                lower_points = lower_points + m;
                fprintf(task_lower, "%f", lower_points->x_coordinate);
                fprintf(task_lower, "%s", " ");
                fprintf(task_lower, "%f", lower_points->y_coordinate);
                fprintf(task_lower, "%s", "\n");
                lower_points = lower_points - m;
            }
        }

        if (size_higher_array >= 1)
        {
            for (int m = 0; m < size_higher_array; m++)
            {
                higher_points = higher_points + m;
                fprintf(task_higher, "%f", higher_points->x_coordinate);
                fputs(" ", task_higher);
                fprintf(task_higher, "%f", higher_points->y_coordinate);
                fputs("\n", task_higher);
                higher_points = higher_points - m;
            }

            fflush(task_higher);
        }

        if (fclose(task_lower) != 0)
        {
            fprintf(stderr, "fclose failed: %s \n", strerror(errno));
        }

        if (fclose(task_higher) != 0)
        {
            fprintf(stderr, "fclose failed: %s \n", strerror(errno));
        }

        free(lower_points);
        free(higher_points);

        //////////////////////////// reading solutions from children ///////////////////////
        FILE *solution_stream;
        struct point *P1 = malloc(sizeof(struct point) * 2);

        solution_stream = fdopen(pipe_1_solution[0], "r");
        if (solution_stream == NULL)
        {
            fprintf(stderr, "fdopen failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        struct point *pos = P1;
        while ((ret = getline(&buffer, &lineLength, solution_stream)) != -1)
        {
            errno = 0;
            char *endpoint;
            float coord_x = strtof(buffer, &endpoint);
            float coord_y = strtof(endpoint + 1, NULL);

            if ((coord_x == 0) || (coord_y == 0))
            {
                if (errno != 0)
                {
                    errno = 22;
                    fprintf(stderr, "strtof failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            if ((coord_x == HUGE_VAL) || (coord_y == HUGE_VAL))
            {
                fprintf(stderr, "strtof error: %s \n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            pos->x_coordinate = coord_x;
            pos->y_coordinate = coord_y;
            pos++;
        }

        fflush(solution_stream);
        if (fclose(solution_stream) != 0)
        {
            fprintf(stderr, "fclose failed: %s \n", strerror(errno));
        }

        struct point *P2 = NULL;
        P2 = calloc(2, sizeof(struct point));
        solution_stream = fdopen(pipe_2_solution[0], "r");
        if (solution_stream == NULL)
        {
            fprintf(stderr, "fdopen failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        pos = P2;
        while ((ret = getline(&buffer, &lineLength, solution_stream)) != -1)
        {
            errno = 0;
            char *endpoint;
            float coord_x = strtof(buffer, &endpoint);
            float coord_y = strtof(endpoint + 1, NULL);

            if ((coord_x == 0) || (coord_y == 0))
            {
                if (errno != 0)
                {
                    errno = 22;
                    fprintf(stderr, "strtof failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            if ((coord_x == HUGE_VAL) || (coord_y == HUGE_VAL))
            {
                fprintf(stderr, "strtof error: %s \n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            pos->x_coordinate = coord_x;
            pos->y_coordinate = coord_y;

            pos++;
        }

        fflush(solution_stream);
        if (fclose(solution_stream) != 0)
        {
            fprintf(stderr, "fclose failed: %s \n", strerror(errno));
        }

        ///////////////////////////////////// waiting for children //////////////////////////
        int status;

        while (waitpid(cid_1, &status, 0) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            fprintf(stderr, "Cannot wait!\n");
            exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            fprintf(stderr, "Child exited on error: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        while (waitpid(cid_2, &status, 0) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            fprintf(stderr, "Cannot wait!\n");
            exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            fprintf(stderr, "Child exited on error: %s \n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /////////////////////////////////// selecting points for minimal distance ///////////////////////////////
        double distance_1 = 0;
        double distance_2 = 0;
        int lowest_distance_number;
        double lowest_distance;

        distance_1 = sqrt(pow(((P1 + 1)->y_coordinate - (P1)->y_coordinate), 2) + pow(((P1 + 1)->x_coordinate - (P1)->x_coordinate), 2));
        distance_2 = sqrt(pow(((P2 + 1)->y_coordinate - (P2)->y_coordinate), 2) + pow(((P2 + 1)->x_coordinate - (P2)->x_coordinate), 2));
        if (distance_2 > distance_1)
        {
            lowest_distance_number = 1;
            lowest_distance = distance_1;
        }
        else
        {
            lowest_distance_number = 2;
            lowest_distance = distance_2;
        }

        distance_1 = sqrt(pow(((P2)->y_coordinate - (P1)->y_coordinate), 2) + pow(((P2)->x_coordinate - (P1)->x_coordinate), 2));
        if (distance_1 < lowest_distance)
        {
            lowest_distance = distance_1;
            lowest_distance_number = 32010;
        }

        distance_1 = sqrt(pow(((P2)->y_coordinate - (P1 + 1)->y_coordinate), 2) + pow(((P2)->x_coordinate - (P1 + 1)->x_coordinate), 2));
        if (distance_1 < lowest_distance)
        {
            lowest_distance = distance_1;
            lowest_distance_number = 32011;
        }

        distance_1 = sqrt(pow(((P2 + 1)->y_coordinate - (P1)->y_coordinate), 2) + pow(((P2 + 1)->x_coordinate - (P1)->x_coordinate), 2));
        if (distance_1 < lowest_distance)
        {
            lowest_distance = distance_1;
            lowest_distance_number = 32110;
        }

        distance_1 = sqrt(pow(((P2 + 1)->y_coordinate - (P1 + 1)->y_coordinate), 2) + pow(((P2 + 1)->x_coordinate - (P1 + 1)->x_coordinate), 2));
        if (distance_1 < lowest_distance)
        {
            lowest_distance = distance_1;
            lowest_distance_number = 32111;
        }

        switch (lowest_distance_number)
        {
        case 1:
            printf("%f ", (P1)->x_coordinate);
            printf("%f\n", (P1)->y_coordinate);
            printf("%f ", (P1 + 1)->x_coordinate);
            printf("%f", (P1 + 1)->y_coordinate);
            break;
        case 2:
            printf("%f ", (P2)->x_coordinate);
            printf("%f\n", (P2)->y_coordinate);
            printf("%f ", (P2 + 1)->x_coordinate);
            printf("%f", (P2 + 1)->y_coordinate);
            break;
        case 32010:
            printf("%f ", (P1)->x_coordinate);
            printf("%f\n", (P1)->y_coordinate);
            printf("%f ", (P2)->x_coordinate);
            printf("%f", (P2)->y_coordinate);
            break;
        case 32011:
            printf("%f ", (P2)->x_coordinate);
            printf("%f\n", (P2)->y_coordinate);
            printf("%f ", (P1 + 1)->x_coordinate);
            printf("%f", (P1 + 1)->y_coordinate);
            break;
        case 32110:
            printf("%f ", (P2 + 1)->x_coordinate);
            printf("%f\n", (P2 + 1)->y_coordinate);
            printf("%f ", (P1)->x_coordinate);
            printf("%f", (P1)->y_coordinate);
            break;
        case 32111:
            printf("%f ", (P2 + 1)->x_coordinate);
            printf("%f\n", (P2 + 1)->y_coordinate);
            printf("%f ", (P1 + 1)->x_coordinate);
            printf("%f", (P1 + 1)->y_coordinate);
            break;
        default:
            break;
        }

        printf("\n");

        free(buffer);
        free(P1);
        free(P2);
    }

    exit(EXIT_SUCCESS);
}