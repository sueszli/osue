/**
 * @file mycompress.c
 * @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Main program module.
 *
 * This program is an implementation to compress text.
 **/
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>

/**
 * @brief Trims the end of a string
 * @details Removes newlines from the end of the string.
 * @param line The to be trimmed string
 * @param read The length of the string
 * @return string without newline at the end
 */
char *mytrim(char *line, ssize_t read) {
    int readInt = (int) read;

    //checks for newlines
    while(line[--readInt] == '\n' || line[readInt]== '\r' ){
        line[readInt] = '\0';
    }

    return line;
}

/**
 * @brief Compresses lines read
 * @details Replace followup-characters with the character once followed by the amount of the chars
 * @param newlines number of newlines. They are added to the front
 * @param line current line to be read
 * @param read length of line
 */
char* printLine(int newlines, const char *line, size_t read) {
    if(line[0] == '\0' || read == 0) {
        return "";
    }

    FILE *stream;
    char *currentLine;
    size_t len;
    stream = open_memstream(&currentLine, &len);

    char currentChar = line [0];

    if(newlines > 0) {
        fprintf(stream,"%d",newlines);
    }

    fprintf(stream,"%c",currentChar);
    int size = (int) read;
    int count = 1;

    for (int i = 1; i < size; i++) {
        if(currentChar == line[i]) {
            count++;
        } else {
            currentChar = line[i];
            fprintf(stream, "%d%c",count,currentChar);
            count = 1;
        }
    }
    fprintf(stream, "%d",count);
    fclose(stream);

    return currentLine;
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters, and most of the logic.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {
    int opt;
    size_t len = 32;
    FILE *of = NULL;

    while((opt = getopt(argc,argv,"o:")) != -1){
        switch (opt){
            case 'o':
                of = fopen(optarg,"w+");
                break;
            default:
                break;
        }
    }

    int readC = 0;
    int writtenC = 0;

    if(optind == argc) {
        char *lines = malloc(len * sizeof (char));
        char *pline = NULL;

        printf("Enter text to be compressed:\n");
        fgets(lines,(int)len,stdin);

        readC += (int) strlen(lines);

        pline = printLine(0,lines,strlen(lines));
        writtenC += (int) strlen(pline);
        if (of != NULL) {
            fprintf(of, "%s\n", pline);
        } else {
            printf("%s\n", pline);
        }
        free(pline);
        free(lines);
    } else {
        for (int i = optind; i < argc; ++i) {
            ssize_t read;
            int newlines = 0;

            FILE *inputFile = fopen(argv[i],"r");

            if(inputFile == NULL) {
                if(of != NULL) {
                    fclose(of);
                }

                fprintf(stderr,"Input file: %s not found!\n",argv[i]);
                return EXIT_FAILURE;
            }

            char *line = (char *)malloc(len * sizeof(char));

            while ((read = getline(&line, &len, inputFile)) != -1) {

                readC += (int) read;
                line = mytrim(line,read);
                if(line[0] == '\0') {
                    newlines++;
                } else {
                    char *printline = printLine(newlines,line,read);

                    writtenC += (int) strlen(printline);
                    if(of != NULL){
                        fprintf(of, "%s\n", printline);
                    } else {
                        printf("%s\n",printline);
                    }

                    newlines = 1;
                    free(printline);
                }
            }

            free(line);

            fclose(inputFile);
        }
    }

    double rate = 0;
    if(readC > 0) {
        rate = 100 - (((double)writtenC) / readC) * 100;
    }

    fprintf(stderr,"Read:\t\t%d characters\nWritten:\t%d characters\nCompression ratio: %.1f%%\n",
            readC,writtenC,rate);

    if(of != NULL) {
        fclose(of);
    }
    return EXIT_SUCCESS;
}