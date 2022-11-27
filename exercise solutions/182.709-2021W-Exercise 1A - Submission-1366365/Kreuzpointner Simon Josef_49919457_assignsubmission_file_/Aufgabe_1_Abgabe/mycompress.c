/**
 * @file mycompress.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 22 October 2021
 *
 * @brief This is the main module.
 * 
 * @details This program implements compression of text in style of the Run Length Encoding. 
 * It can read in multiple given input files or stdin and write the compressed content to
 * either a given output file or to stdout.
 */

/**
 * @mainpage Assignment A - mycompress
 * 
 * This program is the realization of "Assignment A - mycompress" 
 * of the course Operating Systems.
 * 
 * @section synopsis Synopsis
 * mycompress [-o outfile] [file...]
 * 
 * The program compiles the total input of every given input file 
 * and compresses it. If no input files are given stdin will be used
 * and the user will be promted to input the desired string directly.
 * The stdin input can be finished of by pressing Ctrl-D right after a 
 * new line.
 * 
 * After the compression the file outputs the result either into the given
 * output file or - if none are passed in - writes the result directly to 
 * the terminal via stdout.
 * 
 * @section howitworks How it works
 * mycompress substitutes subsequent identical caracters by the number of
 * occurrences. Therefor implementing a type of Run Length Encoding.
 * 
 * It is important to note, that this does not necessarily always reduce the size of the input. 
 * A poorly choosen input may aswell use up double the amount of memory as before.
 * 
 * In addition this program only guarantees correct use when working with
 * ASCII characters. There is no explicit functionality implemented to handle UTF-8
 * characters.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "utility.h"
#include "debugUtility.h"
#include "fileActions.h"

/**
 * @brief Compresses the source string and saves it to the dest string.
 * 
 * @details This functions takes the source string and compresses it
 * based on the Run Length Encoding before writing it to the given dest 
 * string.
 * This function returns -1 if an error occurs else it returns 0.
 * 
 * @param source The source string that should be compressed.
 * @param dest The destination string where the result should be saved to.
 * @return Returns 0 upon success else -1.
 */
static int compress(char *source, char **dest);

/**
 * @brief Prints a summary after the compression
 * 
 * @details This function prints the number of read character and written
 * characters followed by the compression ratio.
 * 
 * @param read The number of characters read in from all input files given.
 * @param written The number of characters written while compressing. This is the number
 * of characters after the compression.
 * 
 * @return Returns 0 upon success and -1 on failure.
 */
static int printSummary(int read, int written);

/**
 * @brief Closes all given files and frees the allocated memory
 * of the given arrays.
 * 
 * @details This function calls closeFiles() and closeFile()
 * on every given file. After the closing the memory of the passed arrays are freed via free(3).
 * 
 * @param inputFiles The array of input files.
 * @param inputFileCount The number of files in the inputFiles array.
 * @param outputFile The outputFile.
 * @param inputFileNames The array of the names of each input file.
 * @param filesContent The string of the uncompressed contents of each input file.
 * @param compressedFilesContent The compressed string of filesContent.
 */
static void cleanup(FILE *inputFiles[], int inputFileCount, FILE *outputFile, char *inputFileNames[], char *filesContent, char *compressedFilesContent);

/**
 * @brief Closes all given files and frees the allocated memory, then calles exit(EXIT_FAILURE).
 * 
 * @details This function calls static cleanup()
 * right before exiting with the exit code EXIT_FAILURE.
 * 
 * @param inputFiles The array of input files.
 * @param inputFileCount The number of files in the inputFiles array.
 * @param outputFile The outputFile.
 * @param inputFileNames The array of the names of each input file.
 * @param filesContent The string of the uncompressed contents of each input file.
 * @param compressedFilesContent The compressed string of filesContent.
 */
static void cleanupExit(FILE *inputFiles[], int inputFileCount, FILE *outputFile, char *inputFileNames[], char *filesContent, char *compressedFilesContent);

/**
 * @brief the program name.
 * 
 * @details This is equal to the first argument of the argv array
 * passed to the main() function.
 */
char *programName;

/**
 * @brief This is the entry point of this program
 * 
 * @details This function handels the given input and sets up the 
 * resources needed to do its job. It calls every other 
 * function that is needed in sequence from here.
 * If the end of this function is reached, the program
 * exits with EXIT_SUCCESS.
 * An optional output file can be passed via the argument -o. If the given file
 * does not exist one will be created with the same name.
 * After the output file declaration multiple input files can optionally be stated.
 * If one of the input files does not exists the program terminates with the status
 * EXIT_FAILURE.
 * If too many output files are passed in the program 
 * prints the correct usage and terminates with the EXIT_FAILURE status.
 * 
 * The program exits immediately if argc is less then 1.
 * 
 * @param argc The argument counter holds the number of arguments in argv.
 * @param argv The argument vector is an array of strings. It holds the passed in parameters aswell as the program name.
 * @return returns EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
    // if the main method is called without any arguments
    // exit the program.
    if (argc <= 0)
    {
        return EXIT_FAILURE;
    }

    programName = argv[0];

    int c = 0; ///< The current option in the loop.
    int o = 0; ///< Keeps track of the number of option "-o" given.

    char *outputFileName = NULL;  ///< Name of the outputfile or "stdout" if no file is specified.
    char **inputFileNames = NULL; ///< Array of names of the input files or "stdin" at index 0 if no files are specified.

    FILE *outputFile = NULL;  ///< The output file stream or stdout if no file is specified.
    FILE **inputFiles = NULL; ///< Array of the input file streams or stdin at index 0 if no files are specified.

    int inputFileCount = 0; ///< The number of input files or 1, to account for stdin, if none are specified

    int usingStdin = 0;  ///< 1 if stdin  is used or 0 if input files are specified.
    int usingStdout = 0; ///< 1 if stdout is used or 0 if an output file is specified.

    char *filesContent = NULL;           ///< The compiled content of every file.
    char *compressedFilesContent = NULL; ///< The compressed filesContent.

    int uncompressedLength = 0; ///< The length of the string in its uncompressed state.
    int compressedLength = 0;   ///< The length of the string after it is compressed.

    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            outputFileName = optarg;
            o++;
            break;

        case '?':
            usage(1);
            break;

        default:
            debug("Reached the default branch in the getopt loop.");
            assert(0);
            break;
        }
    }

    inputFileCount = argc - optind;

    debug("Input files count: %i", inputFileCount);

    // allocating memory for the inputFileNames array
    inputFileNames = malloc(sizeof(char *) * (inputFileCount > 0 ? inputFileCount : 1));
    if (inputFileNames == NULL)
    {
        printError("malloc", "An error ocurred trying to allocate memory.");
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    // allocating memory for the inputFiles array
    // we have to use calloc here, because if an input file is not found
    // the program terminates and tries to close every file in this array
    // but this array might have 'garbage' in since ist is never initialized
    // to anything if using malloc
    inputFiles = calloc((inputFileCount > 0 ? inputFileCount : 1), sizeof(FILE *));
    if (inputFiles == NULL)
    {
        printError("malloc", "An error ocurred trying to allocate memory.");
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    // if more arguments are given
    if (inputFileCount > 0)
    {
        for (int i = 0; i < inputFileCount; i++)
        {
            if (access(argv[optind + i], F_OK) == -1)
            {
                debug("Could not find the file: '%s'", argv[optind + i]);
                usage(0);
                cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
            }
            else
            {
                inputFileNames[i] = argv[optind + i];
            }
        }
    }
    else
    {
        debug("User did not specify an input file, using stdin.");

        usingStdin = 1;
        inputFileCount = 1;
        inputFiles[0] = stdin;
        inputFileNames[0] = "stdin";
    }

    if (o > 1)
    {
        debug("User specified more than one output file");
        usage(0);
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }
    else if (o == 1)
    {
        debug("User specified an output file: '%s'", outputFileName);
    }
    else
    {
        debug("User did not specify an ouput file, using stdout.");

        usingStdout = 1;
        outputFile = stdout;
        outputFileName = "stdout";
    }

    // stdin does not have to be opened
    if (usingStdin == 0)
    {
        if (openFiles(&inputFiles, inputFileNames, "r", inputFileCount) < 0)
        {
            cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
        }
    }

    if (getFilesContent(inputFiles, inputFileCount, &filesContent) < 0)
    {
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    if (filesContent == NULL)
    {
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    uncompressedLength = strlen(filesContent);
    debug("All file contents:\n%s", filesContent);

    if (compress(filesContent, &compressedFilesContent) < 0)
    {
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    compressedLength = strlen(compressedFilesContent);
    debug("Compressed file contents:\n%s", compressedFilesContent);

    // stdout does not have to be opened
    if (usingStdout == 0)
    {
        if (openFile(&outputFile, outputFileName, "w") < 0)
        {
            cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
        }
    }

    if (writeToFile(outputFile, outputFileName, compressedFilesContent) < 0)
    {
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    if (printSummary(uncompressedLength, compressedLength) < 0)
    {
        cleanupExit(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    }

    cleanup(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);

    return EXIT_SUCCESS;
}

static int compress(char *source, char **dest)
{
    debug("Compressing.");

    int l = strlen(source); ///< The length of the source string.

    int charCounter = 1;          ///< Keeps track of the number of occurrences of chars that are equal to curChar.
    char charCounterAsString[12]; ///< A string version of charCounter. It has a length of 12 bcause the largest value an int can have, fits into 11 spaces.
    int charCounterLength = 0;    ///< The number of digits of charCounter.

    char curChar; ///< The current char.

    int pointerOffset = 0; ///< An offset to the dest pointer. Points to the first empty index in the string.
    int size = 0;          ///< Total memory size of dest.

    *dest = NULL; ///< The destination string. This is where the result will be saved.

    for (int i = 0; i < l - 1; i++)
    {
        curChar = source[i];

        if (curChar == source[i + 1])
        {
            charCounter++;
        }
        else
        {
            if (sprintf(charCounterAsString, "%d", charCounter) < 0)
            {
                printError("sprintf", "An error ocurred while trying to convert int to string.");
                return -1;
            }

            charCounterLength = numberOfDigits(charCounter);
            size += sizeof(char *) * (charCounterLength + 1);

            char temp[charCounterLength + 1];
            temp[0] = curChar;
            temp[1] = '\0';

            (void)strcat(temp, charCounterAsString);

            if (appendToString(dest, size, pointerOffset, temp) < 0)
            {
                printError("appendToString", "An error ocurred while trying to append a string.");
                return -1;
            }

            pointerOffset += charCounterLength + 1;
            charCounter = 1;
        }
    }

    // after the loop we have to treat the last character aswell
    if (curChar != source[l - 1])
    {
        curChar = source[l - 1];
        charCounter = 1;
    }

    if (sprintf(charCounterAsString, "%d", charCounter) < 0)
    {
        printError("sprintf", "An error ocurred while trying to convert int to string.");
        return -1;
    }

    charCounterLength = numberOfDigits(charCounter);
    size += sizeof(char *) * (charCounterLength + 1);

    char temp[charCounterLength + 1];
    temp[0] = curChar;
    temp[1] = '\0';

    (void)strcat(temp, charCounterAsString);

    if (appendToString(dest, size, pointerOffset, temp) < 0)
    {
        printError("appendToString", "An error ocurred while trying to append a string.");
        return -1;
    }

    pointerOffset += charCounterLength + 1;

    if (appendToString(dest, size + sizeof(char *), pointerOffset, "\0") < 0)
    {
        printError("appendToString", "An error ocurred while trying to append a string.");
        return -1;
    }

    return 0;
}

static int printSummary(int read, int written)
{
    double compressionRatio = (double)written / (double)read; ///< Ratio between the length of the uncompressed and compressed string.

    if (printVerticalSpacing(3, stdout) != 0)
    {
        return -1;
    }

    (void)fprintf(
        stderr,
        "%-20s%d characters\n%-20s%d characters\n%-20s%.1f%%\n",
        "Read:",
        read,
        "Written:",
        written,
        "Compression ratio:",
        compressionRatio * 100.0);

    return 0;
}

static void cleanup(FILE *inputFiles[], int inputFileCount, FILE *outputFile, char *inputFileNames[], char *filesContent, char *compressedFilesContent)
{
    // we already want to exit the program
    // no need to check for errors.
    (void)closeFiles(inputFiles, inputFileCount);
    (void)closeFile(outputFile);

    free(inputFiles);
    free(inputFileNames);
    free(filesContent);
    free(compressedFilesContent);
}

static void cleanupExit(FILE *inputFiles[], int inputFileCount, FILE *outputFile, char *inputFileNames[], char *filesContent, char *compressedFilesContent)
{
    cleanup(inputFiles, inputFileCount, outputFile, inputFileNames, filesContent, compressedFilesContent);
    exit(EXIT_FAILURE);
}