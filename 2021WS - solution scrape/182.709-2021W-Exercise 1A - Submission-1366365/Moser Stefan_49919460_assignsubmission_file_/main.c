/**
 * @file main.c
 * @author Stefan Moser 12025955
 * @date 9.11.2021
 *
 * @brief mygrep
 *
 * @details the most important usecase of mygrep is to search a given keyword casesensitive or insensitive from one or more input files.
 * The keyword and files are given in the argument list of mygrep.
 * mygrep prints all lines found to stdout or a defined output file
 * in case there are no input files given, mygrep inspects the lines entered via stdin
 * How to call my grep:
 * mygrep [-i] [-o outfile] keyword [file...]
 * [-i]: if -i option is given, the program shall ignore the case
 * [-o outfile]: if -o option is given, the program writes found lines to given output file
 * keyword: not optional
 * [file...]: input files, if there are multiple files, mygrep checks them in order
 *
 * The main program reads the command line arguments into the InputArguments_t datastructure
 * (see readArgs.h) the information of this datastructure guides the process of the whole programm
 * and is passed as a parameter to all processing steps
 * The datastructure additionaly keeps all open file handles and is used to clean up and close files
 * in case of errors.
 * Additionaly the main program differentiates the two main processing steps
 * namely grep from Stdin (see function grepStdin in smgrep.h) or grep from input files
 * (see funtion grepFiles in smgrep.h)
 * program can only be used to grep ascii files
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "readArgs.h"
#include "smgrep.h"

int main(int argc, char *argv[]){

	InputArguments_t arguments;

	/*read and analyze all arguments from command line*/
	initArguments(&arguments);
	analyzeInputArguments(argc, argv, &arguments);

	/*read either from stdin or input files, if there are any*/
	if(arguments.countInputFiles == 0){
		grepStdin(&arguments);
	}
	else grepFiles(&arguments);
	/*free memory*/
	cleanUpInputArgs(&arguments);
}
