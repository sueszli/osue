/**
 * @file myexpand.c
 * @author Georges Kalmar 9803393
 * @date 9.11.2021
 *
 * @brief Implementation of the functions for the program.
 * 
 * This program implements the abort function to be used for exit failure and the replaceTab
 * function the contains the logic for replacing tabulators with spaces. 
 **/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include"myexpand.h"

/** 
 * @brief Function that exits the program
 * @details This function is called if an error occured due to an invalid input of a client, in this case the program cannot continue the tasks properly.
 * Therefore it prints the name of the program accompanied with the valid form inputs should be given and exits the program with EXIT_FAILURE
 * @param myprog is used to give the funtion the program names Argument using argv[0]
 **/
void abortProg(char* myprog){
	fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n", myprog);
	exit(EXIT_FAILURE);
}

/** 
 * @brief Function that replaces the tabs in the input sequence with specified spaces in the output area
 * @details This function opens one or several files/or uses the stdin instead and reads the single chars. Whenever a tab is recognized it is replaced by
 * a specified amount of spaces, the tabstop distance. This distance is calculated each time assuring that the function puts the exact amount of 
 * spaces in the line in order to continue the chars at the next multiple of the tabstop distance.
 * @param filepath gives the funtion the name of the input file or NULL if stdin should be used instead
 * @param out is the pointer to the opened output file or stdout
 * @param tabstop specifies the amount of spaces that replace the tab
 * @param myprog is used to give the funtion the program names Argument using argv[0]
 **/
void replaceTab(char* filepath, FILE* out, int tabstop,char* myprog){
	FILE* in;
	int p;
	int c;
	int counter=0;
	
	//checks if open the input files worked with using fopen
	if(filepath != NULL){
		in = fopen(filepath,"r");
		if (in == NULL){
			fprintf(stderr,"%s ERROR: fopen failed to open [%s]: %s\n",myprog,filepath,strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else{
		in = stdin;
	}
		
	//reads each char separately from input files or stdin and writes it to output files or stdout
	while((c=fgetc(in))!= EOF){
		if(c=='\n'){
			counter = -1;	//needs to be set to -1 because '\n' is read in and increments counter
		}
		if(c=='\t'){
			p = tabstop*((counter/tabstop) + 1);
			int spaces = p - counter;
			for(int i = 0; i <spaces;i++){
				if(fputc(' ',out) == EOF){
					fprintf(stderr,"%s ERROR: fputs failed: %s\n",myprog,strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			counter +=spaces;
		}
		else{
			if(fputc(c,out) == EOF){
				fprintf(stderr,"%s ERROR: fputs failed: %s\n",myprog,strerror(errno));
				exit(EXIT_FAILURE);
			}
			counter++;
		}
	}
	if(ferror(in)){
		fprintf(stderr,"%s ERROR: fgets failed [%s]: %s\n",myprog,filepath,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(in != stdin){
		if(fclose(in)!=0){abortProg(myprog);}
	}
}