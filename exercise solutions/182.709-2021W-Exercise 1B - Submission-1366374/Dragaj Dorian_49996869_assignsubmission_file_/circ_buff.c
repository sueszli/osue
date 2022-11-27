#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "circ_buff.h" 
#include "shared_mem.h" 

#define SEMA_FREE "/11702371_free"
#define SEMA_USED "/11702371_used"
#define SEMA_MUTEX "/11702371_mutex"

static const char *myprog = "circ_buff.c"; /** The program name.*/

static int close_sems(bool is_sv, bool close_used, bool close_free, bool close_mutex)
{
    int state = 0;

    if (close_used)
    {
        if (sem_close(sem_free) == -1)
        {
            fprintf(stderr, "%s: sem_close failed: %s\n", myprog, strerror(errno));
		    state = -1;
        }
        if (is_sv)
        {
            if (sem_unlink(SEMA_FREE) == -1)
            {
                fprintf(stderr, "%s: sem_unlink failed: %s\n", myprog, strerror(errno));
                state = -1;
            }
        }
    }

    if (close_used)
    {
        if (sem_close(sem_used) == -1)
        {
            fprintf(stderr, "%s: sem_close failed: %s\n", myprog, strerror(errno));
		    state = -1;
        }
        if (is_sv)
        {
            if (sem_unlink(SEMA_USED) == -1)
            {
                fprintf(stderr, "%s: sem_unlink failed: %s\n", myprog, strerror(errno));
                state = -1;
            }    
        }
    }

    if (close_mutex)
    {
        if (sem_close(sem_mutex) == -1)
        {
            fprintf(stderr, "%s: sem_close failed: %s\n", myprog, strerror(errno));
		    state = -1;
        }
        if (is_sv)
        {
            if (sem_unlink(SEMA_MUTEX) == -1)
            {
                fprintf(stderr, "%s: sem_unlink failed: %s\n", myprog, strerror(errno));
                state = -1;
            }
        }
    }
    return state;
}

static int create_or_get_sems(bool is_sv)
{
    if (is_sv) {
        sem_free = sem_open(SEMA_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA);
		if (sem_free == SEM_FAILED)
		{
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
			return -1;
		}

        sem_used = sem_open(SEMA_USED, O_CREAT | O_EXCL, 0600, 0);
        if (sem_used == SEM_FAILED)
        {
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
            close_sems(is_sv, true, false, false);
            return -1;
        }

        sem_mutex = sem_open(SEMA_MUTEX, O_CREAT | O_EXCL, 0600, 1);
        if (sem_mutex == SEM_FAILED)
        {
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
            close_sems(is_sv, true, true, false);
            return -1;
        }
    } else {
        sem_free = sem_open(SEMA_FREE, 0);
		if (sem_free == SEM_FAILED)
		{
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
			return -1;
		}

        sem_used = sem_open(SEMA_USED, 0);
		if (sem_used == SEM_FAILED)
		{
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
			return -1;
		}

        sem_mutex = sem_open(SEMA_MUTEX, 0);
		if (sem_mutex == SEM_FAILED)
		{
            fprintf(stderr, "%s: sem_open failed: %s\n", myprog, strerror(errno));
			return -1;
		}
    }
    return 0;
}

struct circ_buff *create_buff(bool is_sv)
{
    if (create_or_get_sems(is_sv) == -1)
    {
        fprintf(stderr, "%s: create_or_get_sems: %s\n", myprog, strerror(errno));
        return NULL;
    }
    struct circ_buff *circ_buff = create_or_get_sh_mmap(is_sv);

    if (circ_buff == NULL)
    {
        close_sems(is_sv, true, true, true);
        fprintf(stderr, "%s: create_or_get_sh_mmap: %s\n", myprog, strerror(errno));
        return NULL;
    }

    if (is_sv)
    {
        circ_buff->sv_stopped = false;
        circ_buff->rd_index = 0;
        circ_buff->wr_index = 0;
    }
    return circ_buff;
}

int close_buff(bool is_sv, struct circ_buff *circ_buff)
{
    int state = 0;

    if (is_sv)
    {
        circ_buff->sv_stopped = true;
        sem_post(sem_free);
    }

    if (sh_mmap_cleanup(circ_buff, is_sv, true) == -1)
    {
        fprintf(stderr, "%s: sh_mmap_cleanup failed: %s\n", myprog, strerror(errno));
        state = -1;
    }

    if (close_sems(is_sv, true, true, true) == -1)
    {
        fprintf(stderr, "%s: close_sems failed: %s\n", myprog, strerror(errno));
        state = -1;
    }
    return state;
}

int write_circ_buff(struct circ_buff *circ_buff, char *solution)
{
    if (sem_wait(sem_mutex) == -1)
    {
        fprintf(stderr, "%s: sem_wait failed: %s\n", myprog, strerror(errno));
        return -1;
    }

    int index = 0;
    do {
        if (circ_buff->sv_stopped) {
            printf("IT STOPPED!!!");
            break;
        }
        if (sem_wait(sem_free) == -1)
        {
            sem_post(sem_mutex);
            fprintf(stderr, "%s: sem_wait failed: %s\n", myprog, strerror(errno));
            return -1;
        }
        circ_buff->solutions[circ_buff->wr_index] = solution[index];
        sem_post(sem_used);
        circ_buff->wr_index += 1;
        circ_buff->wr_index %= MAX_DATA;
    } while ((solution[index++] != 0));

    if (sem_post(sem_mutex) == -1)
    {
        fprintf(stderr, "%s: sem_post failed: %s\n", myprog, strerror(errno));
        return -1;
    }
    return 0;
}

char *read_circ_buff(struct circ_buff *circ_buff)
{
    int size_incr = 35;
    int sol_size = size_incr;
    char *solution = malloc(sizeof(char) * sol_size);
    int index = 0;
    do {
        if (sem_wait(sem_used) == -1)
        {
            free(solution);
            fprintf(stderr, "%s: sem_wait failed: %s\n", myprog, strerror(errno));
            return NULL;
        }

        if (sol_size == index) 
        {
            sol_size += size_incr;
            solution = realloc(solution, sizeof(char) * sol_size);
        }
        
        solution[index] = circ_buff->solutions[circ_buff->rd_index];
        sem_post(sem_free);
        circ_buff->rd_index += 1;
        circ_buff->rd_index %= MAX_DATA;
        
    } while (solution[index++] != 0);
    return solution;
}
