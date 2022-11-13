#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

char *checkPalindroms(char input[], short ignoreCaseSensitivity, short ignoreSpaces)
{
    char copy[strlen(input)];
    strcpy(copy, input);

    if (ignoreSpaces == 1)
    {
        int j = 0;
        int i = 0;
        while (input[i])
        {
            if (input[i] != ' ')
            {
                copy[j++] = input[i];
            }
            i++;
        }
        copy[j] = '\0';
    }

    if (ignoreCaseSensitivity == 1)
    {
        for (int i = 0; i < strlen(copy); i++)
        {
            copy[i] = tolower(copy[i]);
        }
    }
    
    if (input[strlen(input) - 1] == '\r') {
        input[strlen(input) - 1] = '\0';
    }
    
    int start = 0;
    int end = strlen(copy) - 1;
    
    if (copy[end] == '\r'){
      copy[end] = '\0';
      --end;
    }
    
    printf("Computing %s\n", input);
    
    while (end > 1)
    {
        if (copy[start] != copy[end])
        {
            return strcat(input, " is not a palindrom");
        }
        start++;
        end--;
    }

    return strcat(input, " is a palindrom");
}

int main(int argc, char **argv)
{
    char *outputFileName = malloc(256 * sizeof(char));

    short useFileOutput = 0; // useFileOutput equals 1 if the -o flag is used. When the -o flag is not used the output is written to StdOut instead.
    short useFileInput = 0; // useFileInput equals 1 if there is an input specified via the commandline-arguments and 0 if the console should be used as input
    short ignoreCaseSensitivity = 0; // 1 if case is ignored, 0 otherwise
    short ignoreSpaces = 0; // 1 if whitespace is ignored, 0 otherwise

    int c = 0;

    while ((c = getopt(argc, argv, "s;i;o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            strcpy(outputFileName, optarg);
            useFileOutput = 1;
            break;
        case 'i':
            ignoreCaseSensitivity = 1;
            break;
        case 's':
            ignoreSpaces = 1;
            break;
        case '?':
            if (optopt == 'o'){
                fprintf(stderr, "[%s] Option -o requires an argument!", argv[0]);
                exit(EXIT_FAILURE);
            }
        default:
            break;
        }
    }
    
    if (useFileInput == 1 && outputFileName == NULL)
    {
        fprintf(stderr, "Error at %s: No output file name specified.", argv[0]);
        exit(EXIT_FAILURE);
    }

    // iterate over input files
    if (optind < argc)
    {
        useFileInput = 1;
        for (int index = optind; index < argc; index++)
        {
            FILE *inputfile;
            if ((inputfile = fopen(argv[index], "r")) == NULL)
            {
                printf("Error while opening input file.");
                exit(EXIT_FAILURE);
            }
            char currentLine[256];

            FILE *outputFile;
            if (useFileInput == 1)
            {
                if ((outputFile = fopen(outputFileName, "w")) == NULL)
                {
                    printf("Error while creating output file.");
                    exit(EXIT_FAILURE);
                }
            }

            while (fgets(currentLine, sizeof(currentLine), inputfile) != NULL)
            {
                if (currentLine[strlen(currentLine) - 1] == '\n')
                {
                    currentLine[strlen(currentLine) - 1] = '\0';
                }
                char *palindrome = checkPalindroms(currentLine, ignoreCaseSensitivity, ignoreSpaces);

                if (useFileOutput == 1)
                {
                    fputs(strcat(palindrome, "\n"), outputFile);
                }
                else
                {
                    printf(palindrome);
                    printf("\n");
                }
            }
        }
    }
    else
    {
        while (1)
        {
            char line[BUFSIZ];
            while (fgets(line, BUFSIZ, stdin) != NULL)
            {
                line[strcspn(line, "\n")] = 0;
                printf(checkPalindroms(line, ignoreCaseSensitivity, ignoreSpaces));
                printf("\n");
            }
        }
    }
    free(outputFileName);
    exit(EXIT_SUCCESS);
}