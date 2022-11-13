/**
 * @file mydiff.c
 * @author Janis Schneeberger (e12027411@student.tuwien.ac.at)
 * @brief Program to find the differences between two files
 * @version 0.1
 * @date 2021-11-10
 * 
 * @details The program finds the differences between two files. A file is compared line by line and if two lines differ the number of the line as well as the 
 * amount of differing characters is printed out. If one line is shorter, the program only compares until the end of the shorter line. The same is true, if one
 * of the files has fewer lines. Two optional flags can be given [-i], to make the program case insensitive and [-o outfile], to make the program write the
 * output to the specified outfile. Additionally the two files to be compared are expected.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * @brief compares two Strings and counts the amount of differing characters
 * @details This function takes two Strings and a boolean as parameters. The boolean values describes if the comparison of the strings is case sensitive or not.
 * The rough sequence of events is first a comparison if the strings are exactly the same. Afterwards, the function iterates through the strings, character by
 * character and for each differing character increments the return value by one. This way the number of differing characters is returned.
 * 
 * @param strA First String to be compared
 * @param strB Second String to be compared
 * @param caseSensitive When false, the function is not caseSensitive
 * @return number of differing characters
 */
int checkDiffChar(char strA[], char strB[], bool caseSensitive) {
    int count = 0;
    int i = 0;
    if (strlen(strA) <= strlen(strB)) {
        if (caseSensitive) {
            if (strncmp(strA, strB, strlen(strA) - 1) == 0) return 0;
            // Strings differ, count the differences
            for(i = 0; i < strlen(strA) -1 ; i++){
                if(strA[i] != strB[i]) count++;
            }
            return count;
        } else {
            if (strncasecmp(strA, strB, strlen(strA) - 1) == 0) return 0;
            // Strings differ, count the differences
            for(size_t j = 0; j < strlen(strA) -1; j++) {
                strA[j] = tolower((unsigned char)strA[j]);
                strB[j] = tolower((unsigned char)strB[j]);
            }
            for(i = 0; i < strlen(strA) -1 ; i++){
                if(strA[i] != strB[i]) count++;
            }
            return count;
        }
    } else {
        if (caseSensitive) {
            if (strncmp(strA, strB, strlen(strB) - 1) == 0) return 0;
            // Strings differ, count the differences
            for(i = 0; i < strlen(strB) -1 ; i++){
                if(strA[i] != strB[i]) count++;
            }
            return count;
        } else {
            if (strncasecmp(strA, strB, strlen(strB) - 1) == 0) return 0;
            // Strings differ, count the differences
            for(size_t j = 0; j < strlen(strB) -1; j++) {
                strA[j] = tolower((unsigned char)strA[j]);
                strB[j] = tolower((unsigned char)strB[j]);
            }
            for(i = 0; i < strlen(strB) -1 ; i++){
                if(strA[i] != strB[i]) count++;
            }
            return count;
        }
    }
    return 0;
}



/**
 * Program entry point.
 * @brief Takes care about the entry arguments and prints out the file differences as specified
 * @details Checks if the entry arguments have the correct form. If they don't the user is informed, how to correctly call the program and the program
 * terminates with exit code greater than 0. If one of the files cannot be opened the user is also informed. Otherwise the two specified files are compared
 * and the difference of each line is printed out, with the associated line number.
 * 
 * @param argc the argument counter
 * @param argv the argument vector
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    int option;
    bool caseSensitive = true;
    bool outputToFile = false;
    FILE *fp1;
    FILE *fp2;
    char outfile[128];

    // checks if any flags are given. If so the corresponding boolean values are changed and the name of a the corresponding outfile is safed
    while ((option = getopt(argc, argv, ":io:")) != -1)
    {
        switch (option)
        {
        case 'i':
            caseSensitive = false;
            break;
        case 'o':
            outputToFile = true;
            sscanf(optarg, "%s", outfile);
            break;
        default: // wrong flags
            fprintf(stderr, "Usage %s [-i] [-o outfile] file1 file2 \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        // first and second filename not given
        fprintf(stderr, "Usage %s [-i] [-o outfile] file1 file2 \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int file1Index = optind;

    optind++;
    if(optind >= argc) {
        // second file name not given
        fprintf(stderr, "Usage %s [-i] [-o outfile] file1 file2 \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int file2Index = optind;

    optind++;
    if (optind != argc) {
        // too many arguments given
        fprintf(stderr, "Usage %s [-i] [-o outfile] file1 file2 \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char * lineA = NULL;
    size_t lenA = 0;
    ssize_t readA;

    char * lineB = NULL;
    size_t lenB = 0;
    ssize_t readB;

    fp1 = fopen(argv[file1Index], "r");
    fp2 = fopen(argv[file2Index], "r");

    int i = 1;

    if (fp1 == NULL || fp2 == NULL) {
        fprintf(stderr, "Error, %s could not open specified files.", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (outputToFile) {
        freopen(outfile, "w", stdout);
    }
    
    while ((readA = getline(&lineA, &lenA, fp1)) != -1 && (readB = getline(&lineB, &lenB, fp2)) != -1) {
        char newline = '\n';
        if (lineA[strlen(lineA) - 1] != '\n') {
            // append newline char to end of string, if it was the last one of the file
            strncat(lineA, &newline, 1);
        }
        if (lineA[strlen(lineB) - 1] != '\n') {
            // append newline char to end of string, if it was the last one of the file
            strncat(lineB, &newline, 1);
        }
        int diffCharacters = checkDiffChar(lineA, lineB, caseSensitive);
        if (diffCharacters != 0) {
            printf("Line: %d, characters: %d\n", i, diffCharacters);
        }
        i++;
    }

    fclose(fp1);
    fclose(fp2);
    if (lineA) free(lineA);
    if (lineB) free(lineB);

    exit(EXIT_SUCCESS);
    
}

