/*
Module: mygrep.c
Author: Lukas Jessl M-Nr 01604985
created: 03.11.2021

This programm compares lines with a keyword. If they are contained in the lines, the line will be printed.*/

#include <unistd.h>
#include "mygrep.h"

/* This Function reads lines one by one. Depending on the input it reads from the commandoline or from files
 as descriped by the input. -i is used to declare that the keyword and the written line are not case sensetive.
 The -o parameter followed by a filename defines the output file to which the function will write.

 The function works as follows: It compares a read line with the keyword. If the keyword is contained in this line, then
 the line will be printed, if the keyword is not contained, then the line will not be printed. After each line
 it continues to read the file untill the end of file. If the input is only given per commando line, it will read the input 
 line by line.

   @param argc: is the argument counter that will be given to this function
   @param argv: are the arguments given to this function in a char*
   @return : Exit_Success on successfully finishing the programm.
*/
int main(int argc, char *argv[]){

	int c;
    bool cs = true;                 //cs stands for case sensitive, true = it is case sensitive, if the option -i is chosen, it is not case sensitive.
    char* keyword = NULL;
    char* output = NULL;
    int opt_o= 0;
 

	while (( c = getopt(argc, argv, "io:")) != -1){
		switch(c){
			case 'i': 
				cs = false;
				break;
			case 'o':
				output = optarg;
        		opt_o ++;
				break;
			default: 
				assert(0);
			
		}
	}

	if(opt_o > 1){
		fprintf(stderr, "Error, trying to write on 2 Output files.");
		exit(EXIT_FAILURE);
	}

	//The keyword, is always the first argument that is not relevant to the option parameters.
	//can only happen if there is atleast 1 parameter outside of the optional ones.
	if((argc-optind)>0){
		keyword = argv[optind];
   		optind++;

	}	    

	//
	if((argc - optind)== 0){
		readfromCommandoLine(argv[0],keyword, cs, output);
	}else if((argc - optind)>= 1){
		int i = argc - optind;
		while(i > 0){
			readfromFiles(argv[0], keyword, cs, output, argv[optind]);
			optind ++;
			i--;	
		}
	}

    return EXIT_SUCCESS;
}


