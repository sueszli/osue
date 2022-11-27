/*!
  @file mycompress.h
  @author Mario Fentler 12025969
  @date 11.11.2021
  @brief File containing method definitions and constants
*/

//! maximum size used for allocating memory for temporary saving the lines from the input file
#define MAX_SIZE 1024
//! path of temporary file
#define TEMP_FILE "./temp.txt"

/*!
  @brief does a compression of the given input words by counting the consecutive occourences 
  of letters and merging them together to letter+occurence count and writes it to the output_stream
  @param fp pointer to the temporary file containing the read data from the input stream
  @param output_stream pointer to the stream used for printing the result
*/
void doCompression(FILE *fp, FILE *output_stream);

/*!
  @brief prints the usage method to the user if the option arguments when calling the program are not used correctly
*/
void print_usage(void);

/*!
  @brief Checks if the given file was successfully opened. Throws error message instead
  @param fp pointer to the file to be checked
  @param programName pointer to the programName which should be included in the error message
*/
void checkFileOpenedWithoutError(FILE *fp, char *programName);