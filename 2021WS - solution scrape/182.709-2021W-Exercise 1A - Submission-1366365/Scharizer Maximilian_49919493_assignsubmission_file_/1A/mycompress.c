/**
 * @file mycompress.c
 * @author Maximilian Scharizer
 * @date 08.11.2021
 * @brief Implementation of mycompress
 **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

/**
 * usage function
 * @brief This function prints a usage explanation and exits.
 * @details Exits the process with EXIT_FAILURE.
 * @param prog_name The name of the program.
 **/
void usage(char *name)
{
    fprintf(stderr, "%s [-o output] [file...]", name);
    exit(EXIT_FAILURE);
}
/**
 * compression function
 * @brief This function compresses a given input.
 * @details Function reads from input and compresses characters accordingly.
 * The result is written to output. Two Integer pointers are used, to track the
 * compression rate.
 * @param input The pointer to the input file.
 * @param output The pointer to the output file.
 * @param writtenChars The pointer to the Integer, which stores the value for the characters written during compression.
 * @param readChars The pointer to the Integer, which stores the value for the characters read during compression.
 * @return Function returns 0 if successful, else -1 is returned.
 **/
int mycompress(FILE *input, FILE *output, int *writtenChars, int *readChars)
{
    int counter = 0;
    int prev;
    while (true)
    {
        int current = fgetc(input);

        //If end of file is reached, exit loop
        if (feof(input))
        {
            break;
        }

        //Every iteration, exactly one char is read
        (*readChars)++; 

        //First occurence of char
        if (counter == 0)
        {
            counter++;
            prev = current;
            continue;
        }

        //If currently read char equals the previous char
        if (current == prev)
        {
            counter++;
            continue;
        }

        //If current char, does not equal previous char
        if (current != prev)
        {
            //write to output
            int written = fprintf(output, "%c%d", prev, counter);
            if(written==-1){
                return 1;
            }
            fflush(output);
            counter = 1;
            prev = current;
            (*writtenChars) += written;
            continue;
        }
    }
    int written = fprintf(output, "%c%d", prev, counter);
    if(written==-1){
        return 1;
    }
    fflush(output);
    (*writtenChars) += written;
    return 0;
}

/**
 * main function
 * @brief This function serves as the entrypoint.
 * @details Function parses the command line arguments and calls mycompress function
 * @param argc Integer representing number of command line arguments given by user.
 * @param argv Array containg the command line arguments given by user.
 * @return Function returns 0 if successful, else 1 is returned.
 **/
int main(int argc, char *argv[])
{
    bool output = 0;
    char *outputName = NULL;
    int c;
    char *program = argv[0];
    FILE *outputFile = NULL;
    int writtenChars = 0;
    int readChars = 0;
    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            if (outputName != NULL)
            {
                fprintf(stderr, "%s ERROR: flag -o can only appear once\n", program);
                usage(program);
            }
            outputName = optarg;
            output = 1;
            break;
        case '?':
            // illegal option
            usage(program);
            break;
        default:
            // unreachable code
            assert(0);
        }
    }
    if (output == 1)
    {
        if ((outputFile = fopen(outputName, "w")) == NULL)
        {
            fprintf(stderr, "%s ERROR opening output file: %s", program ,outputName);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        outputFile = stdout;
    }

    int numberOfInputFiles = argc - optind;

    //option - no input files
    if (numberOfInputFiles == 0)
    {
        if(mycompress(stdin, outputFile, &writtenChars, &readChars)!=0){
            fprintf(stderr, "%s ERROR compressing input to output file: %s", program ,outputName);
            fclose(outputFile);
            exit(EXIT_FAILURE);
        }
    }
    //option one or more input files given
    else
    {
        char *inputFiles[numberOfInputFiles];
        int index = 0;
        for (int i = optind; i < argc; i++)
        {
            inputFiles[index] = argv[i];
            index++;
        }

        for (int i = 0; i < numberOfInputFiles; i++)
        {
            FILE *inputFile = fopen(inputFiles[i], "r");
            if(inputFile==NULL){
                fprintf(stderr, "%s ERROR opening input file: %s", program ,inputFiles[i]);
                fclose(outputFile);
                exit(EXIT_FAILURE);
            }
            if(mycompress(inputFile, outputFile, &writtenChars, &readChars)!=0){
                fprintf(stderr, "%s ERROR compressing input file: %s to output file: %s", program ,inputFiles[i],outputName);
                fclose(outputFile);
                fclose(inputFile);
                exit(EXIT_FAILURE);
            }
            fclose(inputFile);
        }
    }
    //finally close output file
    if(outputFile != stdout){
        fclose(outputFile);
    }
    //calculate compression ratio
    double compression = (100.0 * writtenChars) / (100.0 * readChars)*100.0;
    fprintf(stderr, "\nRead: %d\nWritten: %d\nCompression Ratio: %f%%\n", readChars, writtenChars, compression);

    return EXIT_SUCCESS;
}
