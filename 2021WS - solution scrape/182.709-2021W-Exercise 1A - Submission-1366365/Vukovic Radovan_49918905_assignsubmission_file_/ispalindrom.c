/**
 * @file ispalindrom.c
 * @author Radovan Vukovic <e1428736@student.tuwien.ac.at>
 * @date 13.11.2021
 * @studentid 01428736
 * 
 * @brief Main program for ispalindrom.c
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

static char* programName; 


/**
 * @brief      { writes the correct usage to standard error and exits the program with EXIT_FAILURE }
 *
 * @param[in]  <programName>  { name of program}
 */
static void usage (char* programName){
    fprintf(stderr,"Usage: ./%s  [-s] [-i] [-o outfile] [inputfile/s]", programName);
    exit(EXIT_FAILURE);
}

/**
 * @brief      { Checks line by line from input if the parsed string is palindrom and returns results to output  }
 *
 * @param      inputFile    The input file
 * @param      outputFile   The output file
 * @param      programName  The program name
 * @param[in]  sFlag        The s flag tells us if option [-s] is used
 * @param[in]  iFlag        The i flag tells us if option [-i] is used
 */
static void ispalindrom (FILE* inputFile, FILE* outputFile, char* programName, int sFlag, int iFlag); 

/**
 * @brief      { main method of ispalindrom.c }
 *
 * @param[in]  argc  The count of arguments
 * @param      argv  The arguments array
 *
 * @return     { returns EXIT_SUCCESS on succes and EXIT_FAILURE on failure }
 */
int main(int argc, char *argv[]){

    int c=0;
    FILE *inputFile[argc], *outputFile;   //input file array and output file
    int sFlag = 0;  //ignoring whitespaces when checking whether a line is a palindrom
    int iFlag = 0;  //not differentiate between lower and upper case letters
    int oFlag = 0;  //output file
    int inputfiles = 0; //number of input files
    int inputFlag = 0; 
    int i = 0; //variable for "for" loop

    if (argc > 0){
        programName = argv[0];
    }

    while(optind < argc){

        if ((c = getopt(argc, argv, "sio:")) != -1){
            switch (c){
            case 's':
                sFlag = 1;
                break;

            case 'i':
                iFlag = 1;
                break;

            case 'o':
                outputFile = fopen(optarg, "w");
                if(outputFile == NULL){
                    usage(programName);
                }
                oFlag=1;
                break;

            case '?':
                usage(programName);
                break;

            default:
                assert(0);
                break;
            }
        }

        else {

            inputFile[inputfiles] = fopen(argv[optind],"r");
            if(inputFile[inputfiles] == NULL){
                usage(programName);
            }
            inputFlag=1;
            optind++;
            inputfiles++;
        }
    }


     //no inputFile file
    if(inputFlag == 0){
        // no outputfile
        if(oFlag == 0){ 
            //input from terminal, output on terminal
            ispalindrom(stdin, stdout, programName, sFlag, iFlag);
        }
        else {
            //no inputfile, but outputfile
            ispalindrom(stdin, outputFile, programName, sFlag, iFlag);
        }
    }


    //no outputfile, but inputfile/s
    else if(oFlag == 0) { 
        for(i = 0; i < inputfiles; i++) {
            ispalindrom(inputFile[i], stdout, programName, sFlag, iFlag);
            fclose(inputFile[i]);
        }
    }
    else { 
        //inputfile/s and outputfile.
        for(i = 0; i < inputfiles; i++) {
            ispalindrom(inputFile[i], outputFile, programName, sFlag, iFlag);
            fclose(inputFile[i]);
        }
    }

    fclose(outputFile);
    exit(EXIT_SUCCESS);
}

static void ispalindrom (FILE* inputFile, FILE* outputFile, char* programName, int sFlag, int iFlag) {
       

    char *line = (char *) malloc(1000);
    char *temp = (char *) malloc(1000);
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, inputFile)) != -1) {


        if (line[read - 1] == '\n') 
        {            
            line[read - 1] = '\0';
        }

        strcpy(temp, line);

        
        int isP = 0;

        if (sFlag == 1)
        {

            int i = 0, j = 0;
            while (temp[i])
            {
                if (temp[i] != ' ')
                    temp[j++] = temp[i];
                i++;
            }
            temp[j] = '\0';
        }


        int half = strlen(temp) / 2;  

        int full = strlen(temp) ;

        if (iFlag == 1)
        {
           for (size_t i = 0; i < len; ++i) 
           {
            temp[i] = tolower(temp[i]);
            }
        }
        
            for (int i = 0; i < half; ++i)
            {   
               
                if (temp[i] == temp[full - i - 1])
                {
                    isP = 1;
                   
                }
                 else
                {
                    isP = 0;
                    
                }
            } 
            if (isP == 1)
            {
              fprintf(outputFile, "%s is palindrom\n", line);
              fflush(outputFile);  
            }
            if (isP == 0)
            {
              fprintf(outputFile, "%s is not palindrom\n", line);
              fflush(outputFile);  
            }
        }
        free(line);
        free(temp);
    }
        
        



                              