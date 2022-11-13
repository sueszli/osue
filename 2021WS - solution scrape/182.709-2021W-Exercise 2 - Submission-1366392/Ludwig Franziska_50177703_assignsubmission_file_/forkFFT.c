/**
 * @file forkFFT.c
 * @author Franziska Ludwig 01100784 <e1100784@student.tuwien.ac.at>
 * @date 11.12.21
 *
 * @brief forkFFT Module
 * 
 * This program performs the Cooley-Tukey fourier transformation algorithm
 **/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PI (3.141592654)

/**
 * @brief Name of the program
*/

static char *prog_name;

/**
 * @brief Struct for imaginary number
*/

struct complex_value{
    float real;
    float imaginary;
};

/**
 * @brief Usage message for the forkFFT module
 * @details Global variable program name
*/

static void usage(void){

    fprintf(stderr, "USAGE:%s\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Function for exiting from main in case of error
 * @details Also frees allocated memory for read lines
 * @param entry1 First char buffer for reading
 * @param entry2 Second char buffer for reading
 * @param message String for error message
*/

static void error_exit_main(char *entry1, char *entry2, char *message){

    if(entry1 != NULL){
        free(entry1);
    }

    if(entry2 != NULL){
        free(entry2);
    }

    fprintf(stderr, "%s", message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Parses the input to a floating point number
 * @details Checks, if formating is: Real number [imaginary number *i]. Also checks for numbers out of range or 
 * if given input is convertible to float at all. 
 * @param input The string to be converted to a complex number
 * @param output The resulting complex number struct
 * @return -1 on error, 0 else
*/

static int input_to_complex(char *input, struct complex_value *output){

    errno = 0;
    char *endptr1;
    char *endptr2;
    float float1 = strtof(input, &endptr1);
    float float2 = strtof(endptr1, &endptr2);

    //If number is max or min value
    if(errno == ERANGE){
        fprintf(stderr, "Real value out of bounds:%s\n", strerror(errno));
        return -1;
    }

    //Endpointer is set to input, if not convertible
    if(input == endptr1){
        fprintf(stderr, "Parsing real value to float failed: %s\n", strerror(errno));
        return -1;
    }

    //No imaginary part given
    if(*endptr1 == '\n'){
        output->real = float1;
        output->imaginary = 0.0;
        return 0;

    //Imaginary part given
    } else {

        if(strcmp(endptr2, "*i\n") != 0){
            fprintf(stderr, "Parsing imaginary value to float failed: Ensure format: im_number*i.\n");
            return -1;
        }

        //Parsing imaginary part successful
        if(endptr1 != endptr2){
            output->real = float1;
            output->imaginary = float2;
        //Endpointer1 == Endpointer2
        } else {
            fprintf(stderr, "Parsing imaginary to float failed: %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Prints an imaginary number to the console
 * @details Format: Real imaginary*i
 * @param number Complex value to be printed to sdout
*/

static void print_imaginary(struct complex_value *number){
    printf("%f ", number->real);
    printf("%f", number->imaginary);
    printf("*i\n");
}

/**
 * @brief Writes input to children
 * @details First, writes the entries, already saved to the char buffers to the children. After that, iterates as long
 * as stdin gets lines. Writes the first entry to the child with even number, the second to the child with odd ones.
 * @param write_odd_fd File descriptor for the writing pipe to the odd child
 * @param write_even_fd File descriptor for the writing pipe to the even child
 * @param entry1 Char buffer holding input for the even child
 * @param entry2 Char buffer holding input for the odd child
 * @return -1 in case of an error, size of read array otherwise
*/

static int write_to_children(int write_odd_fd, int write_even_fd, char *entry1, char *entry2){

    size_t len1 = 0;
    size_t len2 = 0;

    //If this function is reached, input has to be at least size 2
    int size = 2;

    FILE *write_odd = fdopen(write_odd_fd, "w");
    if(write_odd == NULL){
        return -1;
    }

    FILE *write_even = fdopen(write_even_fd, "w");
    if(write_even == NULL){
        fclose(write_even);
        return -1;
    }
    //Writing the inputs already saved in the char buffers
    if(fputs(entry1, write_even) == EOF){
        free(entry1);
        free(entry2);
        fclose(write_even);
        fclose(write_odd);
        return -1;
    }
    
    if(fputs(entry2, write_odd) == EOF){
        free(entry1);
        free(entry2);
        fclose(write_even);
        fclose(write_odd);
        return -1;
    }

    //Reads entries for even child
    while((getline(&entry1, &len1, stdin)) != -1){

        if(fputs(entry1, write_even) == EOF){
            free(entry1);
            free(entry2);
            fclose(write_even);
            fclose(write_odd);
            return -1;
        }

        //Reads entries for odd child
        //If fails, input is not even
        if((getline(&entry2, &len2, stdin)) == -1){
            fprintf(stderr, "Input has to be even.\n");
            free(entry1);
            free(entry2);
            fclose(write_even);
            fclose(write_odd);
            return -1;
        }

        if(fputs(entry2, write_odd) == EOF){
            free(entry1);
            free(entry2);
            fclose(write_even);
            fclose(write_odd);
            return -1;
        }
        size += 2;
    }

    free(entry1);
    free(entry2);
    fclose(write_even);
    fclose(write_odd);
    return size;
}

/**
 * @brief Butterfly operation
 * @details Calculates the result of the Fourier Transformation using the even and the odd result. Factor is the term with which
 * the odd result is multiplied. Multiplication is done according to the following rule:
 *              (a + i · b)(c + i · d) = a · c − b · d + i · (a · d + b · c)
 * The result is a term, here called odd_part, that is either added or substracted fro the even result.
 * @param even The even result
 * @param odd The odd result
 * @param k The current index in the array
 * @param n The total size of the array
*/

static void butterfly_operation(struct complex_value *even, struct complex_value *odd, int k, int n){
    
    struct complex_value factor;
    factor.real = cos((-((2*PI)/n)) * k);
    factor.imaginary = sin((-((2*PI)/n)) * k);

    float a, b, c, d;
    struct complex_value odd_part;

    a = factor.real;
    b = factor.imaginary;
    c = odd->real;
    d = odd->imaginary;

    odd_part.real = (a*c) - (b*d);
    odd_part.imaginary = (a*d) + (b*c);

    //R[k+n/2]
    odd->real = even->real - odd_part.real;
    odd->imaginary = even->imaginary - odd_part.imaginary;

    //R[k]
    even->real = even->real + odd_part.real;
    even->imaginary = even->imaginary + odd_part.imaginary;
    
    return;
}


/**
 * @brief Reads from children
 * @details Loop is executed n/2 times, alternating between reading from the odd and the even child. Butterfly operation
 * is called for both results and the even and odd values are changed by calling this function and subsequently saved in the result array.
 * This array is printed to stdout, eventually to be read by the parent process
 * @param read_odd_fd File descriptor for reading pipe for the odd child
 * @param read_even_fd File descriptor for reading pipe for the even child
 * @param n Total size of the resulting array
 * @return -1 on error, 0 otherwise
*/

static int read_from_children(int read_odd_fd, int read_even_fd, int n){

    struct complex_value even, odd;
    char *input = NULL;
    size_t len;
    struct complex_value result[n];
    
    FILE *read_odd = fdopen(read_odd_fd, "r");
    if(read_odd == NULL){
        return -1;
    }
    FILE *read_even = fdopen(read_even_fd, "r");
    if(read_even == NULL){
        fclose(read_odd);
        return -1;
    }

    for(int i = 0; i < n/2; i++){

        if((getline(&input, &len, read_even)) == -1){
            free(input);
            fclose(read_odd);
            fclose(read_even);
            return -1;
        }

        if(input_to_complex(input, &even) == -1){
            free(input);
            fclose(read_odd);
            fclose(read_even);
            fprintf(stderr, "Parsing even input t imaginary number failed.\n");
            return -1;
        }
        

        if((getline(&input, &len, read_odd)) == -1){
            free(input);
            fclose(read_odd);
            fclose(read_even);
            return -1;
        }

        if(input_to_complex(input, &odd) == -1){
            free(input);
            fclose(read_odd);
            fclose(read_even);
            fprintf(stderr, "Parsing odd input to imaginary number failed.\n");
            return -1;
        }
        

        butterfly_operation(&even, &odd, i, n);

        result[i] = even;
        result[i + n/2] = odd;

    }

    for(int i = 0; i < n; i++){
        print_imaginary(&result[i]);
    }


    free(input);
    fclose(read_odd);
    fclose(read_even);
    return 0;
}


/**
 * @brief Creates child process
 * @details Pipes are created, unused ends are closed and used ones duplicated in case of child process. Parent process reads from
 * child write pipe and vice versa.
 * @param pid Process ID is saved when fork() is called.
 * @param write_fd File descriptor for pipe write end in parent process
 * @param read_fd File descriptor for pipe read end in parent process
 * @return -1 on error, 0 otherwise
*/

static int create_child_process(pid_t *pid, int *write_fd, int *read_fd){

    int pipe_read[2];
    int pipe_write[2];


    if(pipe(pipe_read) == -1){
        return -1;
    }

    if(pipe(pipe_write) == -1){
        return -1;
    }

    *pid = fork();

    switch(*pid){
        case -1:        //Forking failed
            return -1;
            break;
        case 0:         //Child process
            close(pipe_read[1]);
            close(pipe_write[0]);
            if((dup2(pipe_read[0], STDIN_FILENO)) == -1){
                close(pipe_read[0]);
                close(pipe_write[1]);
                return -1;
            }
            if((dup2(pipe_write[1], STDOUT_FILENO)) == -1){
                close(pipe_read[0]);
                close(pipe_write[1]);
                return -1;
            }
            close(pipe_read[0]);
            close(pipe_write[1]);
            if((execlp(prog_name, prog_name, NULL)) == -1){
                return -1;
            }
            break;
        default:        //Parent process
            *write_fd = pipe_read[1];
            *read_fd = pipe_write[0];
            close(pipe_read[0]);
            close(pipe_write[1]);
            break;
    }

    return 0;
}

/**
 * @brief Waits on children
 * @details If child exits on failure, parent exits on failure too
 * @param pid Process id of child
 * @return -1 if child exited on failure, 0 otherwise
*/

static int wait_on_children(pid_t *pid){

    int status;

    waitpid(*pid, &status, 0);
    if(WEXITSTATUS(status) == EXIT_FAILURE){
        return -1;
    }
    return 0;
}

/**
 * @brief Entry point of the program
 * @details Global variable program name is set. Base case no input and one input are handled. Two inputs are safed to char buffer
 * and checked if convertible to complex number. Child processes are created, and process ids and file descriptors saved.
 * Further input (>2) is read from stdin and written to children. Waits on children and reads result from children via pipes.
 * @param argc Number of arguments given
 * @param argv[] Arguments given as string
 * @return EXIT_SUCCESS in case of successful calculation, EXIT_FAILURE otherwise.
*/

int main(int argc, char* argv[]){

    prog_name = argv[0];

    //No arguments should be given
    if(argc > 1){
        usage();
    }

    char *entry1 = NULL;
    char *entry2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;

    //No input given
    if(getline(&entry1, &len1, stdin) == -1){
        error_exit_main(entry1, NULL, "No input given.\n");
    }

    //One input given
    if(getline(&entry2, &len2, stdin) == -1){
        //Checks if input one valid float
        struct complex_value output1;
        if(input_to_complex(entry1, &output1) == -1){
            error_exit_main(entry1, entry2, "Parsing input to complex number failed.\n");
        }
        fprintf(stdout, "%s\n", entry1);
        free(entry1);
        free(entry2);
        exit(EXIT_SUCCESS);
    }
    //Checks if input 2 is valid float
    struct complex_value output2;
    if(input_to_complex(entry2, &output2) == -1){
        error_exit_main(entry1, entry2, "Parsing input to complex number failed.\n");
    }

    int write_even_fd;
    int read_even_fd;
    pid_t pid_even;

    int write_odd_fd;
    int read_odd_fd;
    pid_t pid_odd;

    //Creates child for even indices
    if(create_child_process(&pid_even, &write_even_fd, &read_even_fd) == -1){
        error_exit_main(entry1, entry2, "Failed to create child process for even entries.\n");
    }
    
    //Creates child for odd indices
    if(create_child_process(&pid_odd, &write_odd_fd, &read_odd_fd) == -1){
        close(read_even_fd);
        close(write_even_fd);
        wait_on_children(&pid_even);
        error_exit_main(entry1, entry2, "Failed to create child process for odd entries.\n");
    }

    int n = 0;      //Size of array
    if((n = write_to_children(write_odd_fd, write_even_fd, entry1, entry2)) == -1){
        close(read_even_fd);
        close(read_odd_fd);
        close(write_even_fd);
        close(write_odd_fd);
        wait_on_children(&pid_even);
        wait_on_children(&pid_odd);
        fprintf(stderr, "Child even %d and child odd %d: ", pid_even, pid_odd);
        error_exit_main(entry1, entry2, "Failed to write to child processes.\n");
    }
    
   free(entry1);
   free(entry2);
   close(write_even_fd);
   close(write_odd_fd);

    //Wait for even child
    if(wait_on_children(&pid_even) == -1){
        close(read_even_fd);
        close(read_odd_fd);
        fprintf(stderr, "Child %d: ", pid_even);
        error_exit_main(NULL, NULL, "Child process exited on failure.\n");
    }

    //Wait for odd child
    if(wait_on_children(&pid_odd) == -1){
        close(read_even_fd);
        close(read_odd_fd);
        fprintf(stderr, "Child %d: ", pid_odd);
        error_exit_main(NULL, NULL, "Child process exited on failure.\n");
    }
    
    //Reads result from children and calculates subsequent result
    if(read_from_children(read_odd_fd, read_even_fd, n) == -1){
        close(read_even_fd);
        close(read_odd_fd);
        fprintf(stderr, "Child even %d and child odd %d: ", pid_even, pid_odd);
        error_exit_main(NULL, NULL, "Reading output from children failed.\n");
    }

    close(read_even_fd);
    close(read_odd_fd);
    return EXIT_SUCCESS;
}