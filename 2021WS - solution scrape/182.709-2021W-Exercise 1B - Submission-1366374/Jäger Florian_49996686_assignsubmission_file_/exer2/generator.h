#define SHM_NAME "/11810847gen"

#define SEM_1 "/11810847sem_1"
#define SEM_2 "/11810847sem_2"
#define SEM_3 "/11810847sem_3"

#define MAX_DATA 100
#define BUFFER_SIZE 10
#define MAX_SOLUTION 50
#define edge { int v1; int v2; }

static int findNewSolution(int edgeCount,int *nodes, int nodeCount);
static void reset();
static void addNode(int *nodes, int *nodeCount, int v);
