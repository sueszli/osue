/**
 * @file cpair.c
 * @author Marvin Flandorfer, 52004069
 * @date 22.11.2021
 * 
 * @brief Main module.
 * @details This module contains not only the main function but all other necessary, directly involved functions for obtaining the result.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include "list.h"
#include "misc.h"

char* program_name;                                 /**< Program name*/

/**
 * @brief Get the coordinate list.
 * @details This function obtains the coordinate list from the given filepointer.
 * For every point (each line must include one point) a new node in the list will be created and added.
 * There must be at least two point (two lines) for correct use of the function.
 * If the file is empty (only EOF) a list will be created with two points that both have INFINTY as x- and y-coordinate.
 * 
 * @param fp Pointer to the File
 * @return Returns a pointer to the list on success. Returns NULL if an error occured.
 */
static coord_t *get_coordinate_list(FILE *fp){
    coord_t *head = NULL;                                   /**< Pointer to the head of the list*/
    size_t size = 0;                                        /**< Size used for getline*/
    char *line = NULL;                                      /**< Char pointer to the line acquired via getline*/
    float x;                                                /**< Current x-coordinate*/
    float y;                                                /**< Current y-coordinate*/
    int a;
    int i = 0;
    while((a = getline(&line, &size, fp)) > 0){
        if(a == -1){
            error_message("getline");
            free_list(head);
            free(line);
            return NULL;
        }
        if(sscanf(line, "%f %f", &x, &y) == EOF){
            error_message("sscanf");
            free_list(head);
            free(line);
            return NULL;
        }
        if(head == NULL){
            head = init(x, y);
            if(head == NULL){
                free(line);
                return NULL;
            }
        }
        else{
            if(push(head, x, y) == -1){
                free_list(head);
                free(line);
                return NULL;
            }
        }
        free(line);
        size = 0;
        i++;
    }
    if(i == 0){
        head = init(INFINITY, INFINITY);
        if(head == NULL){
            return NULL;
        }
        if(push(head, INFINITY, INFINITY) == -1){
            free_list(head);
            return NULL;
        }
        return head;
    }
    free(line);
    return head;
}

/**
 * @brief Get the mean of all x-coordinates
 * @details Iterates through all list elements and calculates the arithmetic mean of all x-values.
 * 
 * @param head Pointer to the head of the list
 * @param size Size of the list
 * @return Returns the mean of all x-coordinates in the given list.
 */
static float get_x_mean(coord_t *head, int size){
    float mean = 0;                                     /**< Mean of the x-coordinates*/
    coord_t *cur = head;                                /**< Current node of the list*/
    while(cur != NULL){
        mean += cur->x;
        cur = cur->next;
    }
    mean /= size;
    return mean;
}

/**
 * @brief Get distance between two points.
 * @details This function calculates the euclidean distance between two points represented by the four floats parsed.
 * 
 * @param x1 x-coordinate of the first point
 * @param y1 y-coordinate of the first point
 * @param x2 x-coordinate of the second point
 * @param y2 y-coordiante of the second point
 * @return Returns the distance between the two points.
 */
static float get_distance(float x1, float y1, float x2, float y2){
    float x = x1 - x2;
    float y = y1 - y2;
    float dist = sqrtf(x*x + y*y);
    if(isnan(dist)){
        dist = INFINITY;
    }
    return dist;
}

/**
 * @brief Get P3.
 * @details Calculates the two points with the shortest distance from the two different lists.
 * The first point is always from the first list and the second point from the second list.
 * 
 * @param first_part First list of points
 * @param second_part Second list of points
 * @return Returns a list of the two closest points from both parts on success. Returns NULL on error.
 */
static coord_t *get_p3(coord_t *first_part, coord_t *second_part){
    float p3_dist = -1;                                     /**< Distance between the point pair P3*/
    coord_t *cur1 = first_part;                             /**< Current node of the first part*/
    coord_t *cur2 = second_part;                            /**< Current node of the second part*/
    coord_t *p3 = NULL;                                     /**< Head of P3*/
    while(cur1 != NULL){
        while(cur2 != NULL){
            float dist = get_distance(cur1->x, cur1->y, cur2->x, cur2->y);
            if(dist == -1){
                free_list(p3);
                return NULL;
            }
            if(dist < p3_dist || p3_dist == -1){
                p3_dist = dist;
                free_list(p3);
                p3 = init(cur1->x, cur1->y);
                if(p3 == NULL){
                    return NULL;
                }
                if(push(p3, cur2->x, cur2->y) == -1){
                    free_list(p3);
                    return NULL;
                }
            }
            cur2 = cur2->next;
        }
        cur2 = second_part;
        cur1 = cur1->next;
    }
    return p3;
}

/**
 * @brief Get a list of closes points
 * @details Compares all distances between P1, P2 and P3 and returns the closest pair.
 * 
 * @param p1 P1, closest pair of points from child 1
 * @param p2 P2, closest pair of points from child 2
 * @param p3 P3, closest pair of points from the first and second part.
 * @return Returns a pointer to the closest pair of points from P1, P2 and P3 on success. Returns NULL on failure.
 */
static coord_t *get_closest_points(coord_t* p1, coord_t *p2, coord_t *p3){
    float p1_dist = get_distance(p1->x, p1->y, p1->next->x, p1->next->y);
    float p2_dist = get_distance(p2->x, p2->y, p2->next->x, p2->next->y);
    float p3_dist = get_distance(p3->x, p3->y, p3->next->x, p3->next->y);
    if(p1_dist == -1 || p2_dist == -1 || p3_dist == -1){
        return NULL;
    }
    coord_t *shortest_p = NULL;
    if(p1_dist < p2_dist){
        if(p1_dist < p3_dist){
            shortest_p = p1;
        }
        else{
            shortest_p = p3;
        }
    }
    else{
        if(p2_dist < p3_dist){
            shortest_p = p2;
        }
        else{
            shortest_p = p3;
        }
    }
    return shortest_p;
}

/**
 * @brief Closes all given pipes.
 * @details Closes all ends of the given read and write pipes.
 * 
 * @param write_pipe Array of file descriptors for the write pipe
 * @param read_pipe Array of file descriptors for the read pipe
 * @return Returns 0 on success and -1 on failure.
 */
static int close_child_pipes(int write_pipe[2], int read_pipe[2]){
    if(close(write_pipe[0]) == -1 || close(write_pipe[1]) == -1 || close(read_pipe[0]) == -1 || close(read_pipe[1]) == -1){
        error_message("close");
        return -1;
    }
    return 0;
}

/**
 * @brief Write points to the given file descriptor.
 * @details This function writes all points in the given list to the file descriptor via dprintf.
 * 
 * @param head Pointer to the head of the list
 * @param fd File descriptor
 * @return Returns 0 on success and -1 on failure.
 */
static int write_to_fd(coord_t *head, int fd){
    coord_t *cur = head;
    while(cur != NULL){
        if(dprintf(fd, " %f %f\n", cur->x, cur->y) < 0){
            error_message("dprintf");
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

/**
 * @brief Read from points from the given file descriptor.
 * @details A file gets opened with the given file descriptor. 
 * After that the get_coordinate_list function gets called with the created file pointer.
 * After receiving the result, the file pointer (and the file descriptor) gets closed.
 * 
 * @param fd File descriptor
 * @return Returns list of points read from the file descriptor on success. Returns NULL on failure.
 */
static coord_t *read_from_fd(int fd){
    coord_t *head = NULL;                                   /**< Pointer pointing to the head of the list*/
    FILE *fp = fdopen(fd, "r");                             /**< File pointer pointing to the opened file*/
    if(fp == NULL){
        error_message("fdopen");
        return NULL;
    }
    head = get_coordinate_list(fp);
    if(head == NULL){
        (void) fclose(fp);
        return NULL;
    }
    if(fclose(fp) == EOF){
        error_message("fclose");
        return NULL;
    }
    return head;
}

/**
 * @brief Function for waiting for children termination
 * @details This function waits for the termination of the child processes.
 * The number of waits performed is given via the parsed integer.
 * 
 * @param children Number of children that need to be waited on
 * @return Returns 0 on success and -1 on failure or when the exit status of on of the children is not EXIT_SUCCESS.
 */
static int wait_for_children_term(int children){
    int n = 0;                                          /**< Iteration counter*/
    int temp;                                           /**< Temporary variable for saving the exit status of the child*/
    while(n < children){
        if(wait(&temp) == -1){
            error_message("wait");
            return -1;
        }
        if(WEXITSTATUS(temp) != EXIT_SUCCESS){
            return -1;
        }
        n++;
    }
    return 0;
}


/**
 * @brief Main function
 * @details This function controls the flow of the cpair program.
 * At first all points get read from stdin. 
 * If the number of points equals 1, the program exits. If the number of points equals 2, the program writes these two points to stdout and exits.
 * If the number of points is bigger than 2 the program continues as follows:
 * The points get divided into two parts according to the calculated x mean.
 * Two child processes get created and all necessary pipes get created and redirected (in the child processes).
 * If the process is a child process the program restarts at this point, otherwise it continues.
 * Both parts gets written into the pipes for the two children.
 * The program waits on the children for termination and subsequently reads the results from both children from the pipes.
 * After that the program decides which pair of points is the closest one and writes this to stdout and exits successfully.
 * 
 * @param argc Argument counter
 * @param argv Argument vector
 * @return Returns EXIT_SUCCESS on success and EXIT_FAILURE on error.
 */
int main(int argc, char* argv[]){
    coord_t *head = NULL;                               /**< Pointer pointing to the head of the list*/
    coord_t *first_part = NULL;                         /**< Pointer pointing to the first part*/
    coord_t *second_part = NULL;                        /**< Pointer pointing to the second part*/
    coord_t *p1 = NULL;                                 /**< Pointer pointing to the pair of points P1*/
    coord_t *p2 = NULL;                                 /**< Pointer pointing to the pair of points P2*/
    coord_t *p3 = NULL;                                 /**< Pointer pointing to the pair of points P3*/
    pid_t child1;                                       /**< Process ID of the first child*/
    pid_t child2;                                       /**< Process ID of the second child*/
    float x_mean;                                       /**< Mean of all x-coordinates*/
    int write_child1[2];                                /**< Array of file descriptors for the first write pipe*/
    int write_child2[2];                                /**< Array of file descriptors for the second write pipe*/
    int read_child1[2];                                 /**< Array of file descriptors for the first read pipe*/
    int read_child2[2];                                 /**< Array of file descriptors for the second read pipe*/
    int size;                                           /**< Size of the list of all points read*/
    program_name = argv[0];
    if(argc > 1){
        usage();
        exit(EXIT_FAILURE);
    }
    head = get_coordinate_list(stdin);
    if(head == NULL){
        exit(EXIT_FAILURE);
    }
    size = size_list(head);
    if(size == 1 || head->x == INFINITY){
        free_list(head);
        return EXIT_SUCCESS;
    }
    else if(size == 2){
        if(fprintf(stdout,"%f %f\n%f %f\n", head->x, head->y, head->next->x, head->next->y) < 0){
            error_message("fprintf");
            free_list(head);
            exit(EXIT_FAILURE);
        }
        free_list(head);
        return EXIT_SUCCESS;
    }
    x_mean = get_x_mean(head, size);
    coord_t *cur = head;
    while(cur != NULL){
        if(cur->x <= x_mean){
            if(first_part == NULL){
                first_part = init(cur->x, cur->y);
                if(first_part == NULL){
                    free_list(second_part);
                    free_list(head);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                if(push(first_part, cur->x, cur->y) == -1){
                    free_list(first_part);
                    free_list(second_part);
                    free_list(head);
                    exit(EXIT_FAILURE);
                }
            }
        }
        else{
            if(second_part == NULL){
                second_part = init(cur->x, cur->y);
                if(second_part == NULL){
                    free_list(first_part);
                    free_list(head);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                if(push(second_part, cur->x, cur->y) == -1){
                    free_list(first_part);
                    free_list(second_part);
                    free_list(head);
                    exit(EXIT_FAILURE);
                }
            }
        }
        cur = cur->next;
    }
    free_list(head);
    if(pipe(write_child1) == -1 || pipe(write_child2) == -1 || pipe(read_child1) == -1 || pipe(read_child2) == -1){
        error_message("pipe");
        free_list(first_part);
        free_list(second_part);
        exit(EXIT_FAILURE);
    }
    child1 = fork();
    if(child1 == -1){
        error_message("fork");
        free_list(first_part);
        free_list(second_part);
        (void) close_child_pipes(write_child1, read_child1);
        (void) close_child_pipes(write_child2, read_child2);
        exit(EXIT_FAILURE);
    }
    if(child1 == 0){
        if(dup2(write_child1[0], STDIN_FILENO) == -1 || dup2(read_child1[1], STDOUT_FILENO) == -1){
            error_message("dup2");
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
        if(close_child_pipes(write_child2, read_child2) == -1 || close_child_pipes(write_child1, read_child1) == -1){
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
        if(execlp(program_name, program_name, NULL) == -1){
            error_message("execlp");
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
    }
    child2 = fork();
    if(child2 == -1){
        error_message("fork");
        free_list(first_part);
        free_list(second_part);
        (void) close_child_pipes(write_child1, read_child1);
        (void) close_child_pipes(write_child2, read_child2);
        (void) wait_for_children_term(1);
        exit(EXIT_FAILURE);
    }
    if(child2 == 0){
        if(dup2(write_child2[0], STDIN_FILENO) == -1 || dup2(read_child2[1], STDOUT_FILENO) == -1){
            error_message("dup2");
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
        if(close_child_pipes(write_child2, read_child2) == -1 || close_child_pipes(write_child1, read_child1) == -1){
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
        if(execlp(program_name, program_name, NULL) == -1){
            error_message("execlp");
            free_list(first_part);
            free_list(second_part);
            exit(EXIT_FAILURE);
        }
    }
    if(close(write_child1[0]) == -1 || close(write_child2[0]) == -1 || close(read_child1[1]) == -1 || close(read_child2[1]) == -1){
        error_message("close");
        free_list(first_part);
        free_list(second_part);
        (void) wait_for_children_term(2);
        exit(EXIT_FAILURE);
    }
    if(write_to_fd(first_part, write_child1[1]) == -1 || write_to_fd(second_part, write_child2[1]) == -1){
        free_list(first_part);
        free_list(second_part);
        (void) wait_for_children_term(2);
        exit(EXIT_FAILURE);
    }
    if(close(write_child1[1]) == -1 || close(write_child2[1]) == -1){
        error_message("close");
        free_list(first_part);
        free_list(second_part);
        (void) wait_for_children_term(2);
        exit(EXIT_FAILURE);
    }
    if(wait_for_children_term(2) == -1){
        free_list(first_part);
        free_list(second_part);
        exit(EXIT_FAILURE);
    }
    p1 = read_from_fd(read_child1[0]);
    if(p1 == NULL){
        free_list(first_part);
        free_list(second_part);
        exit(EXIT_FAILURE);
    }
    p2 = read_from_fd(read_child2[0]);
    if(p2 == NULL){
        free_list(p1);
        free_list(first_part);
        free_list(second_part);
        exit(EXIT_FAILURE);
    }
    p3 = get_p3(first_part, second_part);
    free_list(first_part);
    free_list(second_part);
    if(p3 == NULL){
        free_list(p1);
        free_list(p2);
        exit(EXIT_FAILURE);
    }
    coord_t *closest_p = get_closest_points(p1, p2, p3);
    if(closest_p == NULL){
        free_list(p1);
        free_list(p2);
        free_list(p3);
        exit(EXIT_FAILURE);
    }
    if(fprintf(stdout, "%f %f\n%f %f\n", closest_p->x, closest_p->y, closest_p->next->x, closest_p->next->y) < 0){
        error_message("fprintf");
        free_list(p1);
        free_list(p2);
        free_list(p3);
        exit(EXIT_FAILURE);
    }
    free_list(p1);
    free_list(p2);
    free_list(p3);
    return EXIT_SUCCESS;
}