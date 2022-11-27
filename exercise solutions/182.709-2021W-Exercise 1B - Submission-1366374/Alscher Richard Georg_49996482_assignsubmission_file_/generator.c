/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module  generator                                                                   *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief   This is the generator process of the feedback_arc_set algorithm             *
 * @details The generator takes a number of edges (min 1) and proceeds to calculate     *
 *          a random subset graph to see if the provided graph was either acyclic       *
 *          or to find the minimum required edges to be removed to make it acyclic      *
 *                                                                                      *
 * @date    14.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

#include "inc/setup.h"

/**
 * @brief   sets the parameters for graceful termination of the program
 *
 * @details sets the variable terminate to 1, to signal a graceful shutdown.
 *
 * @param   int  signal  unused
 * @return  void
 */
void sighandler( int signal ) {
    terminate = 1;
}

/**
 * Prints the usage message and exits the program
 */
void usage( void ) {
    fprintf( stderr, "Usage: %s EDGE1...\n", programName );
    exit(EXIT_FAILURE);
}

/**
 * @brief   allocates the required resources for the generator to run
 *
 * @details opens up the shared memory as well as the semaphores required to run the program.
 *
 * @param   void
 * @return  void
 */
void allocateResources( void ) {
    // Create and/or open the shared memory object
    shmfd = shm_open( SHM_NAME, O_RDWR , 0600 );
    if ( shmfd == -1 ) {
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
    mutex = sem_open( SEM_MUTEX, 0 );
    freeSem = sem_open( SEM_FREE, 0 );
    usedSem = sem_open( SEM_USED, 0 );
}

/**
 * @brief   frees all the resources, is called on termination
 *
 * @details closes the semaphores and unmaps the shared memory fd and then closes it as well.
 *
 * @param   void
 * @return  void
 */
void freeResources( void ) {
    // Close and unlink all the open semaphores
    sem_close( mutex ); sem_close( freeSem ); sem_close( usedSem );
    // Unmap the shared memory
    if ( munmap( sharedMemory, sizeof( *sharedMemory ) ) == -1 ) {
        printf( "[%s] --- [Error]: %s!\n", programName, strerror(errno) );
    }
    if ( close( shmfd ) == -1 ) {
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
    terminate = 0;

    if ( argc <= 1 ) {
        usage();
        exit( EXIT_FAILURE );
    }

    int edgeCount = argc - 1;
    int verticesCounter = 0;
    Edge edges[edgeCount];
    int vertices[edgeCount * 2];
    for ( int i = 0; i < edgeCount ; i++) {
        Edge edge;
        sscanf(argv[i], "%u-%u", &(edge.start), &(edge.end));
        edges[i] = edge;

        if ( inArray( vertices, sizeof( vertices ) / sizeof( int ), edge.start ) == 0 ) {
            vertices[ verticesCounter ] = edge.start;
            verticesCounter++;
        }
        if ( inArray( vertices, sizeof( vertices ) / sizeof( int ), edge.end ) == 0 ) {
            vertices[ verticesCounter ] = edge.end;
            verticesCounter++;
        }
    }

    int highestIndexWithValue = 0;
    int size = sizeof( vertices ) / sizeof( int );
    for ( int i = 0; i < size ; i++ ) {
        if ( vertices[ i ] >= 0 ) {
            highestIndexWithValue = i;
        } else {
            size = highestIndexWithValue;
        }
    }

    if ( atexit( freeResources ) != 0 ) {
        printf( "[%s] --- [Error]: Unable to free all resources!\n", programName );
    }

    allocateResources();

    //srand( time( NULL ) );
    while ( sharedMemory->state != 1 && terminate != 1 ) {
        // Generate a random permutation
        for ( int i = highestIndexWithValue; i > 1; i-- ) {
            int j = rand() % i;

            int helper = vertices[ i ];
            vertices[ i ] = vertices[ j ];
            vertices[ j ] = helper;
        }

        int localEdgeCount = 0;
        for ( int i = 0; i < edgeCount; i++ ) {
            int indexStart = inArray( vertices, sizeof( vertices ) / sizeof( int ), edges[ i ].start );
            int indexEnd = inArray( vertices, sizeof( vertices ) / sizeof( int ), edges[ i ].end );

            if ( indexStart > indexEnd ) {
                localEdgeCount++;
            }
        }

        // Skip all solutions with more than 10 edges
        if (localEdgeCount <= MAX_EDGES) {

            // Generate arc set
            Graph arcSet = { .edgeCount = 0 };
            for ( int i = 0; i < edgeCount; i++ ) {
                int indexStart = inArray( vertices, sizeof( vertices ) / sizeof( int ), edges[ i ].start );
                int indexEnd = inArray( vertices, sizeof( vertices ) / sizeof( int ), edges[ i ].end );

                if ( indexStart > indexEnd ) {
                    arcSet.edges[ arcSet.edgeCount ].start = edges[ i ].start;
                    arcSet.edges[ arcSet.edgeCount ].end = edges[ i ].end;
                    arcSet.edgeCount++;
                }
            }

            if ( sem_wait( mutex ) != 0 ) {
                terminate = 1;
                break;
            }
            if ( sem_wait( freeSem ) != 0 ) {
                terminate = 1;
                break;
            }
            sharedMemory->data[ sharedMemory->writePos ] = arcSet;
            sharedMemory->writePos = ( sharedMemory->writePos + 1 ) % MAX_DATA;
            sem_post( usedSem );
            sem_post( mutex );

        }

    }

    exit( EXIT_SUCCESS );

}
