/**
 * @file mygrep.c
 * @author 12024755, Florin-Elis Buju <e12024755@student.tuwien.ac.at> 
 * @date 04.11.2021
 * @brief Betriebssysteme 1A mygrep
 * @details A program simulating a reduced variation of the Unix command grep
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool typ
#include <getopt.h> //getopt methode
#include <string.h> // Stringcompare und Stringsuche
#include <errno.h> // errno
#include <ctype.h> // tolower,toupper Function 
#include <assert.h> // Asserts

static char *progname = NULL; /** name of the program */
static char *keyword = NULL; /** keyword, string search in each line */
static FILE *out = NULL; /** Output file if available*/
static FILE *in = NULL; /** Current input file if available*/
static char *verify = NULL; /** copy of buffer, all chars are small, used if case insensensitiv */
static char *buffer = NULL; /** All lines of written files or inputs, are saved into this var till the nexx line is read */
static bool case_sensensitiv = true; /** Tells the program if -i was given as input option or not */
static char *o_outfile = NULL;  /** Tells us if there is a outputfile and the path */

static void myErrorout(char *failedto); 
/**
 * @brief Cleanup function, cloes all files and alloceted memorys
 * @details Funktion is called when program terminates or had an error, 
 * @param programname is giving to call further errors if necessary
 * @return no return value (void)  
 **/
static void cleanUp()
{
    if(o_outfile != NULL)
    {
        fclose(out);
    }    
    
    if(case_sensensitiv == false)
    {
        if(verify != NULL)
        {
            free(verify);
        }
        if(keyword != NULL)
        {
            free(keyword);
        }
    }
    
}
/**
 * @brief Error function of the program
 * @details Will output the passed error message and will exit the programm with EXIT_FAILURE
 * @param failedto is the ErrorMessage which should be print
 * @return no return value (void) 
 **/
static void myErrorout(char *failedto)
{
    if(strcmp(strerror(errno),"Success")==0)
    {
        fprintf(stderr, "[%s] ERROR: %s\n", progname, failedto);
    }
    else
    {
        fprintf(stderr, "[%s] ERROR: %s: %s\n", progname, failedto, strerror(errno));
    }
    cleanUp();   
    exit(EXIT_FAILURE);
}
/**
 * @brief Saves the given strings into mystring but with "lower" chars
 * @details Every char from tocopy is saved into myString with l
 * @param myString is a string where the small chars from the string tocopy
 * @return no return value
 **/
static void StringtoLower(char* myString, char* tocopy) {
    int i;
    for (i = 0; i < strlen(tocopy); i++)
    {
        myString[i] = tolower(tocopy[i]);
    }
}
/**
 * @brief Implements the main function of the task, namely 
 * @details After the input parameters have been processed, 
 * a line by line check is made from files or stdin to see if a certain word occurs. 
 * ALl those particual lines are writen to stdout or and outfile
 **/
int main(int argc, char *argv[]) {
    progname = argv[0];

    bool noInputfiles = false;

    //Processing the input parameters
    int c;
    while ((c = getopt(argc, argv, "io:")) != -1 ){
        switch (c) {
        case 'o': o_outfile = optarg;
        break;
        case 'i': case_sensensitiv = false;
        break;
        case '?':
        //ERROR: Illegal option as input 
        myErrorout("Usage: mygrep [-i] [-o outfile] keyword [file...]"); 
        break;
        default:
        // Shouldn't get into this part
        assert(0);
        break;
        }
    }
  
    if(argv[optind] == NULL)
    {
        //ERROR: No keyword was submitted
        myErrorout("No keyword was submitted. Usage: mygrep [-i] [-o outfile] keyword [file...]"); 
    }
    else
    {
        //Check if case insensensitiv and get keyword
        if(case_sensensitiv == false)
        {
            keyword = malloc(strlen(argv[optind]) * sizeof(char));
            StringtoLower(keyword,argv[optind]);
        }
        else
        {
            keyword = argv[optind];
        }
            
        
        // Check if function return value and normal given value is OK
        if(keyword == NULL || strcmp(keyword,"")==0)
        {
            //There was an error processing the keyword.
            myErrorout("There was an error processing the keyword"); 
        }
    }

    if(o_outfile != NULL)
    {
        if ((out = fopen(o_outfile, "w")) == NULL)
        {
            //ERROR: Outfile cannot be opened
            myErrorout("Outfile could not be opened"); 
        }
    }

    int i=optind+1;
    //Are there outputfiles
    if(i==argc)
    {
        noInputfiles = true;
    }
    // Read all files or just stdin 
    while(i<argc || noInputfiles == true)
    {
        //Open "next" file or StdIN
        if(noInputfiles == true)
        {
            noInputfiles = false;
            in = stdin;
        }
        else
        {
            if ((in = fopen(argv[i], "r")) == NULL)
            {
                //ERROR: Failed to open a file
                myErrorout("Could not open an inputfile"); 
            }
        }
        
        size_t length = 0;
        // Get line of current in file and process it
        while (getline(&buffer, &length, in) != -1) 
        {
            if (ferror(in))
            {
                 myErrorout("Could not read an inputfile."); 
            }
            if ('\n' == buffer[0]) 
            { 
                //Empty lines can be ignored
                continue; 
            }
            // Check for case insensensitiv
            if(case_sensensitiv == false)
            {
                //Make a copy of buffer and make all chars in the copy small
                
                verify = malloc(sizeof(char) * length);
                if (verify == NULL) 
                {
                    myErrorout("malloc for verify failed.");
                }
                StringtoLower(verify,buffer);
            }
            else
            {
                verify = buffer;
            }
            //Check if return value is okay
            if(verify == NULL)
            {
                myErrorout("There was an error processing the InputText of a line."); 
            }
            //Check if the keyword is in the buffer
            if(strstr(verify, keyword) != NULL)
            {
                //Found keyword in Line
                if(o_outfile != NULL)
                {
                    // If an outfile was given, write in there
                    if (fputs(buffer, out) == EOF)
                    {
                        // ERROR: Failed to Write
                        myErrorout("Failed to Write in outfile"); 
                    }
                }
                else
                {
                    //Otherwise write in in stdout
                    if (fputs(buffer, stdout) == EOF)
                    {
                        // ERROR: Failed to Write
                        myErrorout("Failed to Write in stdout"); 
                    }
                }
            }
        }
        free(buffer);
        
        if(in != stdin)
        {
            fclose(in);
        }
        i=i+1;
    }
    //CleanUp and exit the program
    
    cleanUp();
    exit(EXIT_SUCCESS);
    return 0;
}
