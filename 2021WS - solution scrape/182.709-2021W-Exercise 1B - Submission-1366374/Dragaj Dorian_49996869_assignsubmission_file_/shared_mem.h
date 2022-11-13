#ifndef SHAREDMEM_H   /* Include guard */
#define SHAREDMEM_H
#define SHM_NAME "/11702371myshm"
#include <stdbool.h>
#include "circ_buff.h" 

int shmfd;

int sh_mmap_cleanup(struct circ_buff *circ_buff, bool unlink, bool unmap);

struct circ_buff *create_or_get_sh_mmap(bool is_sv);
#endif