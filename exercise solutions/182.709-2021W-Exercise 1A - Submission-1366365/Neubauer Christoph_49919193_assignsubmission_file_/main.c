/**
 * @file main.c
 * @author Christoph Neubauer 12023172
 * @brief main file of "myexpand"
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const char *myProg;

/**
 * @brief prints the usage message of this program
 * 
 */
static void printUsage(void)
{
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n" ,myProg);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints the error Message and exits with Failure
 * 
 * @param errMsg Error Message that is printed
 */
static void printErrAndExit(char* errMsg)
{
    fprintf(stderr, "%s: %s: %s", myProg,errMsg,strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief parses the given option arguments with getopt
 * 
 * @param argc number of given arguments
 * @param argv string-array of arguments
 * @param tabstop call-by-reference return value for optional argument '-t'
 * @return adress of the name of the output file
 */
static char* parseArguments(int argc, char * argv[], int* tabstop)
{
    myProg = argv[0];   // save the filename
    char *outFileName = NULL;
    int c;
    char* fail;
    while((c = getopt(argc,argv, "t:o:")) != -1)  {
        switch(c) {
            case 't':
                *tabstop = strtol(optarg,&fail,0); // calc the int value of the char input
                if(fail == optarg){
                    printErrAndExit(" no integer argument with -t option"); 
                }
                if(errno != 0){
                    printErrAndExit(" faulty argument with -t option");
                }
                break;
            case 'o':
                outFileName = optarg;
                break;
            default:        // any other option except the specified
                printUsage();
                break;
        }
    }
    return outFileName;
}

/**
 * @brief reads a file and exchanges '\t' with a specific number of space characters
 * 
 * @param outFile File where the changed text is written to
 * @param inFile  File where text is read from
 * @param tabstop defines how many spaces should be written instead of a tab
 */
static void expand(FILE* outFile, FILE* inFile,int tabstop)
{
    int pos = 0;
    int newPos = 0;
    int spaces = 0;
    char c;
    while((c=fgetc(inFile))!= EOF)
    {
        if(c == '\t'){
            // calculate how many spaces have to be printed into the outputfile
            newPos = tabstop *((pos/tabstop)+1);
            spaces = newPos - pos;
            // write spaces accourding to calculation
            while(spaces>0){        
                if(fprintf(outFile," ")<0){
                    fclose(outFile);
                    fclose(inFile);
                    printErrAndExit("Couldn't write char to outfile");
                }
                spaces--;
            }
            pos = newPos;   // currecnt position after placed spaces
        }
        else{   // print all the other chars
            if(fprintf(outFile,"%c",c)<0){    
                fclose(outFile);
                fclose(inFile);      
                printErrAndExit("Couldn't write char to outfile");
            }
            pos++;
        }
        
        if(c == '\n'){  // reset the position in a line after a linebreak
            pos = 0;
        }

    }
}

/**
 * @brief main function of myexpand
 * 
 * @param argc number of given arguments
 * @param argv string-array of arguments
 */
int main(int argc, char *argv[])
{
    int tabstop = -1;
    char* outFileName = parseArguments(argc,argv,&tabstop);
    FILE* outFile;    

    // standard value for tabstop is 8
    if(tabstop == -1)  {
        tabstop = 8;
    }
    if(outFileName == NULL) {   // if no optional outfile is given, write to stdout
        outFile = stdout;
    }
    else {
        outFile = fopen(outFileName,"w");
    }
    if(outFile == NULL) {
        printErrAndExit("outfile couldn't be opened");
    }

    if((argc-optind) == 0) {    // no positional arguments -> read from stdin
        expand(outFile,stdin,tabstop);
    }
    else {
        while(optind != argc)   // iterate over all given inputfiles
        {
            FILE* inFile = fopen(argv[optind],"r");
            if(inFile == NULL)
            {
                fclose(outFile);
                printErrAndExit("infile couldn't be opened");
            }
            expand(outFile,inFile,tabstop);
            fclose(inFile);
            optind++;
        }
        fclose(outFile);
    }
    return EXIT_SUCCESS;
}