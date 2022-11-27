/*
 * @file forkFFT.h
 * @author Alex Ploner 12024704
 * @date 06.12.2021
 *
 * @brief header file for forkFFT.c
 */

#define PI 3.141592654
#define BUFFER_SIZE 128


/*
 * @brief struct representing a complex number with real and imaginary part
 */
typedef struct complex_number {

    float real;
    float img;

} complex_t;

/*
 * @brief reads partial results from the children pipes and computing the result
 * @details reads the partial results from the child that calculated with the numbers that had even indices and from
 *          the child that calculated with the numbers that had odd indices.
 *          The partial results that are read as strings are than converted to complex numbers using the function string_to_complex
 *          Than the butterfly_operation is executed on the partial results
 *          The calculated results are printed into the pipe in the form of string.
 * @param c1 open file descriptor for the first child (even)
 * @param c2 open file descriptor for the second child (odd)
 * @param length the length of the result that has to be generated
 */
void generate_result(int c1, int c2, int length);


/*
 * @brief converts a string to complex number
 * @details converts a string of format "number number*i" or a string of format "number"
 *          to a complex number. If the conversion is not possible an error is thrown and the program quits
 *          The complex number is stored to cnum.
 * @param number the string that has to be converted to a complex number
 * @param cnum pointer to whom store the complex number
 */
void string_to_complex(char* number, complex_t* cnum);

/*
 * @brief performs the butterfly operation
 * @param re pointer to re after the function result[n]
 * @param ro pointer to ro after the function result[n + k/2]
 * @param n current position in re and ro
 * @param k expected size of result
 */
void butterfly_operation(complex_t* re, complex_t* ro, int n, int k);