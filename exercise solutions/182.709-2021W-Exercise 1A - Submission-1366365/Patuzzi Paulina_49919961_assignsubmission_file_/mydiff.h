/**
 * @file mydiff.h
 * @author Paulina Patuzzi (01607360)
 * @date 2020-11-02
 * @brief This function provides utility to compare to given files for differences
 * @details This program implements the Linux command diff. It takes two files as input and checks for lines with
 * differences. It then writes the line number and number of different characters to a given file or stdout.
 * If the -i option is given, the program ignores case when comparing.
 *
*/

/**
 * Program entry point.
 * @brief Program starts here.
 * @details The function can be called with options -i to ignore case when comparing the lines
 * and -o <outfile> for the file to write the line and number of different characters to (default is stdout)
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on success and EXIT_FAILURE otherwise
 * */
int main(int argc, char *argv[]);