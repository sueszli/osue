/**
 * @file generator.c
 * @author Philipp Gorke <e12022511@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief generator main programm
 * 
 * This programm writes solutions in the circular buffer
 *
 **/


#include "helpFunctions.h"


static int shmfd;      //shared Memory file descriptor
static mySharedMem *myshm = NULL;   //shared Memory
static sem_t *s1 = NULL;    //semaphore 1
static sem_t *s2 = NULL;    //semaphore 2
static sem_t *s3 = NULL;    //semaphore 3

static void initSHM(void);
static void initSEM(void);
static void closeSHM(void);
static void closeSEM(void);
static int totalVertices(int argc, char *argv[], int max);
static int transformRest(char *succ);


/**
 * @brief writes solutions to the circular buffer
 * @details first starts shared Mem. and semaphores
 * second, checks for the total number of vertices and
 * colors them randomly. After that a solution is found.
 * If the solution is smaller than 8 it is stored to the
 * circular buffer in which is read by the supervisor. 
 * The programm terminates if the supervisor sets kill = true. 
 */
int main(int argc, char *argv[])
{
    initSHM();
    initSEM();

    int nmbOfVert = totalVertices(argc, argv, 0);
    srand(time(NULL));

   
    while (myshm->kill == false)
    {
        //semaphore so only one generator at a time can write 
        sem_wait(s3);
        //gives each vertex a random color
        int colors[nmbOfVert+1];
        for (int i = 0; i < nmbOfVert; i++)
        {
            int random = rand() % 3; //0 = red, 1 = blue, 2 = green
            colors[i] = random;
        }
        graph outputGraph;
        outputGraph.nmb = 0;
        int index = 0;
        bool dontWrite = false;
        //finds solutions 
        for (int i = 1; i < argc; i++)
        {
            char *s = argv[i];
            char *temp;
            int vert1, vert2;
            vert1 = strtol(s, &temp, 10);
            vert2 = transformRest(temp);
        
            if (colors[vert1] == colors[vert2])
            {
                edge solution = {.v1 = vert1, .v2 = vert2};
                outputGraph.edges[index] = solution;
                outputGraph.nmb += 1;
                index++;
                if (index >= 8)
                {
                    dontWrite = true;
                    break; 
                }
            }
        }
        if (!dontWrite)
        {
            //writes the solution to the buffer
                sem_wait(s1);
                myshm->graphs[myshm -> wr_pos] = outputGraph;
                sem_post(s2);
                myshm->wr_pos += 1;
                myshm->wr_pos %= 900;
            
        }
        sem_post(s3);
    }
    return 0;
}
/**
 * @brief starts the shared memory
 * @details initiazes atexit, starts and links the shared Memory
 */
static void initSHM(void)
{
    atexit(closeSHM);
    shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd < 0)
    {
        exit(EXIT_FAILURE);
    }
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (myshm == MAP_FAILED)
    {
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) < 0)
    {
        exit(EXIT_FAILURE);
    }
    shmfd = -1;
}

/**
 * @brief starts semaphores
 * @details initiazes atexit, starts and links the semaphores
 */
static void initSEM(void)
{
    atexit(closeSEM);
    s1 = sem_open(SEM_1, 0);
    if (s1 == SEM_FAILED)
    {
        exit(EXIT_FAILURE);
    }
    s2 = sem_open(SEM_2, 0);
    if (s2 == SEM_FAILED)
    {
        exit(EXIT_FAILURE);
    }
    s3 = sem_open(SEM_3, 0);
    if (s3 == SEM_FAILED)
    {
        exit(EXIT_FAILURE);
    }
}
/**
 * @brief counts total number of vertices
 * @details strol is used to transform the string to int
 */
static int totalVertices(int argc, char *argv[], int max)
{
    for (int i = 1; i < argc; i++)
    {
        char *s = argv[i];
        char *temp; 
        int vert1, vert2; 
        vert1 = strtol(s, &temp, 10);
        vert2 = transformRest(temp);
        if (vert1 > max)
        {
            max = vert1;
        }
        if (vert2 > max)
        {
            max = vert2;
        }
    }
    return max;
}


/**
 * @brief closes shared Memory
 * @details stops and unlinks the sharedMemory
 */
static void closeSHM(void)
{
    //unmaps shared Memory
    if (myshm != NULL)
    {
        if (munmap(myshm, sizeof(*myshm)) == -1)
        {
            printf("%s", "ERROR: Shared Memory could not be unmapped");
        }
    }
}


/**
 * @brief closes semaphores
 * @details stops and unlinks the semaphores
 */
static void closeSEM(void)
{
    if (sem_close(s1) == -1)
    {
        printf("%s", "ERROR: Semaphore1 could not be closed");
    }
    if (sem_close(s2) == -1)
    {
        printf("%s", "ERROR: Semaphore1 could not be closed");
    }
    if (sem_close(s3) == -1)
    {
        printf("%s", "ERROR: Semaphore1 could not be closed");
    }
}

/**
 * @brief transform restString to a number
 * @details retruns the second int (vertex) of the edge string
 */
static int transformRest(char *succ)
{
  int val;
  char temp[strlen(succ - 1)];
  for (int i = 1; i < strlen(succ); i++)
  {
    temp[i - 1] = succ[i];
  }
  val = strtol(temp, NULL, 10);
  return val;
}