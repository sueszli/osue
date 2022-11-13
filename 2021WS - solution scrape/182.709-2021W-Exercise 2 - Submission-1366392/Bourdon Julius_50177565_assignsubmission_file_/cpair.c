/**
 * @file cpair.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 10.12.2021
 *
 * @brief Implementation of the program 'cpair', which reads in strings of 2D-point-pairs from stdin,
 *        calculates the point-pair closest to each other and writes this point-pair to stdout.
 * 
 * @details 'cpair' reads in the 2D-points given as strings from stdin. It converts the various point coordinates
 *          to variables of the type float. It then calculates the average of the x-coordinates of all points.
 *          The points are separated into two groups, with one group containing all points with an x-coordinate
 *          greater than the calculated average and the other one containing all other points. 
 *          After that, the process is forked twice and each of the two created child processes calculates 
 *          the point-pair with the smallest distance for one of the two separated groups. 
 *          The parent process combines the two recursively calculated results two the end result by calculating 
 *          the smallest distance between the two separated groups.
 *          The trivial case for the recursion is when there are just two points or even just one point left in 
 *          the respective group. 
 *      
 **/


#include "cpair.h"

/**
 * @brief This static function prints information about the correct use of the cpair program.
**/
void static usage_message(void){
    fprintf(stderr, "Usage: cpair\n");
}

/**
 * @brief This static function frees the allocated resources for various used point-arrays.
 *
 * @param point_array                   The array of points read in from stdin.
 * @param point_array_left              The array of points having a smaller x-coordinate than the average of the x-coordinates of all points.
 * @param point_array_right             The array of points having a larger x-coordinate than the average of the x-coordinates of all points.
 * @param parent_read_buffer_left       The buffer for holding the 2 points with the smallest distance from the left group/child. 
 * @param parent_read_buffer_rigth      The buffer for holding the 2 points with the smallest distance from the right group/child.
 * @param buffer_for_compar_left_right  The buffer for holding the 2 points with the smallest distance between the left and right group.   
 */
void static free_point_arrays(point_t *point_array, point_t *point_array_left, point_t *point_array_right, 
                            point_t *parent_read_buffer_left, point_t *parent_read_buffer_right, 
                            point_t *buffer_for_compar_left_right)
{
    if(point_array != NULL) free(point_array);
    if(point_array_left != NULL) free(point_array_left);
    if(point_array_right != NULL) free(point_array_right);
    if(parent_read_buffer_left != NULL) free(parent_read_buffer_left);
    if(parent_read_buffer_right != NULL) free(parent_read_buffer_right);
    if(buffer_for_compar_left_right != NULL) free(buffer_for_compar_left_right);
}

/**
 * @brief This static function frees the allocated resources for various used char-arrays.
 *
 * @param linebuffer    The char-array for buffering the read-in lines from stdin before parsing.
 * @param line          The char-array for buffering the read-in lines from of the pipe with which the child writes its result back to the parent. 
 */
void static free_char_arrays(char *linebuffer, char *line)
{
    if(linebuffer != NULL) free(linebuffer);
    if(line != NULL) free(line);
}

/**
 * @brief This static function closes all open pipe ends to have the kernel unlink them.
 *
 * @param pipefd_write_from_parent_left_0   The "read" - file descriptor of the pipe, with which the parent writes to the left child.  
 * @param pipefd_write_from_parent_left_1   The "write" - file descriptor of the pipe, with which the parent writes to the left child.
 * @param pipefd_write_from_child_left_0    The "read" - file descriptor of the pipe, with which the left child writes back to the parent.
 * @param pipefd_write_from_child_left_1    The "write" - file descriptor of the pipe, with which the left child writes back to the parent.
 * @param pipefd_write_from_parent_right_0  The "read" - file descriptor of the pipe, with which the parent writes to the right child.
 * @param pipefd_write_from_parent_right_1  The "write" - file descriptor of the pipe, with which the parent writes to the right child.
 * @param pipefd_write_from_child_right_0   The "read" - file descriptor of the pipe, with which the right child writes back to the parent.
 * @param pipefd_write_from_child_right_1   The "write" - file descriptor of the pipe, with which the right child writes back to the parent.
 */
void static close_pipe_fds(int pipefd_write_from_parent_left_0, int pipefd_write_from_parent_left_1, 
                            int pipefd_write_from_child_left_0, int pipefd_write_from_child_left_1,
                            int pipefd_write_from_parent_right_0, int pipefd_write_from_parent_right_1,
                            int pipefd_write_from_child_right_0, int pipefd_write_from_child_right_1,
                            int fd_standard_in, int fd_standard_out)
{
    if(pipefd_write_from_parent_left_0 != -1) close(pipefd_write_from_parent_left_0);
    if(pipefd_write_from_parent_left_1 != -1) close(pipefd_write_from_parent_left_1);
    if(pipefd_write_from_child_left_0 != -1) close(pipefd_write_from_child_left_0);
    if(pipefd_write_from_child_left_1 != -1) close(pipefd_write_from_child_left_1);
    if(pipefd_write_from_parent_right_0 != -1) close(pipefd_write_from_parent_right_0);
    if(pipefd_write_from_parent_right_1 != -1) close(pipefd_write_from_parent_right_1);
    if(pipefd_write_from_child_right_0 != -1) close(pipefd_write_from_child_right_0);
    if(pipefd_write_from_child_right_1 != -1) close(pipefd_write_from_child_right_1);
    if(fd_standard_in != -1) close (fd_standard_in);
    if(fd_standard_out != -1) close (fd_standard_out);
}

/**
 * @brief This static function calculates the minimum of three float-numbers.
 *
 * @param a The first of the 3 float-numbers, for which the minimum is calculated.  
 * @param b The second of the 3 float-numbers, for which the minimum is calculated.
 * @param c The third of the 3 float-numbers, for which the minimum is calculated.
 * 
 * @return Returns the minimum of a, b and c. 
 */
float static min_of_3_floats(float a, float b, float c){

    float curr_min;
    if(a <= b){
        curr_min = a;
    } else {
        curr_min = b;
    }
    
    if(c < curr_min){
        curr_min = c;
    }

    return curr_min;
}

/**
 * @brief   The main function of the cpair program reading in all the lines from stdin doing 
 *          the forking to the respective child processes.
*
 * @param argc The argument counter. If the program is called validly, then argc is always 0, 
 *             as there are no valid options or positional arguments.
 * @param argv The array/pointer to the argument values. In this program it just contains the program name.
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]){

    if(argc != 1){
        fprintf(stderr, "The prog. 'cpair' does not take any options or positional arguments!\n");
        usage_message();
        exit(EXIT_FAILURE);
    }

    point_t *point_array = malloc(10*sizeof(point_t));
    if(point_array == NULL){
        fprintf(stderr, "Prog. 'cpair' - memory allocation failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int point_array_pointer = 0;

    char *linebuffer = NULL; 
    size_t allocated_bytes = 0;
    ssize_t read_length;

    char *current_float;
    char *remaining_string;

    point_t *current_point;

    point_t *intermediate_point_array;

    int runcounter = 0;
    while( (read_length = getline(&linebuffer, &allocated_bytes, stdin)) != EOF){

        runcounter++;
        
        if( (point_array_pointer % 10) == 9){
            intermediate_point_array = point_array;
            point_array = realloc(point_array, ((point_array_pointer+1)+10) * sizeof(point_t));
        } 
        
        if(point_array == NULL){
            fprintf(stderr, "Prog. 'cpair' - memory allocation failed: %s\n", strerror(errno));
            free_point_arrays(intermediate_point_array, NULL, NULL, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            exit(EXIT_FAILURE);
        }

        current_point = point_array+point_array_pointer;

        current_float = linebuffer;
        current_point->x_coord = strtof(current_float, &remaining_string);
        if( (current_point->x_coord == 0) && (current_float == remaining_string)){
            fprintf(stderr, "Prog. 'cpair' - Invalid input (line: %d): The x-coordinate of the point needs to be of the type 'float'!\n", runcounter);
            free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            exit(EXIT_FAILURE);
        }

        current_float = remaining_string;
        current_point->y_coord = strtof(current_float, &remaining_string);
        if( (current_point->y_coord == 0) && (current_float == remaining_string)){
            fprintf(stderr, "Prog. 'cpair' - Invalid input (line: %d): The y-coordinate of the point needs to be of the type 'float'!\n", runcounter);
            free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            exit(EXIT_FAILURE);
        }

        current_float = remaining_string;
        if (strtof(current_float, &remaining_string) != 0){
            fprintf(stderr, "Prog. 'cpair' - Invalid input (line: %d): No further digits after the y-Coordinate of the point are allowed!\n", runcounter);
            free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            exit(EXIT_FAILURE);
        }

        point_array_pointer++;
    }

    
    if(runcounter == 0){
        fprintf(stderr, "Prog. 'cpair' - Invalid input: The input does not contain any points!\n");
        free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);
        exit(EXIT_FAILURE);
    }
    

    if(runcounter == 1){
        free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);
        exit(EXIT_SUCCESS);
    }

    intermediate_point_array = point_array;
    point_array = realloc(point_array, point_array_pointer*sizeof(point_t));
    if(point_array == NULL){
        fprintf(stderr, "Prog. 'cpair' - memory allocation failed: %s\n", strerror(errno));
        free_point_arrays(intermediate_point_array, NULL, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);
        exit(EXIT_FAILURE);
    }
 
    if(runcounter == 2){

        fprintf(stdout, "%f %f\n", point_array->x_coord, point_array->y_coord);
        fprintf(stdout, "%f %f\n", (point_array+1)->x_coord, (point_array+1)->y_coord);

        free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);        
        exit(EXIT_SUCCESS);
    }


    /* NOTE: Continuation of the process, if there are more than 2 points left */

    point_t *point_array_left = malloc(point_array_pointer*sizeof(point_t));
    if(point_array_left == NULL){
        fprintf(stderr, "Prog. 'cpair' - memory allocation failed: %s\n", strerror(errno));
        free_point_arrays(point_array, NULL, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);  
        exit(EXIT_FAILURE);
    }
    int point_array_left_pointer = 0;

    point_t *point_array_right = malloc(point_array_pointer*sizeof(point_t));
    if(point_array_right == NULL){
        fprintf(stderr, "Prog. 'cpair' - memory allocation failed: %s\n", strerror(errno));
        free_point_arrays(point_array, point_array_left, NULL, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);  
        exit(EXIT_FAILURE);
    }
    int point_array_right_pointer = 0;

    int sum_x_coords = 0;
    int i;
    for(i = 0; i < point_array_pointer; i++){
        sum_x_coords += (point_array+i)->x_coord;
    }

    int arith_mean_x_coords = sum_x_coords/point_array_pointer;

    i = 0;
    for(i = 0; i < point_array_pointer; i++){
        if((point_array+i)->x_coord <= arith_mean_x_coords){
            (point_array_left+point_array_left_pointer)->x_coord = (point_array+i)->x_coord;
            (point_array_left+point_array_left_pointer)->y_coord = (point_array+i)->y_coord;
            point_array_left_pointer++;
        } else {
            (point_array_right+point_array_right_pointer)->x_coord = (point_array+i)->x_coord;
            (point_array_right+point_array_right_pointer)->y_coord = (point_array+i)->y_coord;
            point_array_right_pointer++;
        }
    }

    bool left_contains_exactly_1_point = false;
    bool right_contains_exactly_1_point = false;

    if(point_array_left_pointer == 1) left_contains_exactly_1_point = true;
    if(point_array_right_pointer == 1) right_contains_exactly_1_point = true;
    

    int pipefd_write_from_parent_left[2];
    int pipefd_write_from_child_left[2];

    if(pipe(pipefd_write_from_parent_left) == -1) {
        fprintf(stderr, "Prog. 'cpair' - Creating of pipe failed: %s\n", strerror(errno));
        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);
        exit(EXIT_FAILURE);
    }

    if(pipe(pipefd_write_from_child_left) == -1){
        fprintf(stderr, "Prog. 'cpair' - Creating of pipe failed: %s\n", strerror(errno));
        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
        free_char_arrays(linebuffer, NULL);
        close_pipe_fds(pipefd_write_from_parent_left[0], pipefd_write_from_parent_left[1], 
                        -1, -1, -1, -1, -1, -1, -1, -1);
        exit(EXIT_FAILURE);
    }


    pid_t pid_left_fork = fork();

    switch(pid_left_fork){
        case -1:
            fprintf(stderr, "Prog. 'cpair' - The forking of the parent 'to the left' failed: %s\n", strerror(errno));
            free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            close_pipe_fds(pipefd_write_from_parent_left[0], pipefd_write_from_parent_left[1], 
                            pipefd_write_from_child_left[0], pipefd_write_from_child_left[1],
                             -1, -1, -1, -1, -1, -1);
            exit(EXIT_FAILURE);
            break;

        case 0: 
            close(pipefd_write_from_parent_left[1]);
            if(dup2(pipefd_write_from_parent_left[0], STDIN_FILENO) == -1){
                fprintf(stderr, "Prog. 'cpair' - dup2 for 'parent_left_write_pipe' failed: %s\n", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(pipefd_write_from_parent_left[0], -1, 
                                pipefd_write_from_child_left[0], pipefd_write_from_child_left[1],
                                -1, -1, -1, -1, -1, -1);
                exit(EXIT_FAILURE);
            }
            close(pipefd_write_from_parent_left[0]);


            close(pipefd_write_from_child_left[0]);
            if(dup2(pipefd_write_from_child_left[1], STDOUT_FILENO) == -1){
                fprintf(stderr, "Prog. 'cpair' - dup2 for 'child_left_write_pipe' failed: %s\n", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(-1, -1, 
                                -1, pipefd_write_from_child_left[1],
                                -1, -1, -1, -1, STDIN_FILENO, -1);
                exit(EXIT_FAILURE);
            }
            close(pipefd_write_from_child_left[1]);


            execlp("./cpair", "cpair", NULL);

            fprintf(stderr, "Prog. 'cpair' - execlp in child failed: %s\n", strerror(errno));
            free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
            free_char_arrays(linebuffer, NULL);
            close_pipe_fds(-1, -1, 
                            -1, -1,
                            -1, -1, -1, -1, STDIN_FILENO, STDOUT_FILENO);
            exit(EXIT_FAILURE);
            break;
        
        default:
            close(pipefd_write_from_parent_left[0]);
            close(pipefd_write_from_child_left[1]);

            FILE *write_parent_left = fdopen(pipefd_write_from_parent_left[1], "w");
            if(write_parent_left == NULL){
                fprintf(stderr, "Prog. 'cpair' - fdopen of pipe-end in parent failed: %s\n", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                pipefd_write_from_child_left[0], -1,
                                -1, -1, -1, -1, -1, -1);
                exit(EXIT_FAILURE);
            }
                
            for(int j = 0; j < point_array_left_pointer; j++){
                fprintf(write_parent_left, "%f %f\n", (point_array_left+j)->x_coord, (point_array_left+j)->y_coord);
            }

            if( fclose(write_parent_left) != 0){
                fprintf(stderr, "Prog. 'cpair' - fclose failed: %s", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                pipefd_write_from_child_left[0], -1,
                                -1, -1, -1, -1, -1, -1);
                exit(EXIT_FAILURE);
            }


            int pipefd_write_from_parent_right[2];
            int pipefd_write_from_child_right[2];

            if(pipe(pipefd_write_from_parent_right) == -1){
                fprintf(stderr, "Prog. 'cpair' - Creating of pipe failed: %s\n", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                pipefd_write_from_child_left[0], -1,
                                -1, -1, -1, -1, -1, -1);
                exit(EXIT_FAILURE);
            }


            if(pipe(pipefd_write_from_child_right) == -1){
                fprintf(stderr, "Prog. 'cpair' - Creating of pipe failed: %s\n", strerror(errno));
                free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                free_char_arrays(linebuffer, NULL);
                close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                pipefd_write_from_child_left[0], -1,
                                pipefd_write_from_parent_right[0], pipefd_write_from_parent_right[1],
                                -1, -1, -1, -1);
                exit(EXIT_FAILURE);
            }

            pid_t pid_right_fork = fork();
           
            switch(pid_right_fork){
                case -1:
                    fprintf(stderr, "Prog. 'cpair' - The forking of the parent 'to the right' failed: %s\n", strerror(errno));
                    free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                    free_char_arrays(linebuffer, NULL);
                    close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                    pipefd_write_from_child_left[0], -1,
                                    pipefd_write_from_parent_right[0], pipefd_write_from_parent_right[1],
                                    pipefd_write_from_child_right[0], pipefd_write_from_child_right[1],
                                    -1, -1);
                    exit(EXIT_FAILURE);

                case 0: 
                    close(pipefd_write_from_parent_right[1]);
                    if(dup2(pipefd_write_from_parent_right[0], STDIN_FILENO) == -1){
                        fprintf(stderr, "Prog. 'cpair' - dup2 for 'parent_right_write_pipe' failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        pipefd_write_from_parent_right[0], -1,
                                        pipefd_write_from_child_right[0], pipefd_write_from_child_right[1],
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    } 
                    close(pipefd_write_from_parent_right[0]);


                    close(pipefd_write_from_child_right[0]);
                    if(dup2(pipefd_write_from_child_right[1], STDOUT_FILENO) == -1){
                        fprintf(stderr, "Prog. 'cpair' - dup2 for 'child_right_write_pipe' failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, -1,
                                        -1, pipefd_write_from_child_right[1],
                                        STDIN_FILENO, -1);
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd_write_from_child_right[1]);


                    execlp("./cpair", "cpair", NULL);

                    fprintf(stderr, "Prog. 'cpair' - execlp in child failed: %s\n", strerror(errno));
                    free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                    free_char_arrays(linebuffer, NULL);
                    close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                    pipefd_write_from_child_left[0], -1,
                                    -1, -1,
                                    -1, -1,
                                    STDIN_FILENO, STDOUT_FILENO);
                    exit(EXIT_FAILURE);
                    break;
        
                default:
                    close(pipefd_write_from_parent_right[0]);
                    close(pipefd_write_from_child_right[1]);

                    FILE *write_parent_right = fdopen(pipefd_write_from_parent_right[1], "w");
                    if(write_parent_right == NULL){
                        fprintf(stderr, "Prog. 'cpair' - fdopen of pipe-end in parent failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    for(int j = 0; j < point_array_right_pointer; j++){
                        fprintf(write_parent_right, "%f %f\n", (point_array_right+j)->x_coord, (point_array_right+j)->y_coord);
                    }

                    if( fclose(write_parent_right) != 0){
                        fprintf(stderr, "Prog. 'cpair' - fclose failed: %s", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    int status_left_child;
                    int status_right_child;

                    pid_t p_left;
                    pid_t p_right;

                    /* NOTE: Here it is waited for the return of the 2 child-processes. 
                            Furthermore the status of the terminated child-processes is read */
                    while( (p_left = waitpid(pid_left_fork, &status_left_child, 0)) == -1){
                        if(errno == EINTR) continue;

                        fprintf(stderr, "Prog. 'cpair' - Waiting for child process to terminate (via 'waitpid') failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    while( (p_right = waitpid(pid_right_fork, &status_right_child, 0)) == -1){
                        if(errno == EINTR) continue;

                        fprintf(stderr, "Prog. 'cpair' - Waiting for child process to terminate (via 'waitpid') failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    if(WIFEXITED(status_left_child) == true){
                        if(WEXITSTATUS(status_left_child) != EXIT_SUCCESS){
                            fprintf(stderr, "Prog. 'cpair' - The child process with PID '%d' terminated with 'EXIT_FAILURE': %s\n", (int) p_left, strerror(errno));
                            free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                            free_char_arrays(linebuffer, NULL);
                            close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                            pipefd_write_from_child_left[0], -1,
                                            -1, pipefd_write_from_parent_right[1],
                                            pipefd_write_from_child_right[0], -1,
                                            -1, -1);
                            exit(EXIT_FAILURE);
                        }
                    }

                    if(WIFEXITED(status_right_child) == true){
                        if(WEXITSTATUS(status_right_child) != EXIT_SUCCESS){
                            fprintf(stderr, "Prog. 'cpair' - The child process with PID '%d' terminated with 'EXIT_FAILURE': %s\n", (int) p_right, strerror(errno));
                            free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                            free_char_arrays(linebuffer, NULL);
                            close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                            pipefd_write_from_child_left[0], -1,
                                            -1, pipefd_write_from_parent_right[1],
                                            pipefd_write_from_child_right[0], -1,
                                            -1, -1);
                            exit(EXIT_FAILURE);
                        }
                    }
                
                    // NOTE: read-buffer to read in and store the result of the respective child
                    point_t *parent_read_buffer_left = malloc(2*sizeof(point_t));
                    point_t *parent_read_buffer_right = malloc(2*sizeof(point_t));

                    FILE *read_parent_left = fdopen(pipefd_write_from_child_left[0], "r");
                    FILE *read_parent_right = fdopen(pipefd_write_from_child_right[0], "r");

                    if(read_parent_left == NULL || read_parent_right == NULL){
                        fprintf(stderr, "Prog. 'cpair': fdopen of pipe-end in parent failed: %s\n", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, NULL, NULL, NULL);
                        free_char_arrays(linebuffer, NULL);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    char *curr_float;

                    char *line = NULL; 
                    size_t alloc_bytes = 0;
                    char *remaining_line = NULL;

                    getline(&line, &alloc_bytes, read_parent_left);
                    curr_float = line;
                    parent_read_buffer_left->x_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;
                    parent_read_buffer_left->y_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;

                    getline(&line, &alloc_bytes, read_parent_left);
                    curr_float = line;
                    (parent_read_buffer_left+1)->x_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;
                    (parent_read_buffer_left+1)->y_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;

                    getline(&line, &alloc_bytes, read_parent_right);
                    curr_float = line;
                    parent_read_buffer_right->x_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;
                    parent_read_buffer_right->y_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;

                    getline(&line, &alloc_bytes, read_parent_right);
                    curr_float = line;
                    (parent_read_buffer_right+1)->x_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;
                    (parent_read_buffer_right+1)->y_coord = strtof(curr_float, &remaining_line);
                    curr_float = remaining_line;


                    if(fclose(read_parent_left) != 0){
                        fprintf(stderr, "Prog. 'cpair' - fclose failed: %s", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right, 
                                            parent_read_buffer_left, parent_read_buffer_right, NULL);
                        free_char_arrays(linebuffer, line);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }

                    if(fclose(read_parent_right) != 0){
                        fprintf(stderr, "Prog. 'cpair' - fclose failed: %s", strerror(errno));
                        free_point_arrays(point_array, point_array_left, point_array_right,
                                            parent_read_buffer_left, parent_read_buffer_right, NULL);
                        free_char_arrays(linebuffer, line);
                        close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                        pipefd_write_from_child_left[0], -1,
                                        -1, pipefd_write_from_parent_right[1],
                                        pipefd_write_from_child_right[0], -1,
                                        -1, -1);
                        exit(EXIT_FAILURE);
                    }


                    float min_distance_left_child;
                    float min_distance_right_child;
                    
                    if(left_contains_exactly_1_point == false){
                        min_distance_left_child = sqrtf(pow((parent_read_buffer_left->x_coord - (parent_read_buffer_left+1)->x_coord), 2)
                                                    + pow((parent_read_buffer_left->y_coord - (parent_read_buffer_left+1)->y_coord), 2));
                    } else {
                        min_distance_left_child = FLT_MAX;
                    }


                    if(right_contains_exactly_1_point == false){
                        min_distance_right_child = sqrtf(pow((parent_read_buffer_right->x_coord - (parent_read_buffer_right+1)->x_coord), 2)
                                                    + pow((parent_read_buffer_right->y_coord - (parent_read_buffer_right+1)->y_coord), 2));
                    } else {
                        min_distance_right_child = FLT_MAX;
                    }

                    float curr_min_distance_between_left_and_right = FLT_MAX;

                    int k;
                    int l;
                    point_t *curr_point_left;
                    point_t *curr_point_right;
                    point_t *best_point_left;
                    point_t *best_point_right;
                    point_t *buffer_for_compar_left_right = malloc(2*sizeof(point_t));
                    float distance_between_current_points;
                    for(k = 0; k < point_array_left_pointer; k++){
                        curr_point_left = (point_array_left+k);
                        
                        for(l = 0; l < point_array_right_pointer; l++){
                            curr_point_right=(point_array_right+l);

                            distance_between_current_points = sqrtf(pow((curr_point_left->x_coord - curr_point_right->x_coord), 2)
                                                                + pow((curr_point_left->y_coord - curr_point_right->y_coord), 2));

                            if(distance_between_current_points < curr_min_distance_between_left_and_right){
                                curr_min_distance_between_left_and_right = distance_between_current_points;
                                best_point_left = curr_point_left;
                                best_point_right = curr_point_right;
                            }
                        }
                    }
                    buffer_for_compar_left_right->x_coord = best_point_left->x_coord;
                    buffer_for_compar_left_right->y_coord = best_point_left->y_coord;
                    (buffer_for_compar_left_right+1)->x_coord = best_point_right->x_coord;
                    (buffer_for_compar_left_right+1)->y_coord = best_point_right->y_coord;

                    float min = min_of_3_floats(min_distance_left_child, min_distance_right_child, curr_min_distance_between_left_and_right);


                    if(min == min_distance_left_child){
                        fprintf(stdout, "%f %f\n",
                                parent_read_buffer_left->x_coord, 
                                parent_read_buffer_left->y_coord);
                        fprintf(stdout, "%f %f\n", 
                                (parent_read_buffer_left+1)->x_coord, 
                                (parent_read_buffer_left+1)->y_coord);
                    } else if(min == min_distance_right_child){
                        fprintf(stdout, "%f %f\n",
                                parent_read_buffer_right->x_coord,
                                parent_read_buffer_right->y_coord);
                        fprintf(stdout, "%f %f\n",
                                (parent_read_buffer_right+1)->x_coord,
                                (parent_read_buffer_right+1)->y_coord);
                    } else {
                        fprintf(stdout, "%f %f\n",
                                buffer_for_compar_left_right->x_coord,
                                buffer_for_compar_left_right->y_coord);
                        fprintf(stdout, "%f %f\n",
                                (buffer_for_compar_left_right+1)->x_coord,
                                (buffer_for_compar_left_right+1)->y_coord);
                    }

                    //fprintf(stderr, "End of program reached: %s\n", strerror(errno));
                    
                    free_point_arrays(point_array, point_array_left, point_array_right,
                                        parent_read_buffer_left, parent_read_buffer_right, buffer_for_compar_left_right);
                    free_char_arrays(linebuffer, line);
                    close_pipe_fds(-1, pipefd_write_from_parent_left[1], 
                                    pipefd_write_from_child_left[0], -1,
                                    -1, pipefd_write_from_parent_right[1],
                                    pipefd_write_from_child_right[0], -1,
                                    -1,-1);
                                    
                    return EXIT_SUCCESS;


                    break;
            } // /* NOTE: end of inner switch-statement */

    break; 
    } // /* NOTE: end of outer switch-statement */
  
}