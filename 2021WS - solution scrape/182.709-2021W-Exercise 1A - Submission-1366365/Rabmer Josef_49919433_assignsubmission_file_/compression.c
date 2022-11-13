/**
 * @file compression.c
 * @author Josef Rabmer 11911128
 * 
 * @brief the file implements the compression algorithm, that is used for mycompress.
 * 
 * @details The function compress_input_print_to_output implements a compression algorithm
 *          and the static function calculate_number_digits_of_int is used in the process
 * 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "compression.h"
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief the function calculates the number of digits, that a given integer has
 * 
 * @details the function uses integer division to determine how many digits an integer has
 * 
 * @param num   integer, who's digits are to be determined
 * @return int  returns the number of digits, the integer has
 */
int static calculate_number_digits_of_int(int num)
{   
    int scale = 1;
    int digits = 1;

    if (num == 0)
    {
        return digits;
    }
    
    while (num / scale != 0)
    {
        digits++;
        scale *= 10;
    }

    return digits;
}

void compress_input_print_to_output(FILE *in, FILE *out, int *char_count_read_written)
{   
    char* prog_name = "compress_input_print_to_output";
    int current_char = fgetc(in);
    int prev_char = current_char;
    int char_count = 0;

   while (prev_char != EOF) {
        
        if (current_char != prev_char)
        {
            if(fputc(prev_char,out) == EOF){
                fprintf(stderr, "[%s] fputc failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            char_count_read_written[1]++;
            // Convert char counter to string
            // Reserve memory
            char *char_count_str = malloc(sizeof (char) * calculate_number_digits_of_int(char_count) + 1);
            if (char_count_str == NULL)
            {
                fprintf(stderr, "[%s] malloc failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            // Convert to int
            int length = sprintf(char_count_str, "%i", char_count);
            if(length < 0){
                fprintf(stderr, "[%s] sprintf failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            // Write number into work buffer after char
            if (length > 0)
            {
                for (int j = 0; j < length; j++)
                {
                    if(fputc(char_count_str[j],out) == EOF){
                        fprintf(stderr, "[%s] fputc failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    char_count_read_written[1]++;
                }
            }
            free(char_count_str);
            char_count = 0;
        }
        
        char_count_read_written[0]++;
        char_count++;
        prev_char = current_char;
        current_char = fgetc(in);
    
    }
    
    if(fputc((int) '\n',out) == EOF){
            fprintf(stderr, "[%s] fgetc failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
    }
    if(char_count_read_written[0] > 0){
        char_count_read_written[0]--;
    }
    
}