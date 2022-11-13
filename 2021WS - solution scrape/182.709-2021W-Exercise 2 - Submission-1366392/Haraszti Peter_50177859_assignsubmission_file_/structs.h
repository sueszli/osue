/**
 * @file structs.h
 * @author Peter Haraszti (e12019844@student.tuwien.ac.at)
 * @date 2021-11-22
 * @brief Additional structures for forkFFT
 * @details structs.h contains the struct ChildInfo, which contains all important pieces of information about a child process:
 *          the read end and the write and of the pipes aswell as the children's pid
 */

#include <stdlib.h> 

typedef struct ChildInfo {
    int read_end;
    int write_end;
    pid_t pid;
} child_info;
