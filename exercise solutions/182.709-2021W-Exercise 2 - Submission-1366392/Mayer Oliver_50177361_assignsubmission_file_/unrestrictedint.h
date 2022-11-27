/**
 * @file main.c
 * @author Mayer Oliver, 12023147
 * @brief Functions to read and add integers of any size
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#if !defined _my_unres_int
#define _my_unres_int

/**
 * @brief read int from stdin using an array
 * 
 * @param inp_size is set to number of read chars
 * @param signum not used
 * @return unsigned char* integer number represented as an array
 */
unsigned char *parse_Int_from_String(int *inp_size, int *signum);

unsigned char *addIntegers(unsigned char *num1, unsigned char *num2, int size_num1, int size_num2, int *ret_size);

#endif //_my_unres_int