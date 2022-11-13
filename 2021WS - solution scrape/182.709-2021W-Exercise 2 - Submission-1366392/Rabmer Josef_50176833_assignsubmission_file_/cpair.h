/**
 * @file cpair.h
 * @author Josef Rabmer 11911128
 * 
 * @brief the header defines the necessary constants and structs, which are then used in the implementation of cpair
 * 
 * @details The header defines 3 different things. First it defines relevant constants like the maximum initial size of a buffer 
 *          and the max line size, that can be read from a stream. Then it also defines the structs, which are used to represent 
 *          a 2D point and a buffer. Lastly, it declares a function usage(), which is used in argument handling.
 * 
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief size that buffers should be initialised with. max_size then also determines how large the array of points will be.
 * 
 */
#define INITIAL_SIZE_BUFFER 100

/**
 * @brief the maximum size of a line that can be read from a file using read_file
 * 
 */
#define LINE_SIZE 1000

// TODO: Document if i end up doing bonus example
#define TREE_INDICATOR "BEGINNING_OF_TREE\n"

/**
 * @brief is used to represent a point in 2D space and includes x and y coordinates
 * 
 */
typedef struct point
{
    double x;
    double y;
} point_t;

/**
 * @brief is used to represent a buffer storing a list of points. It includes the current index, max_size of points array and array of points.
 * 
 */
typedef struct buffer 
{
    int current_index;
    int max_size;
    point_t* points;
} buffer_t;

/**
 * @brief is used to print the correct usage of a programm
 *
 * @details specifies the correct usage in the implementation and prints it to stdout.
 * 			It then terminates the programm.
 * 
 */
void usage(void);