/**
 *
 * @file cpair.c
 * @author Szymon Karabin (11839439)
 * @date 01.12.2021
 *
 * @brief cpair in its entirety. Programme reads in point pairs from stdin and calculates 
 * the pair of points that are closest to each other
 * 
 * @details Programme reads in from stdin until an EOF is encountered. Any amount of points 
 * can be entered in the format "x y", where x is the x-coordinate and y is the y-coordinate 
 * of the point. Points are separated by new lines. Programme has no positional arguments, 
 * no flags and no optional arguments and will not accept any of them.
 * 
 **/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <float.h>
#include <limits.h>
#include <unistd.h>

// Structure for a point, with x- and y-coordinates.
typedef struct point {
    float x;
    float y;
} point;

// Structure for a pair of points, has 2 points and the distance between them.
typedef struct pt_pair {
    point p1;
    point p2;
    float dist;
} pt_pair;

static char *PROGRAMME_NAME; // Needed for execlp

// Pipe array indexes in the form c#x, where # is number of child and x is r (for read pipe 
// of child) and w (for write pipe of child).
static int c1r = 0;
static int c1w = 1;
static int c2r = 2;
static int c2w = 3;

// Indexes for the pipe ends
static int r = 0;
static int w = 1;

/**
 * Error exit method
 * @brief Method for cleaning up and exiting with an error status
 * @param pt1 First point to be freed
 * @param pt2 Second point to be freed
 * @param pt3 Third point to be freed
 * @param msg Message string to be printed in stderr
 * @return void -- has no return value
 *
 * @details Method checks if pt1/pt2/pt3 is NULL (freed already). If not, it frees each of the point pointers. 
 * Afterwards, it prints an error message to stderr with the message provided in msg, as well 
 * as the strerror(errno) result. After that, the programme exits with EXIT_FAILURE.
 **/
static void error_exit(point *pt1, point *pt2, point *pt3, char *msg);

/**
 * Main programme method
 * @brief Entry point of cpair. Programme is implemented in its entirety here
 * @param argc Number of arguments in programme
 * @param argv Arguments (including optional arguments) in programme
 * @return EXIT_SUCCESS (0) on success, EXIT_FAILURE (1) if programme fails.
 *
 * @details
 **/
int main(int argc, char *argv[])
{
    PROGRAMME_NAME = argv[0];
    if (argc != 1) {
        // More than one argument provided -- not allowed
        fprintf(stderr, "Too many arguments!");
        exit(EXIT_FAILURE);
    }

    int pt_size[3] = {8, 4, 4}; // Max capacity of points, initialised at 8 in total, 4 per child.
    int pt_count[3] = {0, 0, 0}; // Counter of points, total, child 1, child 2

    // Initial memory allocation for point collector pointers
    point *points_all = malloc(sizeof(point) * pt_size[0]);
    point *points_c1 = malloc(sizeof(point) * pt_size[1]);
    point *points_c2 = malloc(sizeof(point) * pt_size[2]);
    if (points_all == NULL || points_c1 == NULL || points_c2 == NULL) {
        error_exit(points_all, points_c1, points_c2, "malloc.");
    }

    float x_mean = 0;
    ssize_t result;
    char *line = NULL;
    size_t n = 0;

    // Reading in points from stdin
    while ((result = getline(&line, &n, stdin)) != -1) {
        regex_t regex;

        // Compiling regular expression
        if (regcomp(&regex, "^[-+]?[0-9]*.?[0-9]+ [-+]?[0-9]*.?[0-9]+(\n)?$", REG_EXTENDED) != 0) {
            line = NULL;
            error_exit(points_all, points_c1, points_c2, "Parsing input.");
        }
        
        // Comparing regex against line just read in. If match, continue, else failure.
        if (regexec(&regex, line, (size_t)0, NULL, 0) != 0) {
            regfree(&regex);
            line = NULL;
            error_exit(points_all, points_c1, points_c2, "Parsing input.");
        }
        regfree(&regex); // Free memory for regex

        // Convert string to point
        point p;
        line = strtok(line, " "); // Divide into tokens
        p.x = strtof(line, NULL); // Convert string to float (x-coordinate)
        line = strtok(NULL, " "); // Go to next token
        p.y = strtof(line, NULL); // Convert string to float (y-coordinate)

        pt_count[0]++;

        // Making points_all bigger (realloc), all points won't fit in current one
        if (pt_count[0] >= pt_size[0]) {
            pt_size[0] *= 2;
            if ((points_all = realloc(points_all, (sizeof(point) * pt_size[0]))) == NULL) {
                line = NULL;
                error_exit(points_all, points_c1, points_c2, "realloc.");
            }
        }

        // Add new point to pointer collection and add to x_mean
        points_all[pt_count[0] - 1] = p;
        x_mean += points_all[pt_count[0] - 1].x;
    }

    line = NULL;
    x_mean /= pt_count[0]; // Up until here, x_mean was just the sum of all the x-coordinates, we need to divide it by the number of numbers to get the actual mean.
    
    if (pt_count[0] <= 1) { 
        // One or no points -- End programme.
        if (points_all != NULL) {
            free(points_all);
        }
        if (points_c1 != NULL) {
            free(points_c1);
        }
        if (points_c2 != NULL) {
            free(points_c2);
        }
        exit(EXIT_SUCCESS);
    } else if (pt_count[0] == 2) { 
        // Two points -- Both are closest to each other; print and end programme.
        fprintf(stdout, "%f %f\n%f %f\n", points_all[0].x, points_all[0].y, points_all[1].x, points_all[1].y);
        fflush(stdout);

        if (points_all != NULL) {
            free(points_all);
        }
        if (points_c1 != NULL) {
            free(points_c1);
        }
        if (points_c2 != NULL) {
            free(points_c2);
        }
        exit(EXIT_SUCCESS);
    } else { 
        // Three or more points -- Fork
        pid_t pid_child1, pid_child2;
        int pipes[4][2];

        // Pipe setup
        if (pipe(pipes[c1r]) == -1) {
            // Pipe creation failed -- Close all created pipes so far and exit
            error_exit(points_all, points_c1, points_c2, "Pipe setup.");
        }
        if (pipe(pipes[c1w]) == -1) {
            // Pipe creation failed -- Close all created pipes so far and exit
            close(pipes[c1r][r]);
            close(pipes[c1r][w]);
            error_exit(points_all, points_c1, points_c2, "Pipe setup.");
        }
        if (pipe(pipes[c2r]) == -1) {
            // Pipe creation failed -- Close all created pipes so far and exit
            close(pipes[c1r][r]);
            close(pipes[c1r][w]);
            close(pipes[c1w][r]);
            close(pipes[c1w][w]);
            error_exit(points_all, points_c1, points_c2, "Pipe setup.");
        }
        if (pipe(pipes[c2w]) == -1) {
            // Pipe creation failed -- Close all created pipes so far and exit
            close(pipes[c1r][r]);
            close(pipes[c1r][w]);
            close(pipes[c1w][r]);
            close(pipes[c1w][w]);
            close(pipes[c2r][r]);
            close(pipes[c2r][w]);
            error_exit(points_all, points_c1, points_c2, "Pipe setup.");
        }

        switch (pid_child1 = fork()) {
            case -1:
                // Fork failed -- Close all pipes and exit
                close(pipes[c1r][r]);
                close(pipes[c1r][w]);
                close(pipes[c1w][r]);
                close(pipes[c1w][w]);
                close(pipes[c2r][r]);
                close(pipes[c2r][w]);
                close(pipes[c2w][r]);
                close(pipes[c2w][w]);
                error_exit(points_all, points_c1, points_c2, "fork.");
                break;
            case 0:
                // Child 1

                // Closing pipes: Close child 2 pipes
                if (close(pipes[c2r][r]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    close(pipes[c1w][r]);
                    close(pipes[c1w][w]);
                    close(pipes[c2r][w]);
                    close(pipes[c2w][r]);
                    close(pipes[c2w][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                if (close(pipes[c2r][w]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    close(pipes[c1w][r]);
                    close(pipes[c1w][w]);
                    close(pipes[c2w][r]);
                    close(pipes[c2w][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                if (close(pipes[c2w][r]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    close(pipes[c1w][r]);
                    close(pipes[c1w][w]);
                    close(pipes[c2w][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                if (close(pipes[c2w][w]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    close(pipes[c1w][r]);
                    close(pipes[c1w][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }

                // Close reading end of writing pipe and writing end of reading pipe
                if (close(pipes[c1w][r]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    close(pipes[c1w][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                if (close(pipes[c1r][w]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                
                // Redirect stdin/stdout: Duplicate file descriptors
                if ((dup2(pipes[c1r][r], STDIN_FILENO)) < 0) {
                    // Duplicating file descriptors failed -- Close all open pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    error_exit(points_all, points_c1, points_c2, "dup2.");
                }
                if ((dup2(pipes[c1w][w], STDOUT_FILENO)) < 0) {
                    // Duplicating file descriptors failed -- Close all open pipes and exit
                    close(pipes[c1r][r]);
                    close(pipes[c1r][w]);
                    error_exit(points_all, points_c1, points_c2, "dup2.");
                }

                // We can close old file descriptor after duplication
                if (close(pipes[c1r][r]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    close(pipes[c1r][w]);
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }
                if (close(pipes[c1w][w]) != 0) {
                    // Closing pipe end failed -- Close all remaining pipes and exit
                    error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                }

                // Execute programme in child 1
                if (execlp(PROGRAMME_NAME, PROGRAMME_NAME, NULL) == -1) {
                    error_exit(points_all, points_c1, points_c2, "execlp.");
                }
                break;
            default:
                // Parent
                switch (pid_child2 = fork()) {
                    case -1:
                        // Second fork failed
                        close(pipes[c1r][r]);
                        close(pipes[c1r][w]);
                        close(pipes[c1w][r]);
                        close(pipes[c1w][w]);
                        close(pipes[c2r][r]);
                        close(pipes[c2r][w]);
                        close(pipes[c2w][r]);
                        close(pipes[c2w][w]);
                        error_exit(points_all, points_c1, points_c2, "fork.");
                        break;
                    case 0:
                        // Child 2

                        // Closing pipes: Close child 1 pipes
                        if (close(pipes[c1r][r]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1r][w]);
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        if (close(pipes[c1r][w]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        if (close(pipes[c1w][r]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1w][w]);
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        if (close(pipes[c1w][w]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }

                        // Close reading end of writing pipe and writing end of reading pipe
                        if (close(pipes[c2w][r]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        if (close(pipes[c2r][w]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c2r][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        
                        // Redirect stdin/stdout: Duplicate file descriptors
                        if ((dup2(pipes[c2r][r], STDIN_FILENO)) < 0) {
                            // Duplicating file descriptors failed -- Close all open pipes and exit
                            close(pipes[c2r][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "dup2.");
                        }
                        if ((dup2(pipes[c2w][w], STDOUT_FILENO)) < 0) {
                            // Duplicating file descriptors failed -- Close all open pipes and exit
                            close(pipes[c2r][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "dup2.");
                        }

                        // We can close old file descriptor after duplication
                        if (close(pipes[c2r][r]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }
                        if (close(pipes[c2w][w]) != 0) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }

                        // Execute programme in child 2
                        if (execlp(PROGRAMME_NAME, PROGRAMME_NAME, NULL) == -1) {
                            error_exit(points_all, points_c1, points_c2, "execlp.");
                        }
                        break;
                    default:
                        // Parent
                        // Close ends of pipes that are for children's use
                        if (close(pipes[c1r][r]) == -1) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1r][w]);
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing parent pipes.");
                        }
                        if (close(pipes[c1w][w]) == -1) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][r]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing parent pipes.");
                        }
                        if (close(pipes[c2r][r]) == -1) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            close(pipes[c2w][w]);
                            error_exit(points_all, points_c1, points_c2, "Closing parent pipes.");
                        }
                        if (close(pipes[c2w][w]) == -1) {
                            // Closing pipe end failed -- Close all remaining pipes and exit
                            close(pipes[c1w][r]);
                            close(pipes[c1w][w]);
                            close(pipes[c2r][w]);
                            close(pipes[c2w][r]);
                            error_exit(points_all, points_c1, points_c2, "Closing parent pipes.");
                        }

                        // Open streams to write to pipes
                        FILE *fc1 = fdopen(pipes[c1r][w], "a");
                        FILE *fc2 = fdopen(pipes[c2r][w], "a");

                        for (size_t i = 0; i < pt_count[0]; i++) {
                            point current = points_all[i];
                            int needed_space = (snprintf(NULL, 0, "%f %f\n", current.x, current.y)) + 1; // Calculate space for buffer
                            char *buffer = malloc(needed_space); // Allocate memory
                            if (buffer == NULL) {
                                // malloc failed
                                fclose(fc1);
                                fclose(fc2);
                                error_exit(points_all, points_c1, points_c2, "malloc.");
                            }
                            snprintf(buffer, needed_space, "%f %f\n", current.x, current.y); // Place point in buffer as string

                            if (current.x <= x_mean) {
                                // Below or equal to average -- Pass to first child
                                pt_count[1]++; // Increment counter for first child

                                if (pt_count[1] >= pt_size[1]) {
                                    // Point pointer for child 1 too small -- realloc to make it bigger
                                    pt_size[1] *= 2;
                                    if ((points_c1 = realloc(points_c1, (sizeof(point) * pt_size[1]))) == NULL) {
                                        free(buffer);
                                        fclose(fc1);
                                        fclose(fc2);
                                        error_exit(points_all, points_c1, points_c2, "realloc.");
                                    }
                                }

                                if (fprintf(fc1, "%s", buffer) < 0) {
                                    // Writing into pipe failed
                                    free(buffer);
                                    fclose(fc1);
                                    fclose(fc2);
                                    error_exit(points_all, points_c1, points_c2, "Writing to children.");
                                }

                                points_c1[pt_count[1] - 1] = current;
                            } else {
                                // Above average -- Pass to second child
                                pt_count[2]++; // Increment counter for second child

                                if (pt_count[2] >= pt_size[2]) {
                                    // Point pointer for child 2 too small -- realloc to make it bigger
                                    pt_size[2] *= 2;
                                    if ((points_c2 = realloc(points_c2, (sizeof(point) * pt_size[2]))) == NULL) {
                                        free(buffer);
                                        fclose(fc1);
                                        fclose(fc2);
                                        error_exit(points_all, points_c1, points_c2, "realloc.");
                                    }
                                }

                                if (fprintf(fc2, "%s", buffer) < 0) {
                                    // Writing into pipe failed
                                    free(buffer);
                                    fclose(fc1);
                                    fclose(fc2);
                                    error_exit(points_all, points_c1, points_c2, "Writing to children.");
                                }

                                points_c2[pt_count[2] - 1] = current;
                            }
                            free(buffer);
                        }

                        // Check for errors in pipe stream
                        if (ferror(fc1) || ferror(fc2)) {
                            error_exit(points_all, points_c1, points_c2, "Error in stream.");
                        }

                        // Close pipes parent writes into
                        if (fclose(fc1) < 0 || fclose(fc2) < 0) {
                            error_exit(points_all, points_c1, points_c2, "Closing pipes.");
                        }

                        int status;
                        
                        // Wait for first child to terminate
                        while (waitpid(pid_child1, &status, 0) < 0) {
                            if (errno == EINTR) {
                                // Waiting interrupted
                                continue;
                            }
                            error_exit(points_all, points_c1, points_c2, "Wait interrupted.");
                        }

                        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
                            // First child didn't return EXIT_SUCCESS
                            error_exit(points_all, points_c1, points_c2, "Child failed.");
                        }

                        // Wait for second child to terminate
                        while (waitpid(pid_child2, &status, 0) < 0) {
                            if (errno == EINTR) {
                                // Waiting interrupted
                                continue;
                            }
                            error_exit(points_all, points_c1, points_c2, "Wait interrupted.");
                        }

                        if (WEXITSTATUS(status) != EXIT_SUCCESS)  {
                            // Second child did not return EXIT_SUCCESS
                            error_exit(points_all, points_c1, points_c2, "Child failed.");
                        }

                        // Initialise point pairs to hold candidates for closest pairs
                        pt_pair ptpair1 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .dist = FLT_MAX};
                        pt_pair ptpair2 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .dist = FLT_MAX};
                        pt_pair ptpair3 = {.p1.x = FLT_MAX, .p1.y = FLT_MAX, .p2.x = FLT_MIN, .p2.y = FLT_MIN, .dist = FLT_MAX};

                        // Open stream to read from child 1
                        FILE *fchild = fdopen(pipes[c1w][r], "r");
                        if (fchild == NULL) {
                            error_exit(points_all, points_c1, points_c2, "fdopen.");
                        }

                        line = NULL;
                        size_t n = 0;
                        ssize_t result;

                        if (pt_count[1] >= 1) {
                            // Point 1 from child 1
                            if ((result = getline(&line, &n, fchild)) != -1) {
                                // Parse what was read in from child
                                point p;
                                line = strtok(line, " ");
                                p.x = strtof(line, NULL);
                                line = strtok(NULL, " ");
                                p.y = strtof(line, NULL);
                        
                                ptpair1.p1.x = p.x;
                                ptpair1.p1.y = p.y;
                            }
                        }
                        if (pt_count[1] >= 2) {
                            // Point 2 from child 2
                            if ((result = getline(&line, &n, fchild)) != -1) {
                                // Parse what was read in from child
                                point p;
                                line = strtok(line, " ");
                                p.x = strtof(line, NULL);
                                line = strtok(NULL, " ");
                                p.y = strtof(line, NULL);

                                ptpair1.p2.x = p.x;
                                ptpair1.p2.y = p.y;
                            }
                            ptpair1.dist = sqrtf((powf(ptpair1.p2.x - ptpair1.p1.x, 2) + powf(ptpair1.p2.y - ptpair1.p1.y, 2)));
                        }
                        line = NULL;

                        // Check for errors in stream from child
                        if (ferror(fchild) < 0) {
                            error_exit(points_all, points_c1, points_c2, "Error in stream.");
                        }
                        // Close stream from child
                        if (fclose(fchild) < 0) {
                            error_exit(points_all, points_c1, points_c2, "fclose.");
                        }

                        // Open stream from child 2
                        fchild = NULL;
                        fchild = fdopen(pipes[c2w][r], "r");
                        if (fchild == NULL) {
                            error_exit(points_all, points_c1, points_c2, "fdopen.");
                        }

                        line = NULL;
                        n = 0;
                        if (pt_count[2] >= 1) {
                            // Point 1 from child 2
                            if ((result = getline(&line, &n, fchild)) != -1) {
                                // Parse what was read in from child
                                point p;
                                line = strtok(line, " ");
                                p.x = strtof(line, NULL);
                                line = strtok(NULL, " ");
                                p.y = strtof(line, NULL);
                        
                                ptpair2.p1.x = p.x;
                                ptpair2.p1.y = p.y;
                            }
                        }
                        if (pt_count[2] >= 2) {
                            // Point 2 from child 2
                            if ((result = getline(&line, &n, fchild)) != -1) {
                                // Parse what was read in from child
                                point p;
                                line = strtok(line, " ");
                                p.x = strtof(line, NULL);
                                line = strtok(NULL, " ");
                                p.y = strtof(line, NULL);

                                ptpair2.p2.x = p.x;
                                ptpair2.p2.y = p.y;
                            }
                            ptpair2.dist = sqrtf((powf(ptpair2.p2.x - ptpair2.p1.x, 2) + powf(ptpair2.p2.y - ptpair2.p1.y, 2)));
                        }
                        line = NULL;

                        // Check for errors in stream from child
                        if (ferror(fchild) < 0) {
                            error_exit(points_all, points_c1, points_c2, "Error in stream.");
                        }
                        // Close stream from child
                        if (fclose(fchild) < 0) {
                            error_exit(points_all, points_c1, points_c2, "fclose.");
                        }

                        // Check for better (smaller distance) solutions
                        for (size_t i = 0; i < pt_count[1]; i++) {
                            point temp1 = points_c1[i];
                            for (size_t j = 0; j < pt_count[2]; j++) {
                                point temp2 = points_c2[j];
                                float new_dist = sqrtf((powf(temp2.x - temp1.x, 2) + powf(temp2.y - temp1.y, 2)));
                                if (new_dist <= ptpair3.dist) {
                                    ptpair3.p1 = temp1;
                                    ptpair3.p2 = temp2;
                                    ptpair3.dist = new_dist;
                                }
                            }
                        }

                        // We can use stdout here as stdout is redirected to relevant pipe in children.
                        if ((ptpair1.dist < ptpair2.dist) && (ptpair1.dist < ptpair3.dist)) {
                            // ptpair1 is smallest of 3 -- ptpair1 is solution
                            fprintf(stdout, "%f %f\n%f %f\n", ptpair1.p1.x, ptpair1.p1.y, ptpair1.p2.x, ptpair1.p2.y);
                        } else {
                            // ptpair1 is not the smallest of 3 -- ptpair2 or ptpair3 is solution
                            if (ptpair2.dist < ptpair3.dist) {
                                // ptpair2 is smaller than ptpair3 -- ptpair2 is solution
                                fprintf(stdout, "%f %f\n%f %f\n", ptpair2.p1.x, ptpair2.p1.y, ptpair2.p2.x, ptpair2.p2.y);
                            } else {
                                // ptpair3 is smaller than ptpair2 -- ptpair3 is solution
                                fprintf(stdout, "%f %f\n%f %f\n", ptpair3.p1.x, ptpair3.p1.y, ptpair3.p2.x, ptpair3.p2.y);
                            }
                        }
                        fflush(stdout);
                    break;
                }
            break;
        }
    }
    // Free point pointers and exit successfully
    if (points_all != NULL) {
        free(points_all);
    }
    if (points_c1 != NULL) {
        free(points_c1);
    }
    if (points_c2 != NULL) {
        free(points_c2);
    }
    exit(EXIT_SUCCESS);
}

static void error_exit(point *pt1, point *pt2, point *pt3, char *msg) {
    // Free point pointers
    if (pt1 != NULL) {
        free(pt1);
    }
    if (pt2 != NULL) {
        free(pt2);
    }
    if (pt3 != NULL) {
        free(pt3);
    }

    // Print appropriate error message
    if (errno == 0) {
        // strerror(0) is "Success.", not appropriate in error message
        fprintf(stderr, "Error: %s", msg);
    } else {
        fprintf(stderr, "Error: %s %s", msg, strerror(errno));
    }

    exit(EXIT_FAILURE);
}