#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>

#include "complex.h"
#include "dynarray.h"

/**
 * @file main.c
 * @author Moritz Schalk (52005233)
 * @date 25 Nov 2021
 * @brief Implement the Cooley-Tukey FFT algorithm by recursively forking this program.
 * @details 
 * SYNOPSIS
 *    forkFFT
 *
 * This program reads input line by line from stdin, where each line is a floating point value 
 * represented as a decimal value that may start with either a + or - and uses the '.' symbol as comma.
 * The total number of inputs must be a multiple of two. 
 */

#ifndef TREE
/**
 * @brief read a number of complex numbers from stdin and write them into result.
 * @details 
 * The output array must have enough space for the input. 
 */
static void handle_sub(complex_t *result);
#else
/**
 * @brief print n spaces to stdout.
 */
static void print_spaces(unsigned int n);
#endif

int main(int argc, char **argv) {
    if (argc > 1) {
        fprintf(stderr, "Error: too many arguments\nSYNOPSIS\n\t%s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int exit_code = EXIT_SUCCESS;

    char *line = NULL;
    size_t line_len = 0;
    regex_t floating_point;

    if (regcomp(&floating_point, "^(-|\\+)?([0-9]+)(\\.[0-9]*)?", REG_EXTENDED) != 0) {
        fprintf(stderr, "Error compiling regex!\n");
        exit(EXIT_FAILURE);
    }

    dynarray *input = NULL;
    float *even = NULL;
    float *odd  = NULL;

    if (dyn_new(&input) == DYN_ERROR) {
        fprintf(stderr, "Error allocating dynamic array!\n");
        
        exit_code = EXIT_FAILURE;
        goto cleanup_regex;
    }

    while (getline(&line, &line_len, stdin) != -1) {
        if (regexec(&floating_point, line, 0, NULL, 0) == REG_NOMATCH) {
            line[line_len - 1] = 0;
            // make sure line ends with \0 instead of \n to prevent segfaults
            fprintf(stderr, "Error input %s contains non numerical values!\n", line);
            
            exit_code = EXIT_FAILURE;
            goto cleanup;
        }

        if (dyn_add(input, strtof(line, NULL)) == DYN_ERROR) {
            fprintf(stderr, "Error reallocating dynamic array!\n");
        
            exit_code = EXIT_FAILURE;
            goto cleanup;
        }
    }

    if (input->length == 0) {
        fprintf(stderr, "Did not recieve any parameters!\n");
        exit_code = EXIT_FAILURE;
        goto cleanup;
    }

    if (input->length == 1) {
#ifdef TREE
        printf("%f\n", input->array[0]);
#else
        printf("%f 0.0 * i\n", input->array[0]);
#endif
        goto cleanup;
    }

    if (input->length % 2 != 0) {
        fprintf(stderr, "Error: amount of input values is not divisible by 2!\n");
        exit_code = EXIT_FAILURE;
        goto cleanup;
    }

    if (get_even(&even, input) == DYN_ERROR) {
        fprintf(stderr, "Error getting even array values.\n");
        exit_code = EXIT_FAILURE;
        goto cleanup;
    }

    if (get_odd(&odd, input) == DYN_ERROR) {
        fprintf(stderr, "Error getting odd array values.\n");
        exit_code = EXIT_FAILURE;
        goto cleanup;
    }

    // Spawn first child
    int pipfd_write1[2];
    int pipfd_read1[2];
    
    if (pipe(pipfd_write1) == -1) {
        fprintf(stderr, "Error creating unnamed pipe: %s!\n", strerror(errno));
        goto cleanup;
    }
    
    if (pipe(pipfd_read1) == -1) {
        fprintf(stderr, "Error creating unnamed pipe: %s!", strerror(errno));
        goto cleanup;
    }

    pid_t pid_child1 = fork();

    switch(pid_child1) {
        case -1:
            fprintf(stderr, "Fork did not work: %s!\n", strerror(errno));
            exit_code = EXIT_FAILURE;
            goto cleanup;

        // child
        case 0:
            close(pipfd_write1[1]);
            close(pipfd_read1[0]);
            
            dup2(pipfd_write1[0], STDIN_FILENO);
            dup2(pipfd_read1[1], STDOUT_FILENO);

            close(pipfd_write1[0]);
            close(pipfd_read1[1]);
            
            execlp(argv[0], argv[0], NULL);
            // this line should never be reached
            fprintf(stderr, "execlp did not work!");
            goto cleanup;

        // parent
        default:
            close(pipfd_write1[0]);
            close(pipfd_read1[1]);
            
            for (int i = 0; i < input->length / 2; i++) {
                dprintf(pipfd_write1[1], "%f\n", even[i]);
            }

            close(pipfd_write1[1]);

            break;
    }

    //Spawn second child
    int pipfd_write2[2];
    int pipfd_read2[2];
    
    if (pipe(pipfd_write2) == -1) {
        fprintf(stderr, "Error creating unnamed pipe: %s!", strerror(errno));
        goto cleanup;
    }
    
    if (pipe(pipfd_read2) == -1) {
        fprintf(stderr, "Error creating unnamed pipe: %s!", strerror(errno));
        goto cleanup;
    }

    pid_t pid_child2 = fork();

    switch(pid_child2) {
        case -1:
            fprintf(stderr, "Fork did not work: %s!\n", strerror(errno));
            exit_code = EXIT_FAILURE;
            goto cleanup;

        // child
        case 0:
            close(pipfd_write2[1]);
            close(pipfd_read2[0]);
            
            dup2(pipfd_write2[0], STDIN_FILENO);
            dup2(pipfd_read2[1], STDOUT_FILENO);

            close(pipfd_write2[0]);
            close(pipfd_read2[1]);
           
            execlp(argv[0], argv[0], NULL);
            // this line should never be reached
            fprintf(stderr, "execlp did not work!");
            goto cleanup;

        // parent
        default:
            close(pipfd_write2[0]);
            close(pipfd_read2[1]);
            
            for (int i = 0; i < input->length / 2; i++) {
                dprintf(pipfd_write2[1], "%f\n", odd[i]);
            }
            
            close(pipfd_write2[1]);

            break;
    }

#ifdef TREE  
    int even_newline_reached = -1;
    int odd_newline_reached = -1;

    unsigned int even_tree_len = 0;
    unsigned int odd_tree_len = 0;

    unsigned int even_tree_line_len = 0;
    unsigned int odd_tree_line_len = 0;

    unsigned int even_tree_max_len = 2;
    unsigned int odd_tree_max_len = 2;
    
    char *even_tree = malloc(sizeof(*even_tree) * even_tree_max_len);
    char *odd_tree  = malloc(sizeof(*odd_tree) * odd_tree_max_len);
    
    int exit_status;

    // wait for both children to stop
    for (int i = 0; i < 2; i++) {
        pid_t pid_stopped_child = wait(&exit_status);
        if (WEXITSTATUS(exit_status) != EXIT_SUCCESS) {
            fprintf(stderr, "Child returned non zero exit code!\n");
            exit_code = EXIT_FAILURE;
            goto cleanup;
        }

        if (pid_stopped_child == pid_child1) {
            if (dup2(pipfd_read1[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "dup failed %s", strerror(errno));
                exit_code = EXIT_FAILURE;
                goto cleanup;
            }

            close(pipfd_read1[0]);

            char c;
            while((c = getc(stdin)) != EOF) {
                if (c == '\n')
                    even_newline_reached = 0;
                
                if (even_newline_reached != 0)
                    even_tree_line_len++;

                if (even_tree_len == even_tree_max_len) {
                    even_tree_max_len *= 2;
                    even_tree = realloc(even_tree, even_tree_max_len);
                }

                even_tree[even_tree_len] = c;
                even_tree_len++;
            }

            even_tree[even_tree_len] = '\0';
        }

        if (pid_stopped_child == pid_child2) {
            if (dup2(pipfd_read2[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "dup failed %s", strerror(errno));
                exit_code = EXIT_FAILURE;
                goto cleanup;
            }

            close(pipfd_read2[0]);

            char c;
            while((c = getc(stdin)) != EOF) {
                if (c == '\n')
                    odd_newline_reached = 0;
                
                if (odd_newline_reached != 0)
                    odd_tree_line_len++;

                if (odd_tree_len == odd_tree_max_len) {
                    odd_tree_max_len *= 2;
                    odd_tree = realloc(odd_tree, odd_tree_max_len);
                }

                odd_tree[odd_tree_len] = c;
                odd_tree_len++;
            }

            odd_tree[odd_tree_len] = '\0';
        }
    }

    char *p_even, *temp_even;
    char *p_odd, *temp_odd;
    p_even = strtok_r(even_tree, "\n", &temp_even);
    p_odd  = strtok_r(odd_tree,  "\n", &temp_odd);

    unsigned int needed_line_len = strlen(p_even) + strlen(p_odd) + 2;
    
    // calculate the string length of the new tree root
    char buffer[50];
    line_len = strlen("forkFFT(");
    for(int i = 0; i < input->length - 1; i++) {
        snprintf(buffer, 50, "%f, ", input->array[i]);
        line_len += strlen(buffer);
    }
    snprintf(buffer, 50, "%f)\n", input->array[input->length - 1]);
    line_len += strlen(buffer);

    if (needed_line_len > line_len)
        print_spaces((needed_line_len - line_len) / 2);
    

    // print the new tree root
    printf("forkFFT(");
    for(int i = 0; i < input->length - 1; i++)
        printf("%f, ", input->array[i]);
    
    printf("%f)\n", input->array[input->length - 1]);
    
    // position tree branches
    if (needed_line_len < line_len)
        print_spaces(line_len / 4);
    else
        print_spaces(needed_line_len / 4 - 1);
    
    // left tree branch
    printf("/");
    
    //space between tree branches
    if (needed_line_len < line_len)
        print_spaces(line_len / 2 - 1);
    else 
        print_spaces(needed_line_len / 2 - 1);
    
    // right tree branch
    printf("\\");
    
    // space after tree branches (important for the following recursion step)
    if (needed_line_len < line_len) 
        print_spaces(line_len / 4 - 1);
    else 
        print_spaces(needed_line_len / 4 - 1);
    

    printf("\n");

    // print the left and right side of the tree
    do { 
        if (needed_line_len < line_len) 
            print_spaces((line_len - needed_line_len) / 2);
        
        printf("%s  %s", p_even, p_odd);
        
        if (needed_line_len < line_len) 
            print_spaces((line_len - needed_line_len) / 2);
        
        printf("\n");
        p_even = strtok_r(NULL, "\n", &temp_even);
        p_odd  = strtok_r(NULL, "\n", &temp_odd); 
    } while ((p_even != NULL) && (p_odd != NULL));

#else
    // Allocate these on the heap, because goto does not work if arrays of
    // dynamic length are defined between it and the label it jumps to. 
    complex_t *even_result = malloc(sizeof(*even_result) * input->length / 2);
    complex_t *odd_result  = malloc(sizeof(*odd_result)  * input->length / 2);
    
    int exit_status;

    // wait for both children to stop
    for (int i = 0; i < 2; i++) {
        pid_t pid_stopped_child = wait(&exit_status);
        if (WEXITSTATUS(exit_status) != EXIT_SUCCESS) {
            fprintf(stderr, "Child returned non zero exit code!\n");
            exit_code = EXIT_FAILURE;
            goto cleanup_result;
        }

        if (pid_stopped_child == pid_child1) {
            if (dup2(pipfd_read1[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "dup failed %s", strerror(errno));
                exit_code = EXIT_FAILURE;
                goto cleanup_result;
            }

            close(pipfd_read1[0]);
            
            handle_sub(even_result);
        }

        if (pid_stopped_child == pid_child2) {
            if (dup2(pipfd_read2[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "dup failed %s", strerror(errno));
                exit_code = EXIT_FAILURE;
                goto cleanup_result;
            }

            close(pipfd_read2[0]);
            
            handle_sub(odd_result);
        }
    }

    // calculate the output of this recoursion step
    complex_t *result  = malloc(sizeof(*result)  * input->length);
    float n_inverse = 1.0 / ((float) input->length);

    for(int i = 0; i < input->length / 2; i++) {
        float k = (float) i;
        complex_t bracketed_expression;

        bracketed_expression.real    = cos(-2.0 * M_PI * n_inverse * k);
        bracketed_expression.complex = sin(-2.0 * M_PI * n_inverse * k);

        complex_t bracketed_expression_inverse = complex_mult(bracketed_expression,
                                                             (complex_t) {
                                                                 .real    = -1.0,
                                                                 .complex = 0.0
                                                             });

        result[i]                     = complex_mult(bracketed_expression,         odd_result[i]);
        result[i + input->length / 2] = complex_mult(bracketed_expression_inverse, odd_result[i]);
    

        result[i]                     = complex_add(even_result[i], result[i]);
        result[i + input->length / 2] = complex_add(even_result[i], result[i + input->length / 2]);
    }

    for(int i = 0; i < input->length; i++) {
        printf("%f %f * i\n", result[i].real, result[i].complex);
    }

    free(result);
cleanup_result:
    free(even_result);
    free(odd_result);

#endif

cleanup:
    dyn_free(input);
    if (even != NULL)
        free(even);
    if (odd != NULL)
        free(odd);
    if (line != NULL)
        free(line);

cleanup_regex:
    regfree(&floating_point);

    return exit_code;
}

#ifndef TREE
static void handle_sub(complex_t *result) {
    char *line = NULL;
    size_t line_len = 0;

    int index = 0;
    while (getline(&line, &line_len, stdin) != -1) {
        char *offset = NULL;
        float real = strtof(line, &offset);
        float complex = strtof(offset + 1, NULL);

        result[index].real    = real;
        result[index].complex = complex;

        index++;

    }

    if (line != NULL)
        free(line);
}
#else
static void print_spaces(unsigned int n) {
    for(unsigned int i = 0; i < n; i++)
        printf(" ");
}
#endif
