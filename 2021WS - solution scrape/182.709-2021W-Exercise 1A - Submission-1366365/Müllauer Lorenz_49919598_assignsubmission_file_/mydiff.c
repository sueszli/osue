/**
 * @file mydiff.c
 * @author Lorenz Müllauer (e12024710@tuwien.ac.at)
 * @brief Reads in two text files and compares them linewise, if they differ it outputs the difference in characters
 * @version 0.1
 * @date 2021-11-10
 * 
 * 
 * 
 */
#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
 static void printError(const char *errMessage);
 static char *progName;

/**
 * @brief Program start, intializes values and opens file streams and compares two text files and prints out their differences in characters
 * 
 * @details First the arguments of the function call are parsed and file streams are opened, then two buffers are created where the current line of each text file is saved
 * , the strings contained in the buffers are compared with one another and the number of differing characters is then printed. In case a file cannot be opened or options such as -o are missing
 * arguments proper error messages are printed out and the program exits with "EXIT_FAILURE". Upon success "EXIT_SUCCESS" is returned
 * 
 * @param argc Number of Arguments
 * @param argv Values of passed Arguments
 * @return int Returns EXIT_SUCCESS upon success otherwise EXIT_FAILURE is returned
 */
int main(int argc,char* argv[])
{
    progName = argv[0];

    char *filename;
    char *filename2;
    
    FILE *output;
    char *outputPath;
    int writeToOutFile = 0;
    int caseSensitive = 0;
    int opt;
    
    while((opt = getopt(argc, argv, ":o:i")) != -1)
    {
        switch(opt)
        {
            case 'o': outputPath = optarg; writeToOutFile = 1; break;
            case 'i': caseSensitive = 1;    break;
            case '?': printError("One or more invalid options");
            
        }
    }


    char* missingArgument = strstr(argv[optind -1], ".txt");
    if(missingArgument != NULL)
    {
        printError("Missing argument for Option -o");
    }
    
    filename = argv[optind];
    filename2 = argv[optind + 1];
    
    if(writeToOutFile == 1)
    {
        writeToOutFile = 1;
        output = fopen(outputPath, "w");
        if(output == NULL)
        {
            printError("Output Path not valid!");
        }
    }
    FILE *fp = fopen(filename, "r");
    FILE *fp2 = fopen(filename2,"r");


    int compare(char *st1, char *st2, int caseSensitive);
    
    if(fp == NULL)
    {
        printError("Could not open file fp");
    }
    if(fp2 == NULL)
    {
        printError("Could not open file fp2");
    }
    char *buffer;
    char *buffer2;
    int diffNum;
    
    size_t bufsize = 32;

    buffer = (char *) malloc(bufsize * sizeof(char));
    buffer2 = (char *) malloc(bufsize * sizeof(char));

    
    int currLine = 1;
    while(getline(&buffer,&bufsize,fp2) != EOF && getline(&buffer2,&bufsize,fp) != EOF ) 
    {
        /** remove \n from buffer */
        buffer[strcspn(buffer, "\n")] = 0;
        buffer2[strcspn(buffer2,"\n")] = 0;
        if(strnlen(buffer,60) < strnlen(buffer2,60 ))
        {
            diffNum = compare(buffer, buffer2, caseSensitive);
        }else{
            diffNum = compare(buffer2, buffer, caseSensitive);
        }
        if(diffNum != 0)
        {
            if(writeToOutFile == 1)
            {
                char outputstring[140];
                snprintf(outputstring, 100,"Line: %i, characters: %i \n", currLine, diffNum);
                fputs(outputstring,output);
            }else
            {
                printf("Line: %i, characters: %i \n", currLine, diffNum);
            }
        }
        currLine++;
    
    }

    /** free Memory and close file streams */
    free(buffer);
    free(buffer2);
    fclose(fp);
    fclose(fp2);

    if(writeToOutFile == 1)
    {
        fclose(output);
    }


    return EXIT_SUCCESS;
    
}
/**
 * @brief Prints error message, with program name and given error message and then terminating the program 
 * 
 * @param errMessage Error Message printed alongside with the program name and command format
 */
 static void printError(const char *errMessage)
    {
        fprintf(stderr,"%s\n Command Format: %s [-i] [-o outfile.out] file1 file2", errMessage, progName);
        exit(EXIT_FAILURE);
    }

/**
 * @brief Compares two strings and returns the number of differing characters
 * 
 * @param st1 First string of the two compared Strings (Always ensure that st1 is the shorter string of the two)
 * @param st2 Second string of the compared strings
 * @param z Flag (0 when string are compared without accounting for case-sensitivity) (1 strings are compared with respect to case-sensitivity)
 * @return int Contains the number of differing characters between the two strings passed
 */
 int compare(char *st1, char *st2, int z)
    {
        int count = 0;
        for(int i = 0; i < strlen(st1); i++)
        {
            char string1[2] = {st1[i], '\0'};
            char string2[2] = {st2[i], '\0'};
            if(strcmp(string1,string2) && (z == 0))
            {
                count++;
            }else if(strcasecmp(string1,string2))
            {
                count++;
            }
        }
        int r = count;
        return r;
                
    }