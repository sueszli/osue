/**
 * @file circularbuffer.c
 * @author Michael Bumbar <e11775807@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Implementation of the circularbuffer module.
 * 
 * This file contains the implementations of all non-static functions in circularbuffer.c. These functions are related to opening a shared memory, POXIX semaphores, writing and reading
 * the shared memory and closing all shared resources. Additionally it was used to implement a common usage and error handling function. 
 */

#include "circularbuffer.h"


circbuffer* buffer; /**< The name of the circular buffer. */
sem_t *sem_write; /**< The name of the semaphore showing how many buffer spaces are free to be written on. */
sem_t *sem_read; /**< The name of the semaphore showing how many buffer spaces are written on and ready to be read. */
sem_t *sem_process; /**< The name of the semaphore used for mutual exclusion of the generator processes accessing the same circular buffer. */
int buf_fd; /**< The name of the file descriptor used for opening the shared memory.*/


/**
 * Creates or opens all necessary semaphores.
 * @brief This function is responsible for creating POSIX semaphores in the supervisor process and opening the same POSIX semaphores in the generators. 
 * @details This function starts by checking the supervisor argument. If it is 1 the semaphores have to be created. sem_open is called repeatedly with O_RDWR|O_CREAT to creat semaphores that
 * can be written and read. If the semaphore does not exist yet it will be created. Additionally we provide the function with S_IRWXU giving the owner of the file read, write and execute permissions.
 * The semaphore sem_write receieves the size of the buffer as initial size as it represents the open spaces. The semaphore sem_read receieves 0 as initial size as it represents the consumed spaces.
 * The semaphore sem_process receieves the 1 as initial size as it used for the mutual exclusion of various generator process accessing the same circular buffer.
 * If the function is called in a generator we call the same functions without initial values for the semaphores and without the O_CREAT and S_IRWXU flags.
 * If any of the semaphores are NULL after sem_open was called the semaphores couldn't be opened from the generators as the supervisor already unlinked all semaphores. In that case the function returns
 * -1 to notify the caller that the shared memory has already been unlinked.
 * If any of the semaphores equals SEM_FAILED the function exits by calling errorHandling.
 * Otherwise the function returns EXIT_SUCCESS.
 * constants: SEM1, SEM2, SEM3, GEN
 * @param supervisor This parameter determines wether the function was called by a supervisor process or a generator.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t open_semaphores(int supervisor){
    if(supervisor == 1){
        sem_write = sem_open(SEM1,O_RDWR | O_CREAT, S_IRWXU, MAX_SIZE);
        sem_read = sem_open(SEM2,O_RDWR | O_CREAT, S_IRWXU, 0);
        sem_process = sem_open(SEM3,O_RDWR | O_CREAT, S_IRWXU, 1);
    } else {
        sem_write = sem_open(SEM1,O_RDWR);
        sem_read = sem_open(SEM2,O_RDWR);
        sem_process = sem_open(SEM3,O_RDWR);
    }
    if(sem_write == NULL || sem_process == NULL || sem_read == NULL){
        return -1;
    }
    if(sem_write == SEM_FAILED || sem_read == SEM_FAILED || sem_process == SEM_FAILED){
        if(supervisor == 1){
             errorHandling(SUP, "sem_open failed in open_semaphores", strerror(errno));
        }
        errorHandling(GEN, "sem_open failed in open_semaphores", strerror(errno));
    }
    return EXIT_SUCCESS;
}


/**
 * Closes all semaphores and unlinks them if necessary.
 * @brief This function is responsible for closing and unlinking POSIX semaphores in the supervisor process and closing the same POSIX semaphores in the generators. 
 * @details This function starts by checking the supervisor argument. If it is 1 the semaphores are all closed one by one. Afterwards they are unlinked. If any of these functions fails the function
 * calls errorHandling and terminates.
 * Otherwise the semaphores are only closed.
 * The function returns EXIT_SUCCESS.
 * constants: SEM1, SEM2, SEM3, GEN
 * @param supervisor This parameter determines wether the function was called by a supervisor process or a generator.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t free_semaphores(int supervisor){
     if(supervisor == 1){
         if(sem_write != NULL){
            if(sem_close(sem_write)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
        if(sem_read != NULL){
            if(sem_close(sem_read)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
        if(sem_process != NULL){
            if(sem_close(sem_process)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
        if(sem_unlink(SEM1)== -1){
            errorHandling(SUP,"sem_unlink failed in free_original_buffer", strerror(errno));
        }
        if(sem_unlink(SEM2)== -1){
            errorHandling(SUP,"sem_unlink failed in free_original_buffer", strerror(errno));
        }
        if(sem_unlink(SEM3)== -1){
            errorHandling(SUP,"sem_unlink failed in free_original_buffer", strerror(errno));
        }
    } else {
       if(sem_write != NULL){
            if(sem_close(sem_write)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
        if(sem_read != NULL){
            if(sem_close(sem_read)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
        if(sem_process != NULL){
            if(sem_close(sem_process)== -1){
                errorHandling(SUP,"sem_close failed in free_original_buffer", strerror(errno));
            }
         }
    }
    return EXIT_SUCCESS;
    
}


int32_t setup_buffer(void){
    if((buf_fd = shm_open(NAME,O_RDWR | O_CREAT, 0600)) == -1){
        free_original_buffer();
        errorHandling(SUP,"shm_open failed in setup_buffer", strerror(errno));
    }
    if(ftruncate(buf_fd, sizeof(circbuffer)) == -1){
        free_original_buffer();
        errorHandling(SUP,"ftruncate failed in setup_buffer", strerror(errno));
    }
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED ,buf_fd,0);
    if(buffer == MAP_FAILED){
        free_original_buffer();
        errorHandling(SUP,"mmap failed in setup_buffer", strerror(errno));
    }
    if(close(buf_fd) == -1){
        free_original_buffer();
        errorHandling(SUP,"close failed in setup_buffer", strerror(errno));
    }
    if(open_semaphores(1) == EXIT_FAILURE){
        free_original_buffer();
        errorHandling(SUP,"close failed in setup_buffer", strerror(errno));
    }
    buffer->write_pos = 0;
    buffer->read_pos = 0;
    buffer->running = ACTIVE;
    return EXIT_SUCCESS;
}


int32_t load_buffer(void){
    if((buf_fd = shm_open(NAME,O_RDWR | O_CREAT, S_IRWXU)) == -1){
        free_loaded_buffer();
        errorHandling(GEN,"shm_open failed in load_buffer", strerror(errno));
    }
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED ,buf_fd,0);
    if(buffer == MAP_FAILED){
        free_loaded_buffer();
        errorHandling(GEN,"mmap failed in load_buffer", strerror(errno));
    }
    if(close(buf_fd) == -1){
        free_loaded_buffer();
        errorHandling(GEN,"close failed in load_buffer", strerror(errno));
    }
    int c = 0;
    if((c = open_semaphores(0)) == EXIT_FAILURE){
        free_loaded_buffer();
    }
    if(c == -1){
        return c;
    }
    return EXIT_SUCCESS;
}


int32_t free_original_buffer(void){
    if(munmap(buffer, sizeof(*buffer)) == -1){
        errorHandling(SUP,"munmap failed in free_original_buffer", strerror(errno));
    }
    if(shm_unlink(NAME) == -1){
        errorHandling(SUP,"shm_unlink failed in free_original_buffer", strerror(errno));
    }
    free_semaphores(1);
    return EXIT_SUCCESS;
}


int32_t free_loaded_buffer(void){
    if(munmap(buffer, sizeof(*buffer)) == -1){
        errorHandling(GEN,"munmap failed in free_loaded_buffer", strerror(errno));
    }
    free_semaphores(0);
    return EXIT_SUCCESS;
}


void activity_change(void){
    if(buffer->running == ACTIVE){
        buffer->running = INACTIVE;
    } else {
        buffer->running = ACTIVE;
    }
}


int32_t read_buffer(arc_set* s){
    while(sem_wait(sem_read) != 0){
        if(errno == EINTR){
            return -1;
        } else {
            errorHandling(SUP,"sem_wait failed", strerror(errno));
        }
    }
    *s = buffer->buffer[buffer->read_pos];
    if(sem_post(sem_write) == -1){
        if(errno != EINTR){
            errorHandling(SUP,"sem_post failed", strerror(errno));
        }
    }
    buffer->read_pos++;
    buffer->read_pos = buffer->read_pos % MAX_SIZE;
    return EXIT_SUCCESS;
}


int32_t write_buffer(arc_set s){
    while(sem_wait(sem_process) != 0){
        if(errno == EINTR){
            continue;
        } else {
            errorHandling(SUP,"sem_wait failed", strerror(errno));
        }
    }
    while(sem_wait(sem_write) != 0){
        if(errno == EINTR){
            continue;
        } else {
            errorHandling(SUP,"sem_wait failed", strerror(errno));
        }
    }
    buffer->buffer[buffer->write_pos] = s;
    if(sem_post(sem_read) == -1){
        if(errno != EINTR){
            errorHandling(SUP,"sem_post failed", strerror(errno));
        }
    }
     if(sem_post(sem_process) == -1){
        if(errno != EINTR){
            errorHandling(SUP,"sem_post failed", strerror(errno));
        }
    }
    buffer->write_pos++;
    buffer->write_pos = buffer->write_pos % MAX_SIZE;
    return EXIT_SUCCESS;
}


int32_t cmp_edge(edge* a, edge* b){
    if(a->start == b->start){
        if(a->end == b->end){
            return 0;
        } else {
            return 1;
        }
    }
    return 1;
}


void show_solution(arc_set* result){
    for(int i = 0; i < result->length; i++){
        fprintf(stdout,"%d-%d ",result->result[i].start,result->result[i].end);
    }
}


void usage(char* use,char* prog) {
    fprintf(stderr,"Usage: %s %s\n",prog,use);
    exit(EXIT_FAILURE);
}


void errorHandling(char* progName,char* errorMessage,char*cause){
   fprintf(stderr, "[%s] ERROR: %s: %s\n", progName,errorMessage, cause);
   exit(EXIT_FAILURE); 
}

