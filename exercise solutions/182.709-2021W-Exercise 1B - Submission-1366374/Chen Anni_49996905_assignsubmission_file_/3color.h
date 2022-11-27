/**
 * @file 3color.h
 * @author Anni Chen
 * @date 03.11.2021
 * @brief Functions, structs needed for the 3-color problem
 * @details Provides structs and utility functions to store graphs and to compute solutions for the 3-color problem
 */

/**
 * @brief The limit of edges in a solution the generator is allowed to
 * submit to the supervisor.
 */
#define MAX_EDGES 8

/**
 * @brief Vertex of a graph which contains a color and the content
 * @details the colors can take the numbers 1, 2 or 3 which represents the colors red, blue, green
 * the content is restricted to numbers
 */
struct Vertex
{
    int color;
    long content;
};

/**
 * @brief Edge of a graph which contains two vertices
 * @details an edge stores pointers that each point to a vertex
 */
struct Edge
{
    struct Vertex *v1;
    struct Vertex *v2;
};

/**
 * @brief Edge for storing into the buffer
 * @details this edge contains the content of its vertices
 */
struct Solution_Edge
{
    long content1;
    long content2;
};

/**
 * @brief a ready to submit solution
 * @details this solution wraps all the necessary information including the solution edges, the number of edges to
 * be removed and the status of the solution
 */
struct Solution
{
    struct Solution_Edge edges[MAX_EDGES];
    int numOfRemovedEdges;
    int status;
};

/**
 * @brief adds a vertex to the provided vertex array if it does not exist
 * @details vertex will be added to the array at the position provided if it does not exist
 * @param v the vertex to add
 * @param vertices an array of the stored vertices
 * @param numOfVertices the number of vertices currently stored in the provided array of vertices
 * @param freePos the position at which the vertex should be added
 * @return 1 if the vertex was added, 0 if it already exists
 */
int addVertexIfExists(struct Vertex v, struct Vertex vertices[], int numOfVertices, int freePos);

/**
 * @brief returns the address of the vertex stored in the array with the provided content
 * @details the function iterates over all vertices and compares their content to the content provided, it then returns
 * its adress
 * @param vertices an array of the stored vertices
 * @param numOfVertices the number of vertices currently stored in the provided array of vertices
 * @param content of the vertex that is looked for
 * @return the address of the vertex, NULL if the vertex could not be found
 */
struct Vertex *getAdressOfVertex(struct Vertex vertices[], int numOfVertices, long content);

/**
 * @brief adds an edge to the provided edge array if it does not exist
 * @details the function iterates over all edges and compares the content of their vertices to the content of the vertices provided,
 * if the edge exists the vertices of the edge at the given position will be set to the provided vertices
 * @param newV1 the pointer that points to the first vertex
 * @param newV1 the pointer that points to the second vertex
 * @param edges an array of the stored edges
 * @param numOfEdges the number of edges currently stored in the provided array of edges
 * @param freePos the position at which the edge should be added
 * @return 1 if the edge was added, 0 if it already exists
 */
int addEdgesIfExists(struct Edge edges[], struct Vertex *newV1, struct Vertex *newV2, int numOfEdges, int freePos);

/**
 * @brief prints the vertices of an array of vertices
 * @details the function iterates over all vertices in the array an prints out their content, one by one
 * @param vertices an array of the stored vertices
 * @param numOfVertices the number of vertices currently stored in the provided array of vertices
 */
void printAllVertices(struct Vertex vertices[], int numOfVertices);

/**
 * @brief prints the vertices of an array of vertices
 * @details the function iterates over all edges in the array an prints out the content of their vertices, one by one
 * @param edges an array of the stored edges
 * @param numOfEdges the number of edges currently stored in the provided array of edges
 */
void printAllEdges(struct Edge edges[], int numOfEdges);

/**
 * @brief randomly assigns colors to the vertices provided
 * @details the function calculates the colors randomly and assigns them to the vertices one by one
 * @param vertices an array of the stored vertices
 * @param numOfVertices the number of vertices currently stored in the provided array of vertices
 */
void assignRandomColors(struct Vertex vertices[], int numOfVertices);

/**
 * @brief removes edges which vertices have the same color
 * @details the function checks if the vertices of the edges have the same color, those edges with vertices of the same color
 * will be stored in the array that stores the edges to be removed
 * @param edges an array of the stored edges
 * @param numOfEdges the number of edges currently stored in the provided array of edges
 */
void removeEdges(struct Edge edges[], struct Edge edgesToBeRemoved[], int numOfEdges);

/**
 * @brief counts the number of the edges
 * @details the function iterates over all edges in the array, counts them and returns the number
 * @param edges an array of the stored edges
 * @param numOfEdges the number of edges currently stored in the provided array of edges
 * @return the number of edges that were counted
 */
int countEdges(struct Edge edges[], int numOfEdges);

/**
 * @brief prints the solution
 * @details the function prints the edges that should be removed, along with the number of edges to be removed
 * @param solution the solution that contains the edges to be removed as well as its number
 */
void printSolution(struct Solution solution);

/**
 * @brief wraps the edges to be removed to prepare them for writing to the buffer
 * @details the function wraps the edges and its number in a new struct
 * @param removeEdges an array of the edges to be removed
 * @param numOfEdgesToRemove the number of edges to be removed
 */
struct Solution prepareSolution(int numOfEdgesToRemove, struct Edge removeEdges[]);
