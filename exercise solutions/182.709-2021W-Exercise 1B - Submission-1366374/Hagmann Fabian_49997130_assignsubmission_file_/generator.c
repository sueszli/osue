/**
 * @file generator.c
 * @author Fabian Hagmann (12021352)
 * @brief This file contains the solution for the 3coloring-generator (execrise1b)
 * @details Synopsis: generator EDGE1...<br>
 * The generator uses the shared memeory setup by the supervisor. If it does not exist,
 * the generator will exit with failure. It generates random solutions and edges to remove
 * to validate this solution. Removed edges are writte to the circular buffer.
 * @date 2021-11-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// names of shared memory and semaphores
#define SEM_FREE "/12021352_free"
#define SEM_USED "/12021352_used"
#define SEM_WRITE "/12021352_write"
#define SHM_NAME "/12021352_3coloring"
#define BUFFER_SIZE 8
#define BUFFER_CELL_SIZE 8

// Struct to store one vertex (containes vertex id and assigned color)
struct Vertex {
    int id;
    int color; // 0 = red, 1 = gree, 2 = blue
};

// Struct to store one edge (containes 2 vertex id's as Integer)
struct Edge {
    int vertice1id;
    int vertice2id;
};

// Struct to store the entire graph
struct Graph {
    struct Vertex *vertices;
    struct Edge *edges;
};

// Struct to represent one buffer cell (stores up to BUFFER_CELL_SIZE edges)
struct BufferCell {
    struct Edge edges[BUFFER_CELL_SIZE];
};

/* 
 * Struct to represent the shared memory
 * - terminate (0 = run, 1 = terminate) -> implies the generators to stop
 * - rd_/wr_pos [0,7] -> stores the current rd/wr position in the circular buffer
 */
struct SharedMemory{
    int terminate;
    int wr_pos, rd_pos;
    struct BufferCell bufferCells[BUFFER_SIZE];
};

static void usageGenerator(void);
static int generateAndSaveGraphFromPositionalArguments(int argc, char *argv[]);
static int getEdgesFromPositionalArguments(int argc, char *argv[]);
static int getVerticesFromEdges(struct Edge* edges, int numberOfEdges);
static int isVertexIdPresentIn(int vertexId, int currentNumberOfVertices);
static void updateGraphWithRandomColors(void);
static struct Edge *getRemovedEdgesForValidGraph(void);
static int writeRemovedEdges(struct Edge* edges);
static void terminateSafe(int signal);
static void initSharedMemoryAndSemaphores(void);

struct Edge *finalFoundEdges;		// pointer to all found edges
struct Vertex *finalFoundVertices;	// pointer to all found vertices
struct Graph foundGraph;			// stored graph
struct SharedMemory *sharedMemory;	// shared memory
sem_t *sem_free;					// semaphore for free space in circ. buffer
sem_t *sem_used;					// semaphore for used space in circ. buffer
sem_t *sem_write;					// semaphore for generators' write permission
char *myprog;						// program name
int numberOfEdges;					// number of edges in the graph
int numberOfVertices;				// number of vertices in the graph
int currentlyRemovedEdges = 0;		// count of removed edges
int shmfd;							// filedescriptor for shared memory

/**
 * @brief main function for the generator
 * @details opens shared memory and semaphores provided by the supervisor.
 * generates random solutions for 3coloring until it is interruped or the terminate
 * flag in shared memory is set.
 * @param argc number of given arguments
 * @param argv argument values
 * @return exit code
 */
int main (int argc, char *argv[]) {
	// ensure program is called correctly
    myprog = argv[0];
    char c;
    while ((c = getopt(argc, argv, "")) != -1) {
		switch(c) {
			default:
				usageGenerator();
		}
	}
	// if no positional arguments were provided
	if (argc <= 1) {
        usageGenerator();
    }
   
	initSharedMemoryAndSemaphores();

	// catch signal sigint - in case the generator gets interrupted
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminateSafe;
    sigaction(SIGINT, &sa, NULL);

	// generate graph from positional arguments
    int status = generateAndSaveGraphFromPositionalArguments(argc, argv);
	if (status == EXIT_FAILURE) {
		terminateSafe(SIGQUIT);
		exit(EXIT_SUCCESS);
	}

	// generate solutions until the terminate flag is set
	while(sharedMemory->terminate != 1) {
		// generate a random 3coloring, and remove edges
		updateGraphWithRandomColors();
		struct Edge *removedEdges = getRemovedEdgesForValidGraph();
		
		// write to circular buffer only if the number of removed edges is reasonably small (fits in the buffer)
		if (currentlyRemovedEdges <= BUFFER_CELL_SIZE && removedEdges != NULL) {
			writeRemovedEdges(removedEdges);
		}
		free(removedEdges);
	}

	terminateSafe(SIGQUIT);
	exit(EXIT_SUCCESS);
}

/**
 * @brief initializes shared memory and semaphores
 * @details opens and maps the shared memory. 
 * opens semaphores. Saves necessary data to their correspondig
 * variables in the program. If any step fails, print the
 * corresponding error message and exit with failure
 */
static void initSharedMemoryAndSemaphores(void) {
	// open shm
    shmfd = shm_open(SHM_NAME, O_RDWR, 0600); 
    if (shmfd == -1) {
        fprintf(stderr, "[%s] Could not open shared memory\n", myprog);
        terminateSafe(SIGABRT);
		exit(EXIT_FAILURE);
    }

	// map shm to object
    sharedMemory = mmap(NULL, sizeof(*sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (sharedMemory == MAP_FAILED) {
        fprintf(stderr, "[%s] Could not map shared memory to object\n", myprog);
        terminateSafe(SIGABRT);
		exit(EXIT_FAILURE);
    }

	// open semaphore
    sem_free = sem_open(SEM_FREE, O_CREAT);
    sem_used = sem_open(SEM_USED, O_CREAT);
	sem_write = sem_open(SEM_WRITE, O_CREAT);
    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_write == SEM_FAILED) {
        fprintf(stderr, "[%s] Could not open semaphores\n", myprog);
        terminateSafe(SIGABRT);
		exit(EXIT_FAILURE);
    }
}

/**
 * @brief generate the graph from the positional argument
 * @details parse positional arguments to generate edges. From the edges
 * generate all vertices and construct the graph.
 * @param argc number of given arguments
 * @param argv argument values
 * @return EXIT_SUCCESS if successfuly constructed, otherwise EXIT_FAILURE
 */
static int generateAndSaveGraphFromPositionalArguments(int argc, char *argv[]) {
	// parse argument and create edges
	if (getEdgesFromPositionalArguments(argc, argv) <= 0) {
		// if no edges found or error, no valid graph is present: stop
		terminateSafe(SIGABRT);
		exit(EXIT_FAILURE);
	}
	numberOfEdges = (argc - optind);

	// parse edges and create vertices
	if ((numberOfVertices = getVerticesFromEdges(finalFoundEdges, numberOfEdges)) <= 0) {
		// if no vertices found or error, no valid graph is present: stop
		terminateSafe(SIGABRT);
		exit(EXIT_FAILURE);
	}

	// create graph with vertices and edges
	struct Graph myGraph = {finalFoundVertices, finalFoundEdges};
	foundGraph = myGraph;

	return EXIT_SUCCESS;
}

/**
 * @brief generates edges by parsing the positional arguments
 * @details parses all positional arguments. If they have the correct
 * from <vert1>-<vert2> add them to finalFoundEdges. Otherwise print
 * an error and terminate.
 * @param argc number of given arguments
 * @param argv argument values
 * @return number of found edges, if error then -1
 */
static int getEdgesFromPositionalArguments(int argc, char *argv[]) {
	// if positional arguments are provided
	if ((argc - optind) <= 0) {
		return -1;
	}
	
	finalFoundEdges = malloc(sizeof(struct Edge) * (argc - optind));
	if (finalFoundEdges == NULL) {
		fprintf(stderr, "[%s] alloc failed: could not allocate space for edges", myprog);
		return -1;
	}

	// for every positional argument
	int i;
	for (i = optind; i < argc; i++) {
		// read first vertex of edge
		char *vertexString = strtok(argv[i], "-");
		char *controll;
		int firstEdgeVertex = (int)strtol(vertexString, &controll, 10);
		
		if (controll == vertexString || *controll != '\0') {
			fprintf(stderr, "[%s] Edge No. %d was not well formed. Ending generator!\n", myprog, i);
			// free is in terminateSafe
			return -1;
		}

		// read second vertex of edge
		vertexString = strtok(NULL, "-");
		if (vertexString == NULL) {
			// end edge creation if no second vertex edge
			fprintf(stderr, "[%s] Edge No. %d was not well formed. Ending generator!\n", myprog, i);
			// free is in terminateSafe (because I use one method to terminate, it will always free it there (sucessful exit)
			// if it was freed here already this would cause an error)
			return -1;
		}
		int secondEdgeVertex = (int)strtol(vertexString, &controll, 10);
		
		// end edge creating if more than two vertex edges
		if (strtok(NULL, "-") != NULL) {
			fprintf(stderr, "[%s] Edge No. %d was not well formed. Ending generator!\n", myprog, i);
			// free is in terminateSafe
			return -1;
		}

		if (controll == vertexString || *controll != '\0') {
			fprintf(stderr, "[%s] Edge No. %d was not well formed. Ending generator!\n", myprog, i);
			// free is in terminateSafe
			return -1;
		}

		struct Edge currentEdge = {firstEdgeVertex, secondEdgeVertex};

		finalFoundEdges[i - optind] = currentEdge;
	}
	return (argc - optind);
}

/**
 * @brief generate vertices from edges
 * @details parses a valid list of edges and constructs a distinct list of vertices
 * in finalFoundVertices. If memory (re)allocation fails it prints an error message
 * and returns
 * @param edges valid list of edges to parse
 * @param numberOfEdges number of edges in the list
 * @return number of generated vertices 
 */
static int getVerticesFromEdges(struct Edge* edges, int numberOfEdges) {
	// initialize storage
	int vertexCount = 0, currentSize = 5;
	finalFoundVertices = malloc(sizeof(struct Vertex) * currentSize);
	if (finalFoundVertices == NULL) {
		fprintf(stderr, "[%s] alloc failed: could not allocate space for vertices", myprog);
		return -1;
	}

	int i;
	for(i = 0; i < numberOfEdges; i++) {
		// check vertex 1 of the edge
		if (isVertexIdPresentIn(edges[i].vertice1id, vertexCount) == 0) {
			// if list of vertices is full, extend it
			if (vertexCount == currentSize) {
				currentSize += 5;
				struct Vertex *newFinalFoundVertices = realloc(finalFoundVertices, sizeof(struct Vertex) * currentSize);
				if (newFinalFoundVertices == NULL) {
					fprintf(stderr, "[%s] realloc failed: could not reallocate space for vertices", myprog);
					return -1;
				}
				finalFoundVertices = newFinalFoundVertices;
			}
			// construct vertex and add it
			struct Vertex newVertex = {edges[i].vertice1id, 0};
			finalFoundVertices[vertexCount] = newVertex;
			vertexCount++;
		}
		// check vertex 2 of the edge
		if (isVertexIdPresentIn(edges[i].vertice2id, vertexCount) == 0) {
			// if list of vertices is full, extend it
			if (vertexCount == currentSize) {
				currentSize += 5;
				struct Vertex *newFinalFoundVertices = realloc(finalFoundVertices, sizeof(struct Vertex) * currentSize);
				if (newFinalFoundVertices == NULL) {
					fprintf(stderr, "[%s] realloc failed: could not reallocate space for vertices", myprog);
					return -1;
				}
				finalFoundVertices = newFinalFoundVertices;
			}
			// construct vertex and add it
			struct Vertex newVertex = {edges[i].vertice2id, 0};
			finalFoundVertices[vertexCount] = newVertex;
			vertexCount++;
		}
	}
	
	return vertexCount;
}

/**
 * @brief Checks if a vertex with a specified if is already present in the list of vertices
 * @details loops threw finalFoundVertices and checks if the provided vertexId is 
 * already present
 * @param vertexId id of the vertex being searches
 * @param currentNumberOfVertices current size of finalFoundVertices
 * @return 1 if present, otherwise 0
 */
static int isVertexIdPresentIn(int vertexId, int currentNumberOfVertices) {
	int i;
	for(i = 0; i < currentNumberOfVertices; i++) {
		if (finalFoundVertices[i].id == vertexId) {
			return 1;
		}
	}
	return 0;
}

/**
 * @brief generates a random 3coloring
 * @details loops through all the vertices in foundGraph and sets
 * a random color (0 = red, 1 = green, 2 = blue) for each
 */
static void updateGraphWithRandomColors(void) {
	int i;
	for (i = 0; i < numberOfVertices; i++) {
		foundGraph.vertices[i].color = rand() % 3;
	}
}

/**
 * @brief calculates all edges that need to be removed to validate the current coloring
 * @details loops through all edges of foundGraph and determines if both ends
 * have the same color. If yes, currentlyRemoved edges is increased and the edge
 * is added to the return.
 * @return a list of all edges that need to be removed
 */
static struct Edge *getRemovedEdgesForValidGraph(void) {
	currentlyRemovedEdges = 0;
	int currentlyRemovedEdgesSize = 5;

	// initialize storage
	struct Edge *removedEdges = malloc(sizeof(struct Edge) * currentlyRemovedEdgesSize);
	if (removedEdges == NULL) {
		fprintf(stderr, "[%s] alloc failed: could not allocate space for removed edges", myprog);
		return NULL;
	}
	
	int i;
	for(i = 0; i < numberOfEdges; i++) {
		// get vertices from edge
		int edgeVertex1Id = foundGraph.edges[i].vertice1id;
		int edgeVertex2Id = foundGraph.edges[i].vertice2id;

		int edgeVertex1Color = -1;
		int edgeVertex2Color = -1;

		// get color of edge vertices
		int j;
		for(j = 0; j < numberOfVertices; j++) {
			if (foundGraph.vertices[j].id == edgeVertex1Id) {
				edgeVertex1Color = foundGraph.vertices[j].color;
			} else if (foundGraph.vertices[j].id == edgeVertex2Id) {
				edgeVertex2Color = foundGraph.vertices[j].color;
			}
		}

		// if they have the same color, add them
		if (edgeVertex1Color == edgeVertex2Color) {
			if (currentlyRemovedEdges == currentlyRemovedEdgesSize) {
				// if list of removedEdges is full, extend it
				currentlyRemovedEdgesSize += 5;
				struct Edge *newRemovedEdges = realloc(removedEdges, sizeof(struct Edge) * currentlyRemovedEdgesSize);
				if (newRemovedEdges == NULL) {
					fprintf(stderr, "[%s] realloc failed: could not reallocate space for removed edges", myprog);
					return removedEdges;
				}
				removedEdges = newRemovedEdges;
			}
			removedEdges[currentlyRemovedEdges] = foundGraph.edges[i];
			currentlyRemovedEdges++;
		}
	}

	return removedEdges;
}

/**
 * @brief write removed edges to circular buffer
 * @details creates a buffer cell of the provided edges. Then waits on
 * write permission and free space in the circular buffer and writes it there.
 * Afterwards increase the write position.
 * @param edges list of removed edges (with max. size of BUFFER_SIZE)
 * @return 0 (success if written)
 */
static int writeRemovedEdges(struct Edge* edges) {
	// create new bufferCell
	struct BufferCell newCell;
	int i;
	for(i = 0; i < sizeof(newCell.edges)/sizeof(struct Edge); i++) {
		if (i < currentlyRemovedEdges) {
			newCell.edges[i] = edges[i];	
		} else {
			struct Edge terminateEdge = {-1, -1};
			newCell.edges[i] = terminateEdge;
			break;
		}
	}

	// write edges to buffer
	sem_wait(sem_write);
	sem_wait(sem_free);
	sharedMemory->bufferCells[sharedMemory->wr_pos] = newCell;
	sem_post(sem_used);
	sem_post(sem_write);

	sharedMemory->wr_pos += 1;
	sharedMemory->wr_pos %= BUFFER_SIZE;
	
	return 0;
}

/**
 * @brief terminate the generator safely
 * @details terminate the generator. To ensure other generators can also
 * terminate, increment the sem_free. Free all allocated memory, close and
 * unmap the shared memory and colse the semaphores.
 * @param signal sig that caused the termination
 */
static void terminateSafe(int signal) {
	// ensure that other generators can terminate as well
	if (sem_free != NULL) {
		sem_post(sem_free);
	}

	// free all allocated memory
	free(finalFoundEdges);
	free(finalFoundVertices);
    
	if (shmfd != -1) {
		// close shm
		if (close(shmfd) == -1) {
			fprintf(stderr, "[%s] could not close shared memory\n", myprog);
			exit(EXIT_FAILURE);
		}

		// unmap shm
		if (munmap(sharedMemory, sizeof(*sharedMemory)) == -1) {
			fprintf(stderr, "[%s] could not unmap shared memory\n", myprog);
			exit(EXIT_FAILURE);
		}
	}

	// close semaphore
	if (sem_free != NULL) {
		sem_close(sem_free); 
	}
    if (sem_used != NULL) {
		sem_close(sem_used);
	}
	if (sem_write != NULL) {
		sem_close(sem_write);
	}

	if (signal == SIGINT) {
        exit(EXIT_SUCCESS);   
    }
}

/**
 * @brief prints usage
 * @details prints usage (as specified in the exercise) to stderr
 */
static void usageGenerator(void) {
	fprintf(stderr,"Usage %s EDGE1...\n", myprog);
	exit(EXIT_FAILURE);
}