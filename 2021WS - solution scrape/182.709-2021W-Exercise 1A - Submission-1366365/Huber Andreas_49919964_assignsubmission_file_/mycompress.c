/** 
 *@file mycompress.c
 *@author Andreas Huber 11809629
 *@date 28.10.2021
 *
 *@brief mycompress program module. compresses input files and prints a statistic
 *
 *@details This program will read the content of any files given one after the other, compress it and
 * write the compressed content to an output file given by the option -o. If no input files are
 * given, the program reads from stdin. If no output file is given, the program write to stdout.
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <assert.h>

//getopt for argument parsing.
int getopt(int argc, char *const argv[], const char *optstring);


/**
 *compressor function
 *@brief the compressor function takes an input file, compresses it and writes the compressed result
         into an output file. The chars read and written are counted while doing so.
 *@details This function uses the following global variables:
 *          c: current character used by the function
 *          tmp: int-return value of fprintf
 *          count: counter for repeated occurences of characters
 *          oldChar: last seen character
 *@param input the input that will be read from and compressed
 *@param output the output file where the compressed text will be written
 *@param read pointer for counting how many chars have been read
 *@param written pointer for counting how many chars have been written
 *@param progName name of the program at argv[0] for Errorhandling
 *@return this function has no return value
**/
void compressor(FILE *input, FILE *output, int *read, int *written, const char* progName){
    int count = 0;
    char oldChar;

   while(true){

            //get next char from input
            char c = fgetc(input);
            if(feof(input)){
                break;
            }

            *read = *read + 1;

            //if its the first char, increase count
            if(count == 0){
                count++;
                oldChar = c;
                continue;
            }

            // if its the same char as before, increase count
            if(c==oldChar){
                count++;
                continue;
            }

            //different character than before. 
            //tmp: returns number of characters printed, negative number if it fails
            int tmp = fprintf(output, "%c%d", oldChar, count);
            if(tmp<0){
                fprintf(stderr, "[%s] ERROR: fprintf failed: %s\n", progName, strerror(errno));
                fclose(output);
                exit(EXIT_FAILURE);
            }

            *written += tmp;
            count = 1;
            oldChar = c;
        }

        int tmp = fprintf(output, "%c%d", oldChar, count);
            if(tmp<0){
                fprintf(stderr, "[%s] ERROR: fprintf failed: %s\n", progName, strerror(errno));
                fclose(output);
                exit(EXIT_FAILURE);
            }
            *written+=tmp;
            fflush(stdout);
            return;

}


/**
 *Program entry point.
 *@brief Program starts here. main function takes care of the parameters and uses the compressor function
 *       to compress the given input. 
 *@details This function uses the following global variables:
 *          Outfile: stores the path to the outputfile if -o is given
 *          c: character-variable for getopt-usage
 *          ProgramName: stores the Programname from argv[0]
 *          numberOfInputFiles: stores the ammount of input files
 *          inputFileNames: array of char* to inputFilenames
 *          outputFile: actual File, in which compressed result will be written
 *          read: counter of read characters
 *          written: counter of written characters
 *@param argc The argument counter.
 *@param argv The argument vector.
 *@return Returns EXIT_SUCCESS
**/
int main(int argc, char *argv[]){

   //initialize outputFile to NULL to not have random memory in its location 
   const char *Outfile = NULL;   
   int c;
   const char *ProgramName = argv[0];

   while ((c = getopt(argc, argv, "o:")) != -1)
	{
		switch (c)
		{
		case 'o':       //get output-flag, check if only 1 output-flag is given.
            if(Outfile != NULL){
                fprintf(stderr, "[%s] ERROR: only 1 -o flag permitted.\n", ProgramName);
                exit(EXIT_FAILURE);
            }
			Outfile = optarg;
			break;
		case '?': // invalid option, print correct usage here
            fprintf(stderr, "[%s] ERROR: Correct Synopsis: %s [-o outputFile] [file...]", ProgramName, ProgramName);
                exit(EXIT_FAILURE);
			break;
		default : // default case, should not be reached 
            assert(0);
			break;
		}
	}

    //get ammount of inputfiles and create pointers to each of them.
    int numberOfInputFiles = argc - optind;
    char *inputFileNames[numberOfInputFiles];
    for(int i = 0; i< numberOfInputFiles; i++){
        inputFileNames[i] = argv[optind +i];
    }

    //create outputFile if -o is present, otherwise use stdout as output
    FILE* outputFile = stdout;
    if(Outfile !=NULL){
        outputFile = fopen(Outfile, "w"); 
        if (outputFile == NULL){
            fprintf(stderr, "[%s] ERROR: fopen failed on outputFile %s\n", ProgramName, strerror(errno));
            
        }
    }

    //compression
    int read = 0;
    int written = 0;
    
    for(int i = 0; i < numberOfInputFiles; i++){
        FILE *inputFile = fopen(inputFileNames[i], "r"); 
        if(inputFile == NULL){
            fprintf(stderr, "[%s] ERROR: fopen failed on inputFile %s\n", ProgramName, strerror(errno));
            fclose(outputFile);
            exit(EXIT_FAILURE);
        }

        compressor(inputFile, outputFile, &read, &written, ProgramName);

        fclose(inputFile);
    }

    
    //compression is no input file is given
    if(numberOfInputFiles == 0){

        compressor(stdin, outputFile, &read, &written, ProgramName);

    }

    
    if(outputFile != stdout){
        fclose(outputFile);
    }

    //print statistics
    fprintf(stderr, "\nRead: %d characters\nWritten:%d characters\nCompression ratio: %4.1f%%\n", read, written, ((double)(written)/read)*100);
    return EXIT_SUCCESS;

}