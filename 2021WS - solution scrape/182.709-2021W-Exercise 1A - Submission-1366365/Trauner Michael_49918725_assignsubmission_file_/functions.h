/**
 * @file functions.h
 * @author Michael Trauner <e12019868@student.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Provides functions for the run-encoding and encoding and counting digits of an integer
 *
 */

/**
 * Count digits of an integer
 * @brief This functions counts the number of digits of a given integer
 * @param n The integer of which the digits are counted.
 */
int countDigits(int n);

/**
 * Run-length encoder
 * @brief Reads from a given input_file, run-length encodes the read data and returns the encoded data in output
 * and the number of read characters in input_size
 * @param prog_name Name of the program that is executed (used for throwing errors)
 * @param input_file File from which data is read
 * @param output The output string where the already processed data is stored
 * @param out_index The index of the termination character of the string(\0)
 * @param input_size Length of the read data
 * @return Returns the output string where the new processed data is stored
 */
char* runlengthEncode(char *prog_name, FILE *input_file, char *output, int out_index, int *input_size);