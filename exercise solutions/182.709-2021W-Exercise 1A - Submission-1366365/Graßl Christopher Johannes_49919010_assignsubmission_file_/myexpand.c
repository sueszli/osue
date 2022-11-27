/**
*@file: myexpand.c
*@author: Christopher Graßl (11816883) <e11816883@student.tuwien.ac.at>
*@date: 13.11.2021
*
*@brief: (main) programm module for myexpand
*
*@details:	myexpand reads one or several files and replaces its tabs
*			with spaces and writes the result to a given output file (-o).
*			The number of spaces can be changed with the option -t (default: 8).
*			If now output/input is given, the programm writes to stdout/stdin.
**/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


char *progName;	/**< Programm name */

/**
*Usage function
*@brief: This function writes usage Information to stderr
*@details: global variables: progName
*/
static void usage(void){
    fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n", progName);
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
*expand function
*@brief: This function replaces the tab characters in input and writes the result to the output
*@details: The function checks every character of the input. If it's tab character
*			then it's replaced with the given number of spaces. Other characters are written
*			to the output unchanged
*@param:	*in pointer to the input stream
*@param:	*out pointer to the output stream
*@param:	tabstop number of spaces to replace tabs 
*/
static void expand(FILE *in, FILE *out, int tabstop){
    char c;
    int pos = 0;

    while((c = fgetc(in)) != EOF){
        if(c == '\t'){
            int p = tabstop*((pos/tabstop)+1);
            for(;pos<p;pos++){
                fprintf(out," ");
            }
        }else{
            fprintf(out,"%c",c);
            pos++;
            if(c == '\n'){
                pos = 0;
            }
        }
    }
}


/**
*main programm
*@brief: entry point to the programm.
*@details: This function takes care of the arguments given for the programm
* as well as setting up the input and output stream. Reading and writing are
* done in a separate function.
* @param: argc The argument counter
* @param: argv The argument vector
* @return: The programm terminates with EXIT_SUCCESS
*/
int main(int argc, char *argv[]){
    int tabstop = 8; /**< default value for tabstop*/
    char *endptr = NULL; /**< pointer used to check if entered tabstop is a number */
    int c;					/**< saves the return value of getopt */
    char *outfile = NULL;	/**< name of the output file */
    FILE *out = NULL;		/**< output stream */

	//parse options
    while((c = getopt(argc,argv,"t:o:")) != -1){
        switch(c){
                case 't':
                    tabstop = (int) strtol(optarg,&endptr, 10);
					//checking if value for tabstop is a number
					if(*endptr !='\0'){
						usage();
					}
                    break;
                case 'o':
                    outfile = optarg;
                    break;
                case '?':
                    usage();
                    break;
				default:
					assert(0);
        }
    }

    //checking if value for tabstop is valid
    if(tabstop>INT_MAX || tabstop<=0){
        errorExit("value for tabstop not valid!\n tabstop must be positive integer\n");
    }


    //Preparing the output Stream
    if(outfile == NULL){
        out = stdout;
    }else{
        out = fopen(outfile,"w");
        if(out == NULL){
            errorExit("fopen failed\n");
        }
    }

    //calling the expand function
    if(argc == optind){
        expand(stdin,out,tabstop);
    }else{
        for(int i=optind;i<(argc);i++){
            FILE *input = fopen(argv[i],"r");
            if(input == NULL){
                errorExit("fopen failed\n");
            }
            expand(input,out,tabstop);
            fclose(input);
        }
    }

    //close output stream
    if(outfile != NULL){
        fclose(out);
    }


    exit(EXIT_SUCCESS);
}