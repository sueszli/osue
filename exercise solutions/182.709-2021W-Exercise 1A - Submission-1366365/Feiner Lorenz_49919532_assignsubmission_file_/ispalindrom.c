/**
 * @file ispalindrom.c
 * @author Lorenz Feiner, 11807867
 * @date 14.11.2021
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



char *myprog;
int whitespacesensitive = 1;        //1 if whitespaces are relevant for palindromcheck
int casesensitive = 1;              //1 if lower/uppercase is relevant for palindromcheck
char *outfilePath = NULL;
FILE *outfile = NULL;

/**
 * Mandatory usage function.
 * @brief This function prints helpful usage information.
 * @details global variables: myprog
 */
static void usage(void) {
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Palindromcheck
 * @brief This function checks if the given line is a palindrom or not.
 * @param line The line on which the palindromcheck is performed.
 * @return Returns 1 if line is a palindrom, else returns 0.
 */
static int isPalindrom(char *line) {
    size_t size = strlen(line);
    if(size == 0) { return 0; }

    for(int i = 0; i < size/2; i++) {

        if(line[i] == line[size-i-1]) {
            continue;
        } else {
            return 0;
        }
    }
    return 1;
    
}

/**
 * Prints given lines
 * @brief Prints if given line is a palindrom.
 * @details Performs palindromcheck on line and prints with help of originalline
 * @param originalline The original line which is to be printed
 * @param line The possibly changed line on which the palindromcheck is performed
 */
static void linePrint(char *originalline, char *line) {
    int palindrom = isPalindrom(line);
    if(outfilePath == NULL) {
        printf("%s\n", originalline);
        if(palindrom == 0) {
            printf("%s is not a palindrom\n", originalline);
        } else if(palindrom == 1) {
            printf("%s is a palindrom\n", originalline);
        } else {
            fprintf(stderr, "Something went wrong in palindromcheck in %s\n", myprog);
        }
        return;
    }

    if(palindrom == 0) {
        fprintf(outfile, "%s is not a palindrom\n", originalline);
    } else if(palindrom == 1) {
        fprintf(outfile, "%s is a palindrom\n", originalline);
    }

    

}

/**
 * Adjusts line
 * @brief Changes every uppercase letter in line to lower case.
 * @param line The line in which letters should be lowercase.
 * @return Returns a new char * with just lowercase letters.
 */
static char *allLowerCase(char *line) {
    size_t size = strlen(line);

    for(int i = 0; i < size; i++) {
        if(line[i] == ' ') { continue; }
        line[i] = tolower(line[i]);
    }
    return line;
}

/**
 * Adjusts line
 * @brief Deletes all whitespaces in line.
 * @param line The line in which whitespaces will be deleted.
 * @return Returns a new char * without whitespaces.
 */
static char *deleteWhitespace(char *line) {
    size_t size = strlen(line);
    char *newLine = malloc(size * sizeof(char));
    int counter = 0;

    for(int i = 0; i < size; i++) {
        if(line[i] == ' ') { continue; }
        newLine[counter] = line[i];
        counter++;
    }

    return newLine;
}

/**
 * Performs palindromcheck on line
 * @brief Checks if line is palindrom and prints accordingly
 * @param line The line which will be checked for palindrom
 */
static void computeLine(char *line) {
    char *originalLine = malloc(strlen(line) * sizeof(char));
    strcpy(originalLine, line);

    if(whitespacesensitive == 0) {
        line = deleteWhitespace(line);
    }
    if(casesensitive == 0) {
        allLowerCase(line);
    }

    linePrint(originalLine, line);
}

/**
 * Performs palindromcheck on inputfile.
 * @brief Takes every line in inputfile and checks for palindrom.
 * @details For each line, prints if it is a palindrom.
 * @param input The file on which the palindromcheck is performed.
 */
static void computeFile(FILE *input) {

    if(input == NULL) {
        fprintf(stderr, "computeFile got corrupted input in %s\n", myprog);
        exit(EXIT_FAILURE);
    }
    char *line = NULL;          //set to NULL so getline allocates memory automatically
    size_t size = 0;

    while(getline(&line, &size, input) != -1) {
        if(line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';
        }
        if(strlen(line) == 0) {
            continue;
        }
        computeLine(line);
    }
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care of parameters and acts accordingly.
 * It checks if every line of the specified input is a palindrom and prints its result. 
 * @param argc The number of arguments given on start of this program.
 * @param argv The arguments given on start of this program.
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char *argv[]) {
    //get program name for output
    myprog = argv[0];

    int c;
    while ((c = getopt(argc, argv, "sio:")) != -1) {    //get arguments and save position of first input file (if existing) in argv
        switch (c) {
            case 's': whitespacesensitive = 0;
                break;
            case 'i': casesensitive = 0;
                break;
            case 'o': outfilePath = argv[optind-1];
                break;
            case '?': usage();
                break;
            default: continue;
                break;
        }
    }

    if(outfilePath != NULL) {                       //check if output file is specified and open file for writing
        outfile = fopen(outfilePath, "w");

        if(outfile == NULL) {
            fprintf(stderr, "Failed to open output file in %s\n", myprog);
            exit(EXIT_FAILURE);
        }
    }

    if(optind >= argc) {                            //check if input file is specified. If not, read from stdin.
        computeFile(stdin);
    } else {
        for(int i = optind; i < argc; i++) {

            FILE *inputFile = fopen(argv[i], "r");

            if(inputFile == NULL) {
                fprintf(stderr, "Reading input file failed in %s", myprog);
                exit(EXIT_FAILURE);
            }

            computeFile(inputFile);
            fclose(inputFile);

        }
    }

    fclose(outfile);

    exit(EXIT_SUCCESS);

}