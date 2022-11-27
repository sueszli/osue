/**
 *@name mycompress
 *@author Philip Duerauer 11811848
 *@brief Text compression
 *@details Program compresses input file(s) if given, otherwise input from stdin to output file if given, otherwise to stdout; each char is printed one time followed by the number of occourance.
*/

    #include <unistd.h>
    #include <stdlib.h>  
    #include <stdio.h>
    #include <errno.h>
    #include <string.h>

/**
 * @details Contains the program name for the error message
**/
char *program;


/**
 * @brief Prints error msg on stderr 
 * @param msg msg to print
 * 
**/
void printError(char* msg) {
    fprintf(stderr, "%s : %s %s", program, msg, "\n");  
}


/**
 * @brief Prints error msg on stderr with given code
 * @param msg msg to print
 * @param code code to use on exit
**/
void exitError(char* msg, int code) {
    printError(msg);
    exit(code);
}

/**
 * @brief Prints error msg on stderr and exits with EXIT_FAILURE
 * @param msg msg to print
 * 
**/
void exitErrorFail(char* msg) {
    exitError(msg, EXIT_FAILURE);
}


/**
 * @brief Prints usage for client
**/
void usage(void) {
    exitError("USAGE: mycompress [-o outfile] [file...]", EXIT_FAILURE);
}

int readChars = 0;
int writenChars = 0;

/**
 * @brief Handles on input file and puts it compressed in output file
 * @param out output file
 * @param in input file
**/
void handleInputFile(FILE *out, FILE *in) {
       int lastChar = NULL;
       int currentChar = NULL;
       int currentCharCount = 0;

       while ((currentChar = fgetc(in)) != EOF) {
           if (lastChar == '\0') {
               lastChar = currentChar;
           }
           if ((currentChar != lastChar)) {
      
               fputc(lastChar, out);

               char str[12];
               sprintf(str, "%d", currentCharCount);
               fputs(str, out);
               lastChar = currentChar;
               currentCharCount = 0;
               writenChars++;
               writenChars += strlen(str);
           }
           currentCharCount++;
           readChars++;
       }

       if (readChars > 0) {
   fputc(lastChar, out);

            char str[12];
            sprintf(str, "%d", currentCharCount);
            fputs(str, out);

            currentCharCount = 0;
            writenChars++;
            writenChars += strlen(str);
       }
}

/**
 * @brief Main method of mycompress. Reads parameters and create File Streams, Logs out information about compression.
 * @param argc count of arguments
 * @param argv arguments
 * @details Program uses given input files or output files or if not present, stdin or stdout. After the compression
 the numbers of printet and read chars and rate of compression are given to stderr.
 * @return returns 0 after completing successful
**/
int main(int argc, char *argv[])
{
    program = argv[0];
    char *outputFile = NULL;
    int outputFileCounter = 0;
    if (argc < 1) {
        usage();
    }

    int c = NULL;
    while ((c = getopt(argc, argv, "o:")) != -1) {
        switch (c) {
            case 'o': outputFile = optarg; outputFileCounter++; break;
            case '?': usage(); break;
            default: usage(); break;
        }
    }
    if (outputFileCounter > 1) {
        usage();
    }

    int inputFileCount = 0;
    int startIndex = NULL;

    if (outputFile == NULL) {
        inputFileCount = argc-1;
        startIndex = 1;
    } else {
        inputFileCount = argc-3;
        startIndex = 3;
    }

    char *inputFiles[inputFileCount];


    for (int i = 0; i < inputFileCount; i++) {
        inputFiles[i] = argv[startIndex+i];
    }

    FILE *out = stdout;

    if (outputFile != NULL) {
        if ((out = fopen(outputFile, "w")) == NULL) {
            printError("Error while opening output File:");
            exitErrorFail(strerror(errno));
        }  
    }

    if (inputFileCount != 0) {
        for (int i = 0; i < inputFileCount; i++) {
            FILE *in;
            if ((in = fopen(inputFiles[i], "r")) == NULL) {
               printError("Error while opening input File:");
               exitErrorFail(strerror(errno));
            }


/*
        while (fgets(buffer, sizeof(buffer), in) != NULL) {
            if (fputs(buffer, out) == EOF) {
                printError("Error while writing to output:");
                exitErrorFail(strerror(errno));
            }
        }
*/
            handleInputFile(out, in);

            if (ferror(in)) {
                printError("Error while reading from input file:");            
                exitErrorFail(strerror(errno));
            }
            fclose(in);
        }
    } else {
        handleInputFile(out, stdin);
    }
    
    fclose(out);
    float rate = 0;
    if (writenChars > 0) {
        rate = ((float)writenChars/(float)readChars)*100;
    }
    fprintf(stderr, "%s", "\n");
    fprintf(stderr, "%s %i %s", "Read: ", readChars, " characters\n");
    fprintf(stderr, "%s %i %s", "Written: ", writenChars, " characters\n");
 
    
    fprintf(stderr, "%s %.1f %s", "Compression ratio: ", rate, "%\n");

    exit(EXIT_SUCCESS);
}





