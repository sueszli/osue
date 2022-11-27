/**
 * @author Peter Kabelik (01125096)
 * @file common.c
 * @date 14.11.2021
 *
 * @brief A program to check for palindromes.
 * 
 * @details This program checks various inputs if they contain palindromes.
 * These checks are performed line by line.
 * Inputs can come from stdin or files, outputs can go to stdout or a file.
 * The palindrome checks can also be made ignoring letter case and/or whitespaces.
 **/

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
 
#include "common.h"

#define SHARED_MEMORY_NAME 			"/01125096_SHARED_MEMORY"

void print_custom_error(char* program_name, char* custom_message)
{
	fprintf(stderr, "%s: %s\n", program_name, custom_message);
}

void print_error(char* program_name, char* failed_function_name)
{
	fprintf(stderr, "%s: %s has failed: %s\n", program_name, failed_function_name, strerror(errno));
}

shared_memory_t* initialise_shared_memory(bool should_create, char* program_name)
{
	int flag = O_RDWR;
	
	if (should_create)
	{
		flag |= O_CREAT;
	}

	int file_descriptor = shm_open(SHARED_MEMORY_NAME, flag, 0600);

	if (file_descriptor == -1)
	{
		print_error(program_name, "shm_open");
		
		exit(EXIT_FAILURE);
	}
	
	if (ftruncate(file_descriptor, sizeof(shared_memory_t)) == -1)
	{
		print_error(program_name, "ftruncate");
		
		close_file_descriptor(file_descriptor, program_name);
		
		exit(EXIT_FAILURE);
	}
	
	shared_memory_t* shared_memory = mmap(NULL, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
	
	if (shared_memory == MAP_FAILED)
	{
		print_error(program_name, "mmap");
		
		close_file_descriptor(file_descriptor, program_name);
		
		exit(EXIT_FAILURE);
	}
	
	close_file_descriptor(file_descriptor, program_name);
	
	if (errno != 0)
	{
		exit(EXIT_FAILURE);
	}
	
	return shared_memory;
}

void close_file_descriptor(int file_descriptor, char* program_name)
{
	if (close(file_descriptor) == -1)
	{
		print_error(program_name, "close");
	}
}

void clean_shared_memory(bool should_unlink, shared_memory_t* shared_memory, char* program_name)
{
	if (munmap(shared_memory, sizeof(shared_memory_t)) == -1)
	{
		print_error(program_name, "munmap");
	}
	
	if (should_unlink)
	{
		if (shm_unlink(SHARED_MEMORY_NAME) == -1)
		{
			print_error(program_name, "shm_unlink");
		}
	}
	
	if (errno != 0)
	{
		exit(EXIT_FAILURE);
	}
}

sem_t* initialise_semaphore(bool should_create, char* name, unsigned int value, char* program_name)
{
	sem_t* semaphore = NULL;
	
	if (should_create)
	{
		semaphore = sem_open(name, O_CREAT, 0600, value);
	}
	else
	{
		semaphore = sem_open(name, 0);
	}
	
	if (semaphore == SEM_FAILED)
	{
		print_error(program_name, "sem_open");
	}
	
	return semaphore;
}

void clean_semaphore(bool should_unlink, sem_t* semaphore, const char* name, char* program_name)
{
	if (sem_close(semaphore) == -1)
	{
		print_error(program_name, "sem_close");
	}
	
	if (should_unlink)
	{
		if (sem_unlink(name) == -1)
		{
			print_error(program_name, "sem_unlink");
		}
	}
}
