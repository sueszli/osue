/**
 * @file mycompress.c
 * @author Moser Jakob, 12023979, <e12023979@student.tuwien.ac.at>
 * @brief a programm that uses Run Length Encoding to encode input via files and stdin and writes output to the console or a file
 * 
 * @date 14.11.2021
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

typedef struct
{
    unsigned int readLetters, writtenLetters;
} CharacterList; /**< struct the read and written letters are saved in >*/

char *program_name; /**< program name >*/

/**
 * mandatory usage function
 * @brief prints the usage of this module
 * @details global variables: pgm_name
 */
static void usage()
{
    printf("USAGE: ./%s [EDGE1...]\n\n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * compresses the input via RLE
 * @brief the algorithm that compresses an input file via a RLE encode implementation and writes it to the output file
 * @details first we check if the input file parameter that was passed is NULL, if it is, we know we need to read from stdin 
 * then we create some local variables that we need for the algorithm. 
 * Then we iterate over out input and get a single character out of it, this character is saved and in a different while we iterate over the following 
 * chars till we find a different one and count how many identical chars followed it. Then we print it to our outputfile and take the next char. 
 * This is done till we reach end of file (EOF) of if the input is stdin a linebreak. Then we close our inputfile and end the function
 * 
 * @param outputFile            output file where the result of the algorithm is written to
 * @param inputFile             input for the algorithm
 * @param characters            a CharacterList struct where the written and read letters are saved in for the statistics
 */
static void compressRLE(FILE *outputFile, FILE *inputFile, CharacterList *characters)
{
    int inputIsStdin = 0;
    if (inputFile == NULL)
    {
        inputIsStdin = 1;
        inputFile = stdin;
    }

    int charCount = 0;
    char lastChar = EOF;
    char currChar;
    lastChar = fgetc(inputFile);

    while (1)
    {
        charCount++;
        characters->readLetters++;
        while ((currChar = fgetc(inputFile)) == lastChar)
        {
            charCount++;
            characters->readLetters++;
        }
        fprintf(outputFile, "%c%d", lastChar, charCount);
        int nDigits = floor(log10(abs(charCount))) + 2;
        characters->writtenLetters += nDigits;
        lastChar = currChar;
        charCount = 0;
        if ((currChar == EOF) || ((inputIsStdin == 1) && (currChar == '\n')))
        {
            break;
        }
    }
    fprintf(outputFile, "\n");
    if (fclose(inputFile) < 0)
    {
        fprintf(stderr, "close failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return;
}

/**
 * main function
 * @brief main function of the mycompress program
 * @details First we read out the passed arguments and determine what out input and output are. 
 * Then we create a CharacterList and initialize the values with 0. If the o_arg is not NULL we try to open the passed 
 * passed outputfile and set it as out output. Then we come to the case of optind == argc which means that no input files 
 * we passed so we call our compressRLE function with NULL as our input. When the function is finished we print the statistics to stdout. 
 * If optind != argc we open the passed input files one by one and call out compressRLE function with them as our input. After everything is finished 
 * we print out the statistics and close out outputfile.
 * 
 * @param argc      argument count
 * @param argv      argument vector
 * @return int      return value
 */
int main(int argc, char *argv[])
{
    program_name = argv[0];
    char *o_arg = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "o:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            o_arg = optarg;
            break;
        case '?':
            usage();
            break;
        default:
            usage();
            break;
        }
    }

    CharacterList characters = {characters.readLetters = 0, characters.writtenLetters = 0};

    FILE *outputfile = stdout;
    FILE *inputfile = stdin;

    if (o_arg != NULL)
    {
        if ((outputfile = fopen(o_arg, "w")) == NULL)
        {
            fprintf(stderr, "can't open output, something failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        compressRLE(outputfile, NULL, &characters);
        fprintf(stderr, "\nRead: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n", characters.readLetters, characters.writtenLetters, (characters.writtenLetters * 100.0 / characters.readLetters));
        if (fclose(outputfile) < 0)
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        characters.readLetters = 0;
        characters.writtenLetters = 0;
        for (; optind < argc && *argv[optind] != '-'; optind++)
        {
            if ((inputfile = fopen(argv[optind], "r")) == NULL)
            {
                fprintf(stderr, "can't open input, something failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            compressRLE(outputfile, inputfile, &characters);
        }
        fprintf(stderr, "\nRead: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n", characters.readLetters, characters.writtenLetters, (characters.writtenLetters * 100.0 / characters.readLetters));

        if (fclose(outputfile) < 0)
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
