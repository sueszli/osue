#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>


/**
 * @file mygrep
 * @author Anton Martinovic 52004305
 * @date 10.11.2021
 * @brief This program checks if there is a string in one or more input files
 * Options can be given, whether the search should be case sensitive or insensitive
 * further the option can be given, wheter the output should be printed to stdout or written
 * to a file 
 */


static void mygrep(char keyword[], int lower, FILE *outputFile, FILE *inputFile);



/**
 * @brief The main function checks, if the input is correct, if so 
 * it calls the mygrep function, which does the search, if an output file is given
 * then it saves the filepath to o_arg
 * 
 * @param argc counter of arguments of the program 
 * @param argv values of the arguments
 * return EXIT_FAILURE if a failure occures
 * return EXIT_SUCCESS if everything went well
 */

int main(int argc, char *argv[])
{

    FILE *outputFile = NULL;
    char *toSearch;
    int opt_o = 0;
    // gibt an ob case_sensitive oder nicht
    int opt_i = 0;
    int option;
    while ((option = getopt(argc, argv, "io:")) != -1)
    {
        switch (option)
        {
        case 'i':
            opt_i++;
            if(opt_i > 1){
                fprintf(stderr, "[%s] Error: Option can be given only one time!\n SYNOPSIS mygrep [-i] [-o outfile] keyword [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
            }

            break;
        case 'o':
            
            opt_o++;
            if(opt_o > 1){
                fprintf(stderr, "[%s] Error: Option can be given only one time!\n SYNOPSIS mygrep [-i] [-o outfile] keyword [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            //Outputfile sofort als FILE type speichern
            outputFile = fopen(optarg, "w");
            break;

        default:
            break;
            //Nicht notwendig explizit zu schreiben, dass eine Option nicht gültig ist, wird von getopt schon gemacht
            //fprintf(stderr, "[%s] Error: This is not a valid option!", argv[0]);
        }
    }
    //das prüft, wie viele zwingende operatoren vorkommen, +1 für ein verpflichtender +2 für 2 usw
    if ((optind + 1) > argc)
    {
        fprintf(stderr, "[%s] Error: At least a keyword is needed!\n SYNOPSIS mygrep [-i] [-o outfile] keyword [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    toSearch = argv[optind++];
    // optindex ist nun an der Stelle, wo die input files kommen müssen

    //jetzt kommen die input files

    FILE *inputFile;

    if (optind < argc)
    {
        while (optind < argc)
        {
            if ((inputFile = fopen(argv[optind], "r")) == NULL)
            {
                fprintf(stderr, "ERROR: fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            // in die Methode mit den einzelnen inputFiles und dem einen Outputfile
            mygrep(toSearch, opt_i, outputFile, inputFile);
            fclose(inputFile);
            optind++;
        }
    }
    //wenn es keine input files gibt, dann von stdin lesen
    else
    {
        //read from stdin
        mygrep(toSearch, opt_i, outputFile, stdin);
    }
    //The behaviour of fclose() is undefined if the stream parameter is an illegal pointer
    if (outputFile != NULL)
    {
        fclose(outputFile);
    }
    exit(EXIT_SUCCESS);
    return 0;
}


/**
 * @brief mygrep searches for the keyword in a line. 
 * 
 * @param keyWord word to find
 * @param lower declares whether the search is case sensitive or not
 * @param outputFile File, where the output should be written
 * @param inputFile  File to read
 */

static void mygrep(char keyWord[], int lower, FILE *outputFile, FILE *inputFile)
{

    char *chr = keyWord;

    //speichert eine Zeile zwischen
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), inputFile) != NULL)
    {
        // wenn outputfile nicht leer ist, dann in diesen das Ergebnis schreiben
        if (outputFile != NULL)
        {
            if (lower == 1)
            {
                // falls egal ob groß oder klein geschrieben, dann buffer kopieren, damit der richtige string geschrieben wird
                char *lineToCheck;
                lineToCheck = malloc(sizeof(buffer));
                //man muss den speicher davor allozieren, da der pointer keinen speicher hat, und wenn man versucht dadrauf etwas zu kopieren, kommt es zu einer Segmentation fault
                strcpy(lineToCheck, buffer);

                for (int i = 0; buffer[i]; i++)
                {
                    buffer[i] = tolower(buffer[i]);
                    
                }
                if (strstr(buffer, chr))
                {
                    if (fputs(lineToCheck, outputFile) == EOF)
                    {
                        fprintf(stderr, "ERROR: fputs failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    fflush(outputFile);

                }
                free(lineToCheck);
            }
            else
            {
                // in dem fall kann der buffer geschrieben werden, dass der string gleich bleibt
                if (strstr(buffer, chr))
                {
                    if (fputs(buffer, outputFile) == EOF)
                    {
                        fprintf(stderr, "ERROR: fputs failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    fflush(outputFile);
                }
            }
        }
        // Fall: outpufile ist leer, dh auf stdout ausgeben
        else
        {
            //es muss der ursrüngliche String zwischengespeichert werden, sodass dann nicht der kleingeschriebene String retouniert wird.
            if (lower == 1)
            {
                char *lineToCheck;
                lineToCheck = malloc(sizeof(buffer));
                //man muss den speicher davor allozieren, da der pointer keinen speicher hat, und wenn man versucht dadrauf etwas zu kopieren, kommt es zu einer Segmentation fault
                strcpy(lineToCheck, buffer);

                for (int i = 0; buffer[i]; i++)
                {
                    buffer[i] = tolower(buffer[i]);
                }
                if (strstr(buffer, chr))
                {
                    printf("%s", lineToCheck);
                }
                free(lineToCheck);
            }
            else
            {
                if (strstr(buffer, chr))
                {
                    printf("%s", buffer);
                }
            }
        }
    }

    if (ferror(inputFile))
    {
        fprintf(stderr, "ERROR: fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}