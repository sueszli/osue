/**
@author Adrian Stojanovic 11908093
@brief This module replaces the Tab symbol by a number of spaces (' '). The module has the following synopsis: myexpand [-t tabstop] [-o outfile] [file...]
@details  The amount of spaces needed to replace the Tab symbol is calculated by the formula: tabstop * ((x / tabstop) + 1) where x is the position of the tab symbol.
@date 11.11.2021
**/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//global variables
static char *myprog="myexpand";

/**
@brief reads a inputfile and replaces tabs with spaces the result is printed into an outputfile
@param *input The pointer of the input file
@param *output The pointer of the output file
@param *tabdis the "tabdistance" length of the Tab symbol. Can be defined by user
@return no return value
**/
static void readAndTabcalcOutfile(FILE *input, FILE *output, int tabdis){

    int pos=0;
    int t;
    char c;

    while (((c = fgetc(input))!=EOF)){
        if(c=='\t'){
            t = tabdis*((pos/tabdis) +1);
            int i=pos;
            for(;i<t;i++){
                fputc(' ',output);
            }
            continue;
        }
        pos++;
        fputc(c,output);
        if (c=='\n'){
            pos=0;
        } 
    }
}
/**
@brief reads a inputfile and replaces tabs with spaces the result is printed into stdout
@param *input The pointer of the input file
@param *tabdis the "tabdistance" length of the Tab symbol. Can be defined by user
@return no return value
**/
static void readAndTabcalcStdout(FILE *input,int tabdis){

    int pos=0;
    int t;
    char c;

    while (((c = fgetc(input))!=EOF)){
        if(c=='\t'){
            t = tabdis*((pos/tabdis) +1);
            int i=pos;
            for(;i<t;i++){
                fputc(' ',stdout);
            }
            continue;
        }
        pos++;
        fputc(c,stdout);
        if (c=='\n'){
            pos=0;
        }
    }
}
/**
@brief errormessage informs User that the program has been called in a wrong way and shows the synopsis
@param no parameters
@return no return value
**/
static void usage (void){
    fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n",myprog);
    exit(EXIT_FAILURE);
}


int main(int argc, char * argv[]){

    
    int opt;
    char *o_opt_arg=NULL; 
    int tabdis=8;

    while ((opt=getopt(argc,argv,"t:o:"))!=-1){
        switch (opt){
            case 'o' : o_opt_arg=optarg;
            break;

            case 't' : tabdis=(int) strtol(optarg,(char **)NULL,10);
            break;

            default: usage();
        }
    }

    if (tabdis<=0){
        fprintf(stderr,"[%s] ERROR: arguments of -t cant be smaller or equal to 0.\n",myprog);
        exit(EXIT_FAILURE);
    }
    


    if (optind != argc){                         //check for positional parameters

        if (o_opt_arg!=NULL){                    //check if -o argument conains a value
            FILE* output=fopen(optarg,"w");

            //check if file was opened correctly
            if ((output=fopen(optarg,"w"))==NULL){
                fprintf (stderr,"[%s] ERROR: fopen failed: %s\n",myprog,strerror(errno));
                exit(EXIT_FAILURE);
            }

        //open all input files
        int i=optind;
        for (; i < argc; i++){
        
            FILE* input=fopen(argv[i],"r");
            
            //check if file was opened correctly
            if ((input=fopen(argv[i],"r"))==NULL){
                fprintf (stderr,"[%s] ERROR: fopen failed: %s\n",myprog,strerror(errno));
                exit(EXIT_FAILURE);
            }

            readAndTabcalcOutfile(input,output,tabdis);
            fclose(input);
            fclose(output);
            }//end forloop
            exit(EXIT_SUCCESS);
        }//end if
        else{
            int i=optind;
            for (; i < argc; i++){
        
                FILE* input=fopen(argv[i],"r");

                //check if file was opened correctly
                if ((input=fopen(argv[i],"r"))==NULL){
                    fprintf (stderr,"[%s] ERROR: fopen failed: %s\n",myprog,strerror(errno));
                    exit(EXIT_FAILURE);
            }

            readAndTabcalcStdout(input,tabdis);
            fclose(input);
            }
            exit(EXIT_SUCCESS);
        } 
    
    }//end if inputfilecheck

    if (optind==argc){                           
        if (o_opt_arg!=NULL){                    
            FILE* output=fopen(optarg,"w");

            //check if file was opened correctly
            if ((output=fopen(optarg,"w"))==NULL){
                fprintf (stderr,"[%s] ERROR: fopen failed: %s\n",myprog,strerror(errno));
                exit(EXIT_FAILURE);
            }

            readAndTabcalcOutfile(stdin,output,tabdis);
            fclose(output);
            exit(EXIT_SUCCESS);
        }else{
            readAndTabcalcStdout(stdin,tabdis);
            exit(EXIT_SUCCESS);
        }
        
    }
    

    return 0;
}

