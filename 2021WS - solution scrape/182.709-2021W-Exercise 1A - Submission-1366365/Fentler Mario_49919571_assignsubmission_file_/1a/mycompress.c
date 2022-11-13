/*!
  @file mycompress.c
  @author Mario Fentler 12025969
  @date 11.11.2021
  @brief This programm does a compression of given words from a input stream by counting the consecutive occourences 
  of letters and merging them together to letter+occurence count and then writes them to the output stream
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mycompress.h"

int main(int argc, char *argv[]){

	char *arg_output = NULL;
	char *arg_input = NULL;

	// input stream to read from
	FILE *input_stream = NULL; 
	// output stream to write the compressed characters to
	FILE *output_stream = NULL; 

	char *programName = argv[0];

	// fetch option argument
	int option;
	while((option = getopt(argc,argv,"o:")) != -1){
		switch(option){
			case 'o':
				arg_output = optarg;
				break;
			case '?':
				print_usage();
				break;
			default:
				print_usage();
				break;
		}
	}

	// choose output stream
	if(arg_output == NULL)
		output_stream = stdout;
	else
		output_stream = fopen(arg_output,"w+");
	checkFileOpenedWithoutError(output_stream, programName);

	// fetch optional input file argument
	if(optind < argc){
		if(argc-optind > 1)
			print_usage();

		arg_input = argv[optind];
	}

	// choose input stream
	if(arg_input == NULL) 
		input_stream = stdin;
	else
		input_stream = fopen(arg_input,"r");
	checkFileOpenedWithoutError(input_stream, programName);


	// create temporary file for storing the input
	FILE *tempFP = fopen(TEMP_FILE,"w+");
	checkFileOpenedWithoutError(tempFP, programName);

	// read input and store it to temp file
	char line[MAX_SIZE];
	while(fgets(line, sizeof(line), input_stream) != NULL){
		fprintf(tempFP,"%s",line);
	}

	// close the input stream if still open (only happening when reading from file not from stdin)
	if(ftell(input_stream) != -1)
		fclose(input_stream);

	// reopen the temporary file to read from it and do the compression
	tempFP = freopen(TEMP_FILE,"r",tempFP);
	checkFileOpenedWithoutError(tempFP, programName);
	doCompression(tempFP, output_stream);


	// close resources and delete temporary file
	fclose(tempFP);
	remove(TEMP_FILE);

	if(ftell(output_stream) != -1)
		fclose(output_stream);

	return EXIT_SUCCESS;
}

void print_usage(void){
	printf("Usage: mycompress [-o outputFile] [inputFile] \n");
	exit(EXIT_FAILURE);
}

void checkFileOpenedWithoutError(FILE *fp, char *programName){
	if(fp == NULL){
		fprintf( stderr, "%s: Failed to open file!\n", programName);
		exit(EXIT_FAILURE);
	}
}

void doCompression(FILE *fp, FILE *output_stream){
	int occurenceCounter = 1;
	int curChar;
	int lastChar = -1;

	// double values so that the compression ratio can be calculated
	double readCounter = 0.0;
	double writeCounter = 0.0;
	
	while((curChar = fgetc(fp)) != EOF){
		readCounter += 1;
		if(curChar != lastChar){
			if(lastChar != -1){
				fprintf(output_stream,"%c%d",lastChar,occurenceCounter);
				writeCounter += 2;
			}

			lastChar = curChar;
			occurenceCounter = 1;
		}else
			occurenceCounter += 1;
	}

	// print the last char which was read before EOF and is still unprinted
	fprintf(output_stream,"%c%d",lastChar,occurenceCounter);
	writeCounter += 2;

	fflush(output_stream);

	double compressionRatio = (writeCounter/readCounter)*100;
	fprintf(stderr,"\nRead:\t\t\t%.0f charakters\nWritten:\t\t%.0f charakters\nCompression ratio:\t%.1f %%\n",readCounter,writeCounter,compressionRatio);
}
