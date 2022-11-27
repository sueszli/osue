/**
 * @file mydiff.c
 * @author 00828391 Kemal Öner
 * @date 12.11.2021
 * @brief The program shall read each file line by line and compare the characters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX 1024

char *myprog; // name of program


// print usage
#define USAGE()																	\
	do{																			\
		fprintf(stderr,"USAGE: %s [-i] [-o outfile] file1 file2", myprog);		\
		exit(EXIT_FAILURE);														\
	} while(0)


/**
 * main
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
int main(int argc, char **argv){

    FILE *txt1 = NULL;
    FILE *txt2 = NULL;
    FILE *output = NULL;
    int opt;
    char buf1[MAX];
    char buf2[MAX];
    int flag_i = 0;
    int flag_o = 0;

    while((opt = getopt(argc, argv, "io:"))!=-1){
		switch(opt){
			case 'i':
				flag_i=1;
				break;
			case 'o':
				flag_o=1;
                output = fopen(optarg, "w");
				break;
			default:
				USAGE();
		}
	}

    if(optind+2!=argc){
		USAGE();
	}

    txt1= fopen(argv[optind],"r");
    txt2= fopen(argv[optind+1],"r");
    if(txt1 == NULL|| txt2==NULL){
		USAGE();
	}

    int zeile = 0;

    while(fgets(buf1, MAX, txt1) && fgets(buf2, MAX, txt2)){

        zeile++;
        
        int bf1 = strlen(buf1);
        int bf2 = strlen(buf2);
        int lenght = 0;
        int error = 0;

        if(bf1>bf2){
            lenght = bf2-1;
        }else{
            lenght = bf1-1;
        }

        for(int i = 0 ; i<lenght; i++){

            if(flag_i == 1){
                if(strncasecmp(buf1,buf2,1) != 0){
                error++;
                }
            }
            else{
                
                if(strncmp(buf1,buf2,1) != 0){
                error++;
                }
            }

        memmove(buf1, buf1+1, strlen(buf1));
        memmove(buf2, buf2+1, strlen(buf2));
        }

        if(error>0){
             
             if(flag_o == 0){
                fprintf(stderr,"Line: %d, Characters: %d \n", zeile,error);
            }else{
                fprintf(output,"Line: %d, Characters: %d \n", zeile,error);
            }  
        }
    }
    
    fclose(txt1);
    fclose(txt2);

    return 0;
}