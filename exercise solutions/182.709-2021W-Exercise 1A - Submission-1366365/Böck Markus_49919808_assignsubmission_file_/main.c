/**
 * myexpand
 * 
 * @author Markus Böck, 12020632
 * @date 2021.11.02
 * @brief reimplmenetation of the unix 'expand' utility
 */

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/**
 * @brief points to argv[0], the executable name of the program
 */
static char* executableName = NULL;

#ifdef __GNUC__
#define FORMAT_FUNC __attribute__((format(printf, 1, 2)))
#else
#define FORMAT_FUNC
#endif

/**
 * @brief utility function for outputting errors
 * 
 * This function is used as a utility throughout the program to ease outputting errors.
 * All output is done to stderr with the value of the global 'executableName' preprended to the output.
 * It's signature is identical to printf, and its format strings + varargs allow the same values and options as printf.
 * @param format Format string as defined by printf & friends
 */
FORMAT_FUNC static void error(const char* format,...) 
{
    (void)fprintf(stderr, "%s: ", executableName);
    va_list list;
    va_start(list, format);
    (void)vfprintf(stderr, format, list);
    va_end(list);
    fprintf(stderr, "\n");
}

/**
 * @brief outputs the tools usage to stdout
 */
static void printUsage(void)
{
    printf("SYNOPSIS:\n"
           "\t%s [-t tabstop] [-o outfile] [file...]\n", executableName);
}

/**
 * @brief Enum used to indicate success of the 'parseCommandLineArguments' function
 */
typedef enum CLIParse
{
    CLIParse_Success,
    CLIParse_Failure,
} CLIParse;

/**
 * @brief Struct used to contain successfully parsed command line arguments in 'parseCommandLineArguments'
 */
typedef struct Parameters
{
    unsigned long tabStop;
    bool tabStopSet;
    FILE* outputFile;
    bool outputFileSet;
    int endOfOptions;
} Parameters;

/**
 * @brief parses the command line arguments given in argc and argv into *parameters
 * @param argc argc as is given by main
 * @param argv argv as is given by main
 * @param parameters Pointer to a Parameter struct which will contain the parsed data. Mustn't be NULL
 * @return CLIParse_Success on success, CLIParse_Failure otherwise
 * 
 * This function is responsible for parsing command line arguments as well as validating their format and arguments.
 * Any parsed data is put into the given Parameter struct. Additionally, the fields 'tabStopSet' and 'outputFileSet'
 * will be set to true if 'tabStop' or 'outputFile' have been modified. 'endOfOptions' gets set to the index of the
 * first non option argument. 
 * 
 * Callers are responsible for closing 'outputFile'
 */
static CLIParse parseCommandLineArguments(int argc, char** argv, Parameters* parameters)
{
    char c;
    while ((c = getopt(argc, argv,"t:o:")) != -1) 
    {
        switch (c)
        {
            case '?':
                // getopt should have already printed an error 
                // message. We just need to return now
                printUsage();
                return EXIT_FAILURE;
            case 't':
            {
                // We excplicitly allow '-t' to be specified multiple 
                // times
                errno = 0;
                char* endPtr = NULL;
                parameters->tabStop = strtoul(optarg, &endPtr, 10);
                if (errno)
                {
                    error("Invalid value %s for '-t': %s\n", optarg, strerror(errno));
                    return CLIParse_Failure;
                }
                if (endPtr != optarg + strlen(optarg))
                {
                    error("Invalid value %s for '-t'. Starting at: %s\n", optarg, endPtr);
                    return CLIParse_Failure;
                }
                parameters->tabStopSet = true;
                break;
            }    
            case 'o':
            {
                if (parameters->outputFileSet)
                {
                    error("Output file can't be set more than once");
                    return CLIParse_Failure;
                }
                parameters->outputFile = fopen(optarg, "w");
                if (!parameters->outputFile) 
                {
                    error("Failed to open %s: %s", optarg, strerror(errno));
                    return CLIParse_Failure;
                }
                parameters->outputFileSet = true;
                break;
            }    
            default:
                assert(0);
        }
    }
    parameters->endOfOptions = optind;
    return CLIParse_Success;
}

/**
 * @brief Enum used to indicate success or failure of 'doExpansion'
 */
typedef enum Expansion
{
    Expansion_Success,
    Expansion_Failure
} Expansion;

/**
 * @brief Reads 'inputFile' line by line, expands tabs to 'tabStop' spaces in the line and writes it to 'outputFile'
 * @param inputFile File to read from
 * @param outputFile Outputfile
 * @param tabStop Size of a single tab at max
 * @return Expansion_Success on success, Expansion_Failure otherwise
 * 
 * This function implements the actual expansion algorithm of the program. It reads in a line of 'inputFile' in at a 
 * time, expands it, and writes the lines to 'outputFile'. Expansion is done by replacing any occurences of '\t' and 
 * replacing it with as many spaces as are needed to align the next character to a multiple of 'tabStop'
 */
static Expansion doExpansion(FILE* inputFile, FILE* outputFile, unsigned long tabStop)
{
    char* lineBuffer = NULL;
    size_t lineBufferSize = 0;
    size_t lineLength;
    while ((lineLength = getline(&lineBuffer, &lineBufferSize, inputFile)) != -1)
    {
        for (size_t i = 0; i < lineLength;)
        {
            if (lineBuffer[i] != '\t')
            {
                i++;
                continue;
            }
            
            size_t nextPos = tabStop * ((i / tabStop) + 1);
            size_t extraSpaceNeeded = nextPos - i - 1;
            // Reallocate our buffer in case we do not have the capacity needed
            if (lineLength + extraSpaceNeeded > lineBufferSize)
            {
                lineBufferSize = sizeof(char) * (lineLength + extraSpaceNeeded);
                char* newBuffer = realloc(lineBuffer, lineBufferSize);
                if (!newBuffer)
                {
                    free(lineBuffer);
                    error("Out of memory");
                    return Expansion_Failure;
                }
                lineBuffer = newBuffer;
            }
            
            // move all characters behind the '\t' by extraSpaceNeeded to the right. The gap + '\t' will be filled with 
            // spaces by the subsequent memset. memmove has to be used instead of the more common memcpy due to 
            // overlapping ranges.
            
            // We intentionally don't care about the null terminator. We have and use the actual line size everywhere
            memmove(lineBuffer + i + extraSpaceNeeded + 1, lineBuffer + i + 1,
                                                            lineLength - i - 1);
            memset(lineBuffer + i, ' ', extraSpaceNeeded + 1);
            lineLength += extraSpaceNeeded;
            i += extraSpaceNeeded + 1;
        }
        if (fwrite(lineBuffer, sizeof(char), lineLength, outputFile) != lineLength * sizeof(char))
        {
            error("Failed to write to output file");
            free(lineBuffer);
            return Expansion_Failure;
        }
    }
    free(lineBuffer);
    return Expansion_Success;
}

/**
 * @brief main function of myexpand
 * @param argc argument count
 * @param argv arguments
 * @return exit code
 * 
 * Sets 'executableName' to argv[0] at the very beginning. 
 * Orchestrates command line parsing and expansion using the option given.
 */
int main(int argc,char** argv)
{
    executableName = argv[0];
    int exitStatus = EXIT_SUCCESS;
    Parameters parameters;
    memset(&parameters, 0, sizeof(parameters));
    if (parseCommandLineArguments(argc, argv, &parameters) == CLIParse_Failure)
    {
        exitStatus = EXIT_FAILURE;
        goto exit;
    }
    if (!parameters.tabStopSet)
    {
        parameters.tabStop = 8;
    }
    if (!parameters.outputFileSet)
    {
        parameters.outputFile = stdout;
    }
    
    if (parameters.endOfOptions == argc)
    {
        if (doExpansion(stdin, parameters.outputFile,parameters.tabStop) == Expansion_Failure)
        {
            exitStatus = EXIT_FAILURE;
            goto exit;
        }
    }
    else
    {
        for (int i = parameters.endOfOptions; i < argc; i++)
        {
            FILE* inputFile = fopen(argv[i], "r");
            if (!inputFile)
            {
                error("Failed to open input file %s: %s", argv[i], strerror(errno));
                exitStatus = EXIT_FAILURE;
                goto exit;
            }
            Expansion result = doExpansion(inputFile, parameters.outputFile, parameters.tabStop);
            (void)fclose(inputFile);
            if (result == Expansion_Failure)
            {
                exitStatus = EXIT_FAILURE;
                goto exit;
            }
        }
    }
    
exit:
    if (parameters.outputFile)
    {
        (void)fclose(parameters.outputFile);
    }
    return exitStatus;
}
