/**
 * @file mycompress.c
 * @author Manuel Kisser (12024009)
 * @brief mycompress compresses a text source (stdin/file) by counting character occurence.
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "mycompress.h"
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/// Variable containing the program name.
const char *const PROGRAM_NAME = "mycompress";

/**
 * @brief prints usage synopsis to stderr and exits program with failure.
 * 
 */
static void showUsage(char *callerName) {
    fprintf(stderr,"%s - Usage: %s [-o outfile] [file...]\n", callerName, PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief Calculates the number of digits used in an integer value via divison.
 * 
 * @param value Value to count digits from.
 * @return int Number of digits in value.
 */
int get_int_len(int value){
  int l=1;
  while(value>9){ l++; value/=10; }
  return l;
}

/**
 * @brief Compresses character to printTarget on character change.
 * @details When the curChar is different from lastChar it prints out the lastChar with its occurence appendet to it.
 * 
 * @param printTarget File descriptor pointer for printing to.
 * @param curChar current read character.
 * @param lastChar Pointer to last character that was read. Is overwritten with curChar if different.
 * @param charOccurenceCounter Pointer to character count.
 * @param writeCount Pointer to amount of written characters.
 */
static void processCharacter(FILE *printTarget, char curChar, char *lastChar, int *charOccurenceCounter, int *writeCount){
    if(*lastChar == -1){ // first char read
        *lastChar = curChar;
        *charOccurenceCounter = 1;
        return;
    } else if(curChar == *lastChar){
        (*charOccurenceCounter)++;
    } else{
        fprintf(printTarget, "%c%i", *lastChar, *charOccurenceCounter);
        *writeCount += 1 + get_int_len(*charOccurenceCounter);
        *charOccurenceCounter = 1;
        *lastChar = curChar;
    }
}

/**
 * @brief This processes the last Character in stream since processCharacter only detects changes. 
 * 
 * @param printTarget File descriptor pointer for printing to.
 * @param lastChar Last character that was read.
 * @param charOccurence Count of lastChar.
 * @param writeCount Pointer to amount of written characters.
 */
static void processFinalChar(FILE *printTarget, char lastChar, int charOccurence, int *writeCount){
    fprintf(printTarget, "%c%i", lastChar, charOccurence);
    *writeCount += 1 + get_int_len(charOccurence);
}

/**
 * @brief Prints str n-times to printTarget
 * 
 * @param printTarget File descriptor pointer for printing to.
 * @param count How many times str should be printed.
 * @param str String to print.
 * @return int Amount of written bytes. Negative if error occured.
 */
int printMultiple(FILE *printTarget, int count, char *str){
    int out = 0;
    int cur;
    for(int i = 0; i < count; i++){
        if((cur = fprintf(printTarget, "%s", str)) < 0){
            return -1;
        } else {
            out += cur;
        }
    }
    return out;
}

/**
 * @brief Finds the biggest integer in passed array. MUST BE VALID (No, error check.)
 * 
 * @param numbers Valid non empty array integer array.
 * @param size numbers array size. (>0)
 * @return int biggest integer found in numbers array.
 */
int biggestInt(int numbers[], int size){
    int biggest = numbers[0];
    for(int i = 1; i < size; i++){
        if(numbers[i] > biggest){
            biggest = numbers[i];
        }
    }
    return biggest;
}

/**
 * @brief Very simple algorithm for counting float digits. (works up to x * 10^9, 0 < x < 10)
 * 
 * @param percentage Float to count digits of.
 * @param precision Number of digits after the dot.
 * @return int Number of total digits used up
 */
static int countPercentageDigits(float percentage, int precision){
    if (percentage >= 100000000.0f) return 10 + precision; // 9 digits plus separating dot
    if (percentage >= 10000000.0f) return 9 + precision; // 8 digits plus separating dot
    if (percentage >= 1000000.0f) return 8 + precision; // 7 digits plus separating dot
    if (percentage >= 100000.0f) return 7 + precision; // 6 digits plus separating dot
    if (percentage >= 10000.0f) return 6 + precision; // 5 digits plus separating dot
    if (percentage >= 1000.0f) return 5 + precision; // 4 digits plus separating dot
    if (percentage >= 100.0f) return 4 + precision; // 3 digits plus separating dot
    if (percentage >= 10.0f) return 3 + precision; // 2 digits plus separating dot
    return 2 + precision; // 1 digit plus separating dot
}

/**
 * @brief Prints formatted compression stats.
 * 
 * @param printTarget File descriptor pointer to print to
 * @param readCount Number of characters read
 * @param writeCount Number of characters written
 * @return int Amount of written bytes. Negative if error occured.
 */
static int printResult(FILE *printTarget, int readCount, int writeCount){
    int out = 0;

    float ratio = 100.0f;
    if(readCount > 0) {
        ratio = ((float)writeCount / (float)readCount) * 100;
    }

    int rPadding = 0, wPadding = 0, cPadding = 0;

    int rLineSize = 6 + get_int_len(readCount) + 11; // 6 = length of "Read: ", 11 = length of " characters"
    int wLineSize = 9 + get_int_len(writeCount) + 11; // 9 = length of "Written: ", 11 = length of " characters"
    int cLineSize = 19 + countPercentageDigits(ratio, 1) + 1; // 18 = length of "Compression ratio: ", 1 == length of "%"
    
    int sizes[] = {rLineSize, wLineSize, cLineSize};

    int biggest = biggestInt(sizes, 3);

    if(rLineSize < biggest){
        rPadding = biggest - rLineSize;
    }

    if(wLineSize < biggest){
        wPadding = biggest - wLineSize;
    }

    if(cLineSize < biggest){
        cPadding = biggest - cLineSize;
    }

    int c;
    if((c = fprintf(printTarget, "Read: ")) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = printMultiple(printTarget, rPadding, " ")) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = fprintf(printTarget, "%i characters\nWritten: ", readCount)) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = printMultiple(printTarget, wPadding, " ")) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = fprintf(printTarget, "%i characters\nCompression ratio: ", writeCount)) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = printMultiple(printTarget, cPadding, " ")) < 0){
        return -1;
    } else {
        out += c;
    }

    if((c = fprintf(printTarget, "%.1f%%\n", ratio)) < 0){
        return -1;
    } else {
        out += c;
    }
    return out;
}

/**
 * @brief Parses arguments for program execution.
 * 
 * @param argc Count of arguments passed.
 * @param argv Pointer to argument array.
 * @param outputfile_path Pointer to store output filepath in.
 * @return int Index of argument array where input files begin. -1 if no input files specified.
 */
static int parseOptions(int argc, char *argv[], char **outputfile_path){
    int c; //option character
    int inputFileArgIndex = -1;
    bool usingOffset = false;

    while((c = getopt(argc, argv, ":o:")) != -1){
        switch (c){
            case 'o': {
                *outputfile_path = optarg;
                inputFileArgIndex = optind;
                usingOffset = true;
            }
            case '?':{
                if(optopt != 0){
                    fprintf(stderr,"%s - Unknown option: %c\n", argv[0], optopt);
                    showUsage(argv[0]);
                }
                break;
            }
            case ':': {
                fprintf(stderr,"%s - Missing argument for -%c\n", argv[0], optopt);
                showUsage(argv[0]);
                break;
            }
            default:{
                break;
            };
        }
    }

    if(usingOffset == false && argc > 1){ //if no out argument was set and file parameters exist
        inputFileArgIndex = 1;
    }

    return inputFileArgIndex;
}

/**
 * @brief Determines if program should read/write from stdin/stdout.
 * 
 * @param inputFileArgIndex Index from parseOptions()
 * @param outputfile_path Pointer to output path string.
 * @param readStdin bool flag value which will be overwritten if not using stdin
 * @param writeStdout bool flag value which will be overwritten if not using stdout
 */
static void setOutputTarget(int inputFileArgIndex, int argc, char *outputfile_path, bool *readStdin, bool *writeStdout){    
    if(inputFileArgIndex != argc && inputFileArgIndex != -1){ //if file parameter does not point to end of arguments and a parameter was added to cli
        *readStdin = false;
    }

    if(outputfile_path != NULL){
        *writeStdout = false;
    }
}

int main(int argc, char* argv[])
{
    char *outputfile_path = NULL;
    bool writeToStdout = true;
    bool readFromStdin = true;

    int inputFileArgIndex = parseOptions(argc, argv, &outputfile_path);

    setOutputTarget(inputFileArgIndex, argc, outputfile_path, &readFromStdin, &writeToStdout);

    const int BUFFERSIZE = 64; //amount of bytes read from input source
    int readCounter = 0; //amount of characters processed
    int writeCounter = 0; //amount of characters after compression
    int charOccurenceCounter = 1; //first comparison is skipped since nothing to compare with
    char lastChar = -1; //stores character for processing
    char readCharBuffer[BUFFERSIZE]; //input buffer with BUFFERSIZE as size.
    char curChar; //character to be processed

    FILE *outStream;
    FILE *inStream;
    int streamCount; //amount of streams to read from

    if(writeToStdout){
        outStream = stdout;
    } else {
        if((outStream = fopen(outputfile_path, "w")) == NULL){
            fprintf(stderr, "%s - Error writting to: %s.\n%s\n", argv[0], outputfile_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if(readFromStdin){
        streamCount = 1;
        inStream = stdin;
    } else {
        streamCount = argc - inputFileArgIndex;
    }

    for(int i = 0; i < streamCount; i++){
        char *readpath;
        //read from filesystem
        if(readFromStdin == 0){
            readpath = argv[i + inputFileArgIndex];
            if((inStream = fopen(readpath, "r")) == NULL){
                fprintf(stderr, "%s - Error reading from: %s.\n%s\n", argv[0], readpath, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        while(1){
            if(fgets(readCharBuffer, BUFFERSIZE, inStream) == NULL){ //fgets returns null when EOF or error occurs
                if(lastChar != curChar){
                    //only process final character if it actually changed
                    processFinalChar(outStream, lastChar, charOccurenceCounter, &writeCounter);
                }
                break;
            } else {
                for (int i = 0; i < BUFFERSIZE; i++){
                    curChar = readCharBuffer[i];
                    if(curChar == '\0') break; // fgets automatically terminates strings with \0
                    processCharacter(outStream, curChar, &lastChar, &charOccurenceCounter, &writeCounter);
                    readCounter++;
                }
            }
        }

        //if reading from filesystem close opened resource upon finishing with it.
        if(readFromStdin == 0){
            if(fclose(inStream) != 0){
                fprintf(stderr, "%s - Error closing: %s.\n%s\n", argv[0], readpath, strerror(errno));
            }   
        }
    }

    // close opened write stream if using it.
    if(writeToStdout == 0){
        if(fclose(outStream) != 0){
            fprintf(stderr, "%s - Error closing: %s.\n%s\n", argv[0], outputfile_path, strerror(errno));
        }
    }

    // print out results, exit failure if unable to write.
    if(printResult(stderr, readCounter, writeCounter) < 0){
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}