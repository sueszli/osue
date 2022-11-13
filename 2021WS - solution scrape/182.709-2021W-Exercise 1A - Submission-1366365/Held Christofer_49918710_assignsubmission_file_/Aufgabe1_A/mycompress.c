/**
 *@author Christofer Held 11902128
 *@date 14.11.2021
 * */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	int inputchars;
	int outputchars;
} compress_stats;

/*
 *@brief reads from the in_stream and writes the compressed result in the out_stream
 *@parm: in_stream, out_stream must be opend
 *@parm: stat ist used to store the informaton of the compression
 * */
void compress(FILE* in_stream,FILE* out_stream,compress_stats *stat){
	char str[1024];
	int size;
	int cur_pos;
	while((size = fread(str, sizeof(char), 1024, in_stream)) != 0){
		stat->inputchars += size;
		cur_pos = 0;
		for(int i = 1; i < size; i++){
			if(str[cur_pos] != str[i]){
				fprintf(out_stream, "%c%d",str[cur_pos], i - cur_pos);
				stat->outputchars += 2;
				cur_pos = i;
			}
		}
		fprintf(out_stream, "%c%d", str[cur_pos], size - cur_pos);
		stat->outputchars += 2;
	}
}



int main(int argc, char * argv[]){

	int opt;
	FILE* outstream = stdout;
	
	// Argument Parsing
	while((opt = getopt(argc,argv,"o:"))!= -1){
		switch (opt) {
		case 'o':
			outstream = fopen(optarg,"w");
			if(outstream == NULL){
				fprintf(stderr, "Couldn't open Outputfile");
			}
			break;
		default:
			fprintf(stderr, "Usae: %s [-o outfile] [file...]",argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	
	argc -= optind;
	argv += optind;
	
	// For all input Files
	compress_stats stat = {0,0};
	FILE* instream;
	for (int num = 0; num < argc ; num++){
		if (argc == 0){
			instream = stdin;
		}else{
			instream = fopen(argv[num], "r");
			if(instream == NULL){
				fprintf(stderr, "couldn't open Inputfile: %s", argv[num]);
				exit(EXIT_FAILURE);
			}
		}
		compress(instream, outstream, &stat);
		fclose(instream);
	}
	fflush(outstream);
	fclose(outstream);
	
	// print result
	fprintf(stderr, "\n\nRead:\t\t%d characters\n", stat.inputchars);
	fprintf(stderr, "Written\t\t%d characters\n", stat.outputchars);
	fprintf(stderr, "Compression ratio:\t%'.1f%%\n", 100*(float)stat.outputchars/(float)stat.inputchars);
	
	return 0;
}

