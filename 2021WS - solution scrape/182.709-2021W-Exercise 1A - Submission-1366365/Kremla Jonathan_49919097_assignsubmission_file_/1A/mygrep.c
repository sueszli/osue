#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

char *mygrep;

void usage (void){
	fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword \n", mygrep);
	exit(EXIT_FAILURE);
}


int main (int argc, char const *argv[]) {
	char *o_arg = NULL;
	int opt_i = 0;
	int c;
	bool toLowerCase = false;
	while ((c = getopt(argc,argv, "o:i")) != -1){
		switch (c) 
		{
			case 'o': o_arg = optarg;
				break;
			case 'i': opt_i++;
				break;
			case '?': usage();
				break;
		}
	}
	

	if(opt_i == 1){
		toLowerCase = true;
	}

		
	if(opt_i > 1){
		usage();
	}

	

	if(argc - optind < 1){
		usage();
	}

	int fileCount = argc -optind -1;
	FILE *in[fileCount];
	FILE *out;

	int i = 0;
	while(i < fileCount){
		if((in[i] = fopen(argv[argc - fileCount + i],"r")) == NULL){
			fprintf(stderr,"fopen failed at in",mygrep);
			exit(EXIT_FAILURE);
		}
		i++;
	}

	if(o_arg != NULL){
		if((out = fopen(o_arg,"w")) == NULL){
			fprintf(stderr,"fopen failed at out",mygrep);
			exit(EXIT_FAILURE);
		}
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	char *str = NULL;
	char *stringToFind = argv[argc - fileCount - 1];
	char  buffer[100];
	
	int j = 0;
	if(fileCount == 0){
  		while(fgets(buffer, 100 , stdin)){
	    	//text = realloc( text, strlen(text)+1+strlen(buffer) );
			if(toLowerCase){
				for(j = 0;j < strlen(buffer);++j){
					buffer[j] = tolower(buffer[j]);
				}
			}
			str = strstr(buffer,stringToFind);
			if(str != NULL){
				if(o_arg == NULL){
					printf ("%s",buffer);
				}
				else{
					fwrite(buffer,sizeof(char),strlen(buffer), out);
				}
			}
		}
	}

	i = 0;
	j = 0;
	while (i < fileCount){
		while((nread = getline(&line,&len,in[i])) != -1) {
			if(toLowerCase){
				for(j = 0;j < strlen(line);++j){
					line[j] = tolower(line[j]);
				}
			}
			str = strstr(line,stringToFind);
			if(str != NULL){
				if(o_arg == NULL){
					printf ("%s",line);
				}
				else{
					fwrite(line,sizeof(char),strlen(line), out);
				}
			}
		}
		fclose(in[i]);
		i++;
	}


	if(optarg != NULL){
		fclose(out);
	}
	return (0);



	

}
 
