#ifndef __MYLINE_H__ 
#define __MYLINE_H__

/**
 * @file myline.h
 * @author Bernhard Jung: 12023965
 * @date 2021.12.01
 * @brief Contains #defines und #typedefs for forkFFT
 */

/**
 * @brief Pipe ReadHead
 */
#define PIPE_READ 0

/**
 * @brief Pipe WriteHEad
 */
#define PIPE_WRITE 1

/**
 * @brief Value of PI
 */
#define PI 3.141592654

/**
 * @brief ERROR
 */
#define ERROR -1

/**
 * @brief CHILD
 */
#define CHILD 0

/**
 * @brief PARENT
 */
#define PARENT 1

/**
 * @struct input_t;
 * @brief Struct to store received input
 * int length_even: length of even input elements
 * int length_odd : length of odd input elements
 * float * even   : array of even input elements
 * float * odd    : array of odd input elements
 * float * number : array of all input elements
 */
typedef struct
{
  int length_even;
  int length_odd;
  float * even;
  float * odd;
  float * number;
  int length;
} input_t;

/**
 * @struct output_t;
 * @brief Struct to store geenrated output
 * int length_even  : length of even output elements
 * int length_odd   : length of odd output elements
 * float * even     : real part of even output elements
 * float * even_imag: imag part of even output elements
 * float * odd      : real part of odd output elements
 * float * odd_imag : imag part of odd output elements
 */
typedef struct
{
  int length_even;
  int length_odd;
  float * even;
  float * even_imag;
  float * odd;
  float * odd_imag;
} output_t;
#endif