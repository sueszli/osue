#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shared_mem.h" 
#include "circ_buff.h" 

static const char *myprog = "shared_mem.c"; /** The program name.*/

int sh_mmap_cleanup(struct circ_buff *circ_buff, bool unlink, bool unmap)
{
    int state = 0;

    if (unmap) {
        if (munmap(circ_buff, sizeof(*circ_buff)) == -1)
        {
            fprintf(stderr, "%s: munmap failed: %s\n", myprog, strerror(errno));
            state = -1;
        }
    }

    if (close(shmfd) == -1)
    {
        fprintf(stderr, "%s: close failed: %s\n", myprog, strerror(errno));
        state = -1;
    }

    if (unlink)
    {
        if (shm_unlink(SHM_NAME) == -1)
        {
            fprintf(stderr, "%s: shm_unlink failed: %s\n", myprog, strerror(errno));
            state = -1;
        }
    }
    return state;
}

struct circ_buff *create_or_get_sh_mmap(bool is_sv)
{
    struct circ_buff *circ_buff;
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

    if (shmfd == -1)
    {
        fprintf(stderr, "%s: shm_open failed: %s\n", myprog, strerror(errno));
        return NULL;
    }

    if (is_sv)
    {
        if (ftruncate(shmfd, sizeof(struct circ_buff)) < 0)
        {
            sh_mmap_cleanup(circ_buff, is_sv, false);
            fprintf(stderr, "%s: ftruncate failed: %s\n", myprog, strerror(errno));
            return NULL;
        }
    }

    circ_buff = mmap(NULL, sizeof(*circ_buff), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (circ_buff == MAP_FAILED)
    {
        sh_mmap_cleanup(circ_buff, is_sv, false);
        fprintf(stderr, "%s: mmap failed: %s\n", myprog, strerror(errno));
        return NULL;
    }
    return circ_buff;
}


