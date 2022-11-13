#include "circularbuffer.h"

/**
 * @file circularbuffer.c
 * @author Franz Lloyd Azucena <e1425044@student.tuwien.ac.at>
 * @date 02.11.2021
 *
 * @brief circular buffer used by supervisor and generator
 *
 * @details circular buffer with 3 semiphore - one for free space, used space and one for determining if a generator can write into the buffer (writing_sem). 
 **/

/*
 * sempaphores are basically unsigned ints. changes are atomic. 
 * only 2 operations: wait(), post()
 *
 * wait() tries to decrement the value of the semaphore. if the value is greater than 0 then it succeeds and decrements the value. if the value is equal to zero it waits. it waits until the value is greater than zero again.
 *
 * post() just increments the value of the semaphore.
 * */
 
sem_t* free_space_sem;
sem_t* used_space_sem;
sem_t* writing_sem;
sem_t* termination_sem;

buffer* buf;
int shm_fd;
char* name;

void print_error(char* name, char* message) {
	fprintf(stderr, "%s ERROR: %s\n", name, message);
}

void terminate_with_error(char* name, char* message) {
	print_error(name, message);
	terminate();
	exit(EXIT_FAILURE);
}

void remove_buffer_with_error(char* name, char* message) {
	print_error(name, message);
	remove_buffer();
	exit(EXIT_FAILURE);
}

bool write_buffer(result* result) {
	
	// returns 0 on success; on error, the value of the semaphore is left unchanged, -1 is returned, and errno is set to indicate the error.
	while(sem_wait(free_space_sem) != 0) {
		if(EINTR==errno) {
			// many system calls will report the EINTR error code if a signal occurred while the system call was in progress. No error actually occureed, it is just reportet that way becuase the system is not able to resume the system call automatically. This coding pattern simply retries the system call when this happens, to ignore the interrupt.
			continue;
		} else {
			/*
			 * other errors:
			 * EINVAL sem is not a valid semaphore.
			 * EAGAIN the operation could not be performed without blocking
			 * EINVAL the value of abs_timeout.tv_nsecs is less than 0, or greater than or equal to 1000 million.
			 * ETIMEDOUT the call timed out before the semaphore could be locked.
			 **/
			return false;
		}
	}
	// if writing_sem cant be decremented, this means that another generator is writing into the buffer and this one has to wait
	while(sem_wait(writing_sem) != 0) {
		if(EINTR==errno) {
			continue;
		} else {
			return false;
		}
	}
	
	// write value into buffer
	buf -> results[buf->head] = *result;
	// increment position of head
	buf->head = ((buf->head +1) % BUFFER_SIZE);
	
	// increment the used_space_sem to indicate that another slot in the buffer is used now
	if(sem_post(used_space_sem) != 0) {
		return false;
	}

	// increment writting_sem to indicate that the write operation is finished now and antoher generator can write into the buffer
	if(sem_post(writing_sem) != 0) {
		return false;
	}

	return true;
}

bool read_buffer(result* result) {
	
	// sem_wait returns 0 on success; on error, the value of the semaphore is left unchanged, -1 is returned, and errno is set to indicate the error.
	while(sem_wait(used_space_sem) != 0) {
		if(EINTR == errno) {
			continue;
		} else {
			return false;
		}
	}

	// read result from buffer
	result = &(buf->results[buf->tail]);
	
	// increment tail
	buf->tail = ((buf->tail +1) % BUFFER_SIZE);
	
	// increment free_space_sem
	// returns the same as sem_wait
	if(sem_post(free_space_sem) != 0) {
		return false;
	}

	return true;
	
}

void print_result(result* result) {
	int size = count_edges(result);
	fprintf(stdout, "Solution with %d edges: ", size);
	edge* removed = result->removed_edges;
	for(int i = 0; i < size; i++) {
		fprintf(stdout, "%d-%d ", removed->node1, removed->node2);
	}
	fprintf(stdout, "\n");
}

int count_edges(result* result) {
	int count = 0;
	for(int i = 0; i<LIMIT_EDGES; i++) {
		edge e = result->removed_edges[i];
		if(e.node1!=0 && e.node2!=0) {
			count++;
		}
	}
	return count;
}

int get_seed_offset(void) {
	fprintf(stderr, "get_seed_offset\n");
	
	fprintf(stderr, "writing_sem: %d\n", writing_sem==NULL);
	
	while(sem_wait(writing_sem) != 0) {
		fprintf(stderr, "sem_wait\n");
		if(EINTR == errno) {
			continue;
		} else {
			// when an error occurs, the seed_offset cant be modified safelly
			return -1;
		}
	}
	
	fprintf(stderr, "sem_wait END\n");

	fprintf(stderr, "DEBUG: buffer.seed_offset: %d", buf->seed_offset);
	
	buf->seed_offset = (buf->seed_offset+500);
	
	fprintf(stderr, "DEBUG: buffer.seed_offset modified: %d", buf->seed_offset);

	if(sem_post(writing_sem) != 0) {
		return -1;
	}

	return buf->seed_offset;
}

void init(bool for_generator) {
	// smh_open returns a file descriptor (a nonnegative integer)
	// on failure, smh_open return -1
	shm_fd = shm_open(SHM, O_RDWR | O_CREAT, 0600);
	if(shm_fd == -1) {
		if(for_generator) {
			remove_buffer_with_error(name, "coudl not get file descriptor for shared memory (shm_open)");
		} else {
			terminate_with_error(name, "could not get file descriptor for shared memory (shm_open)");
		}
	};

	// mmap creates a new mapping in the virtual address space of the calling process. 
	// void *mmap(void *addr, size_t length, int prot, int falgs, int fd, off_t offset)
	// the starting address for the new mapping is specified in addr
	// if addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping; this is the most portable method of creating a new mapping.
	// the length argument specifies the length of the mapping (which must be greater than 0)
	// on success, mmap() returns a pointer to the mapped area. on error, the value MAP_FAILED (that is, (void *= -1) is returned, and errno is set to indicate the cause of the error.
	buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	
	if(MAP_FAILED == buf) {
		if(for_generator) {
			remove_buffer_with_error(name, "failed to create mapping (mmap)");
		} else {
			terminate_with_error(name, "failed to create mapping (mmap)");
		}
	}

	// sem_open() creates a new POSIX semaphore or open an existing semaphore.
	// on success, sem_open() returns the address of the new semaphore; this address is used when calling other semaphore-realted functions.
	// on error, sem_oopen() returns SEM_FAILED, whith errno set to indicate the error.
	if(for_generator) {
		free_space_sem = sem_open(FREE_SPACE_SEM, BUFFER_SIZE);
		if(SEM_FAILED == free_space_sem) {
			remove_buffer_with_error(name, "failed to create free space semaphore");
		}
	} else {
		free_space_sem = sem_open(FREE_SPACE_SEM, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
		if(SEM_FAILED == free_space_sem) {
			fprintf(stderr, "errno: %d", errno);
			terminate_with_error(name, "failed to create free space semaphore");
		}
	}

	if(for_generator) {
		used_space_sem = sem_open(USED_SPACE_SEM, 0);
		if(SEM_FAILED == used_space_sem) {
			remove_buffer_with_error(name, "failed to create used space semaphore");
		}
	} else {
		used_space_sem = sem_open(USED_SPACE_SEM, O_CREAT | O_EXCL, 0600, 0);
		if(SEM_FAILED == used_space_sem) {
			terminate_with_error(name, "failed to create used space semaphore");
		}
	}
	
	if(for_generator) {
		writing_sem = sem_open(WRITING_SEM, 1);
		if(SEM_FAILED == writing_sem) {
			remove_buffer_with_error(name, "failed to create writing semaphore");
		}
	} else {
		writing_sem = sem_open(WRITING_SEM, O_CREAT | O_EXCL, 0600, 1);
		if(SEM_FAILED == writing_sem) {
			terminate_with_error(name, "failed to create writing semaphore");
		}
	}

	if(for_generator) {
		termination_sem = sem_open(TERMINATION_SEM, 1);
		if(SEM_FAILED == termination_sem) {
			remove_buffer_with_error(name, "failed to create termination semaphore");
		}
	} else {
		termination_sem = sem_open(TERMINATION_SEM, O_CREAT | O_EXCL, 0600, 1);
		if(SEM_FAILED == termination_sem) {
			terminate_with_error(name, "failed to create termination semaphore");
		}
	}
}

void remove_buffer(void) {
	close(shm_fd);
	munmap(buf, sizeof(*buf));
	sem_close(free_space_sem);
	sem_close(used_space_sem);
	sem_close(writing_sem);
}

void terminate(void) {
	
	while(sem_wait(termination_sem) != 0) {
		if(EINTR == errno) {
			continue;
		} else {
			print_error(name, "failed to decrement termination_sem: generators are still running, supervisor terminated");
		break;
		}
	}

	close(shm_fd);
	shm_unlink(SHM);

	munmap(buf, sizeof(*buf));
	
	sem_close(free_space_sem);
	sem_unlink(FREE_SPACE_SEM);

	sem_close(used_space_sem);
	sem_unlink(USED_SPACE_SEM);

	sem_close(writing_sem);
	sem_unlink(WRITING_SEM);
}

