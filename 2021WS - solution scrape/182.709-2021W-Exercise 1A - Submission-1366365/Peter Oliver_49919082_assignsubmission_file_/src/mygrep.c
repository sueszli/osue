/**
 * @file mygrep.c
 * @author Oliver Peter <e11915885@student.tuwien.ac.at>
 * @date 03.11.2021
 *
 * @brief Main program module.
 * 
 * The program mygrep reads files line by line and for each line checks whether it contains the search term keyword. 
 * The line is printed if it contains keyword, otherwise it is not printed.
 * 
 **/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>


/**
 * @brief this is a usage function which shows the correct synopsis for the mygrep program and then exits with EXIT_FAILURE
**/
static void usage(void) {
    fprintf(stderr,"mygrep [-i] [-o outfile] keyword [file...]");
    exit(EXIT_FAILURE);
}


/**
 * @brief this function searches for the keyword in the heystack
 * @details  mainly this function is a case differentiation if you want to ignore the case or not
 * @param heystack: the string in which the keyword should be looked for
 * @param keyword: the keyword to look for in the heystack
 * @param opt_i: if 0 then the case should not be ignored, otherwise it should be ignored
 * @return returns a pointer to the beginning of the found substring or NULL if no substring was found
**/
static char* keyWordFound(char* heystack, char* keyword, int opt_i){
    if(opt_i==0){
        return strstr(heystack,keyword);
    }else{
        return strcasestr(heystack,keyword);
    }
}

/**
 * @brief This is the main function of mygrep
**/
int main(int argc, char **argv){


    char *outfile = NULL; 
    int opt_i = 0;
    char *keyword = NULL;
    int c;

    FILE *input, *output;

/**
 * This while reads all the options from the command line argument and handles it in switch statements
 * As soon as -1 is returned there are no more options left
 * option o sets the outfile and option i is a flag if the case of the keyword should be ignored
 * If a not expected option occurs then the usage function is called to show the right synopsis
**/
    while ( (c = getopt(argc, argv, "io:")) != -1 ){
        switch ( c ) {
            case 'o': 
                outfile = optarg;
                break;
            case 'i': 
                opt_i++;
                break;
            case '?': 
                usage();
                break;
            default: 
                break; 
        }
    }

    //If there are no more arguments left after the options that means that no keyword was given
    if(optind == argc){
        fprintf(stderr,"[%s] ERROR: No keyword given\n",argv[0]);
        exit(EXIT_FAILURE);
    }else{
        keyword = argv[optind];
    }

    //If an outputfile was specified open a writer to its path
    if(outfile!=NULL){
        if((output = fopen(outfile,"w"))==NULL){
            fprintf(stderr,"[%s] ERROR: fopen failed: %s\n",argv[0],strerror(errno));
        }
    }else{
        output=stdout;
    }
    //filecount is the index of argv and is now on the first argument that was given after the keyword (if such an argument exists)
    int filecount = optind +1;

    //buffer for reading from file will be dynamicly allocated by function getline()
    char *buf = NULL;
    size_t bufsize = 0;
    
    //if filecount==argc = true --> no filenames for input files are specified -> read from stdin
    if(filecount==argc){
        
        
        while(getline(&buf,&bufsize,stdin)!=-1){
            if(keyWordFound(buf,keyword,opt_i)!=NULL){
                if(fputs(buf,output)==EOF){
                    fprintf(stderr,"[%s] ERROR: fputs failed: %s\n",argv[0],strerror(errno));
                }
                fflush(output); //flush so you can see the line you typed immediately in the outfile
            }
        }
       
        

    }

    
    //case when there are filenames specified --> read them till filecount is at the end of the argumentlist (filecount == argc)
    while(filecount<argc){
        //current inputfile
        char *infile = argv[filecount];
        //try to open a reader on the inputfile
        if((input = fopen(infile,"r"))==NULL){
            fprintf(stderr,"[%s] ERROR: fopen failed: %s\n",argv[0],strerror(errno));
        }

        
        
        while(getline(&buf,&bufsize,input)!=-1){
        
            if(keyWordFound(buf,keyword,opt_i)!=NULL){
                if(fputs(buf,output)==EOF){
                    fprintf(stderr,"[%s] ERROR: fputs failed: %s\n",argv[0],strerror(errno));
                }
            }
            
            
        }
       

        
        //close the input writer
        fclose(input);
        filecount++;

    }
    //Free all allocated memory and the outputwriter
    free(buf);
    buf=NULL;
    fclose(output);
    output=NULL;
    return(EXIT_SUCCESS);
}
    

