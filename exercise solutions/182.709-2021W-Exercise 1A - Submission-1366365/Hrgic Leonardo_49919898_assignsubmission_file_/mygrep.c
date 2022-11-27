 /**
 * @file mygrep.c
 * @author Leonardo Hrgic 12024724
 * @brief prints all lines containing the keyword
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
/**
 * @brief 
 * structure with the programm variables
 * @param name contains the name of the programm
 * @param notcasesensitiv if its 1 the programm does not differentiate between lower and upper case letters if its 0 it does differentiate
 * @param in contains the input file
 * @param out contains the output file
 * @param keyword contains the keyword 
 */
typedef struct progVar{
    char *name;
    int notcasesensitiv;
    FILE *in;
    FILE *out;
    char *keyword;
    
}progvariables;


/**
 * @brief 
 * converts a string to lower case letters
 * @details
 * goes through a string char by char and converts the chars to lowercase
 * @param string the string that gets converted
 */
static void StringToLower(char *string){    //converts the string to lowercase letters
	for(int i=0; string[i]; i++){
		string[i] = tolower(string[i]);
	}
}
/**
 * @brief 
 * prints out lines that contain a keyword
 * @details
 * uses function strstr() to check if lines in the input file contain the keyword
 * if the flag notcasesensitiv is set the keyword and lines is set to lower
 * the lines get printed to the outputfile
 * @param var includes all important variables (input file ,output file , programm name ,keyword, notcasesensitiv flag)
 */
static void KeywordInFile(progvariables var){
    char *line=NULL,*linecopy=NULL;
    size_t length=0;
    if(var.notcasesensitiv==1){     
        StringToLower(var.keyword);
    }
    while(getline(&line,&length,var.in)!=-1){   
        linecopy=strdup(line);  //copies the string so the original can be returned if you use toLower
        if(var.notcasesensitiv==1)StringToLower(linecopy);
        if(strstr(linecopy,var.keyword)!=NULL){ // checks if the keyword is in the copied line
            fprintf(var.out,"%s",line);
            fflush(var.out);   
        }
    }
    
    free(linecopy);
    free(line);
}
/**
 * @brief 
 * Prints the synopsis of the code to show the expected arguments and exits the programm
 * @details 
 * gets called when the user makes an wrong input
 * @param progName contains the name of the programm
 */
static void errorSynopsis(char *progName){
	fprintf(stderr, "Synopsis: %s [-i] [-o outfile] keyword [file...]\n", progName);
	exit(EXIT_FAILURE);
}
/**
 * @brief 
 * checks if the amoung of arguments is right an apllies the options
 * @details
 * checks with getopt() if the right amount of options and arguments where used 
 * and then calls KeywordInFile with the options and arguments that were given
 * if there were multiple input files it has to call it for each file
 * @param argc the amount of arguments given
 * @param argv an array of the arguments
 * @return EXIT_SUCCES on succes and EXIT_FAILURE on failure
 */
int main(int argc, char *argv[]){
progvariables var;
var.name = argv[0];
var.notcasesensitiv=0;
int ocount=0;
int c;

while ( (c = getopt(argc, argv, "io:")) != -1 ){//gets the options one by one till there are no left
    switch(c){
        case 'i':
                
                var.notcasesensitiv=1;  //sets the flag for caseinsensivity
                break;
        case 'o':   //sets the output file to the file given by the user if there was more then  1 file it exits the programm with an error
                if(ocount==0){
                    
                    if((var.out=fopen(optarg,"w"))==NULL){
                        fprintf(stderr,"%s: Opening output file failed: %s\n",var.name,strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    ocount++;
                }else{
                    fprintf(stderr,"%s: Too many output files\n",var.name);
                    errorSynopsis(var.name);
                }break;
        default:
            errorSynopsis(var.name);
    }
}
if(optind>=argc)errorSynopsis(var.name);
if(ocount==0)var.out=stdout; //if there was no output file it uses stdout
var.keyword=argv[optind];    //saves the keyword from the options
optind++;
if(optind>=argc){           // if the optindex is bigger then the argument count there was no input file given and it uses stdin
    var.in=stdin;
    KeywordInFile(var);
}
else{
    while(optind<argc){     // calls KeywordInFile for evrey input file
        if((var.in=fopen(argv[optind],"r"))==NULL){
            fprintf(stderr,"%s: Opening input file failed: %s\n",var.name,strerror(errno));
            exit(EXIT_FAILURE);
        }
        KeywordInFile(var);
        fclose(var.in);
        optind++;

    }
}
if(var.out!=stdout)fclose(var.out);
exit(EXIT_SUCCESS);
}

