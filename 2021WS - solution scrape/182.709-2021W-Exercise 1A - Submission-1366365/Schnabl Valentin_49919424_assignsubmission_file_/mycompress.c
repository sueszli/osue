/**
* @file mycompress.c
* @author Valentin Schnabl, 11848108
* @date 24.10.2021
* @brief mycompress compresses strings and prints them to an output (aabbcc --> a2b2c2). Prints statistics to stderr.
* @details first of all, program inout and output is defined. This is handled with getopt to get the options. Based on that, the in- and output is then set and the algorithm defined in myCompress is executed.
* If the input or output was wrong, an error is printed and the function usage is called to inform the user of the correct usage. If no input file is entered, stdin is read.
**/

#include "mycompress.h"



/**
* @brief this function informs the user about the correct usage of the program. It is beeing called if the user fails to enter the correct amount of arguments or puts in too many.
* @details prints "Usage: mycompress [-o outputFile] [inputFile]" in stderr
**/

void usage(void){
	fprintf(stderr, "Usage: %s [-o outputFile] [inputFile]", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}

/**
* @brief this is being called first when calling the program. It is evaluating if correct arguments are being passed. Then it is calling readInput if no input is defined. If in- and output are defined, it is calling
* the function myCompress.
* @param argc argument counter
* @param argv arguments and options in an array
* @return EXIT_SUCCESS or EXIT FAILURE
**/
int main(int argc, char *argv[]) {
	char *outputFileAddr = NULL;
	int opt_o = 0;
	int c;
	while((c=getopt(argc, argv, "o:")) != -1){
		switch (c){
			default:
				usage();
				break;
			case 'o': outputFileAddr = optarg;
				break;
			case '?':
				usage();
				exit(EXIT_FAILURE);
				break; 
		}
	}
	if(opt_o > 1){
		fprintf(stderr, "Option o can only be used once.\n");
		usage();
		exit(EXIT_FAILURE);
	}
	FILE *outputFile, *inputFile; //declaring input and output

	if(outputFileAddr == NULL){ //no output file defined
		if(argc == optind){ //no input defined
			myCompress(stdin, stdout);
			exit(EXIT_SUCCESS);
		}
		else{
			for(int i=optind; i < argc; ++i){ //input(s) defined
				inputFile = fopen(argv[i], "r"); //read
				if(inputFile == NULL){
					fprintf(stderr, "Canot open input file.\n"); //error
					exit(EXIT_FAILURE);
				}
				myCompress(inputFile, stdout);
				}
			}
	}
	else{ //output defined
		outputFile = fopen(outputFileAddr, "w"); //read
		if(argc == optind){ //no input defined
			myCompress(stdin, outputFile);
			exit(EXIT_SUCCESS);
			}
		else{
			for(int i=optind; i < argc; ++i){ //input(s) defined
				inputFile = fopen(argv[i], "r"); //read
				if(inputFile == NULL){
					fprintf(stderr, "Canot open input file.\n"); //error
					exit(EXIT_FAILURE);
				}
				myCompress(inputFile, outputFile);
				}
			}
		fclose(outputFile);
		}
	return EXIT_SUCCESS;
}

/**
* @brief this funtction holds the main functionality of the program. It reads in every character in a file and counts them. If the next character is equal to the last, count is beeing raised. If the next character
* is different, it prints the character and the counter into the output file. The loops body is repeated once after the loop for the last character. Also, there are statistics. The total amount of characters, the
* actual printed characters and the compression rate (written/total) is printed in stderr. 
* @param in is the input file. It can be either a temp file from stdin or a normal file.
* @param out is the output file. It can be either a normal file or stdout.
**/
void myCompress(FILE* in, FILE* out){
	int read;
	int temp = fgetc(in);
	int count = 1;
	int totalCount = 0;
	int writtenCount = 0;
	float div = 0;

	while((read=fgetc(in)) != EOF){ //while until end of input
		++totalCount;
		if(read == temp){
			++count;
		}
		else{
			writtenCount += 2;			
			fprintf(out, "%c%i", temp, count);
			count = 1;
			temp = read;
		}
	}
	++totalCount;
	if(read == temp){ //case for last char
		++count;
	}
	else{	
		writtenCount += 2;		
		fprintf(out, "%c%i", temp, count);
		count = 1;
		temp = read;
	}
	div = (float)writtenCount/totalCount;
	fclose(in);
	fprintf(stderr, "\nRead: %i characters\nWritten: %i characters\nCompression ratio:  %f\n", totalCount, writtenCount, div);
}