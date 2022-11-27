/**
 * @file forkFFT_output_to_tree.c
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 25.11.2021
 * @brief Contains the implementation of forkFFT with the bonus exercise implemented..
 * @details If the output consists of less than 4 numbers the output will be formatted as a tree. Otherwise only the end result will be printed.
 * @details In contrast to the original implementation in forkFFT.c the complex numbers of the result will be printed in one line.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include "forkFFT.h"
#include "functions.h"

static int print_array(FILE *fd, const complex_number *num, int num_entries);
static int insert_spaces(FILE *fd, int num);
static int read_result_from_child(FILE *fd, int count, complex_number *array);

char *program_name;     //Pointer to program name stored in argv[0].

/**
 * @brief Program entry point.
 * @brief Reads in complex numbers from STDIN and calculates the FFT from these numbers using the Cooley-Tukey Fast Fourier Transform algorithm.
 * @details The recursion of the algorithm is implemented by forking the program.
 * @details Global variable program_name will be set to argv[0].
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return EXIT_FAILURE if an error occured; EXIT_SUCCESS otherwise.
 **/
int main(int argc, char **argv)
{      
    
    program_name = argv[0];
    
    int exit_value = EXIT_SUCCESS;      //Holds the exit status which is returned to the calling process. Will be set to EXIT_FAILURE if an error occures.
    char *input_buffer1 = NULL, *input_buffer2 = NULL;  //Pointers to the input buffers. A value will be assigned in read_line().
    unsigned int input_buffer1_lenght = 0, input_buffer2_lenght = 0;    //Lenght of the corresponding input buffer.
    complex_number num;      //Complex number in which the number which has been read from STDIN will be stored.
    int return_value;   //Stores the return value of functions.
    int finished = 0;       //Flag variable. Will be set if EOF has been read so that no further characters will be read.
    int wait_status;    //Variable which will be passed to wait.
    
    num.re = 0;         //Initialize num. This is not necessary for the program itself but valgrind sometimes outputs an error if num is not initialized.
    num.im = 0;
    
    if(argc > 1)        //Check if argv contains any arguments.
    {
        usage();
        exit(EXIT_FAILURE);
    }
    
    //Read the first line from STDIN into input_buffer1. The first line must be read into a buffer because at this point it cannot be determined yet, if the line should be passed to a child or if the line should be printed to stdout.
    if((return_value = read_line(stdin, &input_buffer1, &input_buffer1_lenght)) <= 0)
    {
        if(return_value == -1)        
        {
            free(input_buffer1);       
            exit(EXIT_FAILURE);
        }
        
        //return_value is 0.
        
        if(input_buffer1[0] != '\n')    //If a complex number was read before EOF.
        { 
            if(parse_number_from_string(input_buffer1, &num) == -1)     //Convert the number in input_buffer1 to a complex_number. If -1 is returned an error occured.
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && print_array(stdout, &num, 1) == -1)        //Print the (only) line read to STDOUT and exit the program.
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
        }
        else    //EOF was the first character, which was read.
        {
            error("No line read");
            exit_value = EXIT_FAILURE;
        }
        
        free(input_buffer1);
        exit(exit_value); 
    }
    
    //Read the second line from STDIN into input_buffer2.
    switch(return_value = read_line(stdin, &input_buffer2, &input_buffer2_lenght))
    {
        case -1:        //An error occured while reading from STDIN -> Exit the program.
            free(input_buffer1);
            free(input_buffer2);
            exit(EXIT_FAILURE);
            break;
            
        case 0: //EOF was read.
            
            finished = 1;   //Set flag to one so that the following while loop will not be executed.
            
            if(input_buffer2[0] == '\n')    //EOF has been read at the beginning of the line -> Only one complex number has been read.
            {
                if(parse_number_from_string(input_buffer1, &num) == -1)     //Convert the number in input_buffer1 to a complex_number. If -1 is returned an error occured.
                {
                    exit_value = EXIT_FAILURE;
                }
                
                if(exit_value != EXIT_FAILURE && print_array(stdout, &num, 1) == -1)        //Print the (only) line read to STDOUT and exit the program.
                {
                    exit_value = EXIT_FAILURE;  
                }
                
                if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == -1)
                {
                    exit_value = EXIT_FAILURE;
                }
                
                free(input_buffer1);
                free(input_buffer2);
                exit(exit_value);            
            }
            
            break;
            
        default:    //More than one line read -> Continue to read lines.
            break;
            
    }
    
    //More than one line read -> fork
    
    FILE *fd_read_from_even, *fd_read_from_uneven, *fd_write_to_even, *fd_write_to_uneven;   //File streams for the pipes to read/write from/to the child processes.
    
    if((return_value = fork_child(&fd_write_to_even, &fd_read_from_even)) < 0)  //fork_child() forks a child and sets the passed file streams. Returns -1 if an error occured.
    {
        free(input_buffer1);
        free(input_buffer2);
        
        if(return_value == -1)  //Since a child has been forked wait for the child to terminate so that no orphan processes will be created. Pipes will be closed by the function.
        {
            wait(&wait_status);
        }
        
        exit(EXIT_FAILURE);   
    }
    
    if((return_value = fork_child(&fd_write_to_uneven, &fd_read_from_uneven)) < 0)  //fork_child() forks a child and sets the passed file streams. Returns -1 if an error occured.
    {
        free(input_buffer1);
        free(input_buffer2);
        
        close_pipes(fd_write_to_even, fd_read_from_even, NULL, NULL, &exit_value);
        
        wait(&wait_status);        //Wait for even child to terminate.
        
        if(return_value == -1)  //Since a child has been forked wait for the child to terminate so that no orphan processes will be created. Pipes will be closed by the function.
        {
            wait(&wait_status);
        }
        
        exit(EXIT_FAILURE);   
    }
    
    //Write first received line to even child
    if(write_line(fd_write_to_even, input_buffer1) == -1)
    {
        exit_value = EXIT_FAILURE;
    }
    
    //Write second received line to uneven child
    if(exit_value == EXIT_SUCCESS && write_line(fd_write_to_uneven, input_buffer2) == -1)
    {
        exit_value = EXIT_FAILURE;
    }
    
    free(input_buffer1);
    free(input_buffer2);
    
    int array_size = 1;   //Size of the array which will hold the number of complex numbers which will be read from one child. Value 1 because one number has alredy been sent to each child.
    int number_of_chars_passed; //Used to pass to pass_characters().
    
    while(exit_value == EXIT_SUCCESS && finished == 0)
    {
        return_value = pass_characters(stdin, fd_write_to_even,  &number_of_chars_passed); //pass_characters() directly passes one line from stdin to fd_write_to_even without the need of a buffer.
        
        if(return_value == 0)   //pass_characters() returs 0 if EOF has been read from STDIN.
        {
            if(number_of_chars_passed > 0)      //If a complex number has been read in the same line as EOF -> An uneven number of values has been read.
            {
                error("Read an invalid amount of numbers");
                exit_value = EXIT_FAILURE;  
            }
            
            break;
        }
        else if(return_value == -1)     //An error occured.
        {
            exit_value = EXIT_FAILURE;
            break;
        }
        
        return_value = pass_characters(stdin, fd_write_to_uneven, &number_of_chars_passed);
        
        if(return_value == 0)   //EOF read from stdin.
        {            
            if(number_of_chars_passed == 0)     //If EOF is the first character in the line an uneven number of complex numbers has been read.
            {
                error("Read an invalid amount of numbers");
                exit_value = EXIT_FAILURE;  
            }
            ++array_size;
            break;
            
        }
        else if(return_value == -1)     //An error occured.
        {
            exit_value = EXIT_FAILURE;
            break;
        }
        ++array_size;
    }
    
    //Close pipes which are used to write to the child processes.
    close_pipes(fd_write_to_even, fd_write_to_uneven, NULL, NULL, &exit_value);         //exit_value will be assigned by the function.
    
    complex_number even_array[array_size], uneven_array[array_size], solution_array[2*array_size];   //Array of complex numbers which store the results of the child processes and the end result.
    
    //Read first line from each child. This line contains the result of the calculation, which has been carried out by the child.  
    
    if(read_result_from_child(fd_read_from_even, array_size, even_array) == -1) //Read one line from even child. This line contains the result from its calculation.
    {
        exit_value = EXIT_FAILURE;  
    }
    
    if(read_result_from_child(fd_read_from_uneven, array_size, uneven_array) == -1)      //Read one line from uneven child. This line contains the result from its calculation.
    {
        exit_value = EXIT_FAILURE;  
    }
    
    if(exit_value != EXIT_FAILURE)
    {
        
        calculateFFT(even_array, uneven_array, solution_array, array_size);
        
        if(array_size*2 <= 4)   //Only print a tree if the result contains up to 4 complex numbers. Larger trees would be unreadable.
        {
            
            if(insert_spaces(stdout, 10*array_size) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            //Print solution calculated by this node.
            if(exit_value != EXIT_FAILURE && print_array(stdout, solution_array, array_size * 2) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && insert_spaces(stdout, 20*array_size+5) == -1)
            {
                exit_value = EXIT_FAILURE;    
            }
            
            if(exit_value != EXIT_FAILURE && fputc('/', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && insert_spaces(stdout, 20*array_size+5) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\\', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && insert_spaces(stdout, 4*array_size+5) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            //Print solution calculated by even child.
            if(exit_value != EXIT_FAILURE && print_array(stdout, even_array, array_size) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && insert_spaces(stdout, 8*array_size+5) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            //Print solution calculated by uneven child.
            if(exit_value != EXIT_FAILURE && print_array(stdout, uneven_array, array_size) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
            
            
            char c;     //Holds the character read by fgetc().
            
            //Print further lines received from child processes to stdout.
            while(feof(fd_read_from_even) == 0 && feof(fd_read_from_uneven) == 0 && exit_value != EXIT_FAILURE)
            {
                
                while(exit_value != EXIT_FAILURE)        //Read one line from even child.
                {
                    c = fgetc(fd_read_from_even);
                    
                    if(c == EOF)
                    {
                        if(ferror(fd_read_from_even) != 0)
                        {
                            error("Error reading from child");
                            exit_value = EXIT_FAILURE;
                        }
                        break;
                    }
                    else if(c == '\n')
                    {
                        break;
                    }
                    
                    else        //A valid character other than \n received -> Print to stdout.
                    {
                        if(fputc(c, stdout) == EOF)
                        {
                            error("Error writing to file");
                            exit_value = EXIT_FAILURE;
                        }
                    }
                }
                
                while(exit_value != EXIT_FAILURE)        //Read one line from uneven child.
                {
                    c = fgetc(fd_read_from_uneven);
                    
                    if(c == EOF)
                    {
                        if(ferror(fd_read_from_even) != 0)
                        {
                            error("Error reading from child");
                            exit_value = EXIT_FAILURE;
                        }
                        break;
                    }
                    
                    else if(c == '\n')
                    {
                        if(fputc('\n', stdout) == EOF)  //Here also print '\n'.
                        {
                            error("Error writing to file");
                            exit_value = EXIT_FAILURE;
                        }
                        break;
                    }
                    
                    else
                    {
                        if(fputc(c, stdout) == EOF)
                        {
                            error("Error writing to file");
                            exit_value = EXIT_FAILURE;
                        }
                    }
                }
            } 
        }
        
        else    //Result is to long to be printed as a tree.
        {
            if(print_array(stdout, solution_array, array_size * 2) == -1)
            {
                exit_value = EXIT_FAILURE;
            }
            
            if(exit_value != EXIT_FAILURE && fputc('\n', stdout) == EOF)
            {
                exit_value = EXIT_FAILURE;
            }
        }
        
        fflush(stdout);
    }
    
    //Close pipes which were used to read from the child processes.
    close_pipes(fd_read_from_even, fd_read_from_uneven, NULL, NULL, &exit_value);       //Function sets exit_value.
    
    wait(&wait_status); //Wait for one child to exit.
    if(WEXITSTATUS(wait_status) == EXIT_FAILURE)
    {
        error("Child returned with exit status EXIT_FAILURE");
        exit_value = EXIT_FAILURE;
    }
    
    wait(&wait_status); //Wait for one child to exit.
    if(WEXITSTATUS(wait_status) == EXIT_FAILURE)
    {
        error("Child returned with exit status EXIT_FAILURE");
        exit_value = EXIT_FAILURE;
    }
    exit(exit_value);
}

/**
 * @brief Writes an array of complex numbers to the passed file stream.
 * @param fd File stream to which the array will be written.
 * @param num Array of complex numbers.
 * @param num_entries Number of entries in the array.
 * @return -1 if an error occured; 0 otherwise.
 */
static int print_array(FILE *fd, const complex_number *num, int num_entries)
{
    for(int i = 0; i < num_entries; ++i)
    {        
        float re = num[i].re, im = num[i].im;     //Variable holds the real part / imaginary part of the complex number, which will be written to the file stream.
        
        if(fprintf(fd, "%f %f*i", re, im) < 1)
        {
            error("Error writing to file");   
            return -1;
        }
        
        if(i+1 < num_entries)   //Insert a blank space after the complex number iff the last printed number is not the last number of the array.
        {
            if(fprintf(fd, " ") < 1)
            {
                error("Error writing to file");   
                return -1;
            }
        }
    }
    fflush(fd);
    
    return 0;
}

/**
 * @brief Inserts a defined amount of space characters to a file stream.
 * @param fd File stream to which space characters should be written to.
 * @param num Number of space characters which should be inserted.
 * @return -1 if an error occured while writing to the file stream; 0 otherwise.
 */
static int insert_spaces(FILE *fd, int num)
{
    for(int i = 0; i< num; ++i)
    {
        if(fputc(' ', fd) == EOF)
        {
            error("Error writing to file");
            return -1;
        }
    }
    fflush(fd);
    return 0;
}
/**
 * @brief Reads a line from a file stream which is interpreted as the result from a child process. The result will be parsed as a complex number and written to an array.
 * @details The line may contain an unspecified amount of space characters before the first complex number.
 * @details After the last complex number a newline character has to follow without any other characters between.
 * @details The complex number must consist of a real and imaginary part.
 * @param fd File stream from which should be read.
 * @param count Number of complex numbers which is expeced to be read from the child.
 * @param array Array to which the parsed complex numbers whill be written.
 * @return -1 if an error occured; 0 otherwise.
 */
static int read_result_from_child(FILE *fd, int count, complex_number *array)
{
    int i = 0;  //Index for writing to the buffer.
    int flag = 0;       //Flag variable. Has value 0 if the next number read from the stream is the real part of a complex number; 1 if the next number read is the imaginary part of the complex number.
    int num_values_read = 0;    //Holds the number of complex number which have already been read from the child.
    int buffer_size = 10;       //Size of the buffer; Initial value is 10
    int remove_spaces = 1;      //Flag variable. Has value 1 if space characters read from fd should be skipped. This is to skip space characters in the output of the child at the beginning of the line which are used to draw the tree.
    char *buffer = malloc(buffer_size); //Buffer in which the line will be written.
    char c;     //Character returned by fgetc().
    char *strtof_endptr;        //Passed to strfof().
    
    if(buffer == NULL)
    {
        error_with_errno("malloc() failed");
        return -1;
    }
    
    fflush(fd);
    
    while(1)
    {
        c = fgetc(fd);
        
        if(remove_spaces == 1 && c == ' ')      //Skip spaces before the real or imaginary part.
        {
            continue;
        }
        remove_spaces = 0;
        
        
        if(c == EOF)    //In this case EOF was read because of an error or if the child terminated early because of an error.
        {
            if(ferror(fd) != 0)
            {
                error("Error reading from file");
            }
            
            free(buffer);
            return -1;
        }
        
        else
        {
            buffer[i++] = c;
            
            if(i >= buffer_size)
            {
                buffer_size = buffer_size * 2;
                buffer = realloc(buffer, buffer_size);
                if(buffer == NULL)
                {
                    error_with_errno("malloc() failed");
                    return -1;
                }
            }
            
            if(c == ' ' || c == '*')    // If a space character or * is read the buffer holds either the complete real or imaginary part.
            {
                if(flag == 0)   //The number read is the real part.
                {
                    flag = 1;
                    array[num_values_read].re = strtof(buffer, &strtof_endptr);
                    
                    if(*strtof_endptr != ' ')
                    {
                        error("Wrong syntax; space expected after real part");
                        free(buffer);
                        return -1;
                    }
                    i = 0;
                }
                else    //The number read is the imaginary part.
                {
                    
                    flag = 0;
                    array[num_values_read].im = strtof(buffer, &strtof_endptr);
                    
                    if(*strtof_endptr != '*')   //The imaginary part must be followed by a '*'
                    {
                        error("Wrong syntax; * expected");
                        free(buffer);
                        return -1;
                    }
                    
                    if(fgetc(fd) != 'i')        //After '*' 'i' must be read from the stream.
                    {
                        error("Wrong syntax; i expected");
                        free(buffer);
                        return -1;
                    }
                    
                    switch(fgetc(fd))   //Check which character is read after a complete complex number.
                    {
                        case '\n':      //'\n' means that all numbers from the line have been read.
                            
                            if(num_values_read+1 != count)      //If an invalid number of complex numbers has been read from the child.
                            {
                                error("Read an invalid number of results from child");
                                free(buffer);
                                return -1;
                            }
                            else        //The expected number of values has been read.
                            {
                                free(buffer);
                                return 0;
                            }
                            break;
                            
                        case ' ':       //The file contains an other complex number in the line.
                            
                            break;
                            
                        default:
                            error("Invalid syntax");
                            free(buffer);
                            return -1;
                            break;
                            
                    }
                    
                    i=0;
                    ++num_values_read;
                    
                    if(num_values_read > count)
                    {
                        error("Read an invalid number of results from child");
                        free(buffer);
                        return -1;   
                    }
                    
                    remove_spaces = 1;
                }
            }
        }  
    } 
}