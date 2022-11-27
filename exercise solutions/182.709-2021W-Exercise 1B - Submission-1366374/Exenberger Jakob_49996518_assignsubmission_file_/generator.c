#include "globals.h"

/* 
* @author: Jakob Exenberger - 11707692
* @name: generator
* @brief: reads files or from stdin, and replaces all tabs by user specified number of spaces
* @details: parses the arguments, opens stdin or in a loop calls per file given the function "replaceTabs". This function replaces the tabs
    with a specified number of spaces given by the user, if non given the default is 8
* @date: 08.11.2021  
*/

sem_t *sem_free;
sem_t *sem_used;
sem_t *sem_write;

/* 
* @brief: Adds one edge to an edge array
* @details: the number i indicates how large the new array needs to be. The new array gets relocated and the single edge gets added 
    on the last index of the array
* @params: 
    edgesArr: array of edges
    e: single edge which should be pushed to the array
    i: integer to indicate the new size and to set the single edge correctly
* @return: pointer to the relocated edge array
*/
static edge* pushEdgeToArray(edge *edgesArr, edge *e, int i) {
    edge *temp = (edge *) realloc(edgesArr, (i*sizeof(edge)));
    if (temp == NULL)
    {
        exit(EXIT_FAILURE);
    }
    edgesArr = temp;
    edgesArr[i-1] = *e;
    
    return edgesArr;
}

/* 
* @brief: Adds one node to a node array
* @details: the number i indicates how large the new array needs to be. The new array gets relocated and the single node gets added 
    on the last index of the array
* @params: 
    nodesArr: array of nodes
    n: single node which should be pushed to the array
    i: integer to indicate the new size and to set the single node correctly 
* @return: pointer to the relocated node array
*/
static node* pushNodeToArray(node *nodesArr, node *n, int i) {
    node *temp = (node *) realloc(nodesArr, (i*sizeof(node)));
    if (temp == NULL)
    {
        exit(EXIT_FAILURE);
    }
    nodesArr = temp;
    nodesArr[i-1] = *n;

    return nodesArr;
}

/* 
* @brief: Checks if a the given array already contains a node with the given name
* @details: for every node in the array, we check if the name of the node matches the name
    if no match is found -> the function returns 0
    if there is match -> the function returns 1
* @params: 
    nodes: nodes array
    name: name to search for
    nodesNum: number of elements in the given array
* @return: 0 if no node was found, 1 if there was a node
*/
static inline int checkIfNodeAlreadyInGraph(node *nodes, int name, int nodesNum) {
    int found = 0;
    for (int i = 0; i < nodesNum; i++)
    {
        if (nodes[i].name == name)
        {
            found = 1;
        }
    }
    return found;
}

/* 
* @brief: Writes the given solution on the next free index in the shared memory array
* @details: Writes the given solution on the shared memory on the index which is also stored in the shared memory. After the write the index gets 
    increased and updated 
* @params: 
    myshm: pointer to the shared memory
    solution: pointer to the given solution
* @return: void
*/
static void writeToSHM(myshm *myshm, solution *solution) {
    myshm->solutions[myshm->writeIndex] = *solution;

    //update write index
    int updatedIndex = myshm->writeIndex; 
    updatedIndex += 1;
    updatedIndex %= SOLUTION_BUFFER_SIZE;
    myshm->writeIndex = updatedIndex; 
}

/* 
* @brief: Returns a node from a array if the names match
* @details: Loops over the array and return the first node where the name of the node in the array matches the given name
* @params: 
    name: name we are looking for
    nodes: array of nodes
    nodeNum: number of nodes in the array
* @return: the node with the matching name
*/
static node getNodeWithName(int name, node *nodes, int nodeNum) {
    for (int i = 0; i < nodeNum; i++)
    {
        if (nodes[i].name == name)
        {
            return nodes[i];
        }
    }
    exit(EXIT_FAILURE);
}

//method to create a valid solution
/* 
* @brief: Creates a random solution for a graph through random assigning colors to all nodes and removing all edges connecting two nodes 
    with the same color
* @details: First the functions runs over the array and assigns random colors to the nodes. After that, all edges which connect two nodes with 
    the same color and adds them to the solution edge array and increase the solution edgeAmount. After all edges are checked, we return the solution 
* @params: 
    nodes: array with nodes
    edges: array with edges
    nodeNum: number of nodes in the node array
    edgeNum: number of edges in the edge array
    solution: solution in which the results should be stored
* @return: solution containg the just created solution
*/
static solution generateSolution(node *nodes, edge *edges, int nodeNum, int edgeNum, solution newSolution) {
    newSolution.edgesAmount = 0;

    //assign random colors
    for (int i = 0; i < nodeNum; i++)
    {
        int color = (rand() %(3+1 - 1)) + 1;
        //printf("Random color is: %d\n", color);
        nodes[i].color = color;
    }

    //remove edges which connect to same colored nodes
    for (int i = 0; i < edgeNum; i++)
    {
        node start = getNodeWithName(edges[i].start, nodes, nodeNum);
        node end = getNodeWithName(edges[i].end, nodes, nodeNum);

        if (start.color == end.color)
        {
            if (newSolution.edgesAmount < 6)
            {
                newSolution.edges[newSolution.edgesAmount] = edges[i];
                newSolution.edgesAmount++;
            } 
            else 
            {
                break;
            }
        }
    }
    return newSolution;
}

/* 
* @brief: Prints the synopsis end exits the program
* @details: Prints the synopsis of the program to the stdout. After that, the exit function with the code "EXIT_FAILURE" 
    is called to end the program 
* @params: void
* @return: void
*/
static void usage(void) {
    printf("Usage: generator EDGE1...\n");
    exit(EXIT_FAILURE);
}


/* 
* @brief: Main function of the program, reads graph as input, opens shared memory and semaphores and creates random 
    solutions until it recives the stop signal 
* @details: Reads the graph and parses the stdin. Every edge gets added to an array. While parsing the edges, its gets checked if the nodes as 
    start or endpoints already exist in the nodes array, if not they get created and added. Then we create random solutions to the graph and write 
    them in the shared memory, until the endProgram variable in the shared memory gets increased to 1 by the supervisor. Then we stop the creation 
    of solutions and free all resources, and end the program
* @params: 
*   argc: number of arguments given
*   argv: array with the arugments
* @return: integer with 0 on success and 1 if errors occured
*/
int main(int argc, char *argv[]) {
     srand(time(NULL));

    if (argc == 1)
    {
        usage();
    }

    edge *newEdge = malloc(sizeof(edge));
    edge *edgesArr = malloc(sizeof(edge));
    node *nodesArr = malloc(sizeof(node));
    int nodesNum = 0;
    int edgeNum = 0;

    for (int i = 1; i < argc; i++)
    {
        char *edg = argv[i];
        char *startS = &edg[0];
        char *endS = &edg[2];
        int start = strtol(startS, &startS, 10);
        int end = strtol(endS, &endS, 10);

        newEdge->start = start;
        newEdge->end = end;
        edgeNum++;
        edgesArr = pushEdgeToArray(edgesArr, newEdge, i);

        int foundStart = checkIfNodeAlreadyInGraph(nodesArr, start, nodesNum);
        if (foundStart == 0)
        {
            nodesNum++;
            node newNode = {
                .name = start
            };
            nodesArr = pushNodeToArray(nodesArr, &newNode, nodesNum);
        }

        int foundEnd = checkIfNodeAlreadyInGraph(nodesArr, end, nodesNum);
        if (foundEnd == 0)
        {
            nodesNum++;
            node newNode = {
                .name = end
            };
            nodesArr = pushNodeToArray(nodesArr, &newNode, nodesNum);
        }        
    }
    if (newEdge != NULL)
    {
        free(newEdge);
    }

    //shm_open
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1) {
        exit(EXIT_FAILURE);
    }

    //map shared memory object:
    myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }

    //sem open
    sem_t *sem_free = sem_open(SEM_free, O_EXCL);
    sem_t *sem_used = sem_open(SEM_used, O_EXCL);
    sem_t *sem_write = sem_open(SEM_write, O_EXCL);

    //generate new solution
    solution *newSolution = malloc(sizeof(solution));

    while (myshm->endProgram == 0)
    {
        *newSolution = generateSolution(nodesArr, edgesArr, nodesNum, edgeNum, *newSolution);
        printf("Solution with %d edges ", newSolution->edgesAmount);
        for (int i = 0; i < newSolution->edgesAmount; i++)
        {
            printf("%d-%d, ", newSolution->edges[i].start, newSolution->edges[i].end);
        }
        printf("\n");
        

        //ensure to be the only generator to write
        sem_wait(sem_write);

        //reduce free space by one
        sem_wait(sem_free);

        //write new solution in shared memory
        writeToSHM(myshm, newSolution);

        //increase used space by one
        sem_post(sem_used);

        //enable other generators to write to shared memory again
        sem_post(sem_write);
    }

    //free space
    if (edgesArr != NULL)
    {
        free(edgesArr);
    }
    if (nodesArr != NULL)
    {
        free(nodesArr);
    }
    if (newSolution != NULL)
    {
        free(newSolution);
    }

    //munmap
    if (munmap(myshm, sizeof(*myshm)) == -1) {
        exit(EXIT_FAILURE);
    }

    //close shm
    if (close(shmfd) == -1)
    {
        exit(EXIT_FAILURE);
    }
    
    //close sem
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

    return EXIT_SUCCESS;
}
