/**
 * @file ErrorHandling.h
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief provides function(s) to handle errors
 *
 * failedWithError can be used to handle errors, especially those who set errno
 *
 **/

/**
 * failedWithError, handling errors and notifying the users
 * @brief This function is responsible for general error handling and especially for those which write additional information to errno
 * @param progName program name of the program which throws the error
 * @param description a human-readable description of the problem
 * @param terminate a flag which indicates whether the program has to terminate
 * */
void failedWithError(char* progName, char* description, int terminate);
