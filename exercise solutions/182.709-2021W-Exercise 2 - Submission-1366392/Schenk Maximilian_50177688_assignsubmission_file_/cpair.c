/**
 * @file cpair.c
 * @details Implements the whole "cpair" excercise.
 * @author Maximilian Schenk 11908528 <e11908528@student.tuwien.ac.at>
 * @date 04.12.2021
 **/

#include "cpair.h"

char const *PROGRAM_NAME;

/**
 * @brief prints the usage message of the program.
 **/
static void usage(void)
{
    fprintf(stderr, "[%s] ERROR: %s does not allow any arguments.\n", PROGRAM_NAME, PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief writes an error message and exits the program.
 * @details prints the parsed message to strerr and exits the program.
 * @param message describing the error.
 **/
static void error_exit(char *message)
{
    fprintf(stderr, "[%s]: %s (%s)\n", PROGRAM_NAME, PROGRAM_NAME, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief frees the allocated resources
 * @details frees all passed parameters.
 * @param points pointer of resource to be freed.
 * @param c1_points pointer of resource to be freed.
 * @param c2_points pointer of resource to be freed.
 **/
static void clean_up(Point *points, Point *c1_points, Point *c2_points)
{
    if (points != NULL)
    {
        free(points);
    }
    if (c1_points != NULL)
    {
        free(c1_points);
    }
    if (c2_points != NULL)
    {
        free(c2_points);
    }
}

/**
 * @brief checks if the given string is a valid point.
 * @details checks  using regex (line of 2 floats, seperated by whitespace).
 * @param line string to be checked.
 * @return true, if the point is valid, otherwise false.
 **/
static bool is_valid_point(char const *line)
{
    regex_t regex;
    int return_val;
    if (regcomp(&regex, "^[-+]?[0-9]*.?[0-9]+ [-+]?[0-9]*.?[0-9]+(\n)?$", REG_EXTENDED) != 0)
    {
        return false;
    }
    return_val = regexec(&regex, line, (size_t)0, NULL, 0);
    regfree(&regex);
    return return_val == 0;
}

/**
 * @brief reads the given input.
 * @details reads all points and stores them into their pointers. Updates array size
 * and calculates the mean.
 * @param points pointer to a pointer of points where the points should be stored.
 * (**) needed for realloc.
 * @param point_counter array holding 3 entries: 
 * [0] -> amount of points of the parent.
 * [1] -> amount of points of lower child.
 * [2] -> amount of points of upper child.
 * @param point_sizes array holding 3 entries: 
 * [0] -> size of points able to hold in pointer of the parent.
 * [1] -> size of points able to hold in pointer of first child.
 * [2] -> size of points able to hold in pointer of second child.
 * @param mean pointer to a float, where the mean should be stored.
 * @return 0 on success, othrwise -1.
 **/
static int handle_input(Point **points, int point_counter[3], int point_sizes[3], float *mean)
{
    char *line = NULL;
    size_t n = 0;

    while (getline(&line, &n, stdin) != -1)
    {
        if (!is_valid_point(line))
        {
            
            free(line);
            return -1;
        }

        // parse into point
        char *tmp = line;

        Point p;
        tmp = strtok(tmp, " ");
        p.x = strtof(tmp, NULL);
        tmp = strtok(NULL, " ");
        p.y = strtof(tmp, NULL);

        point_counter[0]++;

        // if allocated memory is to small
        if (point_counter[0] >= point_sizes[0])
        {
            point_sizes[0] = point_sizes[0] * 2;
            if (((*points) = realloc((*points), sizeof(Point) * point_sizes[0])) == NULL)
            {
                free(line);
                return -1;
            }
        }

        (*points)[point_counter[0] - 1] = p; // save new point
        (*mean) += p.x;
    }
    
    free(line);
    (*mean) /= point_counter[0];
    return 0;
}

/**
 * @brief closes unused pipes for parent.
 * @details following pipe-ends are closed:
 * @param pipe1 holding the file-descritors of the pipe.
 * @param pipe2 holding the file-descritors of the pipe.
 * @param pipe3 holding the file-descritors of the pipe.
 * @param pipe4 holding the file-descritors of the pipe.
 **/
static void close_pipes(int pipe1[2], int pipe2[2], int pipe3[2], int pipe4[2])
{
    close(pipe1[0]); // read

    close(pipe2[1]); // write

    close(pipe3[0]); // read

    close(pipe4[1]); // write
}

/**
 * @brief used to write points to children using pipes.
 * @details parent writes to the according pipe ends.
 * @param points pointer to the total points.
 * @param c1_points pointer to a pointer of points of the lower child.
 * (**) needed for realloc.
 * @param c2_points pointer to a pointer of points of the upper child.
 * (**) needed for realloc.
 * @param point_counter array holding 3 entries: 
 * [0] -> amount of points of the parent.
 * [1] -> amount of points of first child.
 * [2] -> amount of points of second child.
 * @param point_sizes array holding 3 entries: 
 * [0] -> size of points able to hold in pointer of the parent.
 * [1] -> size of points able to hold in pointer of first child.
 * [2] -> size of points able to hold in pointer of second child.
 * @param mean the mean of x-values.
 * @param lower_read_pipe read-pipe of the lower child.
 * @param upper_read_pipe read-pipe of the upper child.
 * @return 0 on success, othweise -1.
 **/
static int write_to_children(Point *points, Point **c1_points, Point **c2_points, int point_counter[3], int point_sizes[3], float mean, int lower_read_pipe[2], int upper_read_pipe[2])
{
    // open write-end of children´s read-pipes
    FILE *c1 = fdopen(lower_read_pipe[1], "a");
    FILE *c2 = fdopen(upper_read_pipe[1], "a");

    if (c1 == NULL || c2 == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < point_counter[0]; i++)
    {
        Point p = points[i];

        int length = snprintf(NULL, 0, "%f %f\n", p.x, p.y) + 1;
        char *buffer = malloc(length); // +1 for null byte
        if (buffer == NULL)
        {
            return -1;
        }

        //write to buffer
        snprintf(buffer, length, "%f %f\n", p.x, p.y);

        //check which child to write to
        if (p.x <= mean) // lower child
        {
            point_counter[1]++;
            if (point_counter[1] >= point_sizes[1])
            {
                point_sizes[1] *= 2;
                if (((*c1_points) = realloc((*c1_points), (sizeof(Point) * point_sizes[1]))) == NULL)
                {
                    free(buffer);
                    return -1;
                }
                //write to child
                if (fprintf(c1, "%s", buffer) == -1)
                {
                    free(buffer);
                    return -1;
                }
            }

            (*c1_points)[point_counter[1] - 1] = p;
        }
        else
        { // upper child

            point_counter[2]++;
            if (point_counter[2] >= point_sizes[2])
            {
                point_sizes[2] *= 2;
                if (((*c2_points) = realloc((*c2_points), (sizeof(Point) * point_sizes[2]))) == NULL)
                {
                    free(buffer);
                    return -1;
                }
                //write to child
                if (fprintf(c2, "%s", buffer) == -1)
                {
                    free(buffer);
                    return -1;
                }
            }

            (*c2_points)[point_counter[2] - 1] = p;
        }
        free(buffer);
    }
    // if a child encounters an error
    if (ferror(c1) || ferror(c2))
    {
        return -1;
    }
    //close pipe-streams
    if (fclose(c1) == -1 || fclose(c2) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief calculates the distance between 2 points.
 * @details formula: sqrt((x2-x1)^2 + (y2-y1)^2).
 * @param p1 first point.
 * @param p2 second point.
 * @returns the distancce between the 2 given points.
 **/
static float calculate_distance(Point p1, Point p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

/**
 * @brief reads the solution of a child.
 * @details reads from the according pipe-end.
 * The result is saved in the passed Pair struct.
 * @param pipe holding the file-descriptor of the pipe.
 * @param upper boolean indicating if the upper child is read from.
 * @param pair pointer to the Pair-struct where the solution should be stored.
 * @param point_counter array holding 3 entries: 
 * [0] -> amount of points of the parent.
 * [1] -> amount of points of first child.
 * [2] -> amount of points of second child.
 * @return 0 on success, otherwise -1.
 **/
static int read_from_child(int pipe[2], bool upper, Pair *pair, int point_counter[3])
{
    FILE *child = fdopen(pipe[0], "r");
    if (child == NULL)
    {
        return -1;
    }

    char *line = NULL;
    size_t n = 0;

    if (point_counter[upper + 1] >= 2)
    {
        // Point 1
        if (getline(&line, &n, child) != -1)
        {
            Point p;
            line = strtok(line, " ");
            p.x = strtof(line, NULL);
            line = strtok(NULL, " ");
            p.y = strtof(line, NULL);

            pair->p1 = p;
        }
        // Point 2
        if (getline(&line, &n, child) != -1)
        {
            Point p;
            line = strtok(line, " ");
            p.x = strtof(line, NULL);
            line = strtok(NULL, " ");
            p.y = strtof(line, NULL);

            pair->p2 = p;
        }
        pair->distance = calculate_distance(pair->p1, pair->p2);
    }
    free(line);
    if (ferror(child) == -1 || fclose(child) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief compute the closest pair of points.
 * @details computes the closest pair of points and returns a Pair struct including the calculated distance.
 * @param c1_points points of the lower child.
 * @param c2_points points of the upper child.
 * @param point_counter array holding 3 entries: 
 * [0] -> amount of points of the parent.
 * [1] -> amount of points of first child.
 * [2] -> amount of points of second child.
 * @return Pair containing the closest pair of points and the distance between them.
 **/
static Pair calculate_best_solution(Point *c1_points, Point *c2_points, int point_counter[3])
{
    Pair p = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .distance = FLT_MAX}; // "worst" Pair possible

    for (size_t i = 0; i < point_counter[1]; i++)
    {
        Point tmp = c1_points[i];
        for (size_t j = 0; j < point_counter[2]; j++)
        {
            Point tmp2 = c2_points[j];
            float dist = calculate_distance(tmp, tmp2);

            if (dist < p.distance)
            {
                p.distance = dist;
                p.p1 = tmp;
                p.p2 = tmp2;
            }
        }
    }
    return p;
}

/**
 * @brief reads from the children, computes the closest pair between their points,
 * compares them and writes the closest/best pair to stdout.
 * @details waits for the child processes to finish and afterwards read their particular solutions.
 * Finally, the 3 solutions, closest pair of first-child, closest pair of second point and 
 * closest point between their points are compared and the pair with the shortest distance
 * is written to stdout.
 * @param c1_id process id of the lower child.
 * @param c2_id process id of the lower child.
 * @param points pointer to the total points of the parent.
 * @param c1_points pointer to the points of the lower child.
 * @param c2_points pointer to the points of the upper child.
 * @param point_counter array holding 3 entries: 
 * [0] -> amount of points of the parent.
 * [1] -> amount of points of first child.
 * [2] -> amount of points of second child.
 * @param lower_write_pipe write-pipe of the lower child.
 * @param upper_write_pipe write-pipe of the upper child.
 * @return 0 on success, otherwise -1.
 **/
static int find_solution(pid_t c1_id, pid_t c2_id, Point *points, Point *c1_points, Point *c2_points, int point_counter[3], int lower_write_pipe[2], int upper_write_pipe[2])
{
    int status;

    //Wait for c1
    while (waitpid(c1_id, &status, 0) == -1)
    {
        if (errno == EINTR)
        { // check if signal was received
            continue;
        }
        return -1;
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    { // c1 failed
        return -1;
    }

    //Wait for c2
    while (waitpid(c2_id, &status, 0) == -1)
    {
        if (errno == EINTR)
        { // check if signal was received
            continue;
        }
        return -1;
    }
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    { // c2 failed
        return -1;
    }

    Pair p1 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .distance = FLT_MAX};
    Pair p2 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .distance = FLT_MAX};
    Pair p3 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .distance = FLT_MAX};

    if (read_from_child(lower_write_pipe, false, &p1, point_counter) == -1)
    {
        return -1;
    }

    if (read_from_child(upper_write_pipe, true, &p2, point_counter) == -1)
    {
        return -1;
    }

    p3 = calculate_best_solution(c1_points, c2_points, point_counter);

    //fprintf(stderr, "%e %e | %e %e\n", p1.p1.x, p1.p1.y, p1.p2.x, p1.p2.y);
    //fprintf(stderr, "%e %e | %e %e\n", p2.p1.x, p2.p1.y, p2.p2.x, p2.p2.y);
    //fprintf(stderr, "%e %e | %e %e\n", p3.p1.x, p3.p1.y, p3.p2.x, p3.p2.y);

    Pair best;

    if (p1.distance < p2.distance && p1.distance < p3.distance)
    {
        best = p1;
    }
    else if (p2.distance < p1.distance && p2.distance < p3.distance)
    {
        best = p2;
    }
    else
    {
        best = p3;
    }

    fprintf(stdout, "%f %f\n%f %f\n", best.p1.x, best.p1.y, best.p2.x, best.p2.y);
    fflush(stdout);
    return 0;
}

/**
 * @brief entry point of the program.
 * @details initializes defined structs, arrays and variables.
 * After reading input, fork will be called to start 2 child programs, which will
 * recursivly execute the program. 
 * Calls according methods for parent/child processes.
 * @param argc argument counter, should only be 1
 * @param argv argument vector, should only hold program name
 * @return 0 on success, otherwise -1.
 **/
int main(int argc, char const *argv[])
{
    PROGRAM_NAME = argv[0];

    if (argc != 1)
    {
        usage();
    }

    int point_counter[3] = {0, 0, 0};
    int point_sizes[3] = {4, 2, 2}; // parent, child1, child2

    Point *points = malloc(sizeof(Point) * 4); // at least 4 points required for fork
    Point *c1_points = malloc(sizeof(Point) * 2);
    Point *c2_points = malloc(sizeof(Point) * 2);

    float mean = 0;

    if (points == NULL || c1_points == NULL || c2_points == NULL)
    {
        clean_up(points, c1_points, c2_points);
        error_exit("ERROR: Could not allocate memory for point storages!");
    }

    if (handle_input(&points, point_counter, point_sizes, &mean) == -1)
    {
        clean_up(points, c1_points, c2_points);
        error_exit("ERROR: Could not handle input!");
    }

    switch (point_counter[0])
    {
    case 0:
    case 1:
        clean_up(points, c1_points, c2_points);
        exit(EXIT_SUCCESS);
        break;
    case 2:
        fprintf(stdout, "%f %f\n%f %f\n", points[0].x, points[0].y, points[1].x, points[1].y);
        fflush(stdout);
        clean_up(points, c1_points, c2_points);
        exit(EXIT_SUCCESS);
        break;
    default:
        break;
    }

    // Initialize Pipes
    int lower_write_pipe[2];
    int lower_read_pipe[2];
    int upper_write_pipe[2];
    int upper_read_pipe[2];
    if (pipe(lower_write_pipe) == -1 || pipe(lower_read_pipe) == -1 ||
        pipe(upper_write_pipe) == -1 || pipe(upper_read_pipe) == -1)
    {
        clean_up(points, c1_points, c2_points);
        error_exit("ERROR: Could not create pipes!");
    }

    // fork for lower child
    int c1 = fork();
    if (c1 == -1) // ERROR
    {
        close_pipes(lower_read_pipe, lower_write_pipe, upper_read_pipe, upper_write_pipe);
        clean_up(points, c1_points, c2_points);
        error_exit("ERROR: Could not fork!");
    }

    if (c1 == 0) // behavior for child
    {
        // Redirect pipes to stdin and stdout
        if ((dup2(lower_write_pipe[1], STDIN_FILENO) == -1) || (dup2(lower_read_pipe[0], STDOUT_FILENO) == -1))
        {
            clean_up(points, c1_points, c2_points);
            error_exit("ERROR: Could not redirect pipes!");
        }

        //close pipes of other child
        close(upper_read_pipe[0]);
        close(upper_read_pipe[1]);
        close(upper_write_pipe[0]);
        close(upper_write_pipe[1]);

        // Close unused pipe ends
        close(lower_read_pipe[0]);  //read end
        close(lower_write_pipe[1]); //write end

        // Close redirected pipes
        close(lower_read_pipe[1]);
        close(lower_write_pipe[0]);

        if (execlp(PROGRAM_NAME, PROGRAM_NAME, (char *)NULL) == -1)
        {
            clean_up(points, c1_points, c2_points);
            error_exit("ERROR: Could run execlp!");
        }
    }
    else
    { // start forking for second child

        int c2 = fork();
        if (c2 == -1)
        {
            close_pipes(lower_read_pipe, lower_write_pipe, upper_read_pipe, upper_write_pipe);
            clean_up(points, c1_points, c2_points);
            error_exit("ERROR: Could not fork!");
        }

        if (c2 == 0) // behavior for child
        {
            // Redirect pipes to stdin and stdout
            if ((dup2(upper_write_pipe[1], STDIN_FILENO) == -1) || (dup2(upper_read_pipe[0], STDOUT_FILENO) == -1))
            {
                clean_up(points, c1_points, c2_points);
                error_exit("ERROR: Could not redirect pipes!");
            }

            //close pipes of other child
            close(lower_read_pipe[0]);
            close(lower_read_pipe[1]);
            close(lower_write_pipe[0]);
            close(lower_write_pipe[1]);

            // Close unused pipe ends
            close(upper_read_pipe[0]);  //read end
            close(upper_write_pipe[1]); //write end

            // Close redirected pipes
            close(upper_read_pipe[1]);
            close(upper_write_pipe[0]);

            if (execlp(PROGRAM_NAME, PROGRAM_NAME, NULL) == -1)
            {
                clean_up(points, c1_points, c2_points);
                error_exit("ERROR: Could not run execlp!");
            }
        }
        else
        { // parent behavior

            close_pipes(lower_read_pipe, lower_write_pipe, upper_read_pipe, upper_write_pipe);

            //write points to children
            if (write_to_children(points, &c1_points, &c2_points, point_counter, point_sizes, mean, lower_read_pipe, upper_read_pipe) == -1)
            {
                clean_up(points, c1_points, c2_points);
                error_exit("ERROR: Could not write to children!");
            }

            if (find_solution(c1, c2, points, c1_points, c2_points, point_counter, lower_write_pipe, upper_write_pipe) == -1)
            {
                clean_up(points, c1_points, c2_points);
                error_exit("ERROR: Could not read from children!");
            }
        }
    }

    clean_up(points, c1_points, c2_points);
    exit(EXIT_SUCCESS);
}