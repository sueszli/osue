/**
 * @file mygrep.c
 * @author 11809925 René Schmidt
 * @brief Checks if specified keyword is occuring in input which is either a file or a line from the command line.
 * @details The program checks if the keyword is occuring in the input by calling the method strstr, which returns
 * the first occurence of a string inside of another string. If NULL is returned it is certain that there is no
 * occurence. In case of an occurence the input will be written to an output file, which is either stdout or a
 * specified file. Otherwise it will not be written to the ouput. A flag can be specified to disable case
 * sensitivity.
 * @date 14.11.2021.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

char *myprog;


/**
 * @brief Prints information about the correct usage and ends the program.
 * @details The information is printed to stderr by using the fprintf method which uses the global variable myprog which contains the name of the program.
 * The program will then terminate with the exit code EXIT_FAILURE which implies that the program was not used correctly.
 * @returns EXIT_FAILURE
 */
void usage(void) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file ...]", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief Transforms all characters of a given string into lower case.
 * @details Transforms the characters of a string to lower case by iterating through the string using a for loop and calling
 * the method tolower on every character.
 * @param string - The string that needs to be transformed
 */
void toLower(char *string) {
    for(int i = 0; string[i]; i++){
        string[i] = tolower(string[i]);
     }
}


/**
 * @brief Checks if a keyword is specified inside of a line in an input file. It will iterate over the file as long as EOF is not reached.
 * In case of an occurence, this line is written to an output.
 * @details The method iterates over the input file by using the getline call inside of a while loop. getline will put a line from the inout file
 * into the variable line which is then compared to the keyword with the strstr method. If the keyword occurs in this line, the line will be written
 * to the output file. The variable iSet specifiefs if the comparison should be case sensitive or not. Once the end of the file is reached the loop
 * will terminate. Finally the feof method is called to check if the loop terminated correctly, getline returns -1 on EOF as well as on errors.
 * @param infile - The input file. It is either stdin or a specified input file.
 * @param outfile - The ouput file. It is either stdout or a specified output file.
 * @param keyword - The keyword which is used to check if it occurs inside of a line.
 * @param iSet - An integer which is used to check if case sensitivity is desired.
 */
void grep (FILE *infile, FILE *outfile, char* keyword, int iSet) {
    char *line = NULL;
    size_t capacity = 0;

     while (getline(&line, &capacity, infile) != -1) {
        if (iSet) {
            char *lineLowerCase = malloc(strlen(line) + 1);
            strcpy(lineLowerCase, line);
            toLower(lineLowerCase);
            if (strstr(lineLowerCase, keyword) != NULL) {
                fputs(line, outfile);
            }
            free(lineLowerCase);
        } else {
            if (strstr(line, keyword) != NULL) {
                fputs(line, outfile);
            }
        }  
    }
    free(line);
    
    if(!feof(infile)) {
        printf("error: An error occured while reading a line!");
        exit(EXIT_FAILURE);
    }
    fflush(outfile);
}

/**
 * @brief The main method of the program. Checks if the program is called correctly and opens all the files that are necessary.
 * It then calls the methods needed to check if a keyword is occuring inside of an input line.
 * @details It first creates the input and the output file pointers which are set do stdin and stdout. It then checks the flags
 * and positional arguments specifed by the user and if desired changes the input and/or output file to a different source.
 * In case the flag i is set the keyword will be set to lower case before calling the grep method. 
 * @param argc - the number of arguments of the program
 * @param argv - an array containing the arguments of the program
 */
int main(int argc, char **argv) {
    myprog = argv[0];
    FILE *infile = stdin;
    FILE *outfile = stdout;
    int c;
    int iSet = 0;
    int oSet = 0;
    while ( (c = getopt(argc, argv, "o:i")) != -1) {
        switch (c) {
        case 'i':
            iSet++;
            break;
        case 'o':
            oSet++;
            outfile = fopen(optarg, "w");
             if (outfile == NULL) {
                fprintf(stderr, "error: fopen failed: output file %s: %s\n", optarg, strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        case '?':
            usage();  
        default:
            break;
        }
    }

    if (iSet > 1) {
        usage();
    }


    if (oSet > 1) {
        usage();
    }

    if (argv[optind] == NULL) usage();

    char *keyword = argv[optind];

    if (iSet) {
        toLower(keyword);
    }

    if ( (argc - optind) > 1) { // at least one input file was specified
        for (int i = 1; i < argc - optind; i++) {
            infile = fopen(argv[optind+i], "r");
            if (infile == NULL) {
                fprintf(stderr, "fopen failed: input file %s: %s\n", argv[optind+i], strerror(errno));
                exit(EXIT_FAILURE);
            }
            grep(infile, outfile, keyword, iSet);
            fclose(infile);
        }
    } else {
         grep(infile, outfile, keyword, iSet);
         fclose(infile);
    }


    fclose(outfile);
    
    return EXIT_SUCCESS;
}
