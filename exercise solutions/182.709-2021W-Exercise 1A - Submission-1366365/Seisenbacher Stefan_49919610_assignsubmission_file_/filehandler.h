/**
 * @file filehandler.h
 * @author Stefan Seisenbacher
 * @date 09.11.2021
 *  
 * @brief Read/Write to files
 *
 * Provides functions to read or write to files
 */
/**
 * Writes a string to a file
 * @brief Simple writes to a file
 * @details Simple writes to a file
 * @param output This will be written to the file
 * @param file To this file will be written
 */
void writeToFile(char *output, char *file);
/**
 * Reads from a file
 * @brief Simple reads to a file
 * @details Simple reads to a file 
 * @param file From this file will be written
 * @return Content from the file
 */
char* readFile(char *file);