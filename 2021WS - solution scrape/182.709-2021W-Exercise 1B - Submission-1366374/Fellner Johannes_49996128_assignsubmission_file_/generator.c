#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<stdbool.h>
#include<time.h>

struct vertex{
	int leftNode;
	int rightNode;
};

int highestNode = 0;

static struct vertex getVertex(char* ver);

static void usage(void);


static struct vertex getVertex(char* ver)
{
	signed int leftNode = -1;
	signed int rightNode = -1;	
	char *endptr = "";

	// to avoid missing left Node
	if(ver[0] == '-')
		(void)usage();

	// split vertex at delimeter "-" to get the left node
	char* token = strtok(ver, "-");
	leftNode = (int) strtol(token, &endptr, 10);
	//fprintf (stderr, "leftnode %d\n", leftNode);
	// check if value is int
	if(*endptr != '\0' || leftNode == -1)
		(void)usage();
	// get the right node of the vertex
	token = strtok(NULL, "-");
	rightNode = (int) strtol(token, &endptr, 10);
	// check if value is int
	if(*endptr != '\0')
		(void)usage();
	if(leftNode == rightNode)
		(void)usage();
	if(leftNode > highestNode)
		highestNode = leftNode;
	if(rightNode > highestNode)
		highestNode = rightNode;
	
	struct vertex v = {leftNode, rightNode};

	return v;
}

static void usage(void)
{
	(void)fprintf(stderr, "Synopsis:\n\tgenerator EDGE1...\n");
	
	exit(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
	struct vertex v[argc-1];
	struct vertex solution[8];
	int solutionCounter;
	bool badSolutionFlag;
	bool running = true;

/*
	Step 1: parse vertices just ones at the start of the program
*/
	for(int i = 1; i < argc; i++)
	{
		v[i-1] = getVertex(argv[i]);
	}
	
	int coloring[highestNode];
	
	fprintf (stderr, "highestNode %d\n", highestNode);
	
	
	
/*
	Repeat steps 2-5 until supervisor sets running flag to false
*/
	while(running)
	{
		badSolutionFlag = false;
		solutionCounter = 0;
		
		for(int u = 0; u < 8; u++)
		{
			solution[u].leftNode = -1;
			solution[u].rightNode = -1;
		}
		
		
/*
	Step 2: generate possible coloring of the nodes
*/
		srand(time(0));
		for(int j = 0; j <= highestNode; j++)
		{
			
			coloring[j] = rand() % 3;
			fprintf (stderr, "coloring[%d] = %d\n", j, coloring[j]);
			
		}
/* 
	Step 3: check if nodes of a vertex are the same color
			if true  -> delete vertex by writing to solution array
			if false -> check next vertex
*/
		for(int k = 0; k < argc-1; k++)
		{
			if(solutionCounter == 7)
			{
				badSolutionFlag = true;
				break;
			}
			fprintf(stderr, "Check color %d == color %d\n", coloring[v[k].leftNode], coloring[v[k].rightNode]);
			if(coloring[v[k].leftNode] == coloring[v[k].rightNode])
			{
				solution[solutionCounter] = v[k];
				solutionCounter++;
			}
		}
		
		for(int u = 0; u < 8; u++)
		{
			fprintf(stderr, "Solution[%d] = %d-%d\n", u, solution[u].leftNode, solution[u].rightNode);
		}
		
		if(badSolutionFlag)
			continue;
	
/*
	Step 4: check running flag from shared memory to know when process should terminate
*/

		running = false;

/*	
	Step 5: write possible solution to cyclic buffer in shared memory
*/
	}
	
	
	  	
	exit(EXIT_SUCCESS);
}
