/**
 * @file main.c
 * @author Georges Kalmar 9803393
 * @date 9.11.2021
 *
 * @brief Main program module.
 * 
 * This program takes care of getting the arguments and coordinates the functions used
 * to replace tabstops with spaces. 
 **/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include"myexpand.h"

/** 
 * @brief Main gets the options and program arguments and starts the replaceTab() function
 * @details Main uses the getopt function to get the input arguments from a client in this Synopsis myexpand [-t tabstop] [-o outfile] [file...].
 * It runs several checks on the input to ensure that the data is sufficient enough to continue the program properly. It opens the output
 * file if specified (if not stdout is used instead) and runs the function replaceTab() on all input sequences. After this, the output 
 * file is closed and the program stops with EXIT_SUCCESS
 * @param argc Stores the amount of arguments
 * @param argv Stores the text string of the arguments in an array
 * @return The program return EXIT_SUCCESS on success or returns EXIT_FAILURE in case of errors 
 **/
int main(int argc, char* argv[]){
	
	FILE* out;
	int tabstop;
	char* t_arg=NULL;
	char* o_arg=NULL;
	char* endptr;
	int c=0;
	
	while((c=getopt(argc,argv,"t:o:"))!=-1){
		switch(c){
			case 't': if(t_arg !=NULL){abortProg(argv[0]);}
			t_arg = optarg;
			break;
			case 'o': if(o_arg !=NULL){abortProg(argv[0]);}
			o_arg = optarg;
			break;
			default:
			abortProg(argv[0]);
			break;
		}
	}
	if(t_arg!=NULL){
		int t = strtol(t_arg,&endptr,10);
		if((t >0) && (*endptr == '\0')){tabstop = t;}
		else {abortProg(argv[0]);}
	}
	else{tabstop = 8;}
	
	//checks if open the output file worked with using fopen
 	if(o_arg!=NULL){
		out = fopen(o_arg,"a");
		if (out == NULL){
			fprintf(stderr,"%s ERROR: fopen failed to open [%s]: %s\n",argv[0],o_arg,strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else{
		out = stdout;
	}
	
	if(optind == argc){replaceTab(NULL,out,tabstop,argv[0]);}
	else{
		for(int i =optind; i<argc;i++){
			replaceTab(argv[i],out,tabstop,argv[0]);
		}
	}
	
	if(out != stdout){
		if(fclose(out)!=0){abortProg(argv[0]);}
	}
	return EXIT_SUCCESS;
}
	
	
	
