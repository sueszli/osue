/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module  supervisor                                                                  *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief   this is the supervisor process for the feedback_arc_set algorithm           *
 * @details The supervisor process starts all the required resources like shared memory *
 *          and the required semaphores for the generators to work                      *
 *                                                                                      *
 * @date    14.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

#include "inc/setup.h"

/**
 * @brief   sets the parameters for graceful termination of the program
 *
 * @details sets the variable terminate, as well as the property state in the shared memory to 1,
 *          to signal a graceful shutdown to the generators.
 *
 * @param   int  signal  unused
 * @return  void
 */
void sighandler( int signal ) {
    terminate = 1;
    sharedMemory->state = 1;
}

/**
 * @brief   allocates the required resources for the supervisor to run
 *
 * @details opens up the shared memory as well as the semaphores required to run the program.
 *          also truncates and maps the memory for the shared memory fd.
 *
 * @param   void
 * @return  void
 */
void allocateResources( void ) {
    // Create and/or open the shared memory object
    shmfd = shm_open( SHM_NAME, O_RDWR | O_CREAT, 0600 );
    if ( shmfd == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
    // Set the size of the shared memory
    if ( ftruncate( shmfd, sizeof( struct sharedMemory ) ) < 0 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
    // Map the shared memory object
    sharedMemory = mmap( NULL, sizeof( *sharedMemory ), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
    if ( sharedMemory == MAP_FAILED ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
    // Initiate the Semaphores
    mutex = sem_open( SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1 );
    if ( mutex == SEM_FAILED ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
    freeSem = sem_open( SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_DATA );
    if ( freeSem == SEM_FAILED ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
    usedSem = sem_open( SEM_USED, O_CREAT | O_EXCL, 0600, 0 );
    if ( usedSem == SEM_FAILED ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
        exit( EXIT_FAILURE );
    }
}

/**
 * @brief   initializes important variables
 *
 * @details initializes the terminate variable to 0 and also initiates the shared memory parameters to 0.
 *
 * @param   void
 * @return  void
 */
void init( void ) {
    terminate = 0;
    sharedMemory->state = 0;
    sharedMemory->readPos = 0;
    sharedMemory->writePos = 0;
}

/**
 * @brief   frees all the resources, is called on termination
 *
 * @details closes and unlinks the semaphores, unmaps the shared memory fd and then closes it and unlinks it as well.
 *
 * @param   void
 * @return  void
 */
void freeResources( void ) {
    // Close and unlink all the open semaphores
    sem_close( mutex ); sem_close( freeSem ); sem_close( usedSem );
    if ( sem_unlink( SEM_MUTEX ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    if ( sem_unlink( SEM_FREE ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    if ( sem_unlink( SEM_USED ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    // Unmap the shared memory
    if ( munmap( sharedMemory, sizeof( *sharedMemory ) ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    if ( close( shmfd ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    // Remove shared memory object
    if ( shm_unlink(SHM_NAME) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
}

int main( int argc, char *argv[] ) {

    struct sigaction sa;
    memset( &sa, 0, sizeof( sa ) );
    sa.sa_handler = sighandler;

    // Save the name of the programme in a variable
    programName = argv[0];
    sigaction( SIGINT, &sa, NULL );
    sigaction( SIGTERM, &sa, NULL );

    if ( atexit( freeResources ) != 0 ) {
        printf( "[%s] --- [Error]: Unable to free all resources!\n", programName );
    }

    allocateResources();
    init();

    Graph currentSmallestArcSet = { .edgeCount = MAX_EDGES + 1 };
    Graph tempArcSet = { .edgeCount = -1 };

    while ( terminate != 1 ) {
        // read shared memory and process the info
        if ( sem_wait( usedSem ) != 0 ) {
            terminate = 1;
            sharedMemory->state = 1;
            break;
        }
        tempArcSet = sharedMemory->data[ sharedMemory->readPos ];
        sharedMemory->readPos = ( sharedMemory->readPos + 1 ) % MAX_DATA;
        sem_post(freeSem);

        if ( currentSmallestArcSet.edgeCount > tempArcSet.edgeCount ) {
            currentSmallestArcSet = tempArcSet;

            if ( currentSmallestArcSet.edgeCount == -1 ) {
                break;
            } else if ( currentSmallestArcSet.edgeCount == 0 ) {
                printf( "[%s] The graph is acyclic!\n", programName );
                terminate = 1;
                sharedMemory->state = 1;
            } else if ( currentSmallestArcSet.edgeCount == 1 ) {
                printf( "[%s] Solution with 1 edge: %i-%i\n", programName, currentSmallestArcSet.edges[ 0 ].start, currentSmallestArcSet.edges[ 0 ].end );
            } else {
                printf( "[%s] Solution with %i edges: ", programName, currentSmallestArcSet.edgeCount );
                for ( int i = 0; i < currentSmallestArcSet.edgeCount; i++ ) {
                    printf( "%i-%i ", currentSmallestArcSet.edges[ i ].start, currentSmallestArcSet.edges[ i ].end );
                }
                printf( "\n" );
            }

        }

    }

    exit( EXIT_SUCCESS );
}
