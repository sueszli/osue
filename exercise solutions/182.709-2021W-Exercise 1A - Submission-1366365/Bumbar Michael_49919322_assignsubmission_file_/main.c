/**
 * @file main.c
 * @author Michael Bumbar <e11775807@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Main program module.
 * 
 * This program reads line from an input stream, stdin if the program is not provided with an optional amount of input streams, and checks each input stream sequentially wether each line in this input 
 * is a palindrom. For each line of the input the program prints the line with the ending "is not a palindrom" if it is not a palindrom, otherwise it prints the line with the ending " is a palindrom".
 * The output is printed to stdout if the option o is not provided with an output stream as argument.
 * The program can optionally ignore whitespaces or capitalization in the input.
 **/


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

/** program name 
 * @brief PROG is the name of the program
 */
#define PROG "./ispalindrom"
/** confirmation message
 * @brief SUCCESS is the confirmation message the a line is a palindrom
 */
#define SUCCESS " is a palindrom\n"
/** error message
 * @brief NO_SUCCESS is the error message that a line is not a palindrom
 */
#define NO_SUCCESS " is not a palindrom\n"



/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details constants: PROG
 */
static void usage(void);

/**
 * Error handling inside the program.
 * @brief This function prints an error message to stderr and exits with EXIT_FAILURE.
 * @details This function prints a formatted string to stderr. The string consists of the name of the program in progName followed by the name of the function which called errorHandling and the cause of the
 * error at the end. The cause is usually the string format of errno. Afterwards this function exits with exit(EXIT_FAILURE).
 * constants: PROG
 * @param progName The name of the program in which the function is executed.
 * @param errorMessage The errorMessage the program is printing.
 * @param cause The cause for the error of the program.
 * @return Returns EXIT_FAILURE.
 */
static void errorHandling(char* progName,char* errorMessage,char*cause);


/**
 * Removes the whitespaces from a string inplace
 * @brief This function receives a char* as parameter and removes inplace all whitespaces of the string.
 * @details This function receives a String from the caller and iterates over all characters in the string. Every iteration we increase the index i and j. If a Character is a whitespace which 
 * we determine by calling isspace we increase the index j. If a character is no whitespace we copy the character at Index j to the position i. This process is continued until i is larger than the
 * length of the String.
 * constants: PROG
 * @param dirtyString The name of the String.
 * @return Returns EXIT_SUCCESS.
 */
static void removeWhitespace(char* dirtyString);


/**
 * Removes the capitalization from a string inplace. by replacing all uppercase letters with theire lowercase form.
 * @brief This function receives a char* as parameter and removes its capitalization by replacing all uppercase letters with theire lowercase form.
 * @details This function receives a String from the caller and iterates over all characters in the string from the back to beginning. Each Character will be checked with isspace wether it is 
 * whitespace. The index of the first character that is not whitespace is returned to the caller.
 * constants: PROG
 * @param dirtyString The name of the String.
 * @return Returns index of the last non whitespace character.
 */
static void lowerAllCharacters(char* dirtyString);


/**
 * Returns the last character of a String.
 * @brief This function returns the last character of a String ignoring any whitespace at the end of the String.
 * @details This function prints a formatted string to stderr. The string consists of the name of the program in progName followed by the name of the function which called errorHandling and the cause of the
 * error at the end. The cause is usually the string format of errno. Afterwards this function exits with exit(EXIT_FAILURE).
 * constants: PROG
 * @param dirtyString The name of the String.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t lastCharacter(char* dirtyString);


/**
 * Checks wether a String is a palindrom.
 * @brief Receives a String and determines wether it is a palindrom.
 * @details This function starts by iterating over the String from both sides. If the Character at the front index matches with the character at the back index the function procedes until it reaches
 * the middle of the word. If the front index is larger than the back index the function terminates and returns EXIT_SUCCESS. If the characters do not match at any index the function exits with EXIT_FAILURE.
 * constants: PROG
 * @param candidate The input file from which the program reads.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t isPalindrom(char* candidate);

/**
 * Handles the lines of an input stream.
 * @brief Iterates over the lines of the input, removes whitespaces and capitalization from if the options were selected and prints wether they are a palindrom or not.
 * @details This function starts by iterating over the lines of the input and duplicating them into a new String. If the option -s was provided to the program the duplicate is given to removeWhitespace.
 * If the option -i was provided to the program the function the String is provided to lowerAllCharacters to remove its capitalization. Afterwards we use the function lastCharacter with the duplicate
 * to determine the index of its last non whitespace character and cut off the rest by replacing the next character with '\0'. Now the String is analysed with isPalindrom. If it returns 0 the line is
 * a palindrom and we print the success message. Otherwise we print the failure message. After the function finishes iterating over all lines we free the space and return.
 * constants: PROG, SUCCESS, NO_SUCCESS
 * @param in The input file from which the program reads.
 * @param out The output file to which the program writes.
 * @param s_setting A number representing wether the program was called with option -s.
 * @param i_setting A number representing wether the program was called with option -i.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t fileAna(FILE*in,FILE*out, int32_t s_setting, int32_t i_setting);


/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters, options and positional arguments. It is also responsible for opening input and output streams, closing streams and
 * calls the function needed to analyse a file.
 * @details This function calls getopt and parses the command line arguments. The options -i -s -o can be provided once each, -o with an optional argument. If any of them is provided more than once the
 * function calls errorHandling and exits. If o_arg is a not a valid address program will write to stdout. Otherwise out will receieve the file opened with o_arg as argument.
 * If optind equals args no positional arguments were provided and the program reads from stdin. In this case we call the function fileAna with the parameters out, in, s_cnt, i_cnt.
 * Otherwise we will loop through the positional arguments and use each one as an input file. These files will be sequentially opened with fopen and the address stored in in. Afterwards the function
 * fileAna with the parameters out, in, s_cnt, i_cnt is called for each of these files.
 * Afterwards the files are closed and the program exits with EXIT_SUCCESS.
 * constants: PROG
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv){
    int32_t s_cnt = 0;
    int32_t i_cnt = 0;
    int32_t o_cnt = 0;
    int32_t c = 0;
    FILE *in; 
    FILE *out;
    char* o_arg = NULL; 
    int32_t pos_arg = 0;
    while((c = getopt(argc,argv,"sio:"))!=-1){
        switch(c){
            case 's': s_cnt++;break;
            case 'i': i_cnt++;break;
            case 'o': o_cnt++; o_arg = optarg; break;
            case '?': usage();break;
            default: usage();
        }
    }
    if (i_cnt > 1 || o_cnt > 1 || s_cnt > 1 ){
        errorHandling(PROG, "getopt failed","an option is provided more than once");
    }
    if (o_arg == NULL){
        out = fdopen(STDOUT_FILENO,"w");
    } else {
        out = fopen(o_arg,"w");
    } 
    if ((argc - optind) == 0 ){
        in = fdopen(STDIN_FILENO,"r");
        if(in == NULL){
            errorHandling(PROG, "fdopen failed", strerror(errno));
        }
        if(fileAna(in,out,s_cnt,i_cnt) == 1){
            errorHandling(PROG, "fileAna failed", strerror(errno));
        }
        fclose(in);
    } else {
        pos_arg = optind;
        while(pos_arg != argc){
            in = fopen(argv[pos_arg],"r");
            if(in == NULL){
                errorHandling(PROG, "fopen failed", strerror(errno));
            }
            if(fileAna(in,out,s_cnt,i_cnt) == 1){
                errorHandling(PROG, "fileAna failed", strerror(errno));
            }
            if(fclose(in) == EOF){
                errorHandling(PROG, "fclose failed", strerror(errno));
            }
             if(fflush(out) == EOF){
                errorHandling(PROG, "fflush(out) failed", strerror(errno));
            }
            pos_arg++;
        }
    }
    fclose(out);
    return EXIT_SUCCESS;
}


static int32_t fileAna(FILE*in,FILE*out, int32_t s_setting, int32_t i_setting){
    char *line = NULL;
    size_t len = 0;
    while(getline(&line,&len, in) != -1){
        if(line == NULL){
            free(line);
            errorHandling(PROG,"getline failed",strerror(errno));
        }
        if(line[ strlen(line) - 1] == '\n') {	
			line[ strlen(line) - 1] = '\0';
        }
        char* tmp = strdup(line);
        if(tmp == NULL){
            errorHandling(PROG,"strdup failed", strerror(errno));
        }
         if(s_setting > 0){
            removeWhitespace(tmp);
        }
        if(i_setting > 0){
            lowerAllCharacters(tmp);
        }
		tmp[lastCharacter(tmp)] = '\0';
        if(isPalindrom(tmp) == 0){
            fprintf(out,"%s%s",line,SUCCESS);
        } else {
            fprintf(out,"%s%s",line,NO_SUCCESS);
        }
        fflush(out);
        free(tmp);
    }
    free(line);
    if(!feof(in)){
		errorHandling(PROG, "Failed while reading input stream",strerror(errno));
	}
    return EXIT_SUCCESS;
}


static int32_t isPalindrom(char* candidate){
    char* tmp = candidate;
    if(strcmp(tmp,"") == 0){
        return EXIT_SUCCESS;
    } else {
        int32_t last = strlen(candidate)-1;
        for(int32_t i = 0; i < last; i++,last--){
            if(tmp[i] != tmp[last]){
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }
}


static void usage(void) {
    fprintf(stderr,"Usage: %s [-s] [-i] [-o outfile] [file...]\n",PROG);
    exit(EXIT_FAILURE);
}


static void errorHandling(char* progName,char* errorMessage,char*cause){
   fprintf(stderr, "[%s] ERROR: %s: %s\n", progName,errorMessage, cause);
   exit(EXIT_FAILURE); 
}


static void removeWhitespace(char* dirtyString){
    char* d = dirtyString;
    int len = strlen(dirtyString);
    for(int i = 0,j = 0; i < len; i++,j++){
        while (isspace(d[j])) {
            j++;
        }
        dirtyString[i] = d[j];
    }
}


static void lowerAllCharacters(char* dirtyString){
   for(int i = 0; dirtyString[i]; i++) {
      dirtyString[i] = tolower(dirtyString[i]);
   }
   fflush(stdout);
}


static int32_t lastCharacter(char* dirtyString){
   int len = strlen(dirtyString);
   while(isspace(dirtyString[len])){
       len--;
   }
   return len+1;
}

