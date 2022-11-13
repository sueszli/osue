/**
 * @file mydiff.c
 * @author Slavomir Slavtchev
 * @id e12021361
 * @date 14.11.2021
 *  
 * @brief The program reads each file line by line und compare the characters. If two lines have 		different length, then the comparison shall stop upon reaching the end of the shorter line.
 *
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

char *myprog;
const char *o_arg = NULL;
int opt_i = 0;
FILE *output;

void usage(void){
	fprintf(stderr, "Usage: %s [-i] [o outfile] file1 file2\n", myprog);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv){
	myprog = argv[0];
	int c;
	while((c = getopt(argc, argv, "io:")) != -1){	
		switch(c){
			case 'i': opt_i++;
				break;
			case 'o': o_arg = optarg;
				break;
			case '?': usage(); 
			break;
			
		}
	}
	
	if((argc - optind) > 2){
		usage();
	}
	
	if(opt_i > 1){
		usage();
	}
	
	if(o_arg != NULL){
		output = fopen(o_arg, "w");
	}
	
	
	
	FILE *input1 = fopen(argv[optind], "r");
	FILE *input2 = fopen(argv[optind+1], "r");
	
	if(input1 == NULL || input2 == NULL){
		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	char buf1[1024];
	char buf2[1024];
	int line = 1;
	
	
	while(fgets(buf1, sizeof(buf1), input1) != NULL & fgets(buf2, sizeof(buf2), input2) != NULL){
		
		int val = 0;
		int i = 0;
		int count = 0;
		
		while(buf1[i] != 10 && buf2[i] != 10){
				
			if(opt_i == 0){	
				val = strncmp(buf1, buf2, i+1);
			}
			else{
				val = strncasecmp(buf1, buf2, i+1);
			}
			if(val != 0){
				count++;
			}
			buf1[i] = buf2[i] = '0';
			i++;
		}
		if(count > 0){
			if(o_arg == NULL){
				printf("Linie: %d, characters: %d\n", line, count);
			}
			else{
				fprintf(output, "Linie: %d, characters: %d\n", line, count);
			}
		}
		line++;
	}
	if(ferror(input1) || ferror(input2)){
		exit(EXIT_FAILURE);
	}
	else{
		fclose(input1);
		fclose(input2);
		if(o_arg != NULL){
			fclose(output);
		}
		exit(EXIT_SUCCESS);
	}
	
	
}
