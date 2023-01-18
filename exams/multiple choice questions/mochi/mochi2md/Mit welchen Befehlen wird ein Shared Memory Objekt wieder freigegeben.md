Mit welchen Befehlen wird ein Shared Memory Objekt wieder freigegeben?
---
```c
munmap(my_shm, sizeof(*myshm));
shm_unlink(SHM_NAME);
close(shm_fd);
```