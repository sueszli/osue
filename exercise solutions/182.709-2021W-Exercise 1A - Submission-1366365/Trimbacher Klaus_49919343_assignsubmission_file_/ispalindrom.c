/**
*@file ispalindrom.c
*@author Klaus Trimbacher 11908086
*
*@brief Takes a string or a file and see if there are palindroms 
*@details The Compress Method is descibed in the pdf document "ispalindrom_td.pdf" the output can be written on the stdout or in a textfile with the option -o
*@date 12.11.2021
**/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
*Palindrom check Method
*@brief Check if the given string is a Palindrom. 
*@details The Function will create a own string which is the input string backwards.
**/
static int Palindrom(char input[],int i, int s);

/**
*Output Method 
*@brief Prints the result of the Palindrom . 
*@details 
**/
static void Output(char input[], int result, char* outputFile);

/**
*Programm entry point/ main method
*@brief The programm starts here. The function takes all the arguments and options and put them into the
*string variables. It Opens the Files if there are given when not then it will use stdin and stdout to 
*print the solution of the Palindrom Check.
**/
int main(int argc, char *argv[]){
	int option;
	char* inputFile = NULL;
	char* outputFile = NULL;
	int i = 0;
	int s = 0;

	while((option = getopt(argc,argv, "iso:"))!= -1){
		switch(option){
			case 'o':
				outputFile = optarg;
			break;
			case 'i':
				i = 1;
			break;
			case 's':
				s = 1;
			break;
			case '?':
			break;
		}
	}

	inputFile = argv[optind]; 


	char *input = NULL;
	size_t len = 0;
	FILE *in;
	
	if(inputFile != NULL){
		in = fopen(inputFile, "r");
		if(in == NULL ){
			fprintf(stderr,"%s: Input-File can't be opened\n" , "ispalindrom");
			return 0;
		}
	}else{
		in = stdin;
	}
	
	if(outputFile != NULL){
		FILE *out = fopen(outputFile, "w");
		fclose(out);
	}
	
	while(getline(&input,&len,in)  > 0){
		if(input[strlen(input)-1]== 10){
			input[strlen(input)-1] = 0;
		}
		int result = Palindrom(input,i,s);
		Output(input,result,outputFile);	
	}

	if(inputFile != NULL){
		fclose(in);
	}
	return 0;
}

static int Palindrom(char input[],int casesensetiv, int s){
	int len = strlen(input);
	
	char teststring[len];
	char inputstring[len];
	int offset = 1;
	
	strcpy(inputstring,input);
	
	if(s){
		char* token;
		char* withoutspace = malloc(len);
		withoutspace[0] = '\0';
		
		token = strtok(inputstring," ");
		while(token != NULL){
			strcat(withoutspace,token);
			token = strtok(NULL," ");
		}
		offset = len - strlen(withoutspace) + 1;
		strcpy(inputstring,withoutspace);
	}
	
	for(int i = 0; inputstring[i] != 0; i++){		
	
		if(inputstring[i] == 13){
			continue;
		}
		
		if(casesensetiv){
			inputstring[i] = tolower(inputstring[i]);
		}
		
		teststring[len-i-offset] = inputstring[i];
		teststring[len-offset+1] = '\0';
	}

	return !strcmp(teststring, inputstring);
}

static void Output(char input[], int result, char* outputFile){
	FILE *out;
	char *output = NULL;

	if(result){
		output = strcat(input," is a palindrom");
	}else{
		output = strcat(input," is not a palindrom");
	}
	
	if(outputFile != NULL){
		if((out = fopen(outputFile, "a")) != NULL){			
			while(fputs(output, out) == EOF);
			fputs("\n",out);
			fclose(out);
		}else{
			fprintf(stderr,"%s Output-File can't be opened" , "ispalindrom");
			exit(EXIT_FAILURE);
		}
	}else{
		printf(output);
		printf("\n");
	}
}