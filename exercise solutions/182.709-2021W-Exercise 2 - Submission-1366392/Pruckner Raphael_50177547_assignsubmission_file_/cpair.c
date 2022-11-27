/**
 * @file cpair.c
 * @author Raphael Pruckner <e11806918@student.tuwien.ac.at> 
 * @date 09.12.21
 * 
 * @brief This is the main module. It contains the entire funcionality.
 * @details This module is initially ecexuted and after reading
 * the entire user input it starts forking, creating new instances of itself,
 * to which it passes parts of the input. These children then repeat the process
 * until the shortest distance between the points specified in the input are found,
 * passing these distances to their parents, which then reevaluate.
 */

#include "cpair.h"

char *prog_name;

/**
 * Mandatory usage function.
 * @brief This function reminds users that this program does not take any arguments.
 * @details After printing an incredibly useful usage message the program exits with Exit Code 1;
 */
void usage()
{
    fprintf(stderr, "Usage: %s \nThis program does not take any arguments. Points shall be passed via the command line.\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Minimum function.
 * @brief This function returns the smaller of the two given integers.
 * @details Mainly used for multiple return value checks.
 * @param a Integer a for comparison.
 * @param b Integer b for comparison.
 * @return The smaller of both integers.
 */
int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

/**
 * Input parsing function.
 * @brief This function takes the input from stdin and parses it into an array of points (point_t).
 * @details The initial length of the array the input is parsed to is 8, if the limit of the array size is reached, the capacity
 * is doubled. Freeing the memory has to be done by the caller.
 * @param points The pointer the array of points shall be saved to.
 * @return Returns EXIT_SUCCESS, or EXIT_FAILURE in case something goes wrong.
 */
int parse_input(points_array_t *points, FILE *in)
{
    //Initializing memory
    int length = 8;
    points->point_c = 0;
    points->points = malloc(sizeof(point_t) * length);
    if (points == NULL)
    {
        fprintf(stderr, "[%s] ERROR! malloc failed: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    char tmp[64];
    while (!feof(in))
    {
        //If maximum capacity is reached, capacity is doubled
        if (points->point_c == length)
        {
            length *= 2;
            point_t *new_ptr = realloc(points->points, sizeof(point_t) * length);
            if (new_ptr == NULL)
            {
                fprintf(stderr, "[%s] ERROR! realloc failed: %s\n", prog_name, strerror(errno));
                return EXIT_FAILURE;
            }
            points->points = new_ptr;
        }

        if (fgets(tmp, 64, in) == NULL)
        {
            if (feof(in))
            {
                break;
            }
            fprintf(stderr, "[%s] ERROR! fgets failed: %s\nPlease limit inputs to 64 characters!\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        if (strcmp(tmp, "\n") == 0)
        {
            continue;
        }

        //variables to store the remainder of the string after each strtof call
        char *tmp_y, *err;

        point_t point;

        errno = 0; //!< set errno to 0 since strtof can return 0 and sets errno to != 0 in case it fails
        point.x = strtof(tmp, &tmp_y);
        point.y = strtof(tmp_y, &err);

        //an error occured during strtof
        if (errno != 0)
        {
            fprintf(stderr, "[%s] ERROR! strtof failed: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        //no more digits were found in after parsing the first number, since the remaining string is the same as the initial
        if (tmp_y == err)
        {
            fprintf(stderr, "[%s] ERROR! No digits found\n", prog_name);
            return EXIT_FAILURE;
        }

        //there are still characters in the string, meaning more than just 2 floats were input
        if (*err != '\n' && *err != '\0')
        {
            fprintf(stderr, "[%s] ERROR! Invalid format. %s is not valid input\n", prog_name, err);
            return EXIT_FAILURE;
        }

        points->points[points->point_c] = point;
        points->point_c++;
    }
    return EXIT_SUCCESS;
}

/**
 * Mean calculation function
 * @brief This function takes an array of points and calculates the mean of the x-coordinates.
 * @param points The array of points.
 * @return Returns the mean of all x-coordinates.
 */
float calculate_mean(points_array_t points)
{
    float sum = 0;
    for (int i = 0; i < points.point_c; i++)
    {
        sum += points.points[i].x;
    }
    float mean = sum / points.point_c;
    return mean;
}

/**
 * Array splitting function
 * @brief This function splits a given point array into two new arrays, split by the mean of all x-coordinates
 * @details This function allocates memory for points arrays, then splits the two arrays by putting values <= mean
 * into left and values > mean into right and stores the points in the given pointers.
 * @param left Pointer where array containing points with x-coordinates <= mean are stored.
 * @param right Pointer where array containing points with x-coordinates > mean are stored.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE if malloc failed or 1 if only one array was filled.
 */
int split_array(points_array_t *left, points_array_t *right, points_array_t points, float mean)
{
    int left_count = 0;
    for (int i = 0; i < points.point_c; i++)
    {
        if (points.points[i].x <= mean)
        {
            left_count++;
        }
    }

    point_t *left_arr = malloc(sizeof(point_t) * left_count);
    point_t *right_arr = malloc(sizeof(point_t) * (points.point_c - left_count));
    if (left_arr == NULL || right_arr == NULL)
    {
        fprintf(stderr, "[%s] ERROR! malloc failed: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    left->points = left_arr;
    right->points = right_arr;

    left->point_c = 0;
    right->point_c = 0;

    for (int i = 0; i < points.point_c; i++)
    {
        if (points.points[i].x <= mean)
        {
            left->points[left->point_c] = points.points[i];
            left->point_c++;
        }
        else
        {
            right->points[right->point_c] = points.points[i];
            right->point_c++;
        }
    }

    //if array could not be split into two smaller arrays (e.g. all values are the same);
    if (right->point_c == points.point_c || left->point_c == points.point_c)
    {
        return 1;
    }

    return EXIT_SUCCESS;
}

/**
 * Point array printing function.
 * @brief This function prints the two coordinates of each point separated by a space, each point separated by a new line
 * Coordinates are printed to stdout, after printing output is terminated by an EOF.
 * @param points Array of points to print.
 */
void print_array(points_array_t points)
{
    for (int i = 0; i < points.point_c; i++)
    {
        printf("%f %f\n", points.points[i].x, points.points[i].y);
    }
    fflush(stdout);
}

/**
 * Point array writing function.
 * @brief This function writes the two coordinates of each point separated by a space, each point separated by a new line
 * Coordinates are printed to the specified file, after printing output is terminated by an EOF.
 * @param points Array of points to write.
 * @param fd File descriptor of file to write to.
 * @return Returns EXIT_SUCCESS, or EXIT_FAILURE if the file could not be opened.
 */
int write_array(points_array_t points, FILE *out)
{
    for (int i = 0; i < points.point_c; i++)
    {
        fprintf(out, "%f %f\n", points.points[i].x, points.points[i].y);
    }
    fflush(out);
    return EXIT_SUCCESS;
}

/**
 * Pair parsing function.
 * @brief This function takes a points array and converts it to a pair.
 * @details The function assumes that the given array only contains two points.
 * @param points The points array.
 * @param pair The pointer in which the pair shall be stored.
 * @return Returns EXIT_SUCCESS, EXIT_FAILURE if something goes wrong or 1 if the array is empty.
 */
int parse_pair(points_array_t points, pair_t *pair)
{
    if (points.point_c == 0)
    {
        return 1;
    }
    if (points.point_c != 2)
    {
        fprintf(stderr, "The given array does not contain exactly 2 points. Parsing to pair not possible\n");
        return EXIT_FAILURE;
    }
    pair->point1 = points.points[0];
    pair->point2 = points.points[1];
    return EXIT_SUCCESS;
}

/**
 * Pair generation function.
 * @brief This function gets the closest pairs of each array by forking the process recursively.
 * @param left The point array of the left side.
 * @param right The point arry of the right side.
 * @param p1 The pointer where the pair from the left side shall be stored to.
 * @param p2 The pointer where the pair from the right side shall be stored to.
 * @return Returns EXIT_SUCCESS, or EXIT_FAILURE if something goes wrong.
 */
int get_pairs(points_array_t left, points_array_t right, pair_t *p1, pair_t *p2)
{
    //create pipes
    int ret = 0;
    int pipefd_p_to_c_left[2], pipefd_p_to_c_right[2];
    int pipefd_c_to_p_left[2], pipefd_c_to_p_right[2];

    ret = min(ret, pipe(pipefd_p_to_c_left));
    ret = min(ret, pipe(pipefd_p_to_c_right));
    ret = min(ret, pipe(pipefd_c_to_p_left));
    ret = min(ret, pipe(pipefd_c_to_p_right));

    if (ret != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not pipe: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    //fork processes
    pid_t left_child = fork();
    if (left_child == -1)
    {
        fprintf(stderr, "[%s] ERROR! Could not fork: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }
    if (left_child == 0)
    {
        ret = 0;
        //redirect child's stdout to parent's read end
        if (dup2(pipefd_c_to_p_left[1], STDOUT_FILENO) == -1)
        {
            fprintf(stderr, "[%s] ERROR! dup2 failed: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        //redirect write end of parent to child's stdin
        if (dup2(pipefd_p_to_c_left[0], STDIN_FILENO) == -1)
        {
            fprintf(stderr, "[%s] ERROR! dup2 failed: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        //close pipes
        ret = min(ret, close(pipefd_p_to_c_right[0]));
        ret = min(ret, close(pipefd_p_to_c_right[1]));
        ret = min(ret, close(pipefd_c_to_p_right[0]));
        ret = min(ret, close(pipefd_c_to_p_right[1]));
        ret = min(ret, close(pipefd_c_to_p_left[0]));
        ret = min(ret, close(pipefd_p_to_c_left[1]));
        ret = min(ret, close(pipefd_p_to_c_left[0]));
        ret = min(ret, close(pipefd_c_to_p_left[1]));

        if (ret != 0)
        {
            fprintf(stderr, "[%s] ERROR! Could not close file: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        execlp(prog_name, prog_name, (char *)NULL);
        fprintf(stderr, "[%s] ERROR! Could not execute: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    pid_t right_child = fork();
    if (right_child == -1)
    {
        fprintf(stderr, "[%s] ERROR! Could not fork: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }
    if (right_child == 0)
    {
        ret = 0;
        //redirect child's stdout to parent's read end
        if (dup2(pipefd_c_to_p_right[1], STDOUT_FILENO) == -1)
        {
            fprintf(stderr, "[%s] ERROR! dup2 failed: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        //redirect write end of parent to child's stdin
        if (dup2(pipefd_p_to_c_right[0], STDIN_FILENO) == -1)
        {
            fprintf(stderr, "[%s] ERROR! dup2 failed: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        //close pipes
        ret = min(ret, close(pipefd_p_to_c_left[0]));
        ret = min(ret, close(pipefd_p_to_c_left[1]));
        ret = min(ret, close(pipefd_c_to_p_left[0]));
        ret = min(ret, close(pipefd_c_to_p_left[1]));
        ret = min(ret, close(pipefd_c_to_p_right[0]));
        ret = min(ret, close(pipefd_p_to_c_right[1]));
        ret = min(ret, close(pipefd_p_to_c_right[0]));
        ret = min(ret, close(pipefd_c_to_p_right[1]));

        if (ret != 0)
        {
            fprintf(stderr, "[%s] ERROR! Could not close file: %s\n", prog_name, strerror(errno));
            return EXIT_FAILURE;
        }

        execlp(prog_name, prog_name, (char *)NULL);
        fprintf(stderr, "[%s] ERROR! Could not execute: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    //close unused pipe ends
    ret = 0;
    ret = min(ret, close(pipefd_p_to_c_left[0]));
    ret = min(ret, close(pipefd_c_to_p_left[1]));
    ret = min(ret, close(pipefd_p_to_c_right[0]));
    ret = min(ret, close(pipefd_c_to_p_right[1]));

    if (ret != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not close file: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    //get information back
    FILE *in_left = fdopen(pipefd_c_to_p_left[0], "r");
    FILE *in_right = fdopen(pipefd_c_to_p_right[0], "r");
    FILE *out_left = fdopen(pipefd_p_to_c_left[1], "w");
    FILE *out_right = fdopen(pipefd_p_to_c_right[1], "w");

    if (in_left == NULL || in_right == NULL || out_left == NULL || out_right == NULL)
    {
        fprintf(stderr, "[%s] ERROR! Could not open file: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    //write from parent to child
    if (write_array(left, out_left) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not write to array: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    if (write_array(right, out_right) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not write to array: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    fclose(out_left);
    fclose(out_right);

    points_array_t points_left;
    points_array_t points_right;

    if (parse_input(&points_left, in_left) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not parse input: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    if (parse_input(&points_right, in_right) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not parse input: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    int ret_p1, ret_p2;
    ret_p1 = parse_pair(points_left, p1);
    ret_p2 = parse_pair(points_right, p2);
    if (ret_p1 != 0)
    {
        if (ret_p1 == -1)
        {
            return EXIT_FAILURE;
        }
        //if 1 was returned, the array only contained 1 point
        if (ret_p1 == 1)
        {
            p1->point1 = left.points[0];
            p1->point2.x = FLT_MAX;
            p1->point2.y = FLT_MAX;
        }
    }
    if (ret_p2 != 0)
    {
        if (ret_p2 == -1)
        {
            return EXIT_FAILURE;
        }
        //if 1 was returned, the array only contained 1 point
        if (ret_p2 == 1)
        {
            p2->point1 = right.points[0];
            p2->point2.x = FLT_MAX;
            p2->point2.y = FLT_MAX;
        }
    }

    free(points_left.points);
    free(points_right.points);

    fclose(in_left);
    fclose(in_right);

    //wait
    int status;
    waitpid(left_child, &status, 0);
    if (status != EXIT_SUCCESS)
    {
        fprintf(stderr, "[%s] ERROR! Child did not return EXIT_SUCCESS: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }
    waitpid(right_child, &status, 0);
    if (status != EXIT_SUCCESS)
    {
        fprintf(stderr, "[%s] ERROR! Child did not return EXIT_SUCCESS: %s\n", prog_name, strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * Distance calculation function.
 * @brief This function calculates the euclidian distance between two points.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return Euclidian distance between the two points.
 */
float calculate_distance(point_t p1, point_t p2)
{
    float d_x_sq = powf((p2.x - p1.x), 2);
    float d_y_sq = powf((p2.y - p1.y), 2);
    float d = sqrt(d_x_sq + d_y_sq);
    return d;
}

/**
 * Closest pair function.
 * @brief This function determines the closest distance between points from the left and the right point array.
 * @param left The left point array.
 * @param right The right point array.
 * @param shortest_pair The pointer of the pair the pair with the shortest distance shall be stored to.
 * @return EXIT_SUCCESS upon success or EXIT_FAILURE if no pair was found.
 */
int find_closest_pair(points_array_t left, points_array_t right, pair_t *shortest_pair)
{
    float shortest_distance = FLT_MAX;
    float tmp_distance;
    for (int i = 0; i < left.point_c; i++)
    {
        for (int j = 0; j < right.point_c; j++)
        {
            tmp_distance = calculate_distance(left.points[i], right.points[j]);
            if (tmp_distance < shortest_distance)
            {
                shortest_distance = tmp_distance;
                shortest_pair->point1 = left.points[i];
                shortest_pair->point2 = right.points[j];
            }
        }
    }
    if (shortest_distance == FLT_MAX)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * Closest of three pairs function.
 * @brief This function determines the pair with the shortest distance.
 * @param p1 The first pair.
 * @param p2 The second pair.
 * @param p3 The third pair.
 * @return The pair with the shortest euclidian distance between its points.
 */
pair_t get_closest_pair(pair_t p1, pair_t p2, pair_t p3)
{
    float p1_distance, p2_distance, p3_distance;
    p1_distance = calculate_distance(p1.point1, p1.point2);
    p2_distance = calculate_distance(p2.point1, p2.point2);
    p3_distance = calculate_distance(p3.point1, p3.point2);
    if (p1_distance <= p2_distance && p1_distance <= p3_distance)
    {
        return p1;
    }
    if (p2_distance <= p1_distance && p2_distance <= p1_distance)
    {
        return p2;
    }
    return p3;
}

/**
 * Print pair function.
 * @brief This function prints the two points of a pair to stdout.
 * @param pair The pair to print.
 */
void print_pair(pair_t pair)
{
    printf("%f %f\n", pair.point1.x, pair.point1.y);
    printf("%f %f\n", pair.point2.x, pair.point2.y);
    fflush(stdout);
}

/**
 * Find closest pair in array function.
 * @brief This function finds the two closest points in a point array.
 * @param points The array of points to find the two closest points in.
 * @param pair The pointer of the pair the closest pair shall be stored in.
 * @return EXIT_SUCCESS or EXIT_FAILURE, if no closest pair could be found.
 */
int find_closest_pair_in_array(points_array_t points, pair_t *pair)
{
    float shortest_distance = FLT_MAX;
    float tmp_distance;
    for (int i = 0; i < points.point_c; i++)
    {
        for (int j = i + 1; j < points.point_c; j++)
        {
            tmp_distance = calculate_distance(points.points[i], points.points[j]);
            if (tmp_distance < shortest_distance)
            {
                shortest_distance = tmp_distance;
                pair->point1 = points.points[i];
                pair->point2 = points.points[j];
            }
        }
    }
    if (shortest_distance == FLT_MAX)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * Program entry point.
 * @brief The main function takes care of calling all the necessary functions.
 * @details This function checks that no arguments have been passed, since
 * the program does not take any arguments.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE if something goes wrong.
 */
int main(int argc, char *argv[])
{
    prog_name = argv[0];
    if (argc > 1)
    {
        usage();
    }

    points_array_t points;
    if (parse_input(&points, stdin) != 0)
    {
        fprintf(stderr, "[%s] ERROR! parse_inputs failed\n", prog_name);
        exit(EXIT_FAILURE);
    }

    if (points.point_c == 0)
    {
        fprintf(stderr, "[%s] ERROR! Please give some input!\n", prog_name);
        exit(EXIT_FAILURE);
    }

    if (points.point_c == 1)
    {
        exit(EXIT_SUCCESS);
    }

    if (points.point_c == 2)
    {
        print_array(points);
        free(points.points);
        exit(EXIT_SUCCESS);
    }

    float mean = calculate_mean(points);

    points_array_t left, right;
    int split;
    split = split_array(&left, &right, points, mean);
    if (split == -1)
    {
        fprintf(stderr, "[%s] ERROR! Array could not be split into two smaller arrays\n", prog_name);
        free(points.points);
        exit(EXIT_FAILURE);
    }
    //if the array cannot be split, all the points in the array are on the same x-coordinate
    if (split == 1)
    {
        pair_t pair;
        if (find_closest_pair_in_array(points, &pair) != 0){
            free(left.points);
            free(right.points);
            free(points.points);
            exit(EXIT_FAILURE);
        }
        print_pair(pair);
        exit(EXIT_SUCCESS);
    }

    pair_t p1;
    pair_t p2;
    if (get_pairs(left, right, &p1, &p2) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not generate pairs\n", prog_name);
        free(left.points);
        free(right.points);
        free(points.points);
        exit(EXIT_FAILURE);
    }

    pair_t p3;
    if (find_closest_pair(left, right, &p3) != 0)
    {
        fprintf(stderr, "[%s] ERROR! Could not find shortest pair\n", prog_name);
        free(left.points);
        free(right.points);
        free(points.points);
        exit(EXIT_FAILURE);
    }

    pair_t closest = get_closest_pair(p1, p2, p3);
    print_pair(closest);

    free(left.points);
    free(right.points);
    free(points.points);
    exit(EXIT_SUCCESS);
}
