/**
 * @file cpair.c
 * @author Josef Rabmer 11911128
 * 
 * @brief reads a list of points from stdin and generates solution to the closest points problem. 
 * 
 * @details the programm reads stdin and parses every line as a point, in the form "x y". If x or y can not be converted to doubles,
 *          the program prints an error and then exits. After EOF is reached in stdin, it calculates the mean of all read points
 *          and recuresively calls itself twice, splitting the input array into an array that contains points with an x value 
 *          greater than the mean and into a different array for the other elements. The program uses pipes to communicate with
 *          the child processes. The arrays are input using an input pipe and the resulting pairs P1 and P2 are read through an
 *          output pipe.
 *          
 *          It then calculates the distance for every point in the first array to every point in the second array. The pair P3 with the
 *          shortest distance is saved. Finally, it selects the pair with the leas distance from P1,P2,P3 and prints it to stdout.
 * 
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "cpair.h"

/*

TODO:

- Test implementation on server
- Documentation

*/ 

char *prog_name;

/**
 * @brief the function reads from the file in and parses every line into a point_t.
 * 
 * @details The function recieves an open filestream. It reads the file stream, line by line until EOF 
 *          is reached. Every line is parsed into a point_t, which needs the format "x y", where x
 *          and y can be parsed into doubles. This is done with the function parse_line.
 *          
 *          Every point that was succesfully parsed is then stored in the buffer buf.
 * 
 * @param in        open filestream, that is read from
 * @param buf       pointer to buffer buf, that stores an array of points
 */
void static read_file(FILE* in, buffer_t* buf);

/**
 * @brief the function writes an array of points into a file.
 * 
 * @details The function recieves a buffer that contains a list of points and writes these into
 *          the file specified by out. After writting all points into the file, the filestream
 *          is flushed.
 * 
 * @param out       open filestream, that is written into
 * @param buf       buffer that contains list of points
 */
void static write_file(FILE* out, buffer_t* buf);

/**
 * @brief function that increases the size of the buffer buf.
 * 
 * @details the function function increases the maximum size of the buffer. 
 *          It increases the max. number of points it can store by 50 and 
 *          allocates the necessary memory.
 * 
 * @param buf       buffer who´s size will be increased
 */
void static increase_buffer_size(buffer_t* buf);

/**
 * @brief the function recieves a string and parses it into an element of type point_t
 * 
 * @details the function expects a string in the form of "x y", where x and y can be 
 *          parsed to doubles. The parsing is done with the function strtod. If
 *          the string can not be parsed, the function returns an error message and 
 *          exits. Else it stores x and y in a point_t p and returns it.
 * 
 * @param str           input string, that is parsed
 * @return point_t      the parsed result is stored in a point_t.
 */
point_t static parse_line(char *str);

/**
 * @brief the function returns the distance between point p1 and p2
 * 
 * @details the function calculates the euclidian distance between 
 *          the two input points. It does this using functions from
 *          the Math.h libary. It then returns the result.
 * 
 * @param p1        first point
 * @param p2        second point
 * @return double   distance between point 1 and point 2
 */
double static distance(point_t p1, point_t p2);

/**
 * @brief the function returns the minimum of values a,b and c
 * 
 * @details the function compares three double values with
 *          fmin (Math.h) and determines the minimum of
 *          the values. If parameters have the same
 *          value, one of them is returned.
 * 
 * @param a         first value of type double
 * @param b         second value of type double
 * @param c         third value of type double
 * @return double   minimum of values a,b and c
 */
double static min(double a, double b, double c);

// TODO: Document functions if i end up doing the bonus example
void static draw_tree(FILE* out, buffer_t* buf);
void static read_prev_trees(FILE* in, char* tree);


int main(int argc, char *argv[]){

    /* --------------------------------------- Input Handling --------------------------------------- */

    prog_name = argv[0];
    int c;
    // Parse options
    while ((c = getopt(argc, argv, "")) != -1)
    {
        switch (c)
        {
            default:
                usage();
        }
    }

    // Assert that no positional arguments are specified
    if (optind != argc)
    {
        usage();
    }


    /* --------------------------------------- Read Input from STDIN --------------------------------------- */

    // Initialise Buffer
    buffer_t buf;
    buf.current_index = 0;
    buf.max_size = INITIAL_SIZE_BUFFER;
    buf.points = malloc(sizeof(point_t) * INITIAL_SIZE_BUFFER);

    if (buf.points == NULL)
    {
        fprintf(stderr, "[%s] malloc failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Read all points from stdin
    read_file(stdin, &buf);
    
    //Flush stdin
    if(fflush(stdin) == EOF){
        fprintf(stderr, "[%s] fflush failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* --------------------------------------- Compute closest pair of points --------------------------------------- */

    if (buf.current_index == 2)
    {
        write_file(stdout, &buf);

    } else if (buf.current_index > 2)
    {
        // Calculate arithmetic mean of x coordinates
        double mean_x, sum_x = 0;
        int i = 0;

        while (i < buf.current_index)
        {
            sum_x += buf.points[i++].x;
        }

        mean_x = sum_x / buf.current_index;

        // Divivde the buffer into two seperate ones
        buffer_t buf_child_1;
        buf_child_1.current_index = 0;
        buf_child_1.max_size = buf.max_size;
        buf_child_1.points = malloc(sizeof(point_t) * buf_child_1.max_size);
        
        buffer_t buf_child_2;
        buf_child_2.current_index = 0;
        buf_child_2.max_size = buf.max_size;
        buf_child_2.points = malloc(sizeof(point_t) * buf_child_2.max_size);
        
        i = 0;
        int flag = 1;
        while (i < buf.current_index)
        {   
            // Avoid endless rekursion by assigning point to buffers in alternating manner if they are equal to the mean
            if (buf.points[i].x == mean_x)
            {
                if (flag == 1)
                {
                    buf_child_1.points[buf_child_1.current_index++] = buf.points[i];
                } else {
                    buf_child_2.points[buf_child_2.current_index++] = buf.points[i];
                }

                flag *= -1;

            } else if (buf.points[i].x < mean_x)
            {   
                buf_child_1.points[buf_child_1.current_index++] = buf.points[i];
            } else {
                buf_child_2.points[buf_child_2.current_index++] = buf.points[i];
            }

            i++;
        }

        // Free memory for buffer buf
        free(buf.points);

        // Create two unnamed pipes per child process to redirect stdin and stdout
        int pipe_child_1_parent_write_fd[2];
        int pipe_child_1_child_read_fd[2];
        int pipe_child_2_parent_write_fd[2];
        int pipe_child_2_child_read_fd[2];

        if(pipe(pipe_child_1_parent_write_fd) == -1){
            fprintf(stderr, "[%s] pipe failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE); 
        }

        if(pipe(pipe_child_1_child_read_fd) == -1){
            fprintf(stderr, "[%s] pipe failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE); 
        }

        if(pipe(pipe_child_2_parent_write_fd) == -1){
            fprintf(stderr, "[%s] pipe failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE); 
        }

        if(pipe(pipe_child_2_child_read_fd) == -1){
            fprintf(stderr, "[%s] pipe failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE); 
        } 

        // Use fork and execlp to recursively execute program in two child processes
        // Configure pipes accordingly:
        //                              input pipes redirect messages to stdin of child
        //                              output pipes redirect messages to stdout from parent
        
        pid_t pid_smaller_equal = fork();
        
        switch (pid_smaller_equal) {
            case -1:
                fprintf(stderr, "[%s] fork for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            case 0: // child tasks (child reads lines from parent and prints to stdout)

                // Configure input pipe: 
                // Close unused write end
                if(close(pipe_child_1_parent_write_fd[1]) == -1){
                    fprintf(stderr, "[%s] close for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Set read end to stdin
                if(dup2(pipe_child_1_parent_write_fd[0], STDIN_FILENO) == -1){
                    fprintf(stderr, "[%s] dup2 for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Close old fd
                if(close(pipe_child_1_parent_write_fd[0]) == -1){
                    fprintf(stderr, "[%s] close for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Configure output pipe:
                // Close unused read end
                if(close(pipe_child_1_child_read_fd[0]) == -1){
                    fprintf(stderr, "[%s] close for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Set write end to stdout
                if(dup2(pipe_child_1_child_read_fd[1], STDOUT_FILENO) == -1){
                    fprintf(stderr, "[%s] dup2 for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Close old fd
                if(close(pipe_child_1_child_read_fd[1]) == -1){
                    fprintf(stderr, "[%s] close for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Recursively execute program
                execlp(prog_name, prog_name, (char *) 0);
                
                // Should not reach this line
                fprintf(stderr, "[%s] execlp for child smaller_equal failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);

            default: // parent tasks (parents writes lines to child)
                
                // Configure input pipe: 
                // Close unused read end
                if(close(pipe_child_1_parent_write_fd[0]) == -1){
                    fprintf(stderr, "[%s] close of parent for child smaller_equal failed (195): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Configure output pipe:
                // Close unused write end
                if(close(pipe_child_1_child_read_fd[1]) == -1){
                    fprintf(stderr, "[%s] close of parent for child smaller_equal failed (202): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Write points, which are smaller or equal than mean to child
                FILE *out;
                // Open file
                if ((out = fdopen(pipe_child_1_parent_write_fd[1], "w")) == NULL){
                    fprintf(stderr, "[%s] fdopen of parent for child smaller_equal failed (210): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Write to child
                write_file(out, &buf_child_1);

                // Close file and fd
                if (fclose(out) == EOF)
                {
                    fprintf(stderr, "[%s] fclose of parent for child smaller_equal failed (220): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                break;
        }

        
        pid_t pid_greater = fork();
        
        switch (pid_greater) {
            case -1:
                fprintf(stderr, "[%s] fork for child greater failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            case 0: // child tasks (child reads lines from parent and prints to stdout)
                
                // Configure input pipe: 
                // Close unused write end
                if(close(pipe_child_2_parent_write_fd[1]) == -1){
                    fprintf(stderr, "[%s] close for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Set read end to stdin
                if(dup2(pipe_child_2_parent_write_fd[0], STDIN_FILENO) == -1){
                    fprintf(stderr, "[%s] dup2 for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Close old fd
                if(close(pipe_child_2_parent_write_fd[0]) == -1){
                    fprintf(stderr, "[%s] close for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Configure output pipe:
                // Close unused read end
                if(close(pipe_child_2_child_read_fd[0]) == -1){
                    fprintf(stderr, "[%s] close for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Set write end to stdout
                if(dup2(pipe_child_2_child_read_fd[1], STDOUT_FILENO) == -1){
                    fprintf(stderr, "[%s] dup2 for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                // Close old fd
                if(close(pipe_child_2_child_read_fd[1]) == -1){
                    fprintf(stderr, "[%s] close for child greater failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Recursively execute program
                execlp(prog_name, prog_name, (char *) 0);
                
                // Should not reach this line
                fprintf(stderr, "[%s] execlp for child greater failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);

            default: // parent tasks
                
                // Configure input pipe: 
                // Close unused read end
                if(close(pipe_child_2_parent_write_fd[0]) == -1){
                    fprintf(stderr, "[%s] close of parent for child greater failed (195): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Configure output pipe:
                // Close unused write end
                if(close(pipe_child_2_child_read_fd[1]) == -1){
                    fprintf(stderr, "[%s] close of parent for child greater failed (202): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Write points, which are smaller or equal than mean to child
                FILE *out;
                // Open file
                if ((out = fdopen(pipe_child_2_parent_write_fd[1], "w")) == NULL){
                    fprintf(stderr, "[%s] fdopen of parent for child greater failed (210): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // Write to child
                write_file(out, &buf_child_2);

                // Close file and fd
                if (fclose(out) == EOF)
                {
                    fprintf(stderr, "[%s] fclose of parent for child greater failed (220): %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                break;

                break;
        }

        // While Children are executing their tasks, calculate min. distance 
        // For each point in the list of points with a smaller mean x_n, calculate the distance to each point in the other list
        // Remember the pair with the smallest distance
        int l = 0, k = 0;
        point_t pair3[2] = {buf_child_1.points[0], buf_child_2.points[0]};
        double dist = distance(pair3[0], pair3[1]);

        point_t tmp_1, tmp_2;
        double tmp_dist;

        while (l < buf_child_1.current_index)
        {   
            tmp_1 = buf_child_1.points[l];

            while (k < buf_child_2.current_index)
            {   
                tmp_2 = buf_child_2.points[k];
                tmp_dist = distance(tmp_1, tmp_2);
                
                if(tmp_dist < dist){
                    pair3[0] = tmp_1;
                    pair3[1] = tmp_2;
                    dist = tmp_dist;
                }
                k++;
            }
            l++;
        }

        // Wait for children to terminate
        while (1) {
            int status;
            pid_t done = wait(&status);

            // wait returns error
            if (done == -1) {
                if (errno == ECHILD){
                    break; // no more child processes
                } else {
                    fprintf(stderr, "[%s] wait failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                } 
            } else {
                if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) { // Child terminated with error
                    fprintf(stderr, "[%s] Child process could not terminate succesfully: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        // Read output from children
        buffer_t pbuf;
        pbuf.current_index = 0;
        pbuf.max_size = INITIAL_SIZE_BUFFER;
        pbuf.points = malloc(sizeof(point_t) * pbuf.max_size);

        // Read from pipe smaller equals (P1) and read from pipe greater (P2)
        FILE *in_child1, *in_child2;

        // Open file
        if ((in_child1 = fdopen(pipe_child_1_child_read_fd[0], "r")) == NULL){
            fprintf(stderr, "[%s] fdopen of parent for child smaller equals failed (347): %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
         
        // Open file
        if ((in_child2 = fdopen(pipe_child_2_child_read_fd[0], "r")) == NULL){
            fprintf(stderr, "[%s] fdopen of parent for child greater failed (367): %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Read pairs of points
        read_file(in_child1, &pbuf);
        read_file(in_child2, &pbuf);    
        
        // Depending on how many points are read from children, return closest pair of points
        int number_of_pairs = pbuf.current_index;
        point_t pair1[2];
        point_t pair2[2];

        switch (number_of_pairs)
        {
        case 0: // Both children did not return any points 
            // This shouldn´t actually happen. If parent has less than or exactly two points, there should not be any children
            // If parent has three points or more, at least one child should return 2 points
            fprintf(stderr, "[%s] Both children did not return any points: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);

        case 2: // One child returned two points.

            // P1 and P2 are the same point. One of them is returned, if point has the min. distance
            pair1[0] = pbuf.points[0];
            pair1[1] = pbuf.points[1];
            pair2[0] = pbuf.points[0];
            pair2[1] = pbuf.points[1];
            break;

        case 4: // Both children returned two points.  

            // Save P1 and P2 in arrays
            pair1[0] = pbuf.points[0];
            pair1[1] = pbuf.points[1];
            pair2[0] = pbuf.points[2];
            pair2[1] = pbuf.points[3];
            break;

        default: // Error: Children returned more than 4 points
            
            fprintf(stderr, "[%s] Children returned more than 4 points: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);

        }
        
        // Empty pbuf
        pbuf.current_index = 0;

        // Compare the distances of the three pairs P1,P2,P3 and write the pair with the shortest distance to stdout
        // Use pbuf as buffer to use the write funktion
        double dist_pair1 = distance(pair1[0], pair1[1]);
        double dist_pair2 = distance(pair2[0], pair2[1]);
        double dist_pair3 = distance(pair3[0], pair3[1]);
        double min_dist = min(dist_pair1, dist_pair2, dist_pair3);

        if (dist_pair1 == min_dist)
        {
            pbuf.points[pbuf.current_index++] = pair1[0];
            pbuf.points[pbuf.current_index++] = pair1[1];

        } else if (dist_pair2 == min_dist)
        {
            pbuf.points[pbuf.current_index++] = pair2[0];
            pbuf.points[pbuf.current_index++] = pair2[1];
        } else 
        {
            pbuf.points[pbuf.current_index++] = pair3[0];
            pbuf.points[pbuf.current_index++] = pair3[1];
        }
        
        write_file(stdout, &pbuf);
        
        /* ---------------------------------------    Print tree      --------------------------------------- */

        
        // TODO: Implement bonus example



        /* --------------------------------------- Clean up resources --------------------------------------- */

        // Buffers
        free(buf_child_1.points);
        free(buf_child_2.points);
        free(pbuf.points);

        // Close file and fd
        if (fclose(in_child1) == EOF){
            fprintf(stderr, "[%s] fclose of parent for child smaller equals failed (358): %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Close file and fd
        if (fclose(in_child2) == EOF){
            fprintf(stderr, "[%s] fclose of parent for child greater failed (375): %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }

    }

    return EXIT_SUCCESS;
}

/* ----------------------------------- Helper function implementations --------------------------------- */

void usage(void)
{
    fprintf(stderr, "Usage: %s \n", prog_name);
    exit(EXIT_FAILURE);
}

void static read_file(FILE *in, buffer_t *buf)
{   
    char line[LINE_SIZE];

    while (fgets(line, LINE_SIZE + 1, in) != NULL)  
    {        
        // Parse input to doubles
        point_t p = parse_line(line);

        // If buffer is full, increase size
        if (buf->current_index == buf->max_size) 
        {
            increase_buffer_size(buf);
        }

        // Store point in buffer
        buf->points[buf->current_index++] = p;
    }
    
}

void static write_file(FILE* out, buffer_t* buf)
{   
    int i = 0;

    while (i < buf->current_index)
    {
        if(fprintf(out, "%lf %lf\n", buf->points[i].x, buf->points[i].y) < 0){
            fprintf(stderr, "[%s] fprintf failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
        i++;
    }

    if(fflush(out) == EOF){
        fprintf(stderr, "[%s] fflush failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
}

void static increase_buffer_size(buffer_t *buf)
{
    buf->max_size += 50;
    point_t* new_array_pointer = realloc(buf->points, sizeof(point_t) * buf->max_size);

    if (new_array_pointer == NULL)
    {
        fprintf(stderr, "[%s] realloc failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    buf->points = new_array_pointer;
    
}

point_t static parse_line(char *str){

    point_t p;
    double val;
    char *endptr;

    // x coordinate
    errno = 0;    /* To distinguish success/failure after call */
    val = strtod(str, &endptr);

    /* Check for various possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        fprintf(stderr, "[%s] strtod failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (endptr == str) {
        fprintf(stderr, "[%s] Invalid format of input: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    p.x = val;

    // y coordinate
    str = endptr;
    endptr = NULL;

    errno = 0;    /* To distinguish success/failure after call */
    val = strtod(str, &endptr);

    /* Check for various possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        fprintf(stderr, "[%s] strtod failed: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (endptr == str) {
        fprintf(stderr, "[%s] Invalid format of input: %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    p.y = val;

    return p;
}

double static distance(point_t p1, point_t p2){
    return sqrt(pow((p2.x - p1.x), 2) + pow((p2.y - p1.y), 2));
}

double static min(double a, double b, double c){
    if (a == fmin(a,b))
    {
        if (a == fmin(a,c))
        {
            return a;
        } else {
            return c;
        } 
    } else 
    {
        if (b == fmin(b,c))
        {
            return b;
        } else 
        {
            return c;
        }
    }
}

/*void static read_file_with_tree(FILE *in, buffer_t *buf)
{   
    char line[LINE_SIZE];

    char tree_indicator[LINE_SIZE];
    strcpy(tree_indicator, TREE_INDICATOR);
    
    // Read in array of points
    while (fgets(line, LINE_SIZE + 1, in) != NULL)  
    {   
        // If tree information starts, break loop
        if (strcmp(tree_indicator, line) == 1) 
        {
            break;
        }
        
        // Parse input to doubles
        point_t p = parse_line(line);

        // If buffer is full, increase size
        if (buf->current_index == buf->max_size) 
        {
            increase_buffer_size(buf);
        }

        // Store point in buffer
        buf->points[buf->current_index++] = p;
    }
    
}

void static construct_tree(FILE *out, FILE *in_child1, FILE *in_child2, buffer_t *buf){

    char line_child1[LINE_SIZE];
    char line_child2[LINE_SIZE];

    if (buf->current_index < 2)
    {
        printf("%s", TREE_INDICATOR);
    }
    
    // Read subtrees from children
    while ((fgets(line_child1, LINE_SIZE + 1, in_child1) != NULL) && (fgets(line_child2, LINE_SIZE + 1, in_child2) != NULL))  
    {   
        // Remove \n from end of string 
        line_child1[strlen(line_child1) - 1] = '\0';
        line_child2[strlen(line_child2) - 1] = '\0';
        
        strcat(tree, line_child1);
        strcat(tree, " ");
        strcat(tree, line_child2);
        strcat(tree, "\n"); 
    }
}*/