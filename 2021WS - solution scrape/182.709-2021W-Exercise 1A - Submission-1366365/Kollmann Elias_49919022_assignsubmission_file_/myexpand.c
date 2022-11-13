/**
*@file myexpand.c
*@author Elias Kollmann
*@date 13.11.2021
*@brief This program from an input and replace the Tab symbols with spaces
*@details The programm will read from either stdin or a file and 
*is goining to replace th Tab symbobols with the right amount of spaces, 
*it can switch the noumber of spaces depending on the given argument.  
*
**/
#include <string.h>
#include "stdio.h"
#include <stdlib.h>
#include "getopt.h"
#include <errno.h>


int tabstop = 8;
char *ptr;
char *outfile;
int posArgFlag;
int oflag;
char *infile;
int lineCount = 0;
char *myprog = "myExpand";

FILE* IN;
FILE* OUT;

/**
*@brief This function calculates the tabstop distance
*@details global variables: tabstop
*
**/

int tabstop_calc(int x,int t)   
{
    return t * (( x / t) + 1 ); 
}


/**
*@brief This function prints an usage message 
*@details global variables: myprog
*
**/

static void usage_message(void)
{
    fprintf(stderr,"Usage: %s [-o outfile] [-t tabspace] [file]\n",myprog);
    exit(EXIT_FAILURE);
}

/**
*@brief The main function provides the functionallity of the programm
*@details The function manages the given arguments and reads from the given Input.
*In the given input the Tabs are replaced with spaces, and are outputed ether to stdout or to a specified file.
*
*@param argc argument counter
*@param argv argument vector
*@return 0
**/

int main(int argc, char **argv)
{
    //printf("<1>");
    int option;
    while ((option = getopt(argc,argv, "t:o:")) != -1)
    {
        switch (option)
        {
            case 't':
                if(optarg == NULL)
                {
                    usage_message();
                }
                tabstop = strtol(optarg,&ptr,10);
                break;

            case 'o':
                if(optarg == NULL)
                {
                    usage_message();
                }
                oflag++;
                outfile = optarg;
                break;
            
            case '?':
                usage_message();
                break;
            default:
                usage_message();
                break;
        }
    }

    if(optind + 1 < argc){
        usage_message();
        exit(EXIT_FAILURE);
    }

    if(optind == argc){
        posArgFlag++;
    }else{
        int ind;
        for (ind = optind; ind < argc; ind++)
        {
            infile = argv[ind];
        } 
    }

    if ( oflag > 0 )
    {
        if ( (OUT = fopen(outfile,"w") ) == NULL )
        {
        fprintf(stderr, "[%s] ERROR: fopen failes : %s\n", outfile, strerror(errno));
        exit(EXIT_FAILURE);
        }   
    }
    
    if(posArgFlag > 0)
    {
        IN = stdin;
    } else
    {
        
        if ( (IN = fopen( infile, "r" ) ) == NULL )
        {
        fprintf(stderr, "[%s] ERROR: fopen failes : %s\n", infile, strerror(errno));
        exit(EXIT_FAILURE);
        } 
    }

    char c;
    while( (c = fgetc(IN)))
    {
        if(c == EOF)
        {
            break;
        }
        if(c == '\t')
        {
            int tabPos = tabstop_calc(lineCount,tabstop);
            while (tabPos != lineCount)
            {
                if ( oflag > 0)
                {
                    fprintf(OUT," ");
                } else
                {
                    printf(" ");
                }
                lineCount++;
            } 
        }else if (c == '\n')
        {
            if ( oflag > 0)
            {
                fprintf(OUT,"\n");
            } else
            {
                printf("\n");
            }                

            lineCount = 0;
        } else
        {
            if( oflag > 0)
            {
                fprintf(OUT,"%c",c);
            }else
            {
                printf("%c",c);
            }
            lineCount++;
        }
    }
    
    if(posArgFlag == 0)
    {
        fclose(IN);
    }
    if ( oflag > 0 )
    {
        fclose(OUT);
    }

    exit(EXIT_SUCCESS);
}
