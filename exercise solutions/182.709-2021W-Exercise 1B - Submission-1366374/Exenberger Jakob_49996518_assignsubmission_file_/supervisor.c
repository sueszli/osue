#include "globals.h"


/* 
* @author: Jakob Exenberger - 11707692
* @name: supervisor
* @brief: opens shared memory and semaphores and reading solutions and prints them
* @details: opens shared memory and semaphores and reading a new solution everytime one of the generators writes one to the shared memory
    if the new solution is smaller then the current best solution, the supervisor replaces the best solution with the new solution and prints 
    it. If the new solution has 0 edges, that means the graph is 3-colorable and the supervisor informs the generators to stop, and cleans up
    all opend resources
* @date: 08.11.2021  
*/

/* 
* @brief: Prints the given solution to stdout
* @details: If the threeColor int equals 1, it means the given solution is perfect, and we print an according message and the graph. 
    If not, we just print the solution with the edges  
* @params: 
    solution: the given solution with the edges
    threeColor: indicator if the given solution is a 3-colorable one
* @return: void
*/
static void printSolution(solution *solution, int threeColor) {
    if (threeColor == 1) {
        printf("The graph is 3-colorable!");   
    } else {
        printf("Solution with %d edges: ", solution->edgesAmount); 
    }

    for (int i = 0; i < solution->edgesAmount; i++) {
        edge edge = solution->edges[i];
        printf("%d-%d, ", edge.start, edge.end);
    }
    printf("\n");
}

/* 
* @brief: Prints the synopsis end exits the program
* @details: Prints the synopsis of the program to the stdout. After that, the exit function with the code "EXIT_FAILURE" 
    is called to end the program 
* @params: void
* @return: void
*/
static void usage(void) {
    printf("Usage: supervisor\n");
    exit(EXIT_FAILURE);
}

//run varibale, program runs as long as run = 0 holds
volatile sig_atomic_t run;

/* 
* @brief: Print a message that the program recived a signal to end the program and increases the run variable
* @details: The program recivied either the SIGINT or SIGTERM signal and prints the according signal with a message to the stdout. After that the run
    variable is set to one, which will lead the supervior to stop
* @params: 
    signal: integer containing the recivied signal
* @return: void
*/
static void handle_signal(int signal) 
{
    printf("Handled signal: %d, end program\n", signal);
    run = 1;
}


/* 
* @brief: Main function which sets up the shared memory and the semaphores. Reads solutions and prints them. Cleanup after signal or if a 
    perfect solution is found
* @details: Function sets up the shared memory and the semaphores. After that the programs waits for new solutions and prints the solution if its 
    better then best current known. If a signal is recieved the program prints out a error message and ends the program. If a solution is perfect 
    (0 edges) the program prints a message and sets a varibale in the shared memory, so the generators know to stop. After that the supervisor cleans
    up all resources
* @params: 
*   argc: number of arguments given
*   argv: array with the arugments
* @return: integer with 0 on success and 1 if errors occured
*/
int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    if (argc != 1)
    {
        usage();
    }


    //shm_open
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        exit(EXIT_FAILURE);
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct myshm*)) < 0) {
       // throw error and exit 
    }

    // map shared memory object:
    myshm *myshm;
    myshm = mmap(NULL, sizeof(struct myshm*), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED) {
        // throw error and exit 
    }

    //sem_open
    sem_t *sem_free = sem_open(SEM_free, O_CREAT | O_EXCL, 0600, 1);
    sem_t *sem_used = sem_open(SEM_used, O_CREAT | O_EXCL, 0600, 0);
    sem_t *sem_write = sem_open(SEM_write, O_CREAT | O_EXCL, 0600, 1);


    int rd_pos = 0;
    solution currentSolution = {
        .edgesAmount = MAX_EDGES
    };
    solution* newSolution = malloc(sizeof(solution));

    // run until solution is found or SIGINT or SIGTERM is recived
    printf("Setup complete and waiting for solutions!\n");
    while (run != 1)
    {
        //wait for solutions
        if (sem_wait(sem_used) == -1) { // wait until something new was written to the shm
            if (errno == EINTR) continue;
            else return EXIT_FAILURE;
        }
        *newSolution = myshm->solutions[rd_pos];
        sem_post(sem_free);
        rd_pos += 1;
        rd_pos %= SOLUTION_BUFFER_SIZE;

        //if 0 edges -> print and close program
        //check if solution has fewer edges -> print
        if (newSolution->edgesAmount == 0) {
            //print newSolution
            printSolution(newSolution, 1);
            myshm->endProgram = 1;
            run++;
        } else if (newSolution->edgesAmount < currentSolution.edgesAmount) {
            //replace currentSolution with the new one
            printSolution(newSolution, 0);
            currentSolution = *newSolution;
        }
    }
    free(newSolution);

    //sempost to avoid deadlock
    sem_post(sem_free);
    sem_post(sem_used);

    //munmap
    if (munmap(myshm, sizeof(myshm)) == -1) {
        exit(EXIT_FAILURE);
    }

    //close shm
    if (close(shmfd) == -1)
    {
        exit(EXIT_FAILURE);
    }

    //smh_unlink
    if (shm_unlink(SHM_NAME) == -1) {
        exit(EXIT_FAILURE);
    }

    //sem_close
    if (sem_close(sem_used) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_free) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_write) == -1)
    {
        exit(EXIT_FAILURE);
    }

    //sem_unlink
    if (sem_unlink(SEM_free) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(SEM_used) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(SEM_write) == -1)
    {
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
