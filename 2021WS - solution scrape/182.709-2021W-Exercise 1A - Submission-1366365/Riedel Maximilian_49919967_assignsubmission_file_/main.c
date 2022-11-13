/*
main.c (ispalidrom)
Version 1
@date 14.11.2021
@author: Maximilian Riedel (StudentID: 11736247)
@brief: main.c fetches options and arguments, reads input, calls ispalindrom, writes output
@details main.c:
fetches options and arguments:
	-s: ignore whitespaces
	-i: ignor lower and upper case
	-o: write results into a output file (-o takes exactly one argument, specifying the destination of the output file)
	takes one optional argument, specifying the destination of the input file. If none is given, the input is read from the command line.
reads input from stdin or from the given input file until a \n occurs.
calls ispalindrom(char * str) and writes it return value to stdout or the given output file
*/

#include "ispalindrom.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>

/*
@brief: used to inform about the usage of the program
@details: prints to stderror and exits the program with EXIT_FAILURE
@params: none
@return: none
*/
void usage(void){
	printf("Usage: ispalindrom [-s] [-i] [-o outfile] [input_file]");
	exit(EXIT_FAILURE);
}

/*
@brief: main fetches options and arguments, reads input, calls ispalindrom, writes output
@details main.c:
fetches options and arguments:
        -s: ignore whitespaces
        -i: ignor lower and upper case
        -o: write results into a output file (-o takes exactly one argument, specifying the destination of the output file)
        takes one optional argument, specifying the destination of the input file. If none is given, the input is read from the command line.
reads input from stdin or from the given input file until a \n occurs.
calls ispalindrom(char * str) and writes it return value to stdout or the given output file
*/
int main(int argc, char *argv[]) {
	char *o_arg = NULL;
	int opt_i = 0;
	int opt_s = 0;
	int c;
	char * prog_name = "main.c"; //argv[0];
	FILE * output = NULL;
	
	while ((c = getopt(argc, argv, "iso:")) != -1 ){
		switch ( c ) {
			case 'o': 
				o_arg = malloc(sizeof(char) * sizeof(optarg));
				if (o_arg == NULL){
					fprintf(stderr, "[%s] ERROR: memory allocation failed: %s\n", prog_name, strerror(errno));
					exit(EXIT_FAILURE);
       				}
				if(optarg == NULL){
					usage();
				}
				o_arg = optarg;
				break;
			case 's':
				opt_s++;
				break;
			case 'i': 
				opt_i++;
                                break;
			case '?': 
				/* invalid option */
				usage();
				break;
		}
	}

	if ( (argc - optind) > 2 ){
		usage();
	}

	FILE *input;
	if ( (argc - optind) == 1 ){
		char *inputfn = argv[optind];
		input = fopen(inputfn, "r");
	}
	if ( (argc - optind) == 0 ){
                input = stdin;
		printf("Please type a line. To check press enter. To quit press q.\n");
        }
	if (input == NULL) {
		fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
        	exit(EXIT_FAILURE);
	}

	char * line;
	char * curPos;
	int i;
	long maxLinelengh;
	while(1){
		maxLinelengh = 8L;
		line = realloc(NULL,sizeof(char) * maxLinelengh);
                if (line == NULL){
                        fprintf(stderr, "[%s] ERROR: memory allocation faild: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
                }
		
		i = 0;
		curPos = line;
		while(i<maxLinelengh){
			char c = fgetc(input);
			if(c<0){
				if(ferror(input) != 0){
					fprintf(stderr, "[%s] ERROR: fgets failed: %s\n", prog_name, strerror(errno));
					exit(EXIT_FAILURE);
				}
                     		if(feof(input) != 0){
					break;
                       		}
			}
			if(c == '\n'){
				*curPos = '\0';
				break;
			}
			strncpy(curPos, &c, 1);
			curPos++;
			i++;
			if(i == maxLinelengh-1){
				maxLinelengh *= 2;
				line = realloc(line, sizeof(char) * (maxLinelengh));
                               	if (line == NULL){
                                        fprintf(stderr, "[%s] ERROR: memory reallocation faild: %s\n", prog_name, strerror(errno));
                                        exit(EXIT_FAILURE);
                                }
			}
		}
			
		if(feof(input) != 0){
			exit(EXIT_SUCCESS);
		}

		//enable quit command for command line application
		if(strcmp(line, "q") == 0){
			if(fileno(input)==0){
				exit(EXIT_SUCCESS);
			}
		}

		if(o_arg != NULL){
			if(output == NULL){
				output = fopen(o_arg, "w");
				if(output == NULL){
					fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
					exit(EXIT_FAILURE);
				}
			} else {
				if(fseek(output, 0L, SEEK_END)!=0){
                                	fprintf(stderr, "[%s] ERROR: fseek failed: %s\n", prog_name, strerror(errno));
                                	exit(EXIT_FAILURE);
				};
                        }
			if(fputs(line, output)<0){
				fprintf(stderr, "[%s] ERROR: fputs failed: %s\n", prog_name, strerror(errno));
				exit(EXIT_FAILURE);
			}
			
                } else {
			printf("%s", line);
		}

		if(opt_s>0){
			char * _line = line;
			char * s = line;

			while(*s != '\0'){
				if(*s != ' '){
					*_line = *s;
					s++;
					_line++;
				} else {
					s++;
				}
			}
			*_line = '\0';
		}

		if(opt_i>0){
			char * _line = line;
                        char * s = line;

                        while(*s != '\0'){
				*_line = tolower(*s);
				_line++;
				s++;
			}
		}

		char * ret_str = ispalindrom(line);
		if(ret_str == NULL){
			fprintf(stderr, "[%s] ERROR: ispalindrom failed: %s\n", prog_name, strerror(errno));
                        exit(EXIT_FAILURE);
		}
		if(o_arg != NULL){
			if(fseek(output, 0L, SEEK_END)!=0){
				fprintf(stderr, "[%s] ERROR: fseek failed: %s\n", prog_name, strerror(errno));
                                exit(EXIT_FAILURE);
			}
			if(fputs(ret_str, output)<0){
                        	fprintf(stderr, "[%s] ERROR: fputs failed: %s\n", prog_name, strerror(errno));
                		exit(EXIT_FAILURE);
			}
		} else {
			printf("%s", ret_str);
		}
	}
	if(output != NULL){
		fclose(output);
	}
	free(line);
	fclose(input);
	exit(EXIT_SUCCESS); 
}
