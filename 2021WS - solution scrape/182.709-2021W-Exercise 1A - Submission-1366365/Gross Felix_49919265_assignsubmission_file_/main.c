/**
@file: mygrep.c
@author: Felix Gross 12019865
@date: 12.11.2021
@usage: mygrep [-i] [-o outfile] keyword [file...]
@brief: mygrep reads all given files line by line and prints all lines that contain keyword.


*/

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/**
@brief: The function strupr transforms a given string @p str to uppercase.
@details: The transformation is executet in place.
@return: The return value is a reference to the transformed string.
*/
char* strupr(char* str){
	if (str==NULL)
		return NULL;
	
	int i = 0;
	while((str[i])!='\0'){
		str[i]=toupper(str[i]);
		i++;
	}
	return str;
}


/**
@brief: getArguments extracts all program arguments from argv and saves to the correct variables.
@param: @p i_files is a reference to where to save all input file names
		@p o_stream is a reference to where to save the chosen output stream
		@p case_sensetive determines if the match is case sensitive
		@p keyword saves the keyword string
		@p argc is the argument count
		@p argv is the argument vector

*/
void getArguments(char*** i_files, FILE** o_stream, bool* case_sensitive, char** keyword, const int argc, char** argv){
	char c;
	char* usage="USAGE: mygrep [-i] [-o outfile] keyword [file...]\n";
	//get program options
	while ((c = getopt(argc, argv, "io:")) != -1) {
		switch (c) {
		case 'i': *case_sensitive = false;
			break;
		case 'o': *o_stream = fopen(optarg, "w");
			if (*o_stream==NULL){
				fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", argv[0], strerror(errno));
				exit(EXIT_FAILURE);
			}
			break;
		default:
			fprintf(stderr, usage);
			exit(EXIT_FAILURE);
		}
	}
	//default output
	if (*o_stream==NULL)
		*o_stream=stdout;

	//get keyword
	if(argv[optind]==NULL){
		fprintf(stderr, "[%s] ERROR: Keyword missing\n %s", argv[0], usage);
		exit(EXIT_FAILURE);
	}
	else{
		*keyword = argv[optind];
		if(*case_sensitive==false)
			strupr(*keyword);
	}

	//get input files
	*i_files=&argv[optind+1];
}


/**
@brief: openNextInputStream opens a stream to the next possible input file.
@details: The function tryes to open one file stream after the next one.
		If a file cant be opened the function outputs an error and continues with the next one.
		If no file is given the function returns stdin.

@param: @p stream_index is to track which file to open next
		@p i_stream is a reference to output the opened stream and does not reference an open stream.
		@p i_files is an array to all file names.

@return: The function returns true if it was able to open a new stream and false otherwise.
		 The opened stream is returned through @p i_stream and
		 the next stream index through @p stream_index 
*/
bool openNextInputStream(int* stream_index, FILE** i_stream, char** i_files, char* p_name){
	//default stream
	*i_stream=NULL;
	if((*i_files)==NULL && (*stream_index)==0){
		*i_stream = stdin;
		(*stream_index)++;
		return true;
	}

	//open the next possible file stream
	while (i_files[*stream_index]!=NULL && (*i_stream)==NULL){
		*i_stream = fopen(i_files[*stream_index], "r");
		if (*i_stream==NULL){
			fprintf(stderr, "[%s] ERROR: fopen failed with %s: %s\n", p_name, i_files[*stream_index], strerror(errno));
		}
		(*stream_index)++;
	}

	if ((*i_stream)==NULL)
		return false;
	return true;
}


int main(int argc, char* argv[]) {
	char* keyword = NULL;
	char** input_files = NULL;
	FILE* output_stream = NULL;
	bool is_case_sensitive = true;
	
	//initialise variables
	getArguments(&input_files, &output_stream, &is_case_sensitive, &keyword, argc, argv);

	//open all streams one by one and print matching lines
	int index = 0;
	FILE* input_stream=NULL;
	while(openNextInputStream(&index, &input_stream, input_files, argv[0])){
		char* line = NULL;
		size_t line_length=0;
		while (getline(&line, &line_length, input_stream) != -1){
			char* result=NULL;
			//search for keyword in line
			if (is_case_sensitive){
				result=strstr(line, keyword);
			}
			else{
				char* temp_line = strdup(line);
				result=strstr(strupr(temp_line),keyword);
				free(temp_line);
			}
			//output
			if(result!=NULL){
				if (fputs(line, output_stream)<0){
					fprintf(stderr, "[%s] ERROR: fputs failed: %s\n", argv[0], strerror(errno));
				}
				if(input_stream==stdin){
					if (fflush(output_stream)!=0){
						fprintf(stderr, "[%s] ERROR: fflusch failed: %s\n", argv[0], strerror(errno));
					}
				}
			}
		}
		//close current file
		if (fclose(input_stream)!=0){
			fprintf(stderr, "[%s] ERROR: fclose failed: %s\n", argv[0], strerror(errno));
			exit(EXIT_FAILURE);
		}
	}



	//cleanup
	if (fclose(output_stream)!=0){
		fprintf(stderr, "[%s] ERROR: fclose failed: %s\n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
	return 0;
}