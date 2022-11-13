/**
 * @file myexpand.c
 * @author Rita Schrabauer <e12025342@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Replaces tab characters (\t) with spaces
 *
 * @details
 * An Input File is read line by line and outputted with all tab characters replaces with spaces.
 * The position of the next character after a tab character is calculated using
 *       p = tabstop * ((x / tabstop) + 1)
 * where
 *   p ... position of the next character
 *   tabstop ... tabstop distance (by default: 8, can be overridden using the option -t)
 *   x ... position of the tab character
 * The output is by default stdout, which can be overridden using option -o.
 * Possible execution without any options:
 * ./myexpand t1.txt
 *
 * Different options:
 *   -t ... This option is used to override the tabstop distance with an arbitrary positive integer.
 *   -o ... This option is used to override the output to the specified file
 * Possible execution using both options:
 * ./myexpand -t 4 -o out.txt t1.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define BUFFERSIZE 15       //Sets the buffersize, change to change size of array for lines

char *myprog;               //Global Variable for Programname

/**
 * @brief Usage function, exits function with EXIT_FAILURE
 * 
 */
void usage();

/**
 * @brief Function for any kind of Error, exits function with EXIT_FAILURE
 * 
 * @param message 
 */
void error(char message[]);

/**
 * Read Input file and output it.
 * 
 * @brief Reads Input file 'in' and outputs it to out with the tab characters replaced with spaces.
 * @details
 * The Input File 'in' is read line by line and outputted with all tab characters replaces with spaces.
 * The position of the next character after a tab character is calculated using
 *       p = tabstop * ((x / tabstop) + 1)
 * where
 *   p ... position of the next character
 *   tabstop ... tabstop distance
 *   x ... position of the tab character
 * The Output File is 'out'.
 * @param in ... Input File
 * @param tabstop ... tabstop distance
 * @param out ... Output File
 */
void myexpand(FILE* in, int tabstop, FILE* out);


/**
 * Program entry point
 * @brief Start of the program. Reads parameters, opens files, calls function myexpand and closes files
 * 
 * @details
 * The parameters are read within the while-loop, where the options are taken care of.
 * The rest of the parameters (should only be one parameter) is set as the name of the Input File.
 * The Input File in is opened. If in is not opened successfully, the program is terminated.
 * Otherwise:
 *      If there is an Output file given as option, the Output File out is also opened. Otherwise out is set to stdout.
 *      myexpand is called.
 * The files are closed.
 * The program is terminated with EXIT_SUCCESS.
 * 
 * @param arc ... argument counter
 * @param argv ... argument vector
 * @return returns EXIT_SUCCESS
 */
int main(int argc, char** argv){
    char * inName = NULL;
    char * outName = NULL;
    FILE * out;

    int tabstop = 8;
    int c;

    myprog = argv[0];

    while((c = getopt(argc, argv, "t:o:")) != -1){                  //gets Options
        switch(c){
            case 't':
                tabstop = (int) strtol(optarg, (char**) NULL, 10);  //gets tabstop-option and turns the string into an Integer
                break;
            case 'o':
                outName = optarg;
                break;
            default:
                usage();
                break;
        }
    }

    inName = argv[optind];                      //gets name for Input File
    
    FILE* in = fopen(inName, "r");              //opens Input File
    

    if(in == NULL){                             //if Input File was not opened
        error("Could not open Input File.");    //calls error function
    }
    else {
        if(outName != NULL) {                   //If Output File was given
            out = fopen(outName, "w");          //Opens Output File
            if(out == NULL){                    //If Output File was not opened
                error("Could not open Output File.");   //calls error function
            }
        }
        else {                                  //If Output File was not given
            out = stdout;                       //Sets stdout as Output
        }
        myexpand(in, tabstop, out);             //Calls myexpand
    }

    //Closing the Files
    fclose(in);
    if(outName != NULL) {
        fclose(out);
    }

    //Starting a new line in the terminal
    printf("\n");

    //ends the program successfully
    return EXIT_SUCCESS;
}

void usage(){
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

void error(char message[]){
    fprintf(stderr, "ERROR: %s\n%s\n", myprog, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief implementation of function myexpand
 * 
 * @param in 
 * @param tabstop 
 * @param out 
 */
void myexpand(FILE* in, int tabstop, FILE* out) {
    char line[BUFFERSIZE];      //Array for the particular line

    //while-loop runs through every line in 'in'
    while(fgets(line, BUFFERSIZE, in) != NULL){         //Saves each line into the array 'line', until there are no lines left in the file in
        int posi, poso = 0;                             //'posi' saves the position in array 'line', 'poso' saves the position of the output
        for(posi = 0; posi < strlen(line); posi++){     //for-loop runs through every character in 'line'
            if(line[posi] == '\t'){                     //if the current character is a tab character
                unsigned int calc = (tabstop * ((poso/tabstop) + 1));   //the position of the next not-tab character is calculated
                for(; poso < calc; poso++){             //for all positions until the calculatied position
                    fputc(' ', out);                    //outputs a space to 'out'
                }
            } else {
                fputc(line[posi], out);                 //outputs the current character of 'line' to 'out'
                poso++;                                 //sets the output position once to the left
            }
        }
    }
}