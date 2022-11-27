/** 
* @file: supervisor.c
* @author: Christopher Graßl (11816883) <e11816883@student.tuwien.ac.at>
* @date: 14.11.2021
*
*@brief: 	main programm module for the 3-coloring supervisor.
* 			Handles shared memory and semaphore creation, compares the
* 			solutions given by the generator(s) and writes them to stdout.

*@details: 	the supervisor sets up a shared memory to communicate
* 			and share solutions with the generator(s). It also compares
*			the solutions and prints new best solutions to stdout. Upon
*			finding a satisfying solution it notifies the generators and
*			terminates.
*/

#include "shm.h"


volatile sig_atomic_t quit = 0; /**< global quit flag for the singal handler */

char *progName;					/**< Programm name */

/**
*signal Handling function
*@brief: Function to handle SIGINT and SIGTERM signals and initiate termination of the programm
*@details: Sets the global 'quit' flag to initiate termination
*@param: signal
*/
void handleSignal(int signal){
	quit = 1;
}

/** 
*Usage function
*@brief: 	This function writes usage information to stderr
@details: 	global variables: progName
*/
static void usage(void){
	fprintf(stderr, "Usage: %s ",progName);
	exit(EXIT_FAILURE);
}
/**
*errorExit function
*@brief: This function writes Information about an occurred error and terminates the programm
*@details: 	The input string is used to print a helpful error message to stderr
*			global variables: progName
*@param: msg: String which is printed to stderr
*/
static void errorExit(char *msg){
	fprintf(stderr, "%s: %s \n", progName, msg);
	fprintf(stderr, "cause: %s \n", strerror(errno));
	exit(EXIT_FAILURE);
}

/** 
*compare function
*@brief: 	This function compares two solutions
*@details: 	compares the two given solutions according to their value
*			returns 1 if newSolution is better and 0 if currentBest is better
*			or equal.
*@param:	currentBest solution which is currently the best
*@param:	newSolution solution which might be better than currentBest
*@return:	returns 1 if newSolution is better, 0 otherwise
*/
static int compare(solution_t currentBest, solution_t newSolution){
	if(newSolution.value < currentBest.value){
		return 1;
	}

	return 0;
}

/** 
*writeToOut function
*@brief:	This function writes the given solution to stdout
*@details:	prints a given Solution to stdout in the format:
*			"Solution with <value> edges: <edge1> <edge2>..."
*@param:	bestSolution solution to be printed to stdout
*/
static void writeToOut(solution_t bestSolution){
	fprintf(stdout,"Solution with %d edges: ", bestSolution.value);
	for(int i=0;i<(bestSolution.value);i++){
		fprintf(stdout,"%lu-%lu ",bestSolution.edges[i].firstNode,bestSolution.edges[i].secondNode);
	}
	fprintf(stdout,"\n");
}


/** 
*main programm
*@brief: 	entry point to the programm
*@details:	This function takes care of initializing resources,
*			reading and comparing solutions and termination if
*			a satisfying solution is found
*@param: 	argc Argument counter
*@param: 	argv Argument vector
*@return:	The programm terminates with EXIT_SUCCESS
*/
int main(int argc, char *argv[]){

	progName = argv[0];
	if(argc > 1){
		usage();
	}

	//set up signal handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handleSignal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);


	solution_t bestSolution;	//holds the best Solution so far
	bestSolution.value = 9;

	buffer_t *circBuff = NULL;	//shared memory object [shm]
	int fileDescriptor = 0;

	//create and open shm
	if((fileDescriptor = createSHM(&circBuff)) == -1){
		errorExit("failed to create shared memory");
	}
	//initilaize readpos and writepos for the circular Buffer
	circBuff->readPos = 0;
	circBuff->writePos = 0;

	//create Semaphores
	sem_t *freeSem = NULL;
	if(createSem(&freeSem,"/11816883_freeSem",10) == -1){
		errorExit("failed to create 'freeSem' Semaphore");
	}
	sem_t *used = NULL;
	if(createSem(&used,"/11816883_usedSem",0) == -1){
		errorExit("failed to create 'used' Semaphore");
	}
	sem_t *excl = NULL;
	if(createSem(&excl,"/11816883_exclSem",1) == -1){
		errorExit("failed to create 'excl' Semaphore");
	}

	//loop to read solutions until SIGINT/SIGTERM or a satisfying solution is found
	while(!quit){
		if(sem_wait(used) == -1){
			if(errno == EINTR){
				continue;
			}else{
				errorExit("error waiting on 'used' Semaphore");
			}
		}

		//check if the solution at readPos is better than the current best
		if(compare(bestSolution, circBuff->solutions[circBuff->readPos]) == 1){
			bestSolution = circBuff->solutions[circBuff->readPos];
			writeToOut(bestSolution);
		}

		circBuff->readPos = ((circBuff->readPos + 1)%10);

		if(sem_post(freeSem) == -1){
			errorExit("failed to increment 'freeSem' Semaphore");
		}

		if(bestSolution.value == 0){
			fprintf(stdout,"The Graph is 3-colorable!\n");
			break;
		}

	}

	circBuff->done = true;
	//to make sure generators are terminating (right now limited to 10 generators)
	for(int i=0;i<10;i++){
		if(sem_post(freeSem) == -1){
			errorExit("failed to incremet 'freeSem' Semaphore");
		}
	}
	
	
	//close and unlink shared mem
	if(closeSHM(circBuff, true, fileDescriptor) == -1){
		errorExit("closing shared memory failed");
	}

	//close and unlink semaphores
	if(closeSem(freeSem, "/11816883_freeSem",true) == -1){
		errorExit("closing/unlinking 'freeSem' Semaphore failed");
	}

	if(closeSem(used, "/11816883_usedSem",true)==-1){
		errorExit("closing/unlinking 'used' Semaphore failed");
	}

	if(closeSem(excl, "/11816883_exclSem",true)==-1){
		errorExit("closing/unlinking 'excl' Semaphore failed");
	}

	exit(EXIT_SUCCESS);

}

