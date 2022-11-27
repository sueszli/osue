/**
 * @author Firat Berk Cakar, e0828668
 * @brief compression implementation for the mycompress assigment
 * @date 3.11.2021
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mycompress.h"

void compress(char *input, char *compressed_output, size_t output_len) {
    size_t input_len = strlen(input);
    assert(output_len > input_len*2);
    int output_ctr = 0,count = 1,output_inc = 1;

    compressed_output[output_ctr] = input[0];
    compressed_output[++output_ctr] = (count + '0');
    for (int i = 1; i < input_len; ++i) {
         if (input[i] != input[i - 1]) {
            output_ctr += output_inc;
            compressed_output[output_ctr] = input[i];
            count = 1;
            compressed_output[++output_ctr] = (count + '0') ;
        } else {
            output_inc = sprintf(compressed_output+output_ctr,"%i",++count);

        }
    }
    compressed_output[output_ctr] = '\0'; // for safety reasons as well

}

