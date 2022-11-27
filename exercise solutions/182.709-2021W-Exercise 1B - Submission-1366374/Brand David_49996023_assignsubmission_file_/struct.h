/**
* @file struct.h
* @author David Brand <e11905164@student.tuwien.ac.at>
* @date 14.11.2021
*
* @brief A header file used by generator.c and supervisor.c.
**/

#define SEM_1 "/11905164_unused"
#define SEM_2 "/11905164_used"
#define SEM_3 "/11905164_xor"

#define SHM_NAME "/11905164_shm"
#define MAX_DATA (50)

struct edge 
{
    volatile unsigned int from;
    volatile unsigned int to;
};

struct solution
{
    unsigned int count;
    struct edge data[MAX_DATA];
};

struct myshm
{
    unsigned int writeNode;
    unsigned int readNode;
    unsigned int done;
    struct solution data[MAX_DATA];
};