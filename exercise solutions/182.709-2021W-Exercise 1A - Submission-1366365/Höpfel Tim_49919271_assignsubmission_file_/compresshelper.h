/**
 * @file compresshelper.h
 * @author Tim Höpfel Matr.Nr.: 01207099 <e01207099@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Provides the compress-function used for all compressing in mycompress.
 *
 * This program compresses text with a simple algorithm. The input is compressed by substituting subsequent identical characters by only one occurence of the
 * character followed by the number of characters. For example, if you encounter the sequence aaa, then it is replaced by a3.
 **/


#ifndef COMPRESSHELPER_H_INCLUDED /* prevent multiple inclusion */
#define COMPRESSHELPER_H_INCLUDED


/**
 * Compress a line to output.
 * @brief This function takes a text, compresses it to an outputstring.
 * @details This program compresses text with a simple algorithm. The input is compressed by substituting subsequent identical characters by only one occurence of the
 * character followed by the number of characters. For example, if you encounter the sequence aaa, then it is replaced by a3.
 * @param text This string is read.
 * @param output This string is written to. It contains the compressed text.
 * @param inputsize This int is written to. It continas the size of the parameter text.
 * @return Returns the size of the compressed text. Returns 0 if the parameters are invalid.
 */

int compress(const char *text, char *output, int *inputsize );

#endif // COMPRESSHELPER_H_INCLUDED
