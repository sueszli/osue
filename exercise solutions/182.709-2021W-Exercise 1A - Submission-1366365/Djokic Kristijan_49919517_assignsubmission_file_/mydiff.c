/**
* @brief = main class of the tool
* @details = all algorithms necesarry for the tool can be found within this class.
* @author = Krisitjan Djokic, 11904660
* @name = mydiff
*/

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int c;
char *a_arg = NULL;
/**
*	A variable that can be used in order to determine if the tool should differentiate between lower and upper case.
*/
int shouldDifferentiate = 1;
/**
* A variable that can be used in order to determine if output should be written to a file or not.
*/
int writeInFile = 1;
/**
*	A variable that contains program name.
*/
char *prog_name = "[./mydiff]";

void writeOutput(int lineCounter, int num);
void getDifferentCharacters(int lineCounter,char *line1, char *line2,int n);

int main(int argc, char *argv[])
{
	while( (c = getopt(argc,argv,"io:")) != -1)
	{
		switch (c)
		{
			case 'i':
				shouldDifferentiate = 0;
			break;
			case 'o':
				a_arg = optarg;
			break;
			default:
				//everything is fine
			break;
		}	
	}


	//If there are more than 2 positional arguments, error is thrown.
	if((argc - optind) != 2)
	{
		fprintf(stderr,"[%s] ERROR: number of positional arguments is invalid: [%d]\nNeeded: 2", prog_name,(argc - optind));
		exit(EXIT_FAILURE);
	}

	if(a_arg == NULL)
	{
		writeInFile = 0;
	}

	char* firstFile = argv[optind];
	optind = optind + 1;
	char* secondFile = argv[optind];

	FILE *txt1, *txt2;

	if((txt1 = fopen(firstFile,"r")) == NULL)
	{
		fprintf(stderr,"[%s] ERROR: first file could not be opened.", prog_name);
		exit(EXIT_FAILURE);
	}

	if((txt2 = fopen(secondFile,"r")) == NULL)
	{
		fprintf(stderr,"[%s] ERROR: second file could not be opened.", prog_name);
		exit(EXIT_FAILURE);
	}

	char *line1 = NULL;
	size_t len1 = 0;
	ssize_t read1;
	char *line2 = NULL;
	size_t len2 = 0;
	ssize_t read2;
	int lineCounter = 0;
	/*
		this code fetches the lines and compares them. It also checks which line is shorter in order to compare the right amount of characters.
	*/
	while(((read1 = getline(&line1, &len1, txt1)) != -1) && ((read2 = getline(&line2, &len2, txt2)) != -1))
	{
		lineCounter = lineCounter+1;
		if(strstr(line1,"\n") != NULL)
		{
			read1 = read1-1;
		}
		if(strstr(line2,"\n") != NULL)
		{
			read2 = read2-1;
		}

		int n =0;
		if(read1 > read2)
		{
			n = read2;
		}
		else
		{
			n = read1;
		}

		int res = 0;
		if(shouldDifferentiate)
		{
			res = strncmp(line1,line2,n);
		}
		else
		{
			res = strncasecmp(line1,line2,n);
		}
		if(res != 0)
		{
			getDifferentCharacters(lineCounter,line1,line2,n);
		}
	}

	fclose(txt1);
	fclose(txt2);
	if(line1)
		free(line1);
	if(line2)
		free(line2);

	exit(EXIT_SUCCESS);
}
/**
*	@brief = The procedure computes the number of differente characters between two char arrays.
*	@details = The procedure computes the number of differente characters between two char arrays. It also uses the global variable shouldDifferentiate and changes it's comparing logic based on the value of the variable.
*	@param = lineCounter - It contains the information in which line differ the two files.
* 	@param = line1 - It contains the information of the currently read line from the file one.
* 	@param = line2 - It contains the information of the currently read line from the file two.
*	@param = n - It contains the information of the size of the shorter line between two given files.
*/
void getDifferentCharacters(int lineCounter,char *line1, char *line2,int n)
{
	int num = 0;
	for (int i =0; i < n; i++)
	{
		if(shouldDifferentiate)
		{
			if(line1[i] != line2[i])
			{
				num = num+1;
			}
		}
		else
		{
			if(tolower(line1[i]) != tolower(line2[i]))
			{
				num = num+1;
			}
		}
	}
	if(writeInFile)
	{
		writeOutput(lineCounter,num);
	}
	else
	{
		fprintf(stdout,"Line: [%d], characters: [%d]\n",lineCounter,num);
	}
}

/**
*	@brief = This procedure writes output either to the file or it prints it, depending on the content of the variable @writeInFile.
*	@details = This procedure writes output either to the file or it prints it, depending on the content of the variable @writeInFile.
* 	@param = lineCounter - It contains the information in which line differ the two files.
*	@param = num - It contains the information in how many characters differ the two lines
*/
void writeOutput(int lineCounter, int num)
{
	FILE *output;
	size_t len = sizeof(char)*25+sizeof(int)*6;
	char* buffer = malloc(len);

	if(buffer == NULL)
	{
		fprintf(stderr,"[%s] ERROR: malloc couldn't allocate memory.", prog_name);
		exit(EXIT_FAILURE);
	}
	
	snprintf(buffer,len,"Line: [%d], characters: [%d]\n",lineCounter,num);//puts string into buffer

	if((output = fopen(a_arg,"a+")) == NULL)
	{
		fprintf(stderr,"[%s] ERROR: first file could not be opened.", prog_name);
		exit(EXIT_FAILURE);
	}

	if(fputs(buffer,output) == EOF)
	{
		fprintf(stderr,"[%s] ERROR: fputs couldn't write.", prog_name);
		exit(EXIT_FAILURE);
	}

	fclose(output);
}