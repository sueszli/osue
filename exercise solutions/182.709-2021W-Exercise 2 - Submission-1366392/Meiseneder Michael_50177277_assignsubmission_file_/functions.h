/**
 * @file functions.h
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 25.11.2021
 * @brief Contains prototypes of functions used in forkFFT.c and forkFFT_output_to_tree.c.
 **/

#ifndef FUNCTIONS_H__
#define FUNCTIONS_H__

/**
 * @brief Calculates the intermediate result of the the Cooley-Tukey Fast Fourier Transform algorithm.
 * @details Uses library math.h for calculating sinus and cosinus.
 * @param even Even Array with complex numbers.
 * @param uneven Uneven Array with complex numbers.
 * @param result Array in which the result of the calculation will be stored.
 * @param size Size of the even and uneven array.
 */
void calculateFFT(const complex_number *even, const complex_number *uneven, complex_number *result, int size);

/**
 * @brief Adds two complex numbers.
 * @brief The result will be stored in result and will also be returned by the function.
 * @param num1 One part of the sum.
 * @param num2 other part of the sum.
 * @param result Result of the calculation.
 * @return Pointer to the result of the calculation.
 */
complex_number *add(const complex_number *num1, const complex_number *num2, complex_number *result);

/**
 * @brief Subtracts two complex numbers.
 * @brief The result will be stored in result and will also be returned by the function.
 * @param num1 Minuend.
 * @param num2 Subtrahend.
 * @param result Result of the calculation.
 * @return Pointer to the result of the calculation.
 */
complex_number *subtract(const complex_number *num1, const complex_number *num2, complex_number *result);

/**
 * @brief Multiplies two complex numbers.
 * @brief The result will be stored in result and will also be returned by the function.
 * @param num1 One part of the product.
 * @param num2 Other part of the product.
 * @param result Result of the calculation.
 * @return Pointer to the result of the calculation.
 */
complex_number *multiply(const complex_number *num1, const complex_number *num2, complex_number *result);

/**
 * @brief Usage function.
 * @details Uses global variable program_name.
 * @return void.
 **/
void usage(void);

/**
 * @brief Parses an complex number from the passed string.
 * @details The a complex number consists of a real an imaginary part. The number may only contain the real or imaginary part. If the number contains both real part and imaginary part are separated by a blank character. The imaginary part is followed by "*i" .
 * @details The string may contain multiple blank spaces ahead of the real and imaginary part of the complex number.
 * @param str String which holds the complex number which should be parsed.
 * @param num Pointer to a complex number in which the parsed complex number will be stored.
 * @return 0 if the number has been parsed successfully. -1 if an error occured.
 **/
int parse_number_from_string(const char *str, complex_number *num);

/**
 * @brief Error function.
 * @brief Prints an error message containing the program name and an error message to stderr.
 * @details Program name is stored in gloabl variable program_name.
 * @param reason String which is contained in the error message.
 * @return void.
 */
void error(const char *reason);

/**
 * @brief Error function.
 * @brief Prints an error message containing the program name and an error message to stderr.
 * @details Program name is stored in gloabl variable program_name.
 * @details The error message also contains the to errno corresponding error message.
 * @param reason String which is contained in the error message.
 * @return void.
 */
void error_with_errno(const char *reason);

/**
 * @brief Closes the pipe referenced by the passed file stream.
 * @details fclose() will only be called if the passed *FILE struct is not NULL,
 * @param fd *FILE struct referencing the pipe which should be closed.
 * @return -1 if an error occured; 0 if the call to fclose() was successfull or if NULL has been passed.
 */
int close_pipe(FILE *fd);

/**
 * @brief Closes the pipe referenced by the passed file descriptor.
 * @details fclose() will only be called if the passed file descriptor is not -1.
 * @param fd file descriptor referencing the pipe, which should be closed.
 * @return -1 if an error occured; 0 if the call to close() was successfull or if NULL has been passed.
 */
int close_pipe_with_fd(int fd);

/**
 * @brief Closes multiple pipes referenced by the passed file streams.
 * @details Calls close_pipe() to close the pipe.
 * @param fd1 *FILE struct of one of the pipes which should be closed.
 * @param fd2 *FILE struct of one of the pipes which should be closed.
 * @param fd3 *FILE struct of one of the pipes which should be closed.
 * @param fd4 *FILE struct of one of the pipes which should be closed.
 * @return -1 if an error occured when closing at least one of the pipes; 0 if all calls to fclose() were successfull.
 */
int close_pipes(FILE *fd1, FILE *fd2, FILE *fd3, FILE *fd4, int *exit_value);

/**
 * @brief Closes multiple pipes referenced by the passed file descriptors.
 * @details Calls close_pipe_with_fd() to close the pipe.
 * @param fd1 File descriptor of the one file which should be closed.
 * @param fd2 File descriptor of the one file which should be closed.
 * @param fd3 File descriptor of the one file which should be closed.
 * @param fd4 File descriptor of the one file which should be closed.
 * @return -1 if an error occured when closing at least one of the pipes; 0 if all calls to close() were successfull.
 */
int close_pipes_with_fd(int fd1, int fd2, int fd3, int fd4, int *exit_value);

/**
 * @brief Forks a child and replaces the process image using execlp() to the process image of the calling process.
 * @brief STDIN / STDOUT of the child will be redirected to the read / write end of a pipe. A file stream to the other end of the pipe will be stored in a pointer passed to the function.
 * @details Process image of the child will be replaced . Therefore the function only returs at the calling parent process. The function only returns in the child process if an error occured.
 * @details All unused read / write ends of the pipes will be closed.
 * @details Uses global variable program_name to call execlp() with the correct name of the executable.
 * @param fd_pipe_stdin Pointer to the file stream which can be used to write to the stdin of the child process.
 * @param fd_pipe_stdout Pointer to the file stream which can be used to read from the stdout of the child process.
 * @return -2 if forking failed; -1 if an other error occured throughout the function call; 0 otherwise.
 */
int fork_child(FILE **fd_pipe_stdin, FILE **fd_pipe_stdout);

/**
 * @brief pass_characters() passes characters read from fd_read_from directly to fd_write_to without the need of a buffer.
 * @brief One line of characters will be passed in one call. The function terminates after passing the new line character or if EOF has been reached.
 * @details The advantage of this function in comparison to read_line() and write_line() is that no buffer is needed.
 * @param fd_read_from File stream from which the characters should be read.
 * @param fd_write_to File stream to which the characters should be written.
 * @param number_of_passed_chars Pointer to an int variable in which the number of passed characters from fd_read_from will be written.
 * @return 0 if EOF was reached; -1 if an error occured; 1 if the line has been passed successfully.
 */
int pass_characters(FILE *fd_read_from, FILE *fd_write_to, int *number_of_passed_chars);

/**
 * @brief Reads one line from the passed file stream and stores it in the passed buffer.
 * @details The size of the buffer will be increased by realloc(). The corresponding buffer size will be increased as well.
 * @details A complete line written to the buffer will be terminated with a null terminator.
 * @param fd File stream of the file from which a line should be read.
 * @param target_buffer Pointer to a buffer to which the line should be written. The value of the pointer must either bei a valid pointer returned by realloc() or NULL.
 * @param buffer_size Pointer to an int variable which holds the number of elements in the buffer.
 * @return -1 if an error occured; 0 if EOF was read; > 0 if a line has been successfully read.
 */
int read_line(FILE *fd, char **target_buffer, unsigned int *buffer_size);

/**
 * @brief Writes the passed string to the passed file stream.
 * @brief The trailing new line character must be contained in the passed string.
 * @param fd File stream to which should be written.
 * @param str The string which should be written.
 * @return 0 if the string has successfully been written to the stream; -1 if an error occured.
 */
int write_line(FILE *fd, const char *str);

#endif //FUNCTIONS_H__
