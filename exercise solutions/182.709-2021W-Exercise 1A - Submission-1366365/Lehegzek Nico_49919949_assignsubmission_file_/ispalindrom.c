/**
 * @file ispalindrom.c
 * @author Nico Lehegzek 12021356
 * @date 05.11.2021
 *  
 * @brief Main program module. It checks if input strings are palindroms. 
 * 
 * @details This program checks if strings of possibly infinite length are palindroms.
 *          It is possible to check the string as it is given, ignore whitespaces, ignore cases
 *          or ignore both. Furthermore it is possible to define an output file and multiple input files.
 *          If nothing is defined stdin will be used for the input and stdout will be used for the output.
 *          Every line from the input files will be analyzed.
 */

#define  _POSIX_C_SOURCE 200809L /**< this is needed for getline()*/

#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>



static char *progName; //*< this stores the program name*/

/**
 * @brief This function writes the correct synopsis of this program to stderr
 * 
 * @details exits the program with the code EXIT-FAILURE after output to stderr
 *          global variable: progName - is the program name as written in argv[0]
 * 
 * @return has no return values (void) 
 */
static void usage(void){
    fprintf(stderr, "USAGE: %s [-s] [-i] [-o outfile] [file...]\n",progName);
    exit(EXIT_FAILURE);
}


/**
 * @brief This function checks if "string" is a palindrom under given circumstances.
 * 
 * @details The function doesn't check if the given string is valid.
 *          The string is either checked the way it is given, ignoring whitespaces,
 *          ignoring cases or ignoring both (depending on parameters).
 * 
 * @param ignoreSpace: if true (1) then whitespaces will be ignored for the palindrom check
 *        ignoreCase : if true (1) then everything will be compared in uppercase
 * 
 * @return Returns true (1) if the "string" is a palindrom under given circumstances.
 *         Returns false (0) otherwise. 
 */
static bool checkPalindrom(bool ignoreSpace, bool ignoreCase, char *string)
{
    //editedString contains the final string that should be tested and will
    //be edited according to the flags ignoreSpace and ignoreCase
    char* editedString = strdup(string);
    int i=0, j=0;

    if(ignoreSpace)
    {
        for(i = 0; i< strlen(string); i++)
        {
            //if the character is not a space then keep it
            if(!isspace(string[i]))
            {
                editedString[j] = string[i];
                j++;
            }           
        }
        // add '\0' at the end of the string to mark its end
        editedString[j] = '\0';
    }

    
    if(ignoreCase)
    {
        for(i = 0; i< strlen(editedString); i++)
        {
            // parse every letter to uppercase
            editedString[i] = toupper(editedString[i]);
            j++;          
        }
        
    }

    int beginning = 0;
    // strlen already ignores the first occurence of '\0'
    int end = strlen(editedString)-1;

    while(beginning<=end){
        if(editedString[beginning]!=editedString[end])
        {
            free(editedString);
            return false;
        }
        beginning++;
        end--;
    }
    free(editedString);
    return true;
}

/**
 * @brief This function reads from the File "in" until the EOF and calls checkPalindrom for each string and writes the result to out.
 * 
 * @details The function reads from the File "in" possibly infinite long strings and calls the static function
 *          bool checkPalindrom for each string with the flags i and s. This Function doesn't check if "in" and
 *          "out" are valid arguments, this should have already happened in the main() function.
 *           global variable: progName - is the program name as written in argv[0]
 * 
 * @param in: is the File that the information should be read from
 *        out: is the File that the result should be written into 
 *        s: if true (1) then whitespaces will be ignored for the palindrom check
 *        i: if true (1) then cases will be ignored for the palindrom check 
 * 
 * @return Returns EXIT_FAILURE if error occured while reading line or writing result.
 *         Returns EXIT_SUCCESS otherwise. 
 */
static int step(FILE *in, FILE *out, bool s, bool i)
{
    //string saves the result
    char *string = NULL;
    int failed = 0;

    //these are needed for memory allocation
    size_t line = 0;    
    ssize_t result;

    //getLine reads possibly infinite long strings and reallocates the needed memory if necessary.
    while((result = getline(&string, &line, in)) != EOF)
    {   
        if(result == -1)
        {
            //error occured
            fprintf(stderr, "[%s] unable to read line from input-File: %s\n",progName,strerror(errno));
            failed = 1;
            break;
        }

        //remove the \n at the end of the read string from getLine
         //strcspn finds the first index of "\n"
        string[strcspn(string,"\n")]='\0';

        //call checkPalindrom
        if(checkPalindrom(s,i,string)==1)
        {
            string = strcat(string," is a palindrom\n");
        }else{
            string = strcat(string," is not a palindrom\n");
        }

        if(fputs(string,out)==EOF)
        {
            //error occured
            fprintf(stderr, "[%s] unable to write to output-File: %s\n",progName,strerror(errno));
            failed = 1;
            break;
        }
    }

    //free string since getLine calls malloc/realloc
    free(string);
    if(failed==1)
        return EXIT_FAILURE;
        
    return EXIT_SUCCESS;
}

/**
 * @brief The program starts here. The function checks if the program was started with a valid synopsis and
 *        if the file paths are found/valid. If true then the palindromcheck will be made for all files.
 * 
 * @details It is not necessary to pass any arguments or options but following rules need to be noted:
 *          i (ignore case), s (ignore whitespace) and o can only be passed once at most.
 *          after "o" a filepath it necessary
 *          there can be as many files to read from as wished but they should be passed as positional arguments
 *          if no "o" is declares stdout will be used and if no fils to read from are declard stdin will be used.
 *          calls int step() for each file and checks each line for a palindrom.
 * 
 * @param argc: contains the amount of arguments
 *        argv[]: contains all the options and arguments. In this case possibly:
 *                 s, i, o [file], [file...]
 * 
 * @return Returns EXIT_FAILURE if an error occured somewhere during the process.
 *         Returns EXIT_SUCCESS otherwise. 
 */
int main(int argc, char *argv[])
{
    //c is used for getopt
    //j is used for a for-loop
    //scount, icount, ocount count the occurences of i,o and s in the arguments
    //check saves the return value from int step(...)
    int c, j, check = 0, scount = 0, icount = 0, ocount = 0;

    //if true then whitespaces should be ignored
    bool s = false;
    
    //if true cases should be ignored
    bool i = false;
    //saves the path to the out file if given
    char *o_arg = NULL; 
    FILE *in, *out;
    //saves the program name
    progName = argv[0];

    //checks all options and arguments
    //case ?: invalid option
    //default: assert(0) since it should never be reached
    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch(c)
        {
            case 's': s = true; scount++;
                break;
            case 'i': i = true; icount++;
                break;
            case 'o': o_arg = optarg; ocount++;
                break;
            case '?':
            if(optopt =='o')
            {
                fprintf(stderr, "[%s] File-path needed after -o [file]: Please consider following usage:\n",argv[0]);
                
            }else{
                fprintf(stderr, "[%s] Invalid option: %c. Please consider following usage:\n",argv[0],optopt);
            }
            usage();
            break;
            default:
            assert(0);
            break;
            
        }
    }

    //if arguments were passed too often call usage();
    if(icount>1 || scount>1 || ocount>1){
        fprintf(stderr, "[%s] too many arguments: Consider following usage:\n",argv[0]);
        usage();
    }

    //check if output file is given.
    //if not then use stdout otherwise try to open the file with fopen.
    //if file not available then create one with the given name.
    if(o_arg == NULL){
        out = stdout;
        o_arg = "stdout";   
    }
    else if(o_arg != NULL){
        if((out = fopen(o_arg,"ab+"))==NULL){
            fprintf(stderr, "[%s] fopen failed: %s (%s)\n",argv[0],strerror(errno),o_arg);
            return EXIT_FAILURE;
        }
    }

    //if no file to read from give, read from stdin
    //otherwise read from all files given.
    if(argv[optind] == NULL)
    {
        in = stdin;
        //since in is now the "file" check can be called with stdin
        check = step(in, out, s, i);

        //if error occured during check write error to stderr and return with EXIT_FAILURE.
        if(check == EXIT_FAILURE){
            if((fclose(in))!=0){
                fprintf(stderr, "[%s] unable to close input-File: %s (stdin)\n",progName,strerror(errno));
            }

            if((fclose(out))!=0){
                fprintf(stderr, "[%s] unable to close output-File: %s (%s)\n",progName,strerror(errno),o_arg);
            }
        }
    } else{
        for(j=0;argv[optind+j]!=NULL;j++){
            if((in = fopen(argv[optind+j],"r"))==NULL){
                fprintf(stderr, "[%s] fopen failed: %s (%s)\n",progName,strerror(errno),argv[optind+j]);
                break;

            }else{
                //check every line of file for palindrom
                check = step(in,out,s,i);

                //if error occured during check write error to stderr and return with EXIT_FAILURE.
                if(check == EXIT_FAILURE){
                    break;
                }
            }
        }
    }

    //close the FILE-streams
    if(in != NULL)
    {
        if((fclose(in))!=0){
            fprintf(stderr, "[%s] unable to close input-File: %s\n",progName,strerror(errno));
        }
    }
    

    if(out != NULL)
    {
        if((fclose(out))!=0){
            fprintf(stderr, "[%s] unable to close output-File: %s (%s)\n",progName,strerror(errno),o_arg);
        }
    }

    return EXIT_SUCCESS;
}