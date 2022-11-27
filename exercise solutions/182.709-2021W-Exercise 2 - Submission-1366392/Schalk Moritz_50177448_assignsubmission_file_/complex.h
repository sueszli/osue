#ifndef __COMPLEX_H__
#define __COMPLEX_H__

#include <math.h>

/**
 * @file dynarray.h
 * @author Moritz Schalk (52005233)
 * @date 25 Nov 2021
 * @brief Stores a data structure representing complex values as well as a few basic arrithmetic functions.
 */

/**
* @brief This value is returned if an arithmetic function encountered an error during its execution.
*/ 
#define COMPLEX_ERR (complex_t) {.real = NAN, .complex = NAN}

/**
 * @brief A simple representation of a complex number in its cartesian form.
 */
typedef struct {
    float real;     //< The real part part.
    float complex;  //< The imaginary part.
} complex_t;

/**
 * @brief Add two complex numbers.
 * @param a The first summand of the addition.
 * @param b The second summand of the addition.
 * @return (a + b) or COMPLEX_ERR, if an error was encountered.
 */
complex_t complex_add(complex_t a, complex_t b);

/**
 * @brief Multiply two complex numbers.
 * @param a Multiplier of the multiplication.
 * @param b Multiplicand of the multiplication.
 * @return (a * b) or COMPLEX_ERR, if an error was encountered.
 */
complex_t complex_mult(complex_t a, complex_t b);

#endif