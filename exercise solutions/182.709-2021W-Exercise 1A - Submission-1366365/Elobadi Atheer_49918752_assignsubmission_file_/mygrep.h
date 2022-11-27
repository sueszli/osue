/**
 * @file mygrep.h
 * @author Atheer Elobadi <e01049225@student.tuwien.ac.at>
 * @date 30.10.2021
 *  
 * @brief This module contains util functions which are used for finding a keyword in lines either given in stdin or in files 
 */ 
#ifndef MYGREP_H
#define MYGREP_H


void usage(char* programName);

int handleOptions(int *opt_i, int *opt_o, char** outputFileName, int argc, char *argv[]);

int putLinesIfContainString(FILE *input, int opt_i,  char *keyword, FILE *output);

void toLowerCase(char *line);

#endif

