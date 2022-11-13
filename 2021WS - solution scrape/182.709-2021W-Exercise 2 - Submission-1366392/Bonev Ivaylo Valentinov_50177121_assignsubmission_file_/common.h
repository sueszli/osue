/**
 * @author 	Ivaylo Bonev, matriculation number: 12025894
 * @date 	11-Dec-2021
 * @brief 	Header file defining several macros and types,
 *          as well as serving as a container for some of the used variables
 * @details	Defines following macros:
 *          - PI (custom definition of PI)
 *          - READ_END (index for the read end of pipes)
 *          - WRITE_END (index for the write end of pipes)
 *          Defines following types:
 *          - args_t: type for storing the input data, as well as its size
 *          - complex_t: implementation of a complex number
 *          Defines variables used in forkFFT.c
**/

//Custom definition of PI
#define PI 3.14159265358979323846

//Index for pipe read end
#define READ_END 0

//Index for pipe write end
#define WRITE_END 1

//The maximum amount of characters read from stdin when calling getline()
size_t maxLineLength = 64;

//Type storing an array of float numbers, as well as its length.
//Used for storing the input numbers in forkFFT.c
typedef struct {
    float* numbers;
    int size;
} args_t;

//Type implementing a complex number.
//A complex number is of the form (a + i*b), where a is the real part and b is the imaginary part.
//Both parts are float values.
typedef struct {
    float real;
    float imaginary;
} complex_t;

//Contains the even-indexed part of the input. Called PE in the exercise.
float* PE;

//Contains the odd-indexed part of the input. Called PO in the exercise.
float* PO;

//Contains the total result calculated by using the algorithm in the exercise.
//It is precisely this result that is printed to stdout by each process.
complex_t* result;

//Contains the result that was calculated by the child that received PE as an input.
complex_t* resultE;

//Contains the result that was calculated by the child that received PO as an input.
complex_t* resultO;

//Stores the input float values.
args_t args;

//Pipe for child1-parent communication.
//Child 1 writes to the write end of this pipe (stdout), and the parent reads from its read end (stdin)
int parent_from_child1_stdin_pipe[2];

//Pipe for parent-child1 communication.
//Parent writes to the write end of this pipe (stdout), and the child reads from its read end (stdin)
int parent_to_child1_stdout_pipe[2];

//Pipe for child2-parent communication.
//Child 2 writes to the write end of this pipe (stdout), and the parent reads from its read end (stdin)
int parent_from_child2_stdin_pipe[2];

//Pipe for parent-child2 communication.
//Parent writes to the write end of this pipe (stdout), and the child reads from its read end (stdin)
int parent_to_child2_stdout_pipe[2];