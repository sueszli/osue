/*
@autor: Jessl Lukas 01604985
@modulename: generator.c
@created 05.11.2021

This programm get's a list of edges. It writes a list of edges to a shared memory, that create an acylic graph. Those edges are the edges, that would need to be removed from our original graph so that the original graph is 
acyclic. The list of edges written to our shared memory can be empty which means that the original graph is already acyclic without removing any edge. */

#include "generator.h"

/* @param argc: defines the amount of edges give to the Programm (argc-1)
   @param argv: Is a list of edges in the form of xx-xx (where xx is a digit). The max number for nodes is 999. If there are more nodes, the solution will not be stored correctly
   on our shared memory.

   Get's a list of edges, which decides the nodes in this graph. First the nodes are "read" from this list. Then every duplicate entry will be deleted from this list, leaving every unqiue node in the graph.
   This list of unique nodes will then be randomized in a loop, to create lists of edges, which form an acyclic graph. Those edges are our possible solutions

    @return: EXIT_SUCCESS when the programm finishes.*/

int main(int argc, char *argv[]){
	
	char* input = NULL;
	int first = 1;
	char* s = " ";

	if(argc == 1){
		fprintf(stderr, "ERROR, no edge has been given.");
		exit(EXIT_FAILURE);
	}

	while(argc-optind >0){
		
		if(first == 1){
			input = calloc(strlen(argv[optind]), sizeof(char));
			first = 0;
		} else{
			input = realloc(input, (strlen(input) + strlen(argv[optind])+1)*sizeof(char));
		}
		
		strcat(input, argv[optind]);
		
		strcat(input, s);
			
		optind ++;
	}
	
	//Open shared memory, last String from input defines the name of the shared memory
	char* shm_name = "1604985_sharedmemory";
	int fd = shm_open(shm_name, O_RDWR, 0600);
	
	if(fd == -1){																			
		fprintf(stderr, "ERROR @generator.c , shared memory could not be opened %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	struct shm_circular_buffer *shm_cm = mmap(NULL, sizeof(*shm_cm), PROT_READ | PROT_WRITE, MAP_SHARED, fd , 0);
	
	if( shm_cm==MAP_FAILED){
		fprintf(stderr, "ERROR @generator.c Could not map circular buffer storage %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	sem_t* mutex, *sem_read_left_1604985, *sem_write_left_1604985;

	if((sem_write_left_1604985 = sem_open("1604985_write_semaphore", O_RDWR)) == SEM_FAILED){
		fprintf(stderr, "ERROR @generator.c could not create write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if((sem_read_left_1604985 = sem_open("1604985_read_semaphore", O_RDWR)) == SEM_FAILED){
		fprintf(stderr, "ERROR @generator.c could not create read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	if((mutex = sem_open("1604985_mutex", O_RDWR)) == SEM_FAILED){
		fprintf(stderr, "ERROR @generator.c could not create write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	char* graphnodes = calloc(1,sizeof(char));
	int numberofnodes = 0;
	int* p_numberofnodes = &numberofnodes;
	
	graphnodes = unsortedinputnodes(input,graphnodes);
	graphnodes = removedoublenodes(graphnodes,p_numberofnodes);					//from here all nodes are only once in the list, not sorted, but they are unique. (it doesn't matter if they are sorted as we will randomize them anyway)

	srand(time(0));
	 
	while(shm_cm->generator_stop == 0){
		
		char* randomnodes = calloc(1,sizeof(char));
		randomnodes = randomizenodes(graphnodes, numberofnodes);

		int maxEdges = shm_cm->best_length;											//takes best length from the shared memory, this will get smaller and smaller depending on the best found solution. If a solution with 6 edges is found, then there is no reason to store something with length 7 or another solution with 6. 
		int number_of_edges = 0;
		int* p_Edges = &number_of_edges;

		char* possiblewedges = calloc(shm_cm->best_length, sizeof(char));
	
		possiblewedges = testwedges(randomnodes, input, numberofnodes, p_Edges);	
	
		sem_wait(mutex);

		int* value = malloc(sizeof(int));

		int count = 0;

		if(number_of_edges < maxEdges){

			sem_wait(sem_write_left_1604985);

			if(sem_getvalue(sem_write_left_1604985, value)== -1){
				fprintf(stderr, "ERROR @generator.c Could not get value from read_left %s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}

			for(count = 0; count < strlen(possiblewedges); count++){
				shm_cm->circular_buffer[shm_cm->write_position][count] = possiblewedges[count];
			}

			printf("Position = %d\n", shm_cm->write_position );
			printf("Solution = %s\n", shm_cm->circular_buffer[shm_cm->write_position]);

			shm_cm->write_position++;
			if(shm_cm->write_position == CIRCULAR_BUFFER_SIZE){
				shm_cm->write_position = 0;
			}

			sem_post(sem_read_left_1604985);
		}

		sem_post(mutex);

		free(possiblewedges);
		possiblewedges = NULL;

	}
			
	free(input);
	free(graphnodes);

	closesharedmemory(shm_cm, fd, shm_name, mutex, sem_read_left_1604985, sem_write_left_1604985);
	
	return EXIT_SUCCESS;
}

/* @param wedges: is the list of edges, given to our programm. 
   @param graphnodes: is a empty list in which the nodes will be stored.

   This function splits all edges by "-", as they are given in the form xx-xx, (where xx stands for any digit, max 999). The left and right part on each edge
   represent nodes, which will all be stored in a new list. This list can contain the same node multiple times.

   @return. returns the pointer to our possible nodes.*/
char* unsortedinputnodes(char* wedges,char* graphnodes){

	char* copywedges = malloc(sizeof(char) * strlen(wedges));
	strcpy(copywedges,wedges);
	char* s = " ";
	
	char* pairs = strtok(copywedges,"-");
	while(pairs != NULL){

		graphnodes = realloc(graphnodes, (strlen(pairs) + strlen(graphnodes)+1) * sizeof(char));

		strcat(graphnodes,pairs);
		strcat (graphnodes, s);
		

		pairs = strtok(NULL,"-");
	}	
	
	if(copywedges != NULL){
		free(copywedges);
		copywedges = NULL;
	}

	return graphnodes;
}

/* @param graphnodes: contains every possible node in this graph.
   @param p_numberofnodes: is a pointer to a int with value 0 .

   Creates a new list, where our nodes are stored. Every node will now only be stored, if it is not already contained in our list, leaving
   us with a unique list of nodes.

   @return: a pointer to our unique nodes*/
char* removedoublenodes (char* graphnodes, int* p_numberofnodes){
	
	char* uniquenodes = NULL;
	uniquenodes = calloc(1,sizeof(char));
	
	int first = 1;	
	char* testnodes = NULL;
	char* s = " ";
	char* firstnumber = NULL;

	char* nodes = strtok(graphnodes, s);
	while(nodes!=NULL){
		
		if(first == 1){
			firstnumber = calloc((strlen(nodes)+2), sizeof(char) );
			uniquenodes = realloc(uniquenodes, sizeof(char) * (strlen(nodes)+1));
			strcat(uniquenodes,nodes);
			strcat(uniquenodes,s);
			
			strcat(firstnumber,s);
			strcat(firstnumber,uniquenodes);
			
			*p_numberofnodes = *p_numberofnodes + 1;
			
			first = 0;
		} else{
			testnodes = NULL;
			testnodes = calloc((2+strlen(nodes)), sizeof(char));
			strcpy(testnodes,s);
			strcat(testnodes,nodes);
			strcat(testnodes,s);
			
			if(strstr(uniquenodes,testnodes)==NULL){
				if(strstr(firstnumber,testnodes) == NULL){
					uniquenodes = realloc(uniquenodes, sizeof(char) * (strlen(uniquenodes) + strlen(nodes) +1));
					strcat(uniquenodes,nodes);
					strcat(uniquenodes,s);
					
					*p_numberofnodes = *p_numberofnodes +1;
				}
			}
			free(testnodes);
			testnodes = NULL;
		}
		
		nodes = strtok(NULL,s);	
	}
	
	graphnodes = realloc(graphnodes,sizeof(char)* strlen(uniquenodes));
	graphnodes = uniquenodes;

	free(firstnumber);
	firstnumber = NULL;

	return graphnodes;
}

/* @param graphnodes: is the list of uniquenodes in our graph
   @param numberofnodes: the amount of nodes in our list graphnodes.

   This functon permutates the list of unique nodes, in a way where a node between 0 and current number of nodes left in graphnodes. 
   This node is added to the end of our new list of nodes. When a new node is chosen it splits our current list into a first and second half, where
   the first half is every node untill our chosen node and the second half is every node after our chosen node. After the chosen node is added to 
   our randomized nodes, the first and second half are being added together and another random node is chosen untill the first and second half are empty.

   @return: a random perumtation of our uniquenodes. */
char* randomizenodes(char* graphnodes, int numberofnodes){
	
	char* copygraphnodes = calloc(strlen(graphnodes),sizeof(char));
	strcpy(copygraphnodes,graphnodes);
	char* randomizednodes = calloc(strlen(graphnodes), sizeof(char));
	char* s = " ";
	int i = 0;
	
	while(i < numberofnodes){
		
		int randomnumber = rand() % (numberofnodes -i);
		int k = 0;
		
		char* firsthalfnodes = calloc(strlen(graphnodes),sizeof(char));
		char* secondhalfnodes = calloc(strlen(graphnodes),sizeof(char));
		
		char* nodes = strtok(copygraphnodes, s);
		while(nodes != NULL){
			
			if(k < randomnumber){
				strcat(firsthalfnodes,nodes);
				strcat(firsthalfnodes, s);
			} else  if(k == randomnumber){
				strcat(randomizednodes,nodes);
				strcat(randomizednodes,s);
			} else{
				strcat(secondhalfnodes,nodes);
				strcat(secondhalfnodes,s);
			}
			
			k++;
			nodes = strtok(NULL, s);
		}

		strcat(firsthalfnodes,secondhalfnodes);
		
		strcpy(copygraphnodes,firsthalfnodes);
		

		free(firsthalfnodes);		
		firsthalfnodes = NULL;
		free(secondhalfnodes);
		secondhalfnodes = NULL;

		i++;
	}
	
	free(copygraphnodes);
	copygraphnodes = NULL;

	return randomizednodes;
}

/* @param randomizednodes: a list of randomized nodes from our graph (contains every node in the graph, just in a random order)
   @wedges: the list of edges given to the programm
   @numberofnodes: the number of entries in our list of randomized nodes
   @p_Edges: is a pointer to a int with value 0. It Defines how many edges are stored to our possible solution.

   Loops through our list of randomized nodes. There are good and bad edges defined. If the currently chosen node (going from left to right in our list), is on the left side of a edge 
   it is seen as a bad edge as it is further left in our list and it is on the left side of a ege. If the chosen node is on the right side of a edge it is seen as good and added to our solution
   Bad nodes are being removed from our list of possible solutions. 

   @returns a list of edges, that are a possible solution*/
char* testwedges(char* randomizednodes, char* wedges, int numberofnodes,int* p_Edges){
	
	char* possiblesolutionwedges = calloc(strlen(wedges), sizeof(char));
	
	char* copywedges = malloc(strlen(wedges)* sizeof(char));
	strcpy(copywedges, wedges);
	
	char* s = " "; 				//just my usual string to split my lists appart.
	char* min = "-";			//makes strcat easier, adds the - for wedges;
	
	char* list[numberofnodes];
	
	int i = 0;
	
	char* nodes = strtok(randomizednodes,s);
	while(nodes!=NULL){
		list[i] = nodes;
		i++;
		nodes = strtok(NULL,s);
	}
	
	for(i = 0; i < numberofnodes; i++){
		
		char* newcopywedge = calloc(strlen(copywedges), sizeof(char));

		char* badnodes = malloc((strlen(list[i]) + 1)* sizeof(char));
		strcpy(badnodes,list[i]);
		strcat(badnodes,min);
			
		char* goodnodes = malloc( (strlen(list[i]) +1)* sizeof(char));
		strcpy(goodnodes,min);
		strcat(goodnodes,list[i]);
			
		char* wedge = strtok(copywedges, s);			//splits the list of wedges into single wedges;
		while(wedge != NULL){

			if(strstr(wedge,badnodes)!=NULL){
				//do not add this wedge.	
			} else if(strstr(wedge,goodnodes)!=NULL){
				strcat(possiblesolutionwedges,wedge);
				strcat(possiblesolutionwedges,s);
				*p_Edges = *p_Edges +1; 
			} else{
				strcat(newcopywedge, wedge);
				strcat(newcopywedge,s);
			}
			
			wedge = strtok(NULL, s);
		}
		
		
		if(strlen(newcopywedge)==0){
			break;
		}
		strcpy(copywedges,newcopywedge);
		
	}
	
	return possiblesolutionwedges;
}

void closesharedmemory(struct shm_circular_buffer *shm_cm, int fd, char* shm_name, sem_t* mutex ,sem_t* sem_read_left_1604985 ,sem_t* sem_write_left_1604985){
	
	if(munmap(shm_cm,CIRCULAR_BUFFER_SIZE) == -1){
		fprintf(stderr, "ERROR @generator.c Error unmap circularbuffer %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(close(fd)==-1){
		fprintf(stderr, "ERROR @generator.c Could not close file descriptor %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(sem_write_left_1604985) == -1){
		fprintf(stderr, "ERROR @generator.c Could not close write semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(sem_read_left_1604985) == -1){
		fprintf(stderr, "ERROR @generator.c Could not close read semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(mutex) == -1){
		fprintf(stderr, "ERROR@generator.c Could not close mutex semaphore %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
}