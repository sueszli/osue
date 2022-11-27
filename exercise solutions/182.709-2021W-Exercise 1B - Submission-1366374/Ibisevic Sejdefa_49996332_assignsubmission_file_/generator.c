/**
 * @file generator.c
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.aca.at>
 * @brief The circBuff.h file represents h file for shared memory for helping execution of generators and supervisor
 * @version 0.1
 * @date 2021-11-14 
 *
 * @copyright Copyright (c) 2021
 * 
 * @details
 * 
**/

#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "circBuff.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

char *myprogram_gen;

static void usage(void);
static unsigned int randomise();
int readEdges(const int *argc, char **argv, edge *edges);
static void vertex_init (int *vertices, size_t num_vertices);
int get_number_of_vertices(edge *edges, int n);
void set_output(char output[], edge *edges, int sol[], int sol_num);
void shuffle_vertices(int *vertices, size_t num_vertices);
int get_len(edge *edges, int sol[], int sol_num);
int index_of_vertice(int *vertices, int num_vertices, int vertice);

/**
 * @brief Usage function called in case of wrong input, decsribes correct input parameters
 * @param none
 * @return none
 */
static void usage(void)
{
	fprintf(stderr, "Usage: ./generator EDGE1 EDGE2 ...");
	exit(EXIT_FAILURE);
}

/**
 * @brief Function that generates random seed for each generator, because generators could be started at the same time
 * 
 * @return unsigned int 
 */
static unsigned int randomise()
{
	return time(NULL) * clock() * getpid();
}
/**
 * @brief Parses the input from stdin and stores into edge struct
 * @details Function takes input data in for of 'u-v' and parses u and v vertices into array of edges node A and node B
 * @param argc Number of arguments
 * @param argv Argument array
 * @param edges Pointer to array of edges in which parsed data will be stored
 * @return int Returns 0 in case of success and -1 in case of wrong input
 */
int readEdges(const int *argc, char *argv[], edge *edges)
{

	int node_A = 0;
	int node_B = 0;
	int i;

	for (i = 1; i < *argc; i++)
	{
		if (sscanf(argv[i], "%d-%d", &node_A, &node_B) != 2)
		{
			fprintf(stderr, "%s %d: ERROR. Edges must be in the form number-number.\n", myprogram_gen, (int)getpid());
			return -1;
		}

		edges[i - 1].Anode = node_A;
		edges[i - 1].Bnode = node_B;
	}
	return 0;
}
/**
 * @brief Fills the array of vertices with vertices from 0 to number of vertices in increasing order
 * 
 * @param vertices pointer to ineger array of vertices
 * @param num_vertices maximal number of vertices
 */
static void vertex_init (int *vertices, size_t num_vertices) {
    for (int i = 0; i < num_vertices; i++) {
        vertices[i] = i;
    }
}
/**
 * @brief Function searches for the maximal value of vertice in edges nodes, 
 * and since it starts with 0, it is increased one more time at the end
 * 
 * @param edges - array of edges with two connected vertices nodeA and nodeB
 * @param n - number of edges
 * @return max - maximal value of vertice in edge, increased by 1
 */
int get_number_of_vertices(edge *edges, int n) {

	int max = 0;
	for (int i = 0; i < n; i++) {
		max = MAX(edges[i].Anode, max);
		max = MAX(edges[i].Bnode, max);
	}
	max++;
  return max;
}
/**
 * @brief Function sets output value of the resulting solution. Output will be written to stdout afterwards
 * @details Function iterates over all solution indexes in sol[], for each index, value in edges at given
 * index is stored in output. For writting in output, strcat (concatinate) funtion is used
 * @param output - array of characters that needs to be filled with solution
 * @param edges - pointer to edges
 * @param sol - array of soultion indexes that are to be set in output
 * @param sol_num - number of solutions, size of sol
 */
void set_output(char output[], edge *edges, int sol[], int sol_num) {
    for (int i = 0; i < sol_num; ++i)
        {
            char Anode[16], Bnode[16];
            sprintf(Anode, "%d", edges[sol[i]].Anode);
            sprintf(Bnode, "%d", edges[sol[i]].Bnode);
            strcat(output, Anode);
            strcat(output, "-");
            strcat(output, Bnode);
            strcat(output, " ");
        }
}
/**
 * @brief Function randomly shuffles array ov vertices by replacing i and j vertice multiple times
 * 
 * @param vertices - pointer to integer array of vertices
 * @param num_vertices - total number of vertices in array
 */
void shuffle_vertices(int *vertices, size_t num_vertices)
{
	vertex_init(vertices, num_vertices);

	//use random function to shuffle vertices
    for (int i = 0; i < num_vertices; i++)
    {
        int j = (rand() % (i+1));
        int temp = vertices[i];
        vertices[i] = vertices[j];
        vertices[j] = temp;
    }
}
/**
 * @brief Function gets the lenght of character array of all solutions produced by sol array
 * 
 * @param edges - pointer to array of edges
 * @param sol - array of indexes of solution 
 * @param sol_num - total number of indexes in sol array, length of sol array
 * @return int - returns sum of all lengths of nodes in solution array
 */
int get_len(edge *edges, int sol[], int sol_num){

    //calcualte the total length needed for output
    int l = sol_num * 2;
    l++;
    for (int i = 0; i < sol_num; ++i)
    {
        char Anode[16], Bnode[16];

        //converting integer value of Anode and Bnode into character array
        sprintf(Anode, "%d", edges[sol[i]].Anode);
        sprintf(Bnode, "%d", edges[sol[i]].Bnode);
		//sums up lengths of character arrays
        l += strlen(Anode);
        l += strlen(Bnode);
    }
    return l;
}
/**
 * @brief Help function that returns index of vertex in vertices array
 * 
 * @param vertices - pointer to array of vertices
 * @param num_vertices - total number of vertices
 * @param vertice - vertex for which index is required
 * @return i - returns index of vertex in vertices array or -1 in case if it is not found
 */
int index_of_vertice(int *vertices, int num_vertices, int vertice)
{
    for (int i = 0; i < num_vertices; i++)
    {
        if (vertices[i] == vertice)
        {
            return i;
        }
    }

    return -1;
}

int main(int argc, char *argv[])
{
	//get name of the program
	myprogram_gen = argv[0];

	if (argc == 1)
	{
		usage();
	}


	//open existing buffer
	buffer *buff = open_buff(0);

	if (buff == NULL) {
		fprintf(stderr, "%s %d: ERROR. Failed to open shared memory.\n", myprogram_gen, (int)getpid());
		return (EXIT_FAILURE);
	}

	int num_edges = argc - 1;

	//create array of edges and vertices
	edge *edges = malloc((num_edges) * sizeof(edge));
	int *vertices = malloc((num_edges) * 2 * sizeof(int));

	//read edges from input
	if (readEdges(&argc, argv, edges) < 0) {

		free(edges);
		free(vertices);
        exit(EXIT_FAILURE);
	}

	//get number of vertices
	int num_vertices = get_number_of_vertices(edges, num_edges);
    
	//get random seed
	srand(randomise());

	int min = MAX_EDGE;

	int num_sol = 0;
	int sol[num_edges];


	while(true){

		if (buff->exit) {
			printf("Exit request!\n");
			break;
		}

		shuffle_vertices(vertices, num_vertices);

		num_sol = 0;

		for (int i = 0; i < num_edges; i++)
        {
            int A = index_of_vertice(vertices, num_vertices, edges[i].Anode);
            int B = index_of_vertice(vertices, num_vertices, edges[i].Bnode);

            if (A == -1 || B == -1)
            {
                exit(-1);
            }

            if (A > B)
            {
                sol[num_sol++] = i;
            }
        }

		if (num_sol > min)
        {
            continue;
        }

        min = num_sol;


		//calcualte the toal length
        int l = get_len(edges, sol, num_sol);
        char output[l];
        output[0] = 0;
        set_output(output, edges, sol, num_sol);
        
		if (write_buff(buff, output) == -1)
        {
			fprintf(stderr, "%s %d: ERROR. Failed to write to shared memory.\n", myprogram_gen, (int)getpid());
            exit(EXIT_FAILURE);
        }
	}

	free(vertices);
	free(edges);

	exit(EXIT_SUCCESS);
}
