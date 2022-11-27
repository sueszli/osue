/**
 * @file   mydiff.c
 * @author Fisnik Miftari (e11776840)
 * @date   14.11.2021
 *
 * @brief A program which compares two textfiles
 *
 * @details This program takes two textfiles as input and then compares them line by line
 * by pointing out the number of different characters in each line. The lines are only compared
 * up to the end of the shortest one. For that reason, "abc" and "abcd" have 0 different characters
 * according to our program. Furthermore, the program has two options: -o (followed by an output file)
 * and -i. If -o (followed by an output file) is typed, then the program stores the line differences
 * in the output file (otherwise it prints them in the standard output). If -i is typed, the program
 * ignores case ('A' and 'a' are treated as equal).
 * 
 **/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <sys/errno.h>

char *programname;

/**
 * @brief prints the correct usage
 * @details prints the correct program usage by specifying the possible options and arguments. Additionally,
 * it prints the specific problem, which is passed with the reason variable.
 * 
 * @param reason reason why the printCorrectUsage was called.
 */ 
void printCorrectUsage(char *reason)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2. %s\n", programname, reason);
    exit(EXIT_FAILURE);
}

/**
 * @brief closes the inputs
 * @details closes the two inputs and checks if their closing was successful. If not, it prints an error and terminates.
 * 
 * @param file1 pointer to the first file to be closed
 * @param file2 pointer to the second file to be closed
 */ 
void closeInputs(FILE *file1, FILE *file2)
{
    int result = 0;
    result = fclose(file1);
    if (result == EOF)
    {
        fprintf(stderr, "%s: fclose failed: %s\n", programname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    result = fclose(file2);
    if (result == EOF)
    {
        fprintf(stderr, "%s: fclose failed: %s\n", programname, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief converts a char to lowercase
 * @details this method checks if the given char parameter is an uppercase (if it's between 'A' and 'Z'),
 * and if it's the case, it converts it to lowercase by adding 32 (which is the distance between two same
 * characters in different cases).
 * 
 * @return the converted character
 * @param c the character to be converted
 */
char tryConvertToLowercase(char c){
    if (c >= 'A' && c <= 'Z'){
        c = c+32;
        return c;
    } else {
        return c;
    }
}

/**
 * @brief puts the outputString content to outputFile
 * @details this method puts the outputString content to outputFile while storing the return value
 * to an integer which is then checked for EOF equality, which is the case after an error has occurred.
 * 
 * @param outputString reference to the string to be put into outputFile
 * @param outputFile reference to outputfile in which the outputString is put.
 */
void checkFputs(char *outputString, FILE *outputFile){
    int fputsResult = fputs(outputString, outputFile);
    if (fputsResult == EOF)
        {
            fprintf(stderr, "%s: fputs failed: %s\n", programname, strerror(errno));
            exit(EXIT_FAILURE);
        }
}

/**
 * @brief flushes outputFile and eventually terminates programm
 * @details this method flushes the outputFile while storing the return value into an integer,
 * which is then checked for inequality to 0 (which only happens in errors)
 * 
 * @param outputFile reference to outputfile to be flushed
 */
void checkFflush(FILE *outputFile){
    int fflushResult = fflush(outputFile);
    if (fflushResult != 0)
        {
            fprintf(stderr, "%s: fputs failed: %s\n", programname, strerror(errno));
            exit(EXIT_FAILURE);
        }
}

/**
 * @brief compares two lines
 * @details this method goes through each index of the lines and compares the correpsonding chars 
 * and increments the charDifferences variable, which is returned in the end. The for-loop only goes
 * up to maxIndex (the index of the last character of the shortest line.)
 * @return number of different characters between two lines
 *
 * @param line1 the corresponding line of the first file
 * @param line2 the corresponding line of the second file
 * @param maxIndex the index up to which the lines should be compared
 * @param caseSensitivity variable for case sensivity. 1 = sensitive, 0 = insensitive.
 */
int compareLinesWithGivenIndex(char *line1, char *line2, int maxIndex, int caseSensitivity){
    int charDifferences = 0;
    for (int i = 0; i < maxIndex; i++)
    {   
        if (caseSensitivity == 1){
            if (line1[i] != line2[i]){
                charDifferences++;
            }
        } else {
            if (tryConvertToLowercase(line1[i]) != tryConvertToLowercase(line2[i])){
                charDifferences++;
            }
        }
    }
    return charDifferences;
}

/**
 * @brief compares two lines
 * @details this method takes two lines and compares them accordingly. Of high importance are here
 * the line lengths, which are compared and then depending on that, we call the next method which compares
 * two lines with a given index.
 * @return number of different characters between two lines
 *
 * @param line1 the corresponding line of the first file
 * @param line2 the corresponding line of the second file
 * @param length1 first line's length
 * @param length2 second line's length
 * @param caseSensitivity variable for case sensivity. 1 = sensitive, 0 = insensitive.
 */
int compareLines(char *line1, char *line2, int length1, int length2, int caseSensitivity)
{
    if (length1 < 0 || length2 < 0)
    {
        fprintf(stderr, "%s: A line with negative size was noticed", programname);
        exit(EXIT_FAILURE);
    }
    
    if (length1 > length2)
    {
        return compareLinesWithGivenIndex(line1, line2, length2-1, caseSensitivity);
    }
    else
    {
        return compareLinesWithGivenIndex(line1, line2, length1-1, caseSensitivity);
    }
}

/**
 * @brief writes to an output file
 * @details this method updates the outputFile with outputString (which is meant
 * to contain the line differences)
 * @return the updated outputFile
 *
 * @param outputString contains the string to be stored in the output file
 * @param outputFilename contains the output file name.
 */
FILE* writeToFile(char *outputString, char *outputFilename){
        FILE *outputFile = fopen(outputFilename, "a");
        
        if (outputFile == NULL)
        {
        fprintf(stderr, "%s: fopen failed: %s\n", programname, strerror(errno));
        exit(EXIT_FAILURE);
        }

        checkFputs(outputString, outputFile);
        checkFflush(outputFile);
        return outputFile;
}

/**
 * @brief prints or writes to an output file
 * @details this method asks if there is no output file. If that's the case, it prints the line differences in
 * the standard output. If not, then it stores the line differences into the specified output file.
 *
 * @param lineIndex contains the index of the lines we are comparing (begins at 1).
 * @param numOfDifferentChars contains the number of different characters in these two lines
 * @param outputFilename contains the filename in which we store the line differences
 */
void printOrWriteToFile(int lineIndex, int numOfDifferentChars, char *outputFilename)
{
    int spaceForIntegers = 2*sizeof(int);
    int spaceForString = 20;
    char *outputString = malloc(spaceForIntegers + spaceForString);
    sprintf(outputString, "Line: %i, characters: %i\n", lineIndex, numOfDifferentChars);
    if (outputFilename == NULL)
    {
        printf("%s", outputString);
    }
    else
    {
        FILE *outputFile = writeToFile(outputString, outputFilename);
        int result = fclose(outputFile);
        if (result == EOF)
        {
            fprintf(stderr, "%s: fclose failed: %s\n", programname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    free(outputString);
}

/**
 * @brief Takes the input arguments.
 * @details This method takes the input arguments and parses them accordingly to the variables, which are
 * later needed for further processing.
 *
 * @param argc number of arguments
 * @param argv the arguments array
 * @param caseSensitivity variable for case sensivity. 1 = sensitive, 0 = insensitive.
 * @param in1_path pointer of first input file's path string
 * @param in2_path pointer of second input file's path string
 * @param out_path pointer of output's path string
 */
static void parseArgs(int argc, char *argv[], int *caseSensitivity, char **in1_path, char **in2_path, char **out_path)
{
    *caseSensitivity = 1;

    int opt;
    while((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch(opt)
        {
            case 'o': 
                *out_path = optarg; 
                break;
            case 'i': 
                *caseSensitivity = 0;   
                break;
            case '?': 
                if (optopt == 'o'){
                    printCorrectUsage("Option -o needs an argument.");
                } else {
                printCorrectUsage("Undefined option.");
                }
            default: 
                printCorrectUsage("Undefined error.");
        }
    }

    if ((argc - optind) != 2)
        printCorrectUsage("Less arguments than expected.");

    *in1_path = argv[optind];
    *in2_path = argv[optind + 1];
}

/**
 * @brief compares two files
 * @details goes through the pairs of lines from the two input files and compares the pairs one after another.
 * The method uses getline() to determine the length of both lines, which are then used for line comparison.
 *
 * @param file1 a pointer to the first input file
 * @param file2 a pointer to the second input file
 * @param caseSensitivity variable for case sensivity. 1 = sensitive, 0 = insensitive.
 * @param out_path pointer of output's path
 */
void compareFiles(FILE *file1, FILE *file2, int caseSensitivity, char *out_path){
    char *line1 = NULL;
    size_t length1 = 0; 
    ssize_t readFile1;
    char *line2 = NULL;
    size_t length2 = 0;
    ssize_t readFile2;
    int lineCounter = 1;
    while ((readFile1 = getline(&line1, &length1, file1)) != -1 && (readFile2 = getline(&line2, &length2, file2)) != -1) 
    {   
        int linedifferences = compareLines(line1, line2, readFile1, readFile2, caseSensitivity);
            if (linedifferences != 0)
            {
                printOrWriteToFile(lineCounter, linedifferences, out_path);
            }
        readFile1 = 0;
        readFile2 = 0;
        lineCounter++;
    }
    free(line1);
    free(line2);
}

int main(int argc, char *argv[])
{
    programname = argv[0];

    int caseSensitivity;
    char *in1_path, *in2_path;
    char *out_path = NULL;
    parseArgs(argc, argv, &caseSensitivity, &in1_path, &in2_path, &out_path);

    FILE *file1 = fopen(in1_path, "r");
    FILE *file2 = fopen(in2_path, "r");
 
    if (file1 == NULL || file2 == NULL)
    {
        fprintf(stderr, "%s: fopen failed: %s\n", programname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    compareFiles(file1, file2, caseSensitivity, out_path);
    closeInputs(file1, file2);
    return EXIT_SUCCESS;
}