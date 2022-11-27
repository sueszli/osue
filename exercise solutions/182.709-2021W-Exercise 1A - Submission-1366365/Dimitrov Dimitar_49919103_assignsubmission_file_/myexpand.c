/**
* @file myexpand.c
* @author Dimitar Dimitrov 11719773
* @date 14.11.2019
*
* SYNOPSIS: myexpand [-t tabstop] [-o outfile] [file...]
*
* The program shall read each file given as positional argument (or stdin if there are no positional
* ments) line by line and search for tab characters (\t).
* 
* The tabstop distance is 8 by default, but this can be overridden with an arbitrary positive integer using
* the option -t.
*
* If the option -i is given, the program shall not di?erentiate between lower and upper case letters,
* i.e. the check for a palindrom shall be case insensitive.
*
* If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is
* written to stdout.
**/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>

FILE *outfile;
FILE *infile;

void errorExit(char* errorMessage)
{
	if(outfile!=NULL) fclose(outfile);
	if(infile!=NULL) fclose(infile);
	fprintf(stderr, errorMessage);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{

    	char *givenString;
    	char *OutFile;
	int fo=0,ft=0,finf=0,fini=0;
	int tabSpaces = 8;
	int opt;
   	//Read each option
   	while((opt = getopt(argc, argv, "o:t:")) != -1)
    	{
        	switch(opt)
        	{
            		case 'o': {
					if(fo)errorExit("Option -o given more than once.");
					fo=1;
					OutFile=optarg;
					outfile=fopen(OutFile,"w");
					if(outfile==NULL)errorExit("Error opening the file.\n");
					break;
				  }
            		case 't': {
					if(ft)errorExit("Option -t given more than once.");
					if(optarg==NULL) errorExit("Filename not given.");
					ft=1;
					for(int i=0;i<strlen(optarg);i++) if(optarg[i]<'0' || optarg[i]>'9')errorExit("Option argument is invalid.");
					tabSpaces=atoi(optarg);
					break;
				  }
            		case '?': {errorExit("Invalid argument!\n");}
            		default:  {errorExit("Invalid argument!\n");}
       		}
    	}
	if(fo==0)
	{
		outfile=stdout;
	}
    	//check if there are .in files
    	if(argv[optind]==NULL)
    	{
        	finf=0;
    	}
    	else
    	{
		finf=1;
            	fini=optind;
    	}
	// Get input from stdin
    	while(finf==0)
    	{
        	size_t lineSize=0;
		givenString = NULL;
        	if(getline(&givenString, &lineSize, stdin) != -1)
		{  // Read until new line
        		for(int i=0;i<strlen(givenString);i++)
			{
				if(givenString[i]==9)
				{
					for(int j=0;j<tabSpaces;j++) fprintf(outfile, "%c", ' ');
				}
				else
				fprintf(outfile, "%c", givenString[i]);
        		}
			givenString = NULL;
		}
       		else
        		finf=2;
    	}

	while(finf==1)
    	{
		infile= fopen(argv[fini], "r"); fini++;
		if(infile== NULL) errorExit("Error opening the in file.\n");
        	size_t lineSize=0;
		givenString = NULL;
        	while(getline(&givenString, &lineSize, infile) != -1)
		{  // Read until new line
        		for(int i=0;i<strlen(givenString);i++)
			{
				if(givenString[i]==9)
				{
					for(int j=0;j<tabSpaces;j++) fprintf(outfile, "%c", ' ');
				}
				else
					fprintf(outfile, "%c", givenString[i]);
			}
			if(givenString[strlen(givenString)-1]!='\n')
			fprintf(outfile, "%c", '\n');
            		givenString = NULL;
        	}
		if(argv[fini]==NULL) finf=2;
    	}

	if(outfile!=NULL) fclose(outfile);
	if(infile!=NULL) fclose(infile);
	return EXIT_SUCCESS;
}