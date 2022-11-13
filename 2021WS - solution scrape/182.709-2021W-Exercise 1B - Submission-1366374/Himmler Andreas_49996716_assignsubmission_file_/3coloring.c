/**
 * @file 3coloring.c
 * @author Andreas Himmler 11901924
 * @date 12.11.2021
 *
 * @brief Implementation of the module 3coloring
 */
#include "3coloring.h"

/**
 * @brief Method that checks the return code of buffer methods and prints an
 *        appropriate error message to stderr.
 * @param ret Return Value of the buffer method
 * @param prog Name of the program
 * @param name Name of the buffer
 * @return 0 if no error occured, -1 otherwise
 */
int check_buffer(int ret, char *prog, char *name)
{
	if (ret == -1)
	{
		switch (errno)
		{
		case EACCES:
			fprintf(stderr, "[%s] Permission to access shared memory %s denied.\n", prog, name);
			break;
		case EEXIST:
			fprintf(stderr, "[%s] Shared memory %s already exists and cannot be created.\n", prog, name);
			break;
		case EINVAL:
			fprintf(stderr, "[%s] Shared memory name %s is invalid.\n", prog, name);
			break;
		case EMFILE:
			fprintf(stderr, "[%s] Per-process limit of open file descriptors has been reached.\n", prog);
			break;
		case ENAMETOOLONG:
			fprintf(stderr, "[%s] Length of %s exceeds PATH_MAX.\n", prog, name);
			break;
		case ENFILE:
			fprintf(stderr, "[%s] System-wide limit of open files has been reached.\n", prog);
			break;
		case ENOENT:
			fprintf(stderr, "[%s] Shared memory %s does not exist.\n", prog, name);
			break;
		default:
			assert(0);
		};
		return -1;
	}
	return 0;
}

/**
 * @brief Method that checks the closing of an buffer and returns an appropriate
 *        error code to stderr.
 * @param ret return value of the closing
 * @param prog Name of the program
 * @param name Name of the buffer.
 * @return 0 if no error occured, -1 otherwise
 */
int check_close(int ret, char *prog, char *name)
{
	if(ret == -1)
	{
		switch (errno)
		{
		case EBADF:
			fprintf(stderr, "[%s] No valid file descriptor for %s.\n", prog, name);
			break;
		case EINTR:
			// File has been closed
			return 0;
		case EIO:
			fprintf(stderr, "[%s] An I/O error occured during closing %s.\n", prog, name);
			break;
		case ENOSPC:
		case EDQUOT:
			fprintf(stderr, "[%s] Available storage space was exceeded.\n", prog);
			break;
		default:
			assert(0);
		}
		return -1;
	}
	return 0;
}

/**
 * @brief Method that checks the opening of an semaphore and prints an
 *        appropriate error message to stderr.
 * @param sem the semaphore to be checked
 * @param prog Name of the program
 * @param name Name of the semaphore
 * @return 0 if no error occured, -1 otherwise
 */
int check_sem_open(sem_t *sem, char *prog, char *name)
{
	if(sem == SEM_FAILED)
	{
		switch (errno)
		{
		case EACCES:
			fprintf(stderr, "[%s] No permission to open semaphore %s.\n", prog, name);
			break;
		case EEXIST:
			fprintf(stderr, "[%s] Semaphore %s already exists.\n", prog, name);
			break;
		case EINVAL:
			fprintf(stderr, "[%s] Either name of %s if just '/' or value is greater than SEM_VALUE_MAX.\n", prog, name);
			break;
		case EMFILE:
			fprintf(stderr, "[%s] Per-process limit of open file desciprots has been reached.\n", prog);
			break;
		case ENAMETOOLONG:
			fprintf(stderr, "[%s] Name %s is to long.\n", prog, name);
			break;
		case ENFILE:
			fprintf(stderr, "[%s] System-wide limit of open files has been reached.\n", prog);
			break;
		case ENOENT:
			fprintf(stderr, "[%s] Semaphore %s does not exist.\n", prog, name);
			break;
		case ENOMEM:
			fprintf(stderr, "[%s] Insufficient memory.\n", prog);
			break;
		default:
			assert(0);
		};
		return -1;
	}
	return 0;
}

/**
 * @brief Method that checks the closing of an semaphore an prints an
 *        appropriate error message to stderr.
 * @param ret Return value of the closing
 * @param prog Name of the program
 * @param name Name of the semaphore
 * @return 0 if no error occured, -1 otherwise
 */
int check_sem_close(int ret, char *prog, char *name)
{
	if(ret == -1)
	{
		if (errno == EINVAL) fprintf(stderr, "[%s] Semaphore %s is invalid.\n", prog, name);
		return -1;
	}
	return 0;
}

/**
 * @brief Method that checks the unlinking of an semaphore an prints an
 *        appropriate error message to stderr.
 * @param ret Return value of the unlinking
 * @param prog Name of the program
 * @param name Name of the semaphore
 * @return 0 if no error occured, -1 otherwise
 */
int check_sem_unlink(int ret, char *prog, char *name)
{
	if (ret == -1)
	{
		switch (errno)
		{
		case EACCES:
			fprintf(stderr, "[%s] No permission to access semaphore %s.", prog, name);
			break;
		case ENAMETOOLONG:
			fprintf(stderr, "[%s] Name %s is to long", prog, name);
			break;
		case ENOENT:
			fprintf(stderr, "[%s] Semaphore %s does not exist.", prog, name);
			break;
		default:
			assert(0);
		};
		return -1;
	}
	return 0;
}

/**
 * @brief Checks the error code after the un/mapping of an buffer.
 * @param prog Name of the program
 * @param name Name of the semaphore
 * @return 0 if no error occured, -1 otherwise
 */
void print_map_error(char *prog, char *name)
{
	switch (errno)
	{
	case EACCES:
		fprintf(stderr, "[%s] Shared memory %s cannot be opened.\n", prog, name);
		break;
	case EAGAIN:
		fprintf(stderr, "[%s] Shared memory %s has been locked.\n", prog, name);
		break;
	case EBADF:
		fprintf(stderr, "[%s] File descriptor of shared memory %s is invalid.\n", prog, name);
		break;
	case EEXIST:
		fprintf(stderr, "[%s] Mapping for shared memory %s already clashes.\n", prog, name);
		break;
	case EINVAL:
		fprintf(stderr, "[%s] Arguments for the un/mapping of the shared memory %s are invalid.\n", prog, name);
		break;
	case ENFILE:
		fprintf(stderr, "[%s] System-wide limit of open files has been reached.\n", prog);
		break;
	case ENODEV:
		fprintf(stderr, "[%s] Filesystem does not support memory mapping.\n", prog);
		break;
	case ENOMEM:
		fprintf(stderr, "[%s] No memory available.\n", prog);
		break;
	case EOVERFLOW:
		fprintf(stderr, "[%s] Number of pages overflew.\n", prog);
		break;
	case EPERM:
		fprintf(stderr, "[%s] Shared memory was mounted no-exec, but PROT_EXEC was requested.\n", prog);
		break;
	case ETXTBSY:
		fprintf(stderr, "[%s] Shared memory %s is writeable, but MAP_DENYWRITE was set.\n", prog, name);
		break;
	default:
		assert(0);
	}
}

/**
 * @brief Checks the return value of the sem_post command and prints an
 *        appropriate error message to stderr.
 * @param ret the return value of sem_post
 * @param prog the name of the program
 * @param name the name of the semaphore
 * @return -1 if an error occured, 0 else
 */
int check_sem_post(int ret, char *prog, char *name)
{
	if (ret == -1)
	{
		switch (errno)
		{
		case EINVAL:
			fprintf(stderr, "[%s] Semaphore %s is invalid.\n", prog, name);
			break;
		case EOVERFLOW:
			fprintf(stderr, "[%s] Semaphore %s overflew.\n", prog, name);
			break;
		default:
			assert(0);
		}
		return -1;
	}
	return 0;
}

/**
 * @brief Check the return value of sigaction and prints an appropriate error
 *        message to stderr.
 * @param ret return value of sigaction
 * @param prog name of the program
 * @return -1 if an error occured, 0 else
 */
int check_sigaction(int ret, char *prog)
{
	if (ret == -1)
	{
		switch (errno)
		{
		case EFAULT:
			fprintf(stderr, "[%s] Sigaction action points to invalid memory\n", prog);
			break;
		case EINVAL:
			fprintf(stderr, "[%s] Illegal signal was specifed with signal\n", prog);
			break;
		default:
			assert(0);
		}
		return -1;
	}
	return 0;
}
