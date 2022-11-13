/**
 * @author
 * Yoan Ivanov
 * e12024745@student.tuwien.ac.at
 * 
 * @brief
 * Does the processing of the grep implementation
 * 
 * @date
 * 14.11.2021
 */

#pragma once

// max size for the buffers
#define MAX_CONSOLE_INPUT 256

void usage(void);
void console_compare(void);
void file_compare(int num_files, char **input_files);