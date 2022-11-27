#ifndef __DYNARRAY_H__
#define __DYNARRAY_H__

/**
 * @file dynarray.h
 * @author Moritz Schalk (52005233)
 * @date 25 Nov 2021
 * @brief Contains a data structure representing a dynamically growing array of floats and its assocciated functions. 
 */

/**
 * @brief Indicates that an operation on a dynamic array was completed successfully.
 */
#define DYN_SUCCESS 0

/**
 * @brief Indicates that an error occurred during an operation on a dynamic array.
 */
#define DYN_ERROR -1

/**
 * @brief A dynamically allocated array of floats plus a few size values.
 * @warning Too avoid memory leaks and segfaults only use the functions provided in this header to modify the state of this data structure.
 */
typedef struct {
    float *array;            //< The array itself.
    unsigned int length;     //< The amount of values stored in the array.
    unsigned int max_length; //< The maximum amount of values that can be stored in the array, before it needs to be resized.
} dynarray;

/**
 * @brief Allocate a new dynamic array.
 * @param array Will point to the newly created array if the funcion exits successfully.
 * @return DYN_SUCCESS on success, DYN_ERROR otherwhise
 */
int dyn_new(dynarray **array);

/**
 * @brief Free a dynamic array.
 * @param array The array that should be freed.
 */
void dyn_free(dynarray *array);

/**
 * @brief Add a value to an array, and increase its size, if there is not enough space.
 * @param array The array will that a will be added to.
 * @param a A floating point value that will be added to the array.
 * @return DYN_SUCCESS on success, DYN_ERROR otherwhise
 */
int dyn_add(dynarray *array, float a);

/**
 * @brief Get all values at even array indices, if the array is of even size.
 * @param even Will point to a copy of all even elements of the array. It is the users responsibillity to free this memory area after use. 
 * @param array The array to be split into even and odd parts.
 * @return DYN_SUCCESS on success, DYN_ERROR otherwhise
 */
int get_even(float **even, dynarray *array);

/**
 * @brief Get all values at odd array indices, if the array is of even size.
 * @param even Will point to a copy of all odd elements of the array. It is the users responsibillity to free this memory area after use. 
 * @param array The array to be split into even and odd parts.
 * @return DYN_SUCCESS on success, DYN_ERROR otherwhise
 */
int get_odd(float **odd, dynarray *array);

#endif