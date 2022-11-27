/**
 * @file ispalindrom.c
 * @author Jakob Kästner 01626126
 * @date 13.11.2021
 *
 * @brief Simple Program that reads input line by line and checks if it is a palindrom 
 **/

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int isPalindrom(char inString[], int sFlag, int iFlag);
void usage(void);

/**
 * @brief Main Method of the ispalindrom Program
 * @details takes text input from files (in given order) (default stdin). When option -s is given ignores whitespaces
 * when option -i is given ignores cases. when option -o and a file is given prints into that file (default stdout).
 * @param argc Argument count
 * @param argv Argument Array
 * @return returns EXIT_SUCCESS or EXIT_FAILIURE  
 */
int main(int argc, char *argv[])
{
    int opt;
    FILE *outFile = stdout;
    FILE *inFile = stdin;

    int iFlag = 0;
    int sFlag = 0;
    int oFlag = 0;

    while ((opt = getopt(argc, argv, "sio:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            if(oFlag){
                    usage();
            }
            outFile = fopen(optarg, "w");
            if(outFile == NULL){
                fprintf(stderr, "%s: File %s could not be opened.", argv[0], optarg);
                exit(EXIT_FAILURE);
            }
            oFlag = 1;
            break;
        case 'i':
        if(iFlag){
                    usage();
            }
            iFlag = 1;
            break;
        case 's':
        if(sFlag){
                    usage();
            }
            sFlag = 1;
            break;
        default:
            break;
        }
    }

    int filesCount = argc - optind;
    if (filesCount)
    {
        fclose(inFile);
    }
    
    char *line = NULL;
    size_t size;
    int i = optind;

    while( i < argc || filesCount == 0){
        if (filesCount)
        {
            inFile = fopen(argv[i], "r");
            if(inFile == NULL){
                fprintf(stderr, "%s: File %s could not be opened.", argv[0], optarg);
                exit(EXIT_FAILURE);
            }
        }
        while (getline(&line, &size, inFile) != -1){
            if (strlen(line) >= 2)
                {
                    if (line[strlen(line) - 2] == '\n' || line[strlen(line) - 2] == '\r') line[strlen(line) - 2] = '\0';
                }
                if (strlen(line) >= 1)
                {
                    if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r') line[strlen(line) - 1] = '\0';
                }

            int isP = isPalindrom(line, sFlag, iFlag);

            if (isP == 0){
                fprintf(outFile, "%s is not a palindrom!\n", line);
            }
            else {
                fprintf(outFile, "%s is a palindrom!\n", line);
            }
            fflush(outFile);

        }
        fclose(inFile);
        i++;
        if (filesCount == 0) break;
    }
    free(line);
    fclose(outFile);
    return EXIT_SUCCESS;
}

/**
 * @brief Prints out the synopsis of the program.
 */
void usage(void){
    printf("SYNOPSIS: ispalindrom [-s] [-i] [-o OUTFILE] [FILE...]\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief method that takes a string and checks if its a palindom.
 * @details ignores newline or carriage returns as last char.
 * @param inString input String
 * @param sFlag when 0 doesnt ignore spaces otherwise does
 * @param iFlag when 0 doesnt ignore cases otherwise does
 * @return returns 1 if the string is a palindrom. otherwise 0.
 */
int isPalindrom(char inString[], int sFlag, int iFlag)
{
    if (strlen(inString) <= 1) return 1;
    
    int stringPointL = 0;
    int stringPointR = strlen(inString) - 1;
    if (inString[stringPointR] == '\r' || inString[stringPointR] == '\n') 
    {
        stringPointR--;
    }
    while (stringPointL < stringPointR)
    {
        if (sFlag)
        {
            while (isspace(inString[stringPointL]))
            {
                stringPointL++;
                if (stringPointL >= stringPointR)
                {
                    break;
                }
            }
            while (isspace(inString[stringPointR]))
            {
                stringPointR--;
                if (stringPointL >= stringPointR)
                {
                    break;
                }
            }
        }

        char l = inString[stringPointL];
        char r = inString[stringPointR];

        if (iFlag)
        {
            l = tolower(l);
            r = tolower(r);
        }
        if (l != r)
        {
            return 0;
        }
        stringPointR--;
        stringPointL++;
    }
    return 1;
}