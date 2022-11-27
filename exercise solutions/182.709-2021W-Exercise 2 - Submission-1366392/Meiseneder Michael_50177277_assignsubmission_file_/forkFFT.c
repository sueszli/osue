/**
 * @file forkFFT.c
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 25.11.2021
 * @brief Contains the implementation of forkFFT.
 * @details Only the end result will be printed to stdout. One result per line.
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
    char *input_buffer1 = NULL, *input_buffer2 = NULL;      //Pointers to the input buffers. A value will be assigned in read_line().
    unsigned int input_buffer1_lenght = 0, input_buffer2_lenght = 0;    //Stores the current lenght of the corresponding input buffer.
    complex_number num;      //Complex number in which the number which has been read from STDIN will be stored.
    int return_value;   //Stores the return value of functions.
    int finished = 0;       //Flag variable. Will be set if EOF has been read so that no further characters will be read.
    int wait_status;    //Variable which will be passed to wait().
    
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
        if(return_value == -1)        //An error occured
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
        
        close_pipes(fd_write_to_even, fd_read_from_even, NULL, NULL, &exit_value);      //Close pipes created for even child.
        
        wait(&wait_status);        //Wait for one child to terminate.
        
        if(return_value == -1)  //Since a child has been forked wait for the second child to terminate so that no orphan processes will be created. Pipes will be closed by the function.
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
    close_pipes(fd_write_to_even, fd_write_to_uneven, NULL, NULL, &exit_value); //exit_value will be assigned by the function.
    
    complex_number even_array[array_size], uneven_array[array_size], solution_array[2*array_size];   //Array of complex numbers which store the results of the child processes and the endresult.
    
    //Read results from children and calculate FFT
    
    int even_finished = 0, uneven_finished = 0; //Flag which will be set to 1 if reading the results from even / uneven child is finished.
    int even_index = 0, uneven_index = 0;       //Index of the next entry in the array even_array / uneven_array.
    
    while((even_finished == 0 || uneven_finished == 0) && exit_value == EXIT_SUCCESS)
    {
        if(even_finished == 0)
        {
            return_value = read_line(fd_read_from_even, &input_buffer1, &input_buffer1_lenght);     //Read one line from even child
            if(return_value == 0)   //EOF read from stdin
            {
                even_finished = 1;
            }
            else if(return_value == -1)     //Error occured while reading from uneven child.
            {
                exit_value = EXIT_FAILURE;
                break;            
            }
            
            else        //A line has been successfully read.
            {
                if (parse_number_from_string(input_buffer1, &(even_array[even_index])) == -1)   //Parse float from the string which has been read from even child.
                {
                    exit_value = EXIT_FAILURE;
                    break;   
                }
                ++even_index; 
            } 
        }
        
        if(uneven_finished == 0)
        {
            return_value = read_line(fd_read_from_uneven, &input_buffer1, &input_buffer1_lenght);
            if(return_value == 0)
            {
                uneven_finished = 1;
            }
            else if(return_value == -1)
            {
                exit_value = EXIT_FAILURE;
                break;            
            }
            
            else        //A line has been successfully read.
            {
                if (parse_number_from_string(input_buffer1, &(uneven_array[uneven_index])) == -1)
                {
                    exit_value = EXIT_FAILURE;
                    break;   
                }
                ++uneven_index; 
            } 
        }
    }
    
    
    free(input_buffer1);
    free(input_buffer2);
    
    //Close pipes which were used to read from the child processes.
    close_pipes(fd_read_from_even, fd_read_from_uneven, NULL, NULL, &exit_value); 
    
    wait(&wait_status); //Wait for one child to exit.
    if(WEXITSTATUS(wait_status) == EXIT_FAILURE)
    {
        error("Child returned with exit status EXIT_FAILURE");
        exit_value = EXIT_FAILURE;
    }
    
    //wait(&wait_status); //Wait for one child to exit.
    if(WEXITSTATUS(wait_status) == EXIT_FAILURE)
    {
        error("Child returned with exit status EXIT_FAILURE");
        exit_value = EXIT_FAILURE;
    }
    
    if(exit_value == EXIT_FAILURE)
    {
        exit(EXIT_FAILURE);
    }
    
    calculateFFT(even_array, uneven_array, solution_array, array_size);
    
    if(print_array(stdout, solution_array, array_size * 2) == -1)
    {
        exit_value = EXIT_FAILURE;
    }
    
    exit(exit_value);
}

/**
 * @brief Writes an array of complex numbers to the passed file stream.
 * @details After each number a newline character is inserted.
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
        
        if(fprintf(fd, "%f %f*i\n", re, im) < 1)
        {
            error("Error writing to file");   
            return -1;
        }  
    }
    fflush(fd);
    
    return 0;
}