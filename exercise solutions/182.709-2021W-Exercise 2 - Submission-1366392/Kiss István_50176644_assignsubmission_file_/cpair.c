/**
 * @file cpair.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Determines the closest pair of points using process fork mechanism.
 * The input sliced to two parts and the parent process forks two child processes.
 * One child process works on one input.
 * @version 0.1
 * @date 2021-12-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <float.h>

/**
 * @brief macro for program name
 * 
 */
#define PROG_NAME "cpair"

/**
 * @brief default buffer size for dynamic memory allocation
 * 
 */
#define BUFFER_SIZE 128

/**
 * @brief global variable to save the command the program is executed with
 * 
 */
static char *command;

/**
 * @brief custom structure to store points
 * 
 */
typedef struct {
    float x;
    float y;
} Point;

/**
 * @brief Prints usage message 
 * 
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s\n", PROG_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints the error message to stderr and exits with failure
 * @details uses the global variable command
 * 
 * @param msg error message
 */
static void handle_error(char* msg) {
    fprintf(stderr, "[%s] fatal error: %s: %s \n", command, msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Compares the provided value and the error value.
 * @details If value and error_value equal, then exists with error
 * 
 * @param value value to check
 * @param error_value error value to check against
 * @param msg error message
 */
static void check_error(int value, int error_value, char* msg) {
    if (value == error_value) {
        handle_error(msg);
    }
}

/**
 * @brief Processes program arguments
 * @details If there is any argument then the program terminates
 * 
 * @param argc number of arguments
 */
static void handle_args(int argc) {
    if (argc > 1) {
        usage();
    }
}

/**
 * @brief Reads a line from input.
 * @details The line is validated, if invalid error is handled.
 * The line must contain 2 float number separated by a single whitespace.
 * 
 * @param line input line
 * @return Point point read from line
 */
static Point read_line(char* line) {
    char* endptr;
    float x, y;
    //first float
    x = strtof(line, &endptr);
    //whitespace
    if (isspace(*endptr) == 0) {
        errno = EINVAL;
        handle_error("invalid line format");
    }
    //second float
    y = strtof(endptr + 1, &endptr);
    //check values
    if (x == -HUGE_VALF || x == HUGE_VALF) {
        handle_error("strtof for x failed");
    }
    if (y == -HUGE_VALF || y == HUGE_VALF) {
        handle_error("strof for y failed");
    }

    //check line ending after second float
    if (*endptr != '\n' && *endptr != EOF) {
        errno = EINVAL;
        handle_error("invalid line");
    }
    Point p1 = {x, y};
    return p1;
}

/**
 * @brief Reads the input from the specified FILE
 * @details Every of the input gets processed and 
 * the points are written to the points pointer.
 * 
 * @param src source of input
 * @param points found points destination
 * @return int number of found points
 */
static int read_input(FILE* src, Point** points) {
    int count = 0;
    size_t linecap = 0;
    ssize_t linelen;
    char* line = NULL;
    int buffer_size = BUFFER_SIZE;
    *points = malloc(sizeof(Point) * BUFFER_SIZE);
    if (*points == NULL) {
        handle_error("malloc failed");
    }

    while((linelen = getline(&line, &linecap, src)) != -1) {
        if (count >= buffer_size) {
            *points = realloc(*points, sizeof(Point) * (buffer_size + BUFFER_SIZE));
            buffer_size = buffer_size + BUFFER_SIZE;
            if (*points == NULL) {
                handle_error("realloc failed");
            }
        }
        Point curr_point = read_line(line);
        (*points)[count] = curr_point;
        count++;
    }
    free(line);
    return count;
}

/**
 * @brief Returns the mean of the X value of points
 * 
 * @param points points
 * @param size number of points
 * @return float mean of x values
 */
static float mean_x(Point* points, int size) {
    float sum_x = 0;
    for (int i = 0; i < size; i++){
        sum_x += points[i].x;
    }
    return sum_x / size;    
}

/**
 * @brief Prints the specified point to the specified FILE stream
 * @details the output is flushed immediately
 * 
 * @param dst output stream
 * @param point point to be printed
 */
static void print_point(FILE* dst, Point point) {
    fprintf(dst, "%.6f %.6f\n", point.x, point.y);
    if (fflush(dst) == EOF) {
        handle_error("fflush failed");
    }
}

/**
 * @brief Prints a pair of points to the specified FILE stream
 * 
 * @param dst output stream
 * @param points two pair of points to be printed
 */
static void print_pair(FILE* dst, Point *points) {
    for (int i = 0; i < 2; i++) {
        print_point(dst, points[i]);
    }
}

/**
 * @brief Redirects the read and write pipe and closes all pipe file descriptors.
 * @details pipes are redirected to stdout and stdin
 * 
 * @param pipe pipe of four file descriptors that should be redirected
 * @param other_pipe specified pipes are closed
 */
static void redirect_child_pipe(int pipe[4], int other_pipe[4]) {
    check_error(dup2(pipe[3], STDOUT_FILENO), -1, "dup2 stdout failed");
    check_error(dup2(pipe[0], STDIN_FILENO), -1, "dup2 stdin failed");
    for (int i = 0; i < 4; i++) {
        check_error(close(pipe[i]), -1, "close failed");
        check_error(close(other_pipe[i]), -1, "close failed");
    }
    
}

/**
 * @brief Checks process exit status.
 * @details handles error if the exit status is not success
 * 
 * @param status process exit status
 */
static void check_child_proc_exit(int status) {
    if (WEXITSTATUS(status) != EXIT_SUCCESS) {
        handle_error("child process failed");
    }
}

/**
 * @brief Returns the distance of two points.
 * 
 * @param point_a point A
 * @param point_b point B
 * @return float distance of point A and point B
 */
static float calculate_point_distance(Point point_a, Point point_b) {
    return (float) sqrt( pow(point_a.x - point_b.x, 2) + pow(point_a.y - point_b.y, 2) );
}

/**
 * @brief Finds the closest pair of points
 * @details The closest pair is written to the closest_pair pointer
 * 
 * @param points all points to process
 * @param count number of points
 * @param closest_pair closest pair of points 
 * @return float distance of closest pair
 */
static float find_closest_pair(Point* points, int count, Point* closest_pair) {
    float min_dist = FLT_MAX;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            if (i == j) {
                continue;
            }
            float dist = calculate_point_distance(points[i], points[j]);
            if (dist < min_dist) {
                closest_pair[0] = points[i];
                closest_pair[1] = points[j];
                min_dist = dist;
            }

        }
    }
    return min_dist;
}
/**
 * @brief Returns the closest pair of points out of two set of points
 * 
 * @param points_a point set A
 * @param count_a size of point set A
 * @param points_b point set B
 * @param count_b size of point set B
 * @param closest_pair found closest pair
 * @return float distance of closest pair
 */
static float find_closest_pair_children(Point* points_a, int count_a, Point* points_b, int count_b, Point* closest_pair) {
    float min_dist = FLT_MAX;
    for (int i = 0; i < count_a; i++) {
        for (int j = 0; j < count_b; j++) {
            if (i == j) {
                continue;
            }
            float dist = calculate_point_distance(points_a[i], points_b[j]);
            if (dist < min_dist) {
                closest_pair[0] = points_a[i];
                closest_pair[1] = points_b[j];
                min_dist = dist;
            }

        }
    }
    return min_dist;
}

/**
 * @brief Program logic
 * @details Reads stdin and check the number of points found.
 * If necessary the program is forked and uses the child processes to find the closest pair of points.
 * it uses the global variable command
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) {
    command = argv[0];
    handle_args(argc);
    Point* points = NULL;
    int count;
    count = read_input(stdin, &points);

    if (count == 2) {

        for (int i = 0; i < count; i++) {
            print_point(stdout, points[i]);
        }

    } else if (count > 2) {
        int pipefd_leq[4], pipefd_gt[4];
        float mean = mean_x(points, count);

        pid_t child_leq, child_gt;
        for (int i = 0; i < 2; i++) {
            check_error(pipe(pipefd_leq + i*2), -1, "pipe failed");
            check_error(pipe(pipefd_gt + i*2), -1, "pipe failed");   
        }

        child_leq = fork();
        check_error(child_leq, -1, "fork failed");

        if (child_leq == 0) { //child leq

            redirect_child_pipe(pipefd_leq, pipefd_gt);
            execlp(command, command, NULL);

        } else {

            child_gt = fork();
            check_error(child_gt, -1, "fork failed");

            if (child_gt == 0) { //child gt

                redirect_child_pipe(pipefd_gt, pipefd_leq);
                execlp(command, command, NULL);

            } else { //parent

                //close unused pipe ends
                //write pipe read end closed
                check_error(close(pipefd_gt[0]), -1, "close failed");
                check_error(close(pipefd_leq[0]), -1, "close failed"); 

                //read pipe write end
                check_error(close(pipefd_gt[3]), -1, "close failed");
                check_error(close(pipefd_leq[3]), -1, "close failed"); 

                //open write streams
                int status_leq, status_gt;
                FILE* pipe_leq_in = fdopen(pipefd_leq[1], "w");
                if (pipe_leq_in == NULL) {
                    handle_error("fdopen to child 1 input pipe failed");
                }
                FILE* pipe_gt_in = fdopen(pipefd_gt[1], "w");
                if (pipe_gt_in == NULL) {
                    handle_error("fdopen to child 2 input pipe failed");
                }

                //write points to children
                //alternative makes guarantees that there is no endless recursion
                int alternate = 0;
                for (int i = 0; i < count; i++) {
                    if ((points[i]).x < mean) {
                        print_point(pipe_leq_in, points[i]);
                    } else if ((points[i]).x > mean) {
                        print_point(pipe_gt_in, points[i]);
                    } else {
                        if (alternate == 1) {
                            print_point(pipe_gt_in, points[i]);
                            alternate = 0;
                        } else {
                            print_point(pipe_leq_in, points[i]);
                            alternate = 1;
                        }
                    }
                }
                //close streams
                check_error(fclose(pipe_leq_in), -1, "fclose pipefd to child 1 input pipe failed");
                check_error(fclose(pipe_gt_in), -1, "fclose pipefd to child 2 input pipe failed");
                int count_leq, count_gt;
                Point *points_leq = NULL, *points_gt = NULL;
                
                //wait for children to terminate
                check_error(waitpid(child_gt, &status_gt, 0), -1, "child process error");
                check_error(waitpid(child_leq, &status_leq, 0), -1, "child process error");
                check_child_proc_exit(status_leq);
                check_child_proc_exit(status_gt);

                //open read streams to children
                FILE* pipe_leq_out = fdopen(pipefd_leq[2], "r");
                if (pipe_leq_out == NULL) {
                    handle_error("fdopen to child 1 output pipe failed");
                }
                FILE* pipe_gt_out = fdopen(pipefd_gt[2], "r");
                if (pipe_gt_out == NULL) {
                    handle_error("fdopen to child 2 output pipe failed");
                }

                //read input then close
                count_leq = read_input(pipe_leq_out, &points_leq);
                count_gt = read_input(pipe_gt_out, &points_gt);
                check_error(fclose(pipe_leq_out), -1, "fclose to child 1 output pipe failed");
                check_error(fclose(pipe_gt_out), -1, "fclose to child 2 output pipe failed");
                


                //calculate closest pair
                Point closest_pair_gt[2]; //P1
                Point closest_pair_leq[2]; //P2
                Point closest_pair_both[2]; //P3

                float cp_gt_dist = find_closest_pair(points_gt, count_gt, closest_pair_gt);
                float cp_leq_dist = find_closest_pair(points_leq, count_leq, closest_pair_leq);
                float cp_both_dist = find_closest_pair_children(points_gt, count_gt, points_leq, count_leq, closest_pair_both);

                //if no output from child
                if (count_leq == 0) {
                    cp_leq_dist = FLT_MAX;
                }

                //if no output from child
                if (count_gt == 0) {
                    cp_gt_dist = FLT_MAX;
                }

                if (cp_gt_dist <= cp_leq_dist) {
                    if (cp_gt_dist <= cp_both_dist) {
                        print_pair(stdout, closest_pair_gt);
                    } else {
                        print_pair(stdout, closest_pair_both);
                    }
                } else {
                    if (cp_leq_dist <= cp_both_dist) {
                        print_pair(stdout, closest_pair_leq);
                    } else {
                        print_pair(stdout, closest_pair_both);
                    }
                }
                free(points_gt);
                free(points_leq);
            }
        }
    }
    free(points);
    return EXIT_SUCCESS;
}
