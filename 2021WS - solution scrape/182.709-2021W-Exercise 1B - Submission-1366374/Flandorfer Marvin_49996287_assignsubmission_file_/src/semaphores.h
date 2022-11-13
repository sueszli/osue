/**
 * Semaphore module
 * @file semaphores.h
 * @author Marvin Flandorfer, 52004069
 * @date 05.11.2021
 * 
 * @brief This module defines the names of all semaphores.
 * @details This module only consists of Macro defines.
 */

#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#define SEM_FREE_SPACE "/52004069sem_free_space"        /**< Semaphore name for the free space semaphore*/
#define SEM_USED_SPACE "/52004069sem_used_space"        /**< Semaphore name for the used space semaphore*/
#define SEM_EXCL_ACCESS "/52004069sem_excl_access"      /**< Semaphore name for the exclusive access semaphore*/

#endif