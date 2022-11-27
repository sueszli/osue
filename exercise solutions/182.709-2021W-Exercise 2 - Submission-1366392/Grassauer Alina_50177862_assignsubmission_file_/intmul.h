/**
 * 
 * @file intmul.h
 * @author Alina Grassauer 11905176
 * @date 07.12.2021
 * @brief BSUE Exercise 2 intmul
 * @details 
 * 
 */

#ifndef _INTMUL_H
#define _INTMUL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief represents the number of child processes/pids/pipes we need/have
 * 
 */
#define CHILD_COUNT 4

/**
 * @brief read end from our pipes
 * 
 */
#define READ_END 0

/**
 * @brief write end form our pipes
 * 
 */
#define WRITE_END 1

#endif