/**
 * @file myexpand.c
 * @author Erik Hado <e11811852@student.tuwien.ac.at>
 * @date 14.11.2021
 * @brief The program implements the functionality of the "expand" function
 * which reads in several files and replaces tabs with spaces.
 * */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>

/**
 * 
 * @brief Reads data from the given input stream(s) and prints it out in given file.
 * @param in input file
 * @param tabstop tab length
 * @param out output file
 * */
 int toFile(FILE *in, int tabstop, FILE *out){
	int p = 0;
	int x = 0;
	char counter;
	while((counter = fgetc(in))!=EOF){
		if (counter == '\t')
		{
			p= tabstop * ((x/tabstop) + 1);
			for (int i = 0; i < tabstop; i++)
			{
				fputc(' ', out);
			}
			x=p;
		} else {
			x++;
			if (counter == '\n')
			{
				x = 0;
			}
			fputc(counter, out);
		}
	}
	return 0;
 }

 /**
  * 
  * @brief Reads data from given input stream and prints it out in console
  * @param in input stream
  * @param tabstop tab length
  **/

 int toConsole(FILE *in, int tabstop){
	int p = 0;
	int x = 0;
	char counter;
	while((counter = fgetc(in))!=EOF){
		if (counter == '\t')
		{
			p= tabstop * ((x/tabstop) + 1);
			for (int i = 0; i < tabstop; i++)
			{
				fputc(' ', stdout);
			}
			x=p;
		} else {
			x++;
			if (counter == '\n')
			{
				x = 0;
			}
			fputc(counter, stdout);
		}
	}
	return 0;
}
/**
 * Main function
 * @brief The function examines the flags and implements the corresponding functions
 * based on the requirements.
 * */

int main(int argc, char **argv){
    unsigned int tabstop = 8;	
    long newTabStopValue;
    char *programName = "myexpand";
    char *endptr;
    int c;
    int opt_t = 0;
    int opt_o = 0;
    char *out_file = NULL;

    if (argc <= 0)	//Examine if the program exists or not
    {
    	fprintf(stderr, "ERROR[%s]: No parameters given", programName);
    	exit(EXIT_FAILURE);
    } else {
    	programName = argv[0];
    }
    while((c=getopt(argc, argv, "t:o:"))!=-1){
    	//printf("in getopt\n");
    	switch(c){
    		case 't':
    		    opt_t++;
    		    errno = 0;
    		    newTabStopValue =  strtol(optarg, &endptr, 10);
    		    if (errno == ERANGE)
    		    {
    		    	fprintf(stderr,"ERROR[%s]: parsing failed",programName);
    		    	exit(EXIT_FAILURE);
    		    }
    		    if (endptr == optarg){
    		    	fprintf(stderr, "ERROR[%s]: no -t argument found", programName);
    		    }
    		    {
    		    	/* code */
    		    }
    		    tabstop = newTabStopValue;
    		    if (argc <= 3) //No more arguments other than name and tabstop
    		    {
    		    	toConsole(stdin, tabstop);
    		    	return 0;    		    }
    		    break;
    		case 'o':
    		    opt_o++;
    		    out_file = optarg;
    		    break;
    		default:
    			fprintf(stderr, "ERROR[%s]: unrecognized argument",programName);
    		    break;
    	}
    }
    if(opt_t> 1){
    	fprintf(stderr, "ERROR[%s]: Option t occurs more than once",programName);
    	exit(EXIT_FAILURE);
    } else
    if(opt_o>1){
    	fprintf(stderr, "ERROR[%s]: Option o occurs more than once",programName);
    	exit(EXIT_FAILURE);
    }
    if((argc-optind)==0){
    	if (opt_o == 0)
    	{
    		//printf("stdIn\n");
    		toConsole(stdin, tabstop);
    	} else {
    		FILE *fout = fopen(out_file, "a");
    		toFile(stdin, tabstop, fout);
    	}
    }
    if ((argc-optind)>0) //There are positional arguments
    {
    	if(opt_o== 0){	//Print to console
            int posArgCount = argc-optind;
            	FILE *inputfp[posArgCount];
            	for (int i = 0; i < posArgCount; i++)
            	{
            		inputfp[i] = fopen(argv[optind + i], "r");
            		if (inputfp[i] == NULL)
            		{
            			fprintf(stderr, "ERROR[%s]: opening file",programName);
            			exit(EXIT_FAILURE);
            		}

            		if (toConsole(inputfp[i], tabstop)!=0){
            		fprintf(stderr, "ERROR[%s]: in file to console", programName);
            		exit(EXIT_FAILURE);
            		}
            		if (fclose(inputfp[i])==EOF)
            		{
            			fprintf(stderr, "ERROR[%s]: closing file", programName);
            			exit(EXIT_FAILURE);
            		}

            	}
            	return 0;
            
    } else { //Print files to file
    	FILE *fout = fopen(out_file, "a");
            int posArgCount = argc-optind;
            	FILE *inputfp[posArgCount];
            	for (int i = 0; i < posArgCount; i++)
            	{
            		inputfp[i] = fopen(argv[optind + i], "r");
            		if (inputfp[i] == NULL)
            		{
            			fprintf(stderr, "ERROR[%s]: opening file",programName);
            			exit(EXIT_FAILURE);
            		}

            		if (toFile(inputfp[i], tabstop, fout)!=0){
            		fprintf(stderr, "ERROR[%s]: in file to console", programName);
            		exit(EXIT_FAILURE);
            		}
            		if (fclose(inputfp[i])==EOF)
            		{
            			fprintf(stderr, "ERROR[%s]: closing file", programName);
            			exit(EXIT_FAILURE);
            		}

            	}
            	return 0;
    }
}
return EXIT_SUCCESS;
}