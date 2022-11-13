/**
 * @file forkFFT.h
 * @author Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
 * @date 25.11.2021
 * @brief Defines struct and macro which is used in forkFFt.c and forkFF_output_to_tree.c.
 **/

#ifndef FORKFFT_H__
#define FORKFFT_H__

#define PI 3.141592654

/**
 * @brief Struct which represents a complex number consisting of real and imaginary part.
 * @details The real and imaginary part are stored as a signle precision float number.
 */
typedef struct complex_number
{
    float re;       //Real part of the complex number.
    float im;       //Imaginary part of the complex number.
} complex_number;

#endif //FORKFFT_H__