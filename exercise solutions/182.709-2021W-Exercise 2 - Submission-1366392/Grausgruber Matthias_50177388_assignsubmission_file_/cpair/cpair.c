/** Closest Pair of Points
 *	@file cpair.c
 *  @author Matthias Grausgruber <e00525708@student.tuwien.ac.at>
 *  @date 11.12.2021
 *  
 *  @brief Recursive algorithm for calculating the closest pair of points.
 *  
 **/

#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define LEFT (0)
#define RIGHT (1)
#define RD (0)
#define WR (1)

#define LR (0)
#define LW (1)
#define RR (2)
#define RW (3)

#define PIPE_C (4)

static char *pgm_name; /**< The program name. */
struct point { float x,y; }; /**< Struct for storing the coordinates of a point. */

/**
 * Print usage notice to shell and exit.
 * @brief This function is called if a false input is discovered during parsing.
 * @details global variables: pgm_name
 **/
void usage() {
    fprintf (stderr, "Usage: %s [stdin]\n", pgm_name);
    exit (EXIT_FAILURE);
}

/**
 * Calculate distance between two points.
 * @brief This function gets two points (struct point) and calculates the distance between them (c = sqrt(a²+b²)).
 * @details return values: distance (float)
 **/
float distance (struct point p1, struct point p2) {
    return sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2));
}

/**
 * Calculates the partial result.
 * @brief This function gets two partial results from the recursive algorithm (p1 and p2) and the number of points in the result array.
 *        p3 is for storing the result. It is either one of the two pair of points (p1 or p2) or a pair between those four (or less) points.
 * @details return values: distance (float) -> returns FLT_MAX if anything went wrong.
 **/
float calcP3 (struct point p1[], struct point p2[], int count1, int count2, struct point p3[]) {
    
    /** Calculates the distance between all points on the left side (1 or 2) and the right side (1 or 2). 
     *  The smallest distance is saved for further processing.
     **/
    float res = FLT_MAX, dist = 0.0;
    for (int i=0; i<count1; i++) {
        for (int j=0; j<count2; j++) {
            if ((dist = distance (p1[i], p2[j])) < res) {
                res = dist;
                p3[0] = p1[i];
                p3[1] = p2[j];
            }
        }
    }
    /** Checks if the distance between the two points on the left/right side
     *  is smaller then the part solution from before. 
     **/
    if (count1 == 2) {
        if ((dist = distance (p1[0], p1[1])) < res) {
                res = dist;
                p3[0] = p1[0];
                p3[1] = p1[1];
            }
    }
    if (count2 == 2) {
        if ((dist = distance (p2[0], p2[1])) < res) {
                res = dist;
                p3[0] = p2[0];
                p3[1] = p2[1];
            }
    }
    return res;
}

/**
 * Processes the input in an array of x and y coordinates.
 * @brief This function reads the points from stdin (either external input or from the redirected pipe) and stores them in an array.
 *        If the input is not as expected, the program will be terminated. The needed space is dynamicly allocated.
 * @details return values: array of points (struct point)
 **/
struct point* parseInput (FILE* in, int *c) {
    
    int count = 0, p_size = 2;
    struct point *p;
    p = malloc (sizeof(struct point) * p_size);
    if (p == NULL) exit(EXIT_FAILURE);
    char *line = NULL, *p2_ptr, *endptr;
    size_t len = 0;
    ssize_t nread;

    /** Line by line is read and converted to floats. If needed, the array is reallocated. */
    while ((nread = getline (&line, &len, in)) != -1) {
        float in1 = strtof (line, &p2_ptr);
        float in2 = strtof (p2_ptr, &endptr);
        /** Input has to be two floats and nothing more (end of line). */
        if (strncmp(endptr, "\n\r", 1) != 0) {
            free (line);
            free (p);
            usage();
        }
        else {
            if (count > p_size) {
                p_size *= 2;
                p = realloc (p, sizeof(p)* p_size);
            }
            p[count].x = in1;
            p[count++].y = in2; 
        }
    }
    free (line);
    *c = count;
    return p;
}

/**
 * Writes the x and y coordinates to stdout.
 * @brief The function gets the filedescriptor (stdout) and the coordinates in a struct point.
 * @details return values: number of bytes written by fprintf()
 **/
int printCoord (FILE* fd, struct point p) { return fprintf (fd, "%f %f\n", p.x, p.y); }

/**
 * Calculates the arithmetic mean of the set of points (x-axis).
 * @brief The function gets the full array of points and the count.
 *        It sums up the numbers (x-coordinate) and divides by the count of points.
 * @details return values: arithmetic mean (float)
 **/
float arithM (struct point *p, int count) {
    float mean = 0.0;
    for(int i = 0; i < count; i++) mean += p[i].x;
    return (mean/count);
}

/**
 * Exit handling in case of an error.
 * @brief If an error occurs, this function prints an error message,
 *        frees the allocated memory for the array of points and terminates the program.
 **/
void exitPoints (struct point *p, int line) {
    fprintf (stderr, "%s: error on line %i! EXIT\n", pgm_name, line);
    free (p);
    exit (EXIT_FAILURE);
}

/**
 * Main function. Handles the communication between parent and childs.
 * @brief The main function starts the input parsing and decides what to do next.
 *        Either end the program (child or parent), write to stdout or fork.
 * @details return values: EXIT_SUCCESS
 **/
int main (int argc, char **argv)
{   
    pgm_name = argv [0];
    if (argc > 1) usage();

    int count = 0;
    struct point *p = parseInput (stdin, &count);
    #ifdef DEBUG
        fprintf (stderr, "ppid: %i, pid: %i, points: %i\n", getppid(), getpid(), count);
    #endif

    /** Only two points can be a (part-) solution, but single points (odd number of points in array) 
     *  are needed for calculating P3 (partial or end result).
     **/
    if (count == 1) {
        if (printCoord(stdout, p[0]) < 0) exitPoints (p, __LINE__);
        free (p);
        return EXIT_SUCCESS;
    }
    if (count == 2) {
        if ((printCoord(stdout, p[0]) < 0) || (printCoord(stdout, p[1]) < 0)) exitPoints (p, __LINE__);
        free (p);
        return EXIT_SUCCESS;
    }
    /** If more then two points are present/left, we need to fork the process. */
    else if (count > 2) {
        
        /** Pipes for communication between the processes are needed (redirection of stdin and stdout). */
        int pipes[4][2];
        if ((pipe(pipes[LR]) == -1) || (pipe(pipes[LW]) == -1) || (pipe(pipes[RR]) == -1) || (pipe(pipes[RW]) == -1)) exitPoints (p, __LINE__);

        /** child 1 */
        pid_t fork1 = fork();
        switch (fork1) {
            case -1:
                for (int i=0; i<PIPE_C; i++) {
                    close (pipes[i][RD]);
                    close (pipes[i][WR]);
                }
                exitPoints (p, __LINE__);
                break;
            case 0: // tasks child1 -> redirect stind/stdout
                if (dup2(pipes[LR][WR], STDOUT_FILENO) == -1) exitPoints (p, __LINE__);
                if (dup2(pipes[LW][RD], STDIN_FILENO) == -1) exitPoints (p, __LINE__);
                for (int i=0; i<PIPE_C; i++) {
                    close (pipes[i][RD]);
                    close (pipes[i][WR]);
                }
                execlp (pgm_name, pgm_name, NULL);
                exitPoints (p, __LINE__);
                break;
        }

        /** child 2 */
        pid_t fork2 = fork();
        switch (fork2) {
            case -1:
                for (int i=0; i<PIPE_C; i++) {
                    close (pipes[i][RD]);
                    close (pipes[i][WR]);
                }
                exitPoints (p, __LINE__);
                break;
            case 0: // tasks child2 -> redirect stdin/stdout 
                if (dup2(pipes[RR][WR], STDOUT_FILENO) == -1) exitPoints (p, __LINE__);
                if (dup2(pipes[RW][RD], STDIN_FILENO) == -1) exitPoints (p, __LINE__);
                for (int i=0; i<PIPE_C; i++) {
                    close (pipes[i][RD]);
                    close (pipes[i][WR]);
                }
                execlp (pgm_name, pgm_name, NULL);
                exitPoints (p, __LINE__);
                break;
        }
        close (pipes[LR][1]);
        close (pipes[LW][0]);
        close (pipes[RR][1]);
        close (pipes[RW][0]);

        /** tasks parent: 
         *  Open FDs for reading and writing from/to the pipe.
         **/
        FILE *writeL_fd = fdopen (pipes[LW][WR], "w"), *writeR_fd = fdopen (pipes[RW][WR], "w");
        FILE *readL_fd = fdopen (pipes[LR][RD], "r"), *readR_fd = fdopen (pipes[RR][RD], "r");
        if ((readL_fd == NULL) || (writeL_fd == NULL) || (readR_fd == NULL) || (writeR_fd == NULL)) {
            for (int i=0; i<PIPE_C; i++) {
                close (pipes[i][RD]);
                close (pipes[i][WR]);
            }
            exitPoints (p, __LINE__);
        }

        /** Assign the points to child "on the left" and child "on the right". */
        float xm = arithM (p, count);
        for (int i = 0; i < count; i++) {
            if(p[i].x <= xm) {
                if (printCoord (writeL_fd, p[i]) >= 0) continue;
            } else if (p[i].x > xm) {
                if (printCoord (writeR_fd, p[i]) >= 0) continue;
            }
            /** In case something went wrong writing to stdout. */
            for (int j=0; j<PIPE_C; j++) {
                close (pipes[j][RD]);
                close (pipes[j][WR]);
            }
            exitPoints (p, __LINE__);
        }
        fflush (writeL_fd);
        fclose (writeL_fd);
        fflush (writeR_fd);
        fclose (writeR_fd);

        /** Wait for the childs to finish. */
        int status = 0;
        while (waitpid(fork1, &status, 0) == -1) ;
        while (waitpid(fork2, &status, 0) == -1) ;
        
        /** Partial result left/right child. */
        int count_p1 = 0, count_p2 = 0;
        struct point *p1 = parseInput (readL_fd, &count_p1);
        struct point *p2 = parseInput (readR_fd, &count_p2);

        /** Check if left or right child is better solution or if there is a smaller distance between both sets. */
        struct point p3[2];
        if (calcP3 (p1, p2, count_p1, count_p2, p3) == FLT_MAX) fprintf (stderr, "%s: error on line %i!\n", pgm_name, __LINE__);

        /** Print (part) solution to stdout. End of processing. */
        printCoord(stdout, p3[0]);
        printCoord(stdout, p3[1]);
        
        fclose (readL_fd);
        fclose (readR_fd);
        for (int j=0; j<PIPE_C; j++) {
            close (pipes[j][RD]);
            close (pipes[j][WR]);
        }
    }
    free (p);
    return EXIT_SUCCESS;
}
