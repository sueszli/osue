/**
 * @author Peter Kabelik (01125096)
 * @file common.h
 * @date 14.11.2021
 *
 * @brief The header file for common use.
 * 
 * @details The header file containing replacement text, structs,
 * typedefs, functions and methodes for common use.
 **/

#ifndef COMMON_H
#define COMMON_H

#define MAX_FEEDBACK_ARC_SET_SIZE	8
#define CIRCULAR_BUFFER_SIZE		16

#define FREED_SPACE_SEMAPHORE_NAME	"/01125096_FREED_SPACE_SEMAPHORE"
#define USED_SPACE_SEMAPHORE_NAME	"/01125096_USED_SPACE_SEMAPHORE"
#define MUTEX_SEMAPHORE_NAME		"/01125096_MUTEX_SEMAPHORE"

struct edge
{
	unsigned int start;
	unsigned int end;
};

typedef struct edge edge_t;

struct feedback_arc_set
{
	edge_t edges[MAX_FEEDBACK_ARC_SET_SIZE];
	unsigned int edge_count;
};

typedef struct feedback_arc_set feedback_arc_set_t;

struct shared_memory
{
	feedback_arc_set_t circular_buffer[CIRCULAR_BUFFER_SIZE];
	unsigned int writing_position;
	unsigned int generator_count;
	bool should_quit;
};

typedef struct shared_memory shared_memory_t;

/**
 * @brief Prints a custom error message.
 *
 * @details Prints a custom error message including the program's name to stderr.
 *
 * @param program_name The program's name.
 * @param custom_message The custom error message.
 */
void print_custom_error(char* program_name, char* custom_message);

/**
 * @brief Prints an error message.
 *
 * @details Prints an error message including the program's name to stderr.
 * It says that a function has failed and prints an error description.
 *
 * @param program_name The program's name.
 * @param failed_function_name The name of the function that failed.
 */
void print_error(char* program_name, char* failed_function_name);

/**
 * @brief Initialises shared memory.
 *
 * @details Initialises the shared memory, which is used by the supervisor
 * and the generators.
 *
 * @param should_create True, if the shared memory should also be created.
 * @param program_name The program's name.
 *
 * @return Returns pointer to initialised and already mapped shared_memory struct.
 */
shared_memory_t* initialise_shared_memory(bool should_create, char* program_name);

/**
 * @brief Closes file descriptor.
 *
 * @details Closes the given file descriptor.
 *
 * @param file_descriptor The given file descriptor.
 * @param program_name The program's name.
 */
void close_file_descriptor(int file_descriptor, char* program_name);

/**
 * @brief Cleans shared memory.
 *
 * @details Frees the resources the shared memory uses.
 *
 * @param should_unlink True, if the shared memory should also be unlinked.
 * @param shared_memory The pointer to the shared_memory struct.
 * @param program_name The program's name.
 */
void clean_shared_memory(bool should_unlink, shared_memory_t* shared_memory, char* program_name);

/**
 * @brief Initialises semaphore.
 *
 * @details Initialises the semaphore, which is used by the supervisor
 * and the generators.
 *
 * @param should_create True, if the semaphore should also be created.
 * @param name The semaphore's name.
 * @param value The semaphore's initial value (only when should_create is true).
 * @param program_name The program's name.
 *
 * @return Returns pointer to initialised semaphore.
 */
sem_t* initialise_semaphore(bool should_create, char* name, unsigned int value, char* program_name);

/**
 * @brief Cleans semaphore.
 *
 * @details Frees the resources the semaphore uses.
 *
 * @param should_unlink True, if the semaphore should also be unlinked.
 * @param semaphore The pointer to the semaphore.
 * @param name The semaphore's name. 
 * @param program_name The program's name.
 */
void clean_semaphore(bool should_unlink, sem_t* semaphore, const char* name, char* program_name);

#endif
