/**
 * @file functions.c
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 25.11.2021
 * @brief Contains functions used in forkFFT.c and forkFFT_output_to_tree.c
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

extern char* program_name;      //Declaration of a gloabl variable which holds the value of argv[0]. Will be assigned in main().

void calculateFFT(const complex_number *even, const complex_number *uneven, complex_number *result, int size)
{
    complex_number sin_cos_coefficient, temp_result;    //Complex numbers holding an intermediate result of the calculation.
    
    for(int i = 0; i < size; ++i)
    {    
        sin_cos_coefficient.re = cos(-1.0*PI * i / size);       //Since size corresponds to n/2 in the formula given no *2 is needed.  
        sin_cos_coefficient.im = sin(-1.0*PI * i / size);
        
        multiply(&sin_cos_coefficient, &uneven[i], &temp_result);
        
        add(&even[i], &temp_result, &result[i]);
        subtract(&even[i], &temp_result, &result[i+size]);
    }
}

complex_number *add(const complex_number *num1, const complex_number *num2, complex_number *result)
{
    result->re = num1->re + num2->re;
    result->im = num1->im + num2->im;
    return result;
}

complex_number *subtract(const complex_number *num1, const complex_number *num2, complex_number *result)
{
    result->re = num1->re - num2->re;
    result->im = num1->im - num2->im;
    return result;
}

complex_number *multiply(const complex_number *num1, const complex_number *num2, complex_number *result)
{
    result->re = num1->re * num2->re - num1->im * num2->im;
    result->im = num1->re * num2->im + num1->im * num2->re;
    return result;
}

void usage(void)
{
    fprintf(stderr, "USAGE: %s\n", program_name);
    fflush(stderr);
}

int parse_number_from_string(const char *str, complex_number *num)
{    
    char *strtof_endptr;        //Endpointer which will be set by strtof() (see strtof(3) ).
    
    num->re = strtof(str, &strtof_endptr);
    num->im = 0;
    
    if(errno == ERANGE)
    {
        error_with_errno("Number overflows / underflows");
        return -1;
    }
    
    if(strtof_endptr == NULL)
    {
        error("Invalid syntax");
        return -1; 
    }
    
    if(*strtof_endptr == '\n')  //If the real part is followed by '\n' the complex number only consists of the real part.
    {
        return 0;
    }
    
    if(*strtof_endptr == '*' && *(strtof_endptr+1) == 'i' && *(strtof_endptr+2) == '\n')   //If the number parsed is the imaginary part and not the real part (number only consists of an imaginary part).
    {
        num->im = num->re;
        num->re=0;
        return 0;
    }
    
    if(*strtof_endptr != ' ')   //The real part may only be followed by a blank space.
    {
        error("Invalid syntax");
        return -1;
    }
    
    str = strtof_endptr + 1;
    
    num->im = strtof(str, &strtof_endptr);
    
    if(errno == ERANGE)
    {
        error_with_errno("Number overflows / underflows");
        return -1;
    }
    
    if(*(strtof_endptr) == '*' && *(strtof_endptr+1) == 'i' && *(strtof_endptr+2) == '\n')   //Check if the imaginary part is followed by "*i" and \n.
    {
        return 0;
    }
    error("Invalid syntax");
    return -1;
}

void error(const char *reason)
{
    fprintf(stderr, "ERROR: %s: %s.\n", program_name, reason);
    fflush(stderr);
}

void error_with_errno(const char *reason)
{
    fprintf(stderr, "ERROR: %s: %s.; %s\n", program_name, reason, strerror(errno));
    fflush(stderr);
}

int close_pipe(FILE *fd)
{
    if(fd != NULL && fclose(fd) == EOF)
    {       
        error_with_errno("Closing of pipe failed");
        return -1;
    }
    return 0;
}

int close_pipe_with_fd(int fd)
{
    if(fd != -1 && close(fd) == -1)
    {       
        error_with_errno("Closing of pipe failed");
        return -1;
    }
    return 0;
}

int close_pipes(FILE *fd1, FILE *fd2, FILE *fd3, FILE *fd4, int *exit_value)
{
    int return_value = 0;       //Holds the value which will be returned by the function. Will be set to -1 if an error occures.
    
    if(close_pipe(fd1) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe(fd2) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe(fd3) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe(fd4) == -1)
    {
        return_value = -1;
    }
    
    if(return_value == -1 && exit_value != NULL)
    {
        *exit_value = EXIT_FAILURE;
    }
    
    return return_value;
}

int close_pipes_with_fd(int fd1, int fd2, int fd3, int fd4, int *exit_value)
{
    int return_value = 0;       //Holds the value which will be returned by the function. Will be set to -1 if an error occures.
    
    if(close_pipe_with_fd(fd1) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe_with_fd(fd2) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe_with_fd(fd3) == -1)
    {
        return_value = -1;
    }
    
    if(close_pipe_with_fd(fd4) == -1)
    {
        return_value = -1;
    }
    
    if(return_value == -1 && exit_value != NULL)
    {
        *exit_value = EXIT_FAILURE;
    }
    
    return return_value;
}

int fork_child(FILE **fd_pipe_stdin, FILE **fd_pipe_stdout)
{
    int pipe_stdin[2], pipe_stdout[2];  //Arrays which will be passed to pipe() to create a pipe.
    
    if(pipe(pipe_stdin) == -1)
    {
        error_with_errno("Error creating pipe");
        return -1;
    }
    
    if(pipe(pipe_stdout) == -1)
    {
        error_with_errno("Error creating pipe");
        close_pipes_with_fd(pipe_stdin[0], pipe_stdin[1], -1, -1, NULL);
        return -1;
    }
    
    pid_t pid;  //Holds the return value of fork().
    
    pid = fork();
    if(pid == -1)
    {
        error_with_errno("Forking failed");
        close_pipes_with_fd(pipe_stdin[0], pipe_stdin[1], pipe_stdout[0], pipe_stdout[1], NULL);
        return -2;
    }
    else if(pid > 0)
    {
        //Code executed by parent.
        
        if(close_pipe_with_fd(pipe_stdin[0]) == -1)
        {
            close_pipes_with_fd(-1, pipe_stdin[1], pipe_stdout[0], pipe_stdout[1], NULL);
            return -1;
        }
        if(close_pipe_with_fd(pipe_stdout[1]) == -1)
        {
            close_pipes_with_fd(pipe_stdin[1], pipe_stdout[0], -1, -1, NULL);
            return -1;  
        }
        
        *fd_pipe_stdin = fdopen(pipe_stdin[1], "w");
        *fd_pipe_stdout = fdopen(pipe_stdout[0], "r");
        
        if(*fd_pipe_stdin == NULL || *fd_pipe_stdout == NULL)
        {
            error("fdopen() failed");
            
            close_pipes(*fd_pipe_stdin, *fd_pipe_stdout, NULL, NULL, NULL);     //If only one fdopen() failed close the other pipe. 
            return -1;
        }
        
        return 0;  
    }
    else
    {
        //Code executed by child.
        
        if(close_pipe_with_fd(pipe_stdin[1]) == -1)
        {
            close_pipes_with_fd(pipe_stdin[0], -1, pipe_stdout[0], pipe_stdout[1], NULL);
            return -1;
        }
        if(close_pipe_with_fd(pipe_stdout[0]) == -1)
        {
            close_pipes_with_fd(pipe_stdin[0], -1, -1, pipe_stdout[1], NULL);
            return -1;
        }
        
        if(dup2(pipe_stdin[0],STDIN_FILENO) == -1)
        {
            error_with_errno("dup2() failed");
            close_pipes_with_fd(pipe_stdin[0], -1, -1, pipe_stdout[1], NULL);
            return -1;
        }
        
        if(close_pipe_with_fd(pipe_stdin[0]) == -1)
        {
            close_pipe_with_fd(pipe_stdout[1]);
            return -1;
        }
        
        if(dup2(pipe_stdout[1],STDOUT_FILENO) == -1)
        {
            error_with_errno("dup2() failed");
            close_pipe_with_fd(pipe_stdout[1]);
            return -1;
        }
        
        if(close_pipe_with_fd(pipe_stdout[1]) == -1)
        {
            return -1;
        }
        
        execlp(program_name, program_name, NULL);
        
        error_with_errno("execlp() failed");
        return -1;
    } 
}

int pass_characters(FILE *fd_read_from, FILE *fd_write_to, int *number_of_passed_chars)
{
    int i = 0;  //Holds the number of characters which have been read from fd_read_from.
    char c;     //Holds the character read by fgetc().
    fflush(fd_read_from);
    do
    { 
        c = fgetc(fd_read_from);
        
        if(c == EOF)
        {
            if(feof(fd_read_from) != 0)
            {
                if(i != 0)      //If a complex number has been read in the same line as EOF -> Write '\n' to the child.
                {
                    if(fputc('\n', fd_write_to) == EOF)
                    {
                        error("Error writing to file");
                        return -1;
                    }
                    fflush(fd_write_to);
                }
                
                *number_of_passed_chars = i;
                return 0;
            }
            
            //If EOF is not reached fgetc() returned EOF because of an error.
            
            error("Error reading from file");
            return -1;
        }
        
        if(fputc(c, fd_write_to) == EOF)
        {
            error("Error writing to file");
            return -1;
        }
        ++i;
        
    }while(c != '\n');  //Pass characters until '\n' was passed.
    
    fflush(fd_write_to);
    
    *number_of_passed_chars = i;
    return 1;
}

int read_line(FILE *fd, char **target_buffer, unsigned int *buffer_size)
{
    int i = 0;  //Next index of target_buffer to which should be written to.
    char c;     //Stores the character read by fgetc().
    do
    {
        c = fgetc(fd);
        
        if(c == EOF)     //If read() returned with -1.
        {
            if(feof(fd) != 0)   //EOF reached. Write '\n' and '\0' to the string.
            {
                
                if(*buffer_size <= i)      //If the buffer is already full.
                {
                    if((*target_buffer = realloc(*target_buffer, sizeof(char) * (++(*buffer_size)))) == NULL)   //Increase buffer size by 1.
                    {
                        error_with_errno("realloc() failed");
                        return -1;
                    }
                }
                
                (*target_buffer)[i++] = '\n';         //Write a newline character to the string.
                
                
                
                if(*buffer_size <= i)      //If the buffer is already full.
                {
                    if((*target_buffer = realloc(*target_buffer, sizeof(char) * (++(*buffer_size)))) == NULL)   //Increase buffer size by 1.
                    {
                        error_with_errno("realloc() failed");
                        return -1;
                    }
                }
                
                (*target_buffer)[i] = '\0';         //Null Termination of string.
                
                return 0;
            }
            
            //If EOF has not been reached fgetc() returns -1 because of an error.
            
            error("Error reading from file");
            return -1;
        }
        
        if(*buffer_size <= i)  //If the buffer is already full.
        {                       
            *buffer_size = (*buffer_size)*2+1;    //+1 needed because  *buffer_size can be zero.             
            if((*target_buffer = realloc(*target_buffer, (*buffer_size))) == NULL)
            {              
                error_with_errno("realloc() failed");
                return -1;
            }
        }
        
        (*target_buffer)[i] = c;
        
        
    } while((*target_buffer)[i++] != '\n');      //Read characters from the file until newline character is read.
    
    if(*buffer_size <= i)      //If the buffer is already full.
    {
        if((*target_buffer = realloc(*target_buffer, sizeof(char) * (++(*buffer_size)))) == NULL)   //Increase buffer size by 1.
        {
            error_with_errno("realloc() failed");
            return -1;
        }
    }
    
    (*target_buffer)[i++] = '\0';         //Null Termination of string.
    
    return i;
}

int write_line(FILE *fd, const char *str)
{
    if(fputs(str, fd) == EOF)
    {
        error("Error writing to file");
        return -1;
    }
    fflush(fd);
    return 0; 
}