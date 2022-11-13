/*
Module mygrep.h
Author: Lukas Jessl M-Nr: 01604985
created: 03.11.2021

This Module has the mainfunctions for mygrep. Depending on the input parameters
it can read from files or from the commandoline. It compares the inputs with a keyword. If the
keyword is part of the input, then the input will be printed. Depending on the option chosen, the input
will be printed on the console or it will be written into a file.
Selecing a keyword is mandatory, if no keyword is selected but two input files, then the first input file will be 
seen as keyword.
*/

#include "mygrep.h"


/* @param path: declares the path in which the programm runs.
   @param keyword: is the keyword with which each line should be compared.
   @param cs: if this variable is true, then the comparison is case sensitive, if false it is not.
   @param output: Output has the path to where the output should be written. If it is NULL, then the output should be printed
   on the stdout.
   @param file: if there are no files, then the input should come from the commandline. Otherhwise file defines from where
   the programm should read the input.

compares keyword, with files. Files are read line by line. If a keyword is written in a line, it will be printed
No return values, uses Path for error messages, keyword to compare if it is part of the lines, cs to see if the comparison should
be case sensetive, output to know where to write the files, file to know where to read the lines filereadom.
BUFFERSIZE is a global variable that is used to set the "smallest" size of the Buffer.

   @return: void*/

void readfromFiles (char* path, char* keyword, bool cs, char* output, char* file){
	
	
	FILE * fileread = NULL;
    FILE * filewrite = NULL;
	
	if(output!=NULL){
		filewrite = fopen(output, "a");
	    if(filewrite == NULL){
	    	fprintf(stderr, "Error in File: %s, opening File to write failed: %s\n",path, strerror(errno));
        	exit(EXIT_FAILURE);
		}
	}
	
	char* input_lines = NULL;
	int biggerMemory = 2;
	char buffer[BUFFERSIZE] = "";
	
	fileread = fopen(file,"r");
	if(fileread == NULL){
		fprintf(stderr, "Error in File: %s, opening File to read failed: %s\n",path, strerror(errno));
        exit(EXIT_FAILURE);
	}
	
	input_lines = (char*) calloc (BUFFERSIZE, sizeof (char));
	
	while((fgets(buffer, BUFFERSIZE, fileread)) != NULL){
		
		strcat(input_lines, buffer);
	
		if(!feof(fileread)){							
			input_lines = realloc(input_lines, sizeof(char) * BUFFERSIZE * biggerMemory);
			biggerMemory++;
			continue;
		}	
    }
    
    char* lines = strtok(input_lines, "\n");
    while(lines != NULL){
    	char input[strlen(lines)];
    	strcpy(input,lines);
    	if(iskey(input,keyword,cs)){
    		if(output!=NULL){
    			fprintf(filewrite, "%s\n", input);
			} else{
				printf("%s\n", input);
			}
		}
		lines = strtok(NULL, "\n");
	}
    
    
	free(input_lines);  
	fclose(fileread);
    	
    if(output!=NULL){
    	fclose(filewrite);
	}
	
}


/* @param path: declares the path in which the programm runs.
   @param keyword: is the keyword with which each line should be compared.
   @param cs: if this variable is true, then the comparison is case sensitive, if false it is not.
   @param output: Output has the path to where the output should be written. If it is NULL, then the output should be printed
   on the stdout.

   Compares lines from the commandoline with the keyword
   No return values, uses Path for error messages, keyword to compare if it is part of the lines, cs to see if the comparison should
   be case sensetive, output to know where to write the files, reads filereadom stdin (commandoline)
   BUFFERSIZE is a global variable that is used to set the "smallest" size of the Buffer.
   @return:void.*/
void readfromCommandoLine(char* path,char* keyword, bool cs, char* output){
	
	    FILE * filewrite = NULL;
	    
	    if(output!=NULL){
	    	filewrite = fopen(output, "a");
	    	if(filewrite == NULL){
	    		fprintf(stderr, "Error in File: %s, opening File to write failed: %s\n",path, strerror(errno));
            exit(EXIT_FAILURE);
			}
		}
		
		char* line = NULL;
		int biggerMemory = 2; 
		char buffer[BUFFERSIZE] = "";
		
		bool reset = true;
		
		//only stops with CTRL + C or by having an empty argument
		while(fgets(buffer,BUFFERSIZE,stdin) != NULL) {
			
			if(reset == true){
				line = NULL;
				line = (char*) calloc (BUFFERSIZE, sizeof(char));
				reset = false;
			}
			
			strcat(line, buffer);
			
			if(line[strlen(line)-1] != '\n'){
				line = realloc(line, sizeof(char) * BUFFERSIZE * biggerMemory);
				biggerMemory++;
				continue;
			}
			
			line[strlen(line)-1] = 0;			// to remove new line symbol at the end
			char input[strlen(line)];
			strcpy(input, line); 

			if(iskey(input, keyword, cs)){
				if(output == NULL){
					printf("%s\n", input);
				} else {
					fprintf(filewrite, "%s\n", input);
					fflush(filewrite);
				}
			}
			
			biggerMemory = 2;
			reset = true;
			free(line);
		}
		
		
		if(filewrite != NULL){
			fclose(filewrite);
		}

}



/* @param input: is the line with which the keyword should be compared.
   @param keyword: is the keyword with which each line should be compared.
   @param cs: if this variable is true, then the comparison is case sensitive, if false it is not.

If the String is a key, true is returned, false otherwhise.
no use of global varriables
Compares input with the keyword. 

   @return: a bool, if the keyword is contained in the input true, false otherwhise*/
bool iskey (char* input, char* keyword, bool cs){
	
	bool x = false;
	
	//Copies of input/keyword, with diffilereadent storage location.
	char* str_input =  malloc (sizeof(char) * strlen(input));		
	char* str_keyword = malloc (sizeof(char) * strlen(keyword));
	
	int i = 0;
	
	if(cs == false){
		for(i = 0; i < strlen(input);i++){
			str_input[i] = tolower(input[i]);
		}
		for(i = 0; i < strlen(keyword);i++){
		 	str_keyword[i] = tolower(keyword[i]);	
		}
	} else{
		strcpy(str_input,input);
		strcpy(str_keyword,keyword);
	}
	
	char* partstring = strstr(str_input, str_keyword);

	if(partstring != NULL){
		x = true;
	} else{
		x = false;
	}
	
	free(str_input);
	free(str_keyword);
	
	return x;
}


