/**
 * @file pipeActions.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 17 November 2021
 *
 * @brief the pipe actions module
 * 
 * @details This module provides functionality for using 
 * pipes to communicate to child processes in both ways.
 */

#ifndef PIPEACTIONS_H
#define PIPEACTIONS_H

#include "forkConfig.h"
#include "pointActions.h"

/**
 * @brief creates all the necessary pipes
 * 
 * @details This function creates all the necessary pipes. This includes 
 * an parent out pipe, aswell as a parent in pipe for each children.
 * 
 * The file descriptors of the pipe ends will be stored in the pipefds array.
 * The first index is the index of the child processes. The second index
 * corresponds to the in our out pipe of the parent. 0 would be the parent in
 * pipe and 1 the parent out pipe. The third index references the read and 
 * write end of the pipes.
 * 
 * pipefds = [child index][parent in / parent out][read end / write end]
 * 
 * @return 0 on success and -1 otherwise
 */
int createAllPipes(int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

/**
 * @brief closes unnecessary pipes on children
 * 
 * @details This function closes all the file descriptors that are not needed
 * on the children. This function should be called on the child itself where 
 * childIndex would be the child process id given to the child in the parent that
 * corresponds to the child index in pipefds.
 * 
 * All the file descriptors that are not for the child with the id given in 
 * childIndex will be closed. That means after this call the only file descriptors, that
 * are open are the on to comunicate with the parent.
 * 
 * @param childIndex the child process id
 * @return 0 on success and -1 otherwise
 */
int closeUnnecessaryPipesOnChildren(int childIndex, int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

/**
 * @brief replaces the from fd with the to fd
 * 
 * @details This function duplicates the from file descriptor to the 
 * to file descriptor. This function also closes the old file descriptors.
 * 
 * If the from file descriptor and the to file descriptor are equal a warning
 * will be printed using printWarning().
 * 
 * @param fromfd from file descriptor
 * @param tofd to file descriptor
 * @return 0 on success and -1 otherwise
 */
int bendPipeOnChildren(int fromfd, int tofd);

/**
 * @brief closes unnecessary pipes on the parent
 * 
 * @details This function closes all the pipes on the parent that are
 * not needed. This function does not clean up the file descriptors.
 * 
 * This function should be called on the parent after initializing all
 * the children.
 * 
 * @return 0 on success and -1 otherwise
 */
int closeUnnecessaryPipesOnParent(int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

/**
 * @brief writes a given point list to the child processes
 * 
 * @details This function writes the point list left to the first child
 * and and the second point list to the second child.
 * 
 * The correct file descriptor will be read from the pipefds array.
 * 
 * After writing the content to the file descriptors using printPoints()
 * the file descriptor will be closed to enforce an EOF which will indicate
 * the end of the input.
 * 
 * @param left point list for the first child
 * @param leftCount number of points in the first point list
 * @param right point list for the second child
 * @param rightCount number of points in the second point list
 * @return 0 on success and -1 otherwise
 */
int writeToChildren(point_t **left, int leftCount, point_t **right, int rightCount, int (*pipefds)[CHILDPROCESS_COUNT][2][2]);

#endif