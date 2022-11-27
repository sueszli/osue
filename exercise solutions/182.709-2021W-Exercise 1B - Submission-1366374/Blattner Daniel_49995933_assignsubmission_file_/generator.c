/**
 * @file generator.c
 * @author Daniel Blattner <e12020646@students.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief This programm generate solutions for a feedback arc set.
 *
 * The generators gets an graph with the arguments and should find a minimal feedback
 * act set for it. This is achieved through a randomized algorithm. Because of that the 
 * feedback are set must not be minimal and therefore the generator produce multiple 
 * possible solutions. It is advices that multiple generators are used at the same time
 * to decrease the time to find the perfect solution. The solutions are send, though 
 * a shared memory, to a supervisor. The generator create solution until the supervisor 
 * sends a signal to terminate.
 *
**/

#include "circularbuffer.h"
#include <time.h>

/** Name of programm as defined in argv[0] **/
static char *progName;

/**
 * @brief This function prints how the arguments of the programm
 * should be written. 
 * @details The nodes should be separated by a '-' and only consist of
 * natural numbers. 
 * global variables: progName
**/
static void printProperUsage(void)
{
   fprintf(stderr,"Usage: %s <startNodeNr>-<endNodeNr>... node must be a natural number\n", progName);
   exit(EXIT_FAILURE);
}

/**
 * @brief This function searches the interger c in the array and returns the index
 * of the first occurrence. 
 * @param array The array which is searched in.
 * @param len The lengh of the array.
 * @param c The integer which is searched. 
 * @return Return the index of the first occurrence or -1 if the integer was not found.
**/
static int searchIntArray(int *array, int len, int c)
{
   assert(array != NULL);
   assert(len > 0);

   //Search int in array and return its index
   for(int i=0; (i < len) && (array[i] != -1); i++){
      if(array[i] == c) return i;
   }
   
   return -1;
}

/**
 * @brief This function searches the integer c in the array. If the integer was not found
 * in the array, it will be added at the position index. 
 * @param array The array which is searched.
 * @param len The lenght of the array.
 * @param c The integer which will be added, if not already in array.
 * @param index The index where the integer will be added.
 * @return Return an incremented index, if the integer is added. 
**/
static unsigned int notInArraySet(int *array, int len, int c, unsigned int index)
{
   assert(array != NULL);
   assert(len > 0);
   assert(index < len);

   //If c is not in array set it at index
   if(searchIntArray(array, len, c) == -1){
      array[index] = c;
      index++;
   }

   return index;
}

/**
 * @brief This function searches the edge array for all its nodes. Every node (id) is saved
 * in the node array. 
 * @details The node array have to be twice the lengh of the edge array. After the extraction
 * of the nodes, the array will be fitted to the right size. 
 * global variables: progName
 * @param edgeArray The array of the edges.
 * @param len Lenght of the array of edges.
 * @param nodeArray The array of nodes. 
 * @return Retuns the amount of nodes found. 
**/
static unsigned int getNodeFromEdgeArray(Edge *edgeArray, unsigned int len, int *nodeArray)
{
   assert(edgeArray != NULL);
   assert(len > 0);

   //Searches if node from edge is already found
   //If not add node to array
   unsigned int index = 0;
   for(int i=0; i<len; i++){
      nodeArray[index] = -1;
      index = notInArraySet(nodeArray, len*2, edgeArray[i].start, index);
      nodeArray[index] = -1;
      index = notInArraySet(nodeArray, len*2, edgeArray[i].end, index);
   }

   //Fit memory from node array to its size
   nodeArray = realloc(nodeArray, index*sizeof(int));
   if(nodeArray == NULL){
      printErrorMsg(progName, "Could not cut node array");
      return -1;
   }

   return index;
}

/**
 * @brief This function goes though every argument and extract the edges.
 * If an argument is not in a proper format, the function printf the proper
 * usage and exits.
 * @details global variables: progName
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @param array The array where the edges are saved.
 * @return Return true if no error happened, else false. 
**/
static bool extractPositionalArguments(int argc, char *argv[], Edge *array)
{
   assert(array != NULL);

   for(int i=1; i<argc; i++)
   {
      //Extract start node
      char *nextNode;
      array[i-1].start = strtol(argv[i], &nextNode, 10);
      if(errno != 0){
         printErrorMsg(progName, "Could not parse positional argument");
         return false;
      }

      if(nextNode == argv[i] || *nextNode == '\0' || *nextNode != '-'){
         printProperUsage();
      }

      //Extract end node
      array[i-1].end = strtol(&(nextNode[1]), NULL, 10);
      if(errno != 0){
         printErrorMsg(progName, "Could not parse positional argument");
         return false;
      }
   }

   return true;
}

/**
 * @brief This function extract first og all every edge given in the arguments.
 * Then it searches every node in the edges. The edges and nodes are saved and
 * returned as a struct.
 * @details If an error occured, the program free/close all shared resources,
 * print an error message and exits with an EXIT_FAILURE.
 * global variables: progName
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @param res The pointer to the shares resources struct.
 * @return Returns the resulting graph.
**/
static Graph getGraphFromArguments(int argc, char *argv[], SharedResource *res)
{
   assert(res != NULL);

   Graph g = {.edgeLen = argc-1};

   //Extract edge from positional arguments
   g.edgeArray = malloc(g.edgeLen * sizeof(Edge));
   if(g.edgeArray == NULL){
      destroySharedResource(progName, res, false);
      printErrorMsgExit(progName, "Could not allocate memory for edges");
   }

   if(!extractPositionalArguments(argc, argv, g.edgeArray)){
      destroySharedResource(progName, res, false);
      exit(EXIT_FAILURE);
   }

   //Extract nodes from edge array
   g.nodeArray = malloc(2*g.edgeLen*sizeof(int));
   if(g.nodeArray == NULL){ 
      free(g.edgeArray);
      destroySharedResource(progName, res, false);
      printErrorMsgExit(progName, "Could not allocate memory for node array");
   }

   g.nodeLen = getNodeFromEdgeArray(g.edgeArray, g.edgeLen, g.nodeArray);
   if(g.nodeLen == -1){
      free(g.nodeArray);
      free(g.edgeArray);
      destroySharedResource(progName, res, false);
      exit(EXIT_FAILURE);
   }

   return g;
}

/** 
 * @brief This function implements a randomized algorithmus to find a
 * feedback arc set of a given garph. The solution is an array of edges,
 * when removed from the graph results in a feedback act set.
 * @details The function only return solution with a maximum lengh of
 * DATA_LIMIT. Therefore the solution can be written to the circular buffer.
 * global variables: progName
 * @param g The struct of an directed graph.
 * @param res The pointer to the shared resources struct.
 * @return Return an array of edges, when removed from a graph result 
 * in a feedback arc set.
**/
static EdgeArray generateSolution(Graph g, SharedResource *res)
{
   assert(res != NULL);

   int indexRem;
   Edge remEdge[g.edgeLen];
   EdgeArray solution;
   srand(time(0));

   //Generate solution until it is small enough to send it
   do{
      //Supervisor terminate generators
      if(res->buffer->termGen == 1) return solution;

      //Random permutation of nodes
      for(int i=g.nodeLen-1; i >= 0; i--){
         int j = rand()%(i+1);
         int buf = g.nodeArray[i];
         g.nodeArray[i] = g.nodeArray[j];
         g.nodeArray[j] = buf;
      }
   
      //Select edges which can be removed
      indexRem = 0;
      for(int i=0; i<g.edgeLen; i++){
         int indexStart = searchIntArray(g.nodeArray, g.nodeLen, g.edgeArray[i].start);
         int indexEnd = searchIntArray(g.nodeArray, g.nodeLen, g.edgeArray[i].end);
         if(indexStart == -1 || indexEnd == -1){
            free(g.nodeArray);
            free(g.edgeArray);
            destroySharedResource(progName, res, false);
            printErrorMsgExit(progName, "Edge with nodes not in graph");
         }
         //This edge can be removed
         if(indexStart > indexEnd){
            remEdge[indexRem].start = g.edgeArray[i].start;
            remEdge[indexRem].end = g.edgeArray[i].end;
            indexRem++;
         }
      }

   }while(indexRem > DATA_LIMIT);

   //Return solution
   solution.len = indexRem;
   memcpy(solution.array,remEdge,indexRem * sizeof(Edge));

   return solution;
}

/**
 * @brief The programm starts here. The generator open the shared memory and semaphores.
 * These must be created by a supervisor before or the programm exits with an error. Then
 * the program extract the graph form the arguments and generate solution for a feedback
 * arc set. When a solution is small enough, the generator write that solution to the 
 * circular buffer. The generator terminate on a signal of the supervisor. The signal is
 * send though the shared memory. 
 * @details global variables: progName
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
**/
int main(int argc, char *argv[])
{
   //Checks if an edge is given
   if(argc < 2) printProperUsage();
   progName = argv[0];

   //Open/map shared memory and semaphores
   SharedResource res;
   initSharedResource(progName, &res, false);

   //Extract graph from positional arguments
   Graph g = getGraphFromArguments(argc,argv, &res);

   do{
      //Generate new solution
      EdgeArray newSolution = generateSolution(g, &res);

      //Supervisor terminate generators
      if(res.buffer->termGen == 1) break;

      //Send solution to circular buffer
      if(!writeEdgeArray(progName, &res, newSolution)) break;
      
   }while(res.buffer->termGen != 1);

   //Free graph ressources
   free(g.edgeArray);
   free(g.nodeArray);
   //Close/unmap shared memory and semaphores
   destroySharedResource(progName, &res, false);

   return EXIT_SUCCESS;
}