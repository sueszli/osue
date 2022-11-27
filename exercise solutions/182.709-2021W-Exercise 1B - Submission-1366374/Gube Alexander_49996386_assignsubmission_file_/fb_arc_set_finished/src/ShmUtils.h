/**
 * @file ShmUtils.h
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief this module provides convenice functions for handling shared memories
 * 
 **/

#include "CircularBufferUtils.h"

/**
 * @brief unmap a shm
 * @param shmLoc pointer to the shm
 * @param progName name of the program
 * @return 0 on success, -1 on error
 * */
int unmap(struct fbArc *shmLoc, char* progName);

/**
 * @brief close a shm
 * @param fd file descriptor of the shm
 * @param progName name of the program
 * @return 0 on success, -1 on error
 * */
int closeSHM(int fd, char* progName);

/**
 * @brief unlink a shm
 * @param name of the shm
 * @param progName name of the program
 * @return 0 on success, -1 on error
 * */
int unlinkSHM(char *name, char* progName);

/**
 * @brief convenice function calling all above functions without terminating on errors
 * @param name of the shm
 * @param shmLoc pointer to the shm
 * @param progName name of the program
 * @param doUnlink  call unlink or not
 * @return 0 on success, -1 on error
 * */
void cleanupSHM(char *name, struct fbArc *shmLoc, int fd, char* progName, int doUnlink);
