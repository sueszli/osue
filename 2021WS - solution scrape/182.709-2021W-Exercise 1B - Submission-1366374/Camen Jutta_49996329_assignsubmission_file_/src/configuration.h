/**
@file configuration.h
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 12.11.2021

@brief constants for supervisor and generator
*/
#ifndef CONFIGURATION
#define CONFIGURATION

//! read semaphore file name
#define SEM_READ "01452468_OSUE_UE1B_read"
//! write semaphore file name
#define SEM_WRITE "/01452468_OSUE_Ue1B_write"
//! exclusive semaphore (for genereator coordination) file name
#define SEM_EXCL "/01452468_OSUE_Ue1B_writeExc"
//! shared memory file name
#define SHM_NAME "/01452468_OSUE_Ue1B"

//! maximum number of entries in circular buffer
#define MAX_DATA (50)
//! maximum number of edges, which may be removed, for valid solution
#define MAX_REMOVE 8

//! struct for shared memory
struct myshm {
    /** circular buffer, index i in [i][] is a set of edges
    index 0 of a set of edges contains the number of edges within the set of edges
    e.g for set of edges: [3,1,2,2,4,4,1] where 3 (index 0) is the number of edges
    contained in the set, 1-2 ist first edge, 2-4 ist second edge, 4-1 is third edge
    */
    unsigned int data[MAX_DATA][MAX_REMOVE*2+1]; //1.Stelle vom Array, wie viele edges entfernt werden
    //! read poistion circular buffer
    unsigned int readPos;
    //! write position circular buffer
    unsigned int writePos;
    //! if quit==1, generators stop
    short int quit;
};
#endif
