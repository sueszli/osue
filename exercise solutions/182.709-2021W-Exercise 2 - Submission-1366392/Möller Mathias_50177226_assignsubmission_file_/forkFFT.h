/**
 * @author Mathias Möller, 12019833
 * @date 08.12.2021
 * @brief recursive implementation of the Cooley-Tukey Fast Fourier Transform using child-processes
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef FORKFFT_H
#define FORKFFT_H

// maximum character-length of an input-value (=one line)
const int MAX_LINE_LENGTH = 256;

// Archimedes' constant
const float PI = 3.1415926f;

// string-format to output complex numbers as (note that parent processes need to be able to read this format)
const char *IMAGINARY_NUMBER_FORMAT = "%f %f*i\n";
/*
 * alternative format, will omit decimal places but may lead to slightly incorrect results
 */
//const char *IMAGINARY_NUMBER_FORMAT = "%.10g %.10g*i\n";

// if set to 1, any write-operation to a stream is immediately followed by an fflush() to that stream to empty any write-buffers
const int FORCE_FLUSH = 0;

// stores all necessary info about a child-process
struct child_info {
    pid_t child_pid; // process-id of the child-process
    FILE *input_write; // stream that is routed to the child's stdin via an unnamed pipe, write-only
    FILE *output_read; // stream that is routed from the child's stdout via an unnamed pipe, read-only
};

// represents a complex number
struct complex_number {
    float real; // real part of the complex number
    float imaginary; // imaginary part of the complex number, i.e. multiples of "i"
};

#endif // FORKFFT_H
