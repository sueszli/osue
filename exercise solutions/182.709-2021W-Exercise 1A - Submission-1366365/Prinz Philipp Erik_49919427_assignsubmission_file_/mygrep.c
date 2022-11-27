/**
*@file mygrep.c
*@author Philipp Prinz 11776855
*@date 27.10.2021
*
*@brief Main Programm module
*
* All necessery functions of the Programm are in this File.
* The programm takes a keyword and an input, Files or stdin and writes every line that contains the keyword
* into the output Files or stdout.
**/


#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<getopt.h>
#include<ctype.h>
#include<string.h>

/**
* Support function
* @brief This function cycles through every line in the given input file determains if it contains the keyword and is case sensitivity 
* and writes the result to the given output file.
* @details Reads every line with getline and cycles through every character from the line and the keyword string and
* checks if they are the same. If the current char of the input line matches with the current char of the keyword the next char of the keyboard
* is checked with the next char of the line, if they dont match the first character of the keyword will be used.
* The characters that are pointed to are safed in lchar and kchar and converted to there upper case form if case sensitivty should be ignored.
* After the keyword check the string(line) is written into the output file with fprintf.
* After the last string line is freed.
* @param casesens Determains if the keyword should be case sensitive
* @param keyword String to check if contained in the input File
* @param in The input File pointer of the strings that should be checked
* @param out The output File pointer where the strings should be written to after the check
**/
void grepInput(bool casesense, char * keyword, FILE * in, FILE * out){
    char * line = NULL;

    size_t len = 0;
    ssize_t nread = 0;

    int kcount = 0;
    int lcount = 0;

    char lchar;
    char kchar;

    bool grepLine = false;

    while((nread = getline(&line,&len,in)) > -1){
        
        kcount = 0;
        lcount = 0;
        grepLine = false;

        while(lcount<nread-1){
            lchar = *(line+lcount);
            kchar = *(keyword+kcount);

            if(!casesense){
                lchar = toupper(lchar);
                kchar = toupper(kchar);
            }

            if(lchar==kchar){
                if(kcount == strlen(keyword)-1){
                    grepLine = true;
                }else{
                    kcount++;
                }
            }else{
                kcount = 0;
            }
            lcount++;
        }

        if(grepLine){
            fprintf(out, "%s", line);
        }
    }
    
    free(line);

}

/**
* Program entry Point
* @brief Takes care of the given arguments and trys to open all given in- and output files if there are any.
* @details Checks the arguments and determains if the program should ignore case sensitivity in the input strings. 
* Also trys to open the output and input files if there are any specified in the arguments.
* Also executes function grepInput multiple or a single time depending on the amount of input files.
* @param argc The argument counter.
* @param argv The argument vector.
* @return EXIT_SUCCESS
**/
int main(int argc,char * argv[]){
    int opt;

    bool casesense = true;
    bool outFileSet = false;
    char * keyword;
    FILE *input = stdin;
    FILE *output = stdout;
    
    while((opt = getopt(argc, argv, "io:")) != EOF){
        switch(opt){
            case 'i':
                casesense = false;
                break;
            case 'o':
                if(!outFileSet){
                    if(!(output = fopen(optarg,"w"))){
                        fprintf(stderr,"Could not open output File!");
                        exit(EXIT_FAILURE);
                    }
                    outFileSet = true;
                }
                else{
                    fprintf(stderr,"Output File already set!");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr,"mygrep [-i] [-o outfile] keyword [file...]");
                exit(EXIT_FAILURE);
                break;
        }
    }

    
    if(argc - optind == 0){
        fprintf(stderr,"Keyword missing!");
        exit(EXIT_FAILURE);
    }else{
        keyword = argv[optind];
        optind++;
        if(optind==argc){
            grepInput(casesense,keyword, input, output);
        }
        while(argc>optind){
            if(!(input=fopen(argv[optind],"r"))){
                fprintf(stderr,"Could not open input file %s", argv[optind]);
                exit(EXIT_FAILURE);
            }
            optind++;
            grepInput(casesense,keyword, input, output);
            fclose(input);
        }
    }

    fflush(output);
    fclose(output);
    exit(EXIT_SUCCESS);

}