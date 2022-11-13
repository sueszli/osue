/**
 * @file cpair.c
 * @author Michael Trauner <e12019868@student.tuwien.ac.at>
 * @date 08.12.2021
 *
 * @brief This program searches for the closest pair of points in a set of 2D-points.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>


char *prog_name;

int str_to_int(int *ret_int,char *str) {
    if(str == NULL || (strcmp(str,"/0") == 0)) {
        fprintf(stderr, " [%s] ERROR: Invalid Input: NULL or empty string\n", prog_name);
        return -1;
    }
    char *endptr = NULL;
    int ret_strtol = (int) (strtol(str, &endptr, 10));
    *ret_int = ret_strtol;

    if(strcmp(endptr,"/0") == 0) {
        fprintf(stderr, " [%s] ERROR: strtol failed: Invalid String\n", prog_name);
        return -1;
    }
    return 0;
}

int str_to_float(float *ret,char *integer_part, char *decimal_part) {

    int a_int = 0, a_dec = 0;

    if (str_to_int(&a_int,integer_part) == -1)
    {
        return -1;
    }

    int len_a_dec = strlen(decimal_part);
    if (str_to_int(&a_dec,decimal_part) == -1)
    {
        return -1;
    }

    if((a_dec) < 0) {
        fprintf(stderr, " [%s] ERROR: Invalid Input: decimal < 0\n", prog_name);
        return -1;
    }

    *ret = ((float) (a_int)) + (((float) (a_dec))/powf(10.0,(float) len_a_dec));

    return 0;
}

int str_to_point(float point[], char *line_buffer) {

        char *x_str_integer = strtok(line_buffer, ".");
        char *x_str_decimal = strtok(NULL, " ");
        char *y_str_integer = strtok(NULL, ".");
        char *y_str_decimal = strtok(NULL, "\n");

        float x, y;
        if(str_to_float(&x, x_str_integer, x_str_decimal) == -1) {
            return -1;
        }
        if(str_to_float(&y, y_str_integer, y_str_decimal) == -1) {
            return -1;
        }
        point[0] = x;
        point[1] = y;
        return 0;
}

void usage(void) {
    fprintf(stderr, "Usage: %s \n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    prog_name = argv[0];

    if(argc > 1) {
        usage();
    }

    char *line_buffer = NULL;
    size_t len = 0;
    int pi = 0;
    int points_size = 10;
    float (*points)[2] = malloc(sizeof(float[2]) * points_size);
    if(points == NULL) {
            fprintf(stderr, " [%s] ERROR: malloc failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
    }

    while(getline(&line_buffer, &len, stdin) != -1) {

        if(line_buffer[0] == '\n') {
            break;            
        }

        float point[2];

        if(str_to_point(point, line_buffer) == -1) {
            free(line_buffer);
            free(points);
            exit(EXIT_FAILURE);
        }

        points[pi][0] = point[0];
        points[pi++][1] = point[1];
    }
    
    free(line_buffer);

    if(pi <= 1) {
        free(points);
        return 0;
    }

    if(pi <= 2) {
        printf("%f %f\n", points[0][0], points[0][1]);
        printf("%f %f\n", points[1][0], points[1][1]);
        fflush(stdout);
        free(points);
        return 0;
    }


    /* calculate mean */
    float sum = 0;
    for(int i = 0; i < pi; i++) {
        sum += points[i][0];
    }
    float mean_x = sum / pi;

    /* Fork/Exec/Pipe part */

    int pipeC1toP[2];
    if(pipe(pipeC1toP) == -1) {
        fprintf(stderr, " [%s] ERROR: pipe failed: %s\n", prog_name, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    int pipeC2toP[2];
    if(pipe(pipeC2toP) == -1) {
        fprintf(stderr, " [%s] ERROR: pipe failed: %s\n", prog_name, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    int pipePtoC1[2];
    if(pipe(pipePtoC1) == -1) {
        fprintf(stderr, " [%s] ERROR: pipe failed: %s\n", prog_name, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    int pipePtoC2[2];
    if(pipe(pipePtoC2) == -1) {
        fprintf(stderr, " [%s] ERROR: pipe failed: %s\n", prog_name, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    switch (pid1) {
        case -1:
            fprintf(stderr, " [%s] ERROR: fork failed: %s\n", prog_name, strerror(errno));
            free(points);
            exit(EXIT_FAILURE);
        case 0:

            /* parent to child1 Pipe */
            if(close(pipePtoC1[1]) == -1) { // close unused write end
                fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                free(points);
                exit(EXIT_FAILURE);
            }
            if(dup2(pipePtoC1[0], STDIN_FILENO) == -1) { // old descriptor - read end // new descriptor
                fprintf(stderr, " [%s] ERROR: dup2 failed: %s\n", prog_name, strerror(errno));
                free(points);
                exit(EXIT_FAILURE);
            }

            /* child1 to parent Pipe */
            if(close(pipeC1toP[0]) == -1) { // close unused read end
                fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                free(points);
                exit(EXIT_FAILURE);
            }
            if(dup2(pipeC1toP[1], STDOUT_FILENO) == -1) { // old descriptor - write end // new descriptor
                fprintf(stderr, " [%s] ERROR: dup2 failed: %s\n", prog_name, strerror(errno));
                free(points);
                exit(EXIT_FAILURE);
            }
            
            // EXEC HERE

            free(points);
            execlp("./cpair", "./cpair", NULL);

            // Code shouldn reach these lines
            // close(pipeC1toP[1]);
            // close(pipePtoC1[0]);
            break;
        default: ;
            pid_t pid2 = fork();
            switch (pid2) {
                case -1:
                    fprintf(stderr, " [%s] ERROR: fork failed: %s\n", prog_name, strerror(errno));
                    free(points);
                    exit(EXIT_FAILURE);
                case 0:

                    /* parent to child2 Pipe */
                    if(close(pipePtoC2[1]) == -1) { // close unused write end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pipePtoC2[0], STDIN_FILENO) == -1) { // old descriptor - read end // new descriptor
                        fprintf(stderr, " [%s] ERROR: dup2 failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }

                    /* child2 to parent Pipe */
                    if(close(pipeC2toP[0]) == -1) { // close unused read end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pipeC2toP[1], STDOUT_FILENO) == -1) { // old descriptor - write end // new descriptor
                        fprintf(stderr, " [%s] ERROR: dup2 failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }

                    // EXEC HERE

                    free(points);
                    execlp("./cpair", "./cpair", NULL);
                    
                    // Code shouldnt reach these lines
                    // close(pipeC2toP[1]);
                    // close(pipePtoC2[0]);
                    break;
                default: ;

                    /* parent to child1 Pipe */
                    if(close(pipePtoC1[0]) == -1) { // close unused read end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }

                    /* parent to child2 Pipe */
                    if(close(pipePtoC2[0]) == -1) { // close unused read end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        free(points);
                        exit(EXIT_FAILURE);
                    }
                    
                    /* Split up array and write to children */
                    for(int i = 0; i < pi; i++) {
                        if(points[i][0] <= mean_x) {

                            /* convert float numbers back to string */
                            char message[1024];
                            sprintf(message,"%f %f\n", points[i][0], points[i][1]);
                            
                            /* write to child1 */
                            write(pipePtoC1[1], message, strlen(message));
                        } else {
                            /* convert float numbers back to string */
                            char message[1024];
                            sprintf(message,"%f %f\n", points[i][0], points[i][1]);

                            /* write to child2 */
                            write(pipePtoC2[1], message, strlen(message));
                        }
                    }
                    free(points);
                    if(write(pipePtoC1[1], "\n", 1) == -1) {
                        fprintf(stderr, " [%s] ERROR: write failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if(write(pipePtoC2[1], "\n", 1) == -1) {
                        fprintf(stderr, " [%s] ERROR: write failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if(close(pipePtoC1[1]) == -1) {
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if(close(pipePtoC2[1]) == -1) {
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    char buffer[1025];
                    int n;
                    float p1[2][2], p2[2][2];
                    char *point1 = NULL;
                    char *point2 = NULL;
                    int child1_out_is_null = 0;
                    int child2_out_is_null = 0;

                    /* wait for child1 to finish*/

                    if(waitpid(pid1, NULL, 0) == -1) {
                        fprintf(stderr, " [%s] ERROR: waitpid failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    /* child1 to parent Pipe */
                    if(close(pipeC1toP[1]) == -1) { // close unused write end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pipeC1toP[0], STDIN_FILENO) == -1) { // old descriptor - read end // new descriptor
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    n = read(STDIN_FILENO, buffer, sizeof(buffer));
                    buffer[n] = 0; // terminate string
                    child1_out_is_null = (n == 0) ? 1 : 0;
                    if(close(pipeC1toP[0]) == -1) {
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    if(!child1_out_is_null) {
                        /* process input from read1 */
                        point1 = strtok(buffer, "\n");
                        point2 = strtok(NULL, "\n");

                        if(str_to_point(p1[0], point1) == -1) {
                            exit(EXIT_FAILURE);
                        }
                        if(str_to_point(p1[1], point2) == -1) {
                            exit(EXIT_FAILURE);
                        }

                        // fprintf(stderr,"Processed input1:\n x: %f y: %f\n x: %f y: %f\n", p1[0][0], p1[0][1], p1[1][0], p1[1][1]);
                    }

                    /* wait for child1 to finish*/
                    if(waitpid(pid2, NULL, 0) == -1) {
                        fprintf(stderr, " [%s] ERROR: waitpid failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    /* child2 to parent Pipe */
                    if(close(pipeC2toP[1]) == -1) { // close unused write end
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pipeC2toP[0], STDIN_FILENO) == -1) { // old descriptor - read end // new descriptor
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    n = read(STDIN_FILENO, buffer, sizeof(buffer));
                    buffer[n] = 0; // terminate string
                    child2_out_is_null = (n == 0) ? 1 : 0;
                    if(close(pipeC2toP[0]) == -1) {
                        fprintf(stderr, " [%s] ERROR: close failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    if(!child2_out_is_null) {
                        /* process input from read1 */
                        point1 = strtok(buffer, "\n");
                        point2 = strtok(NULL, "\n");

                        if(str_to_point(p2[0], point1) == -1) {
                            exit(EXIT_FAILURE);
                        }
                        if(str_to_point(p2[1], point2) == -1) {
                            exit(EXIT_FAILURE);
                        }

                        // fprintf(stderr,"Processed input2:\n x: %f y: %f\n x: %f y: %f\n", p2[0][0], p2[0][1], p2[1][0], p2[1][1]);
                    }


                    /* calculate shortest distance between p1 and p2, save pair in p3*/
                    float p3[2][2];

                    if(child1_out_is_null) {
                        memcpy(p3,p2,sizeof(p2));
                    } else if(child2_out_is_null) {
                        memcpy(p3,p1,sizeof(p1));
                    } else {
                        memcpy(p3,p1,sizeof(p1));
                        float shortest_dist = (float) sqrtf(powf((p3[1][0]-p3[0][0]), 2.0) + powf((p3[1][1]-p3[0][1]), 2.0));
                        for (int i = 0; i < 2; i++) {
                            for (int j = 0; j < 2; j++) {
                                float curr_shortest_dist = (float) sqrtf(powf((p2[j][0]-p1[i][0]), 2.0) + powf((p2[j][1]-p1[i][1]), 2.0));
                                if (curr_shortest_dist < shortest_dist) {
                                    shortest_dist = curr_shortest_dist;
                                    p3[0][0] = p1[i][0];
                                    p3[0][1] = p1[i][1];
                                    p3[1][0] = p2[j][0];
                                    p3[1][1] = p2[j][1];
                                }
                            }
                        }
                    }

                    printf("%f %f\n%f %f", p3[0][0], p3[0][1], p3[1][0], p3[1][1]);
                    
                    break;
            }
            break;
    }
    return 0;
}