/**
 * @file fb_arc_set.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief Implementation of the functions declarated in fb_arc_set.h. For documentation see fb_arc_set.h.
 * 
 *      
 **/

#include "fb_arc_set.h"

/** See fb_arc_set.h for documentation of this function **/ 
void initialize_shared_mem_as_server(int *fd, circular_buffer_t **circ_buffer){

    if( (*fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU)) == -1){
        fprintf(stderr, "shm_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if(ftruncate(*fd, sizeof(circular_buffer_t)) != 0){
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    if( (*circ_buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0)) == MAP_FAILED){
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    (**circ_buffer).read_end = 0;
    (**circ_buffer).write_end = 0;
    (**circ_buffer).quit_flag_generators = false; 

}

/** See fb_arc_set.h for documentation of this function **/
void initialize_shared_mem_as_client(int *fd, circular_buffer_t **circ_buffer){

    *fd = shm_open(SHM_NAME, O_RDWR, S_IRWXU);
    if(*fd == -1){
        fprintf(stderr, "shm_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if( (*circ_buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0)) == MAP_FAILED){
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        close(*fd);
        exit(EXIT_FAILURE);
    }

}

/** See fb_arc_set.h for documentation of this function **/
void close_shared_mem_as_server(int fd, circular_buffer_t *circ_buffer){

    if(munmap(circ_buffer, sizeof(*circ_buffer)) == -1){
        fprintf(stderr, "munmap failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(close(fd) == -1){
        fprintf(stderr, "close of shared memory failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "unlink of shared memory failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

}

/** See fb_arc_set.h for documentation of this function **/
void close_shared_mem_as_client(int fd, circular_buffer_t *circ_buffer){

    if(munmap(circ_buffer, sizeof(*circ_buffer)) == -1){ // oder muss ich hier sizeof(*circ_buffer) machen?
        fprintf(stderr, "munmap failed: %d %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(close(fd) == -1){
        fprintf(stderr, "close of shared memory failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

}

/** See fb_arc_set.h for documentation of this function **/
void initialize_sem_as_server(sem_t** sem, const char *sem_name, unsigned int init_value){

    if( (*sem = sem_open(sem_name, O_CREAT | O_RDWR, S_IRWXU, init_value)) == SEM_FAILED){ // wenn Generator und Supervisor in Schleife laufen, dann könnte man hier O_EXCL hinzufügen
        fprintf(stderr, "creating of semaphore failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


/** See fb_arc_set.h for documentation of this function **/
void initialize_sem_as_client(sem_t** sem, const char *sem_name){

    if( (*sem = sem_open(sem_name, O_RDWR)) == SEM_FAILED){
        fprintf(stderr, "opening of semaphore failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/** See fb_arc_set.h for documentation of this function **/
void remove_sem_as_server(sem_t *sem, const char *sem_name){
    if(sem_close(sem) != 0){
        fprintf(stderr, "closing of semaphore failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(sem_unlink(sem_name) != 0){
        fprintf(stderr, "unlinking of semaphore failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


/** See fb_arc_set.h for documentation of this function **/
void remove_sem_as_client(sem_t *sem){
    if(sem_close(sem) != 0){
        fprintf(stderr, "closing of semaphore failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


/** See fb_arc_set.h for documentation of this function **/
void print_edge_set(struct edge_set *set, char *string_before){
    int i = 0;
    fprintf(stdout, "%s (%d edge(s)): \n", string_before, set->length);
    while(i < set->length){
        fprintf(stdout, "%d-%d ", (set->edge_array[i]).node_out, (set->edge_array[i]).node_in);
        i++;
    }
    fprintf(stdout, "\n");
}