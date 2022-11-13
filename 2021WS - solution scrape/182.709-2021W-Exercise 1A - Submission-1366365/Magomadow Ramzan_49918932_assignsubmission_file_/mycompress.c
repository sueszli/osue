/**
 * @file mycompress.c
 * @author Ramzan Magomadow 12024698
 * @brief 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define TEMP "temp23571113171923"
#define CAT "cat23571113171923"


FILE *ifp;
FILE *ofp;

/**
 * @details temporary files used as temporary buffer
 * tempfp is used to write the compressed input stream all at once to stdoutput 
 * this fixes the issue that input from stdin gets compressed direcetly at a newline
 * 
 * catfp is the file where data from the concatenated file is stored 
 */
FILE *tempfp;
FILE *catfp;


char *myprog;

int intLog10(int);

/**
 * @brief struct to store information about compression
 */
struct compressionData {
    int readChars;
    int writtenChars;
    float compressionRatio;
};

/**
 * @brief compresses the textfile, writes the compressed data to writeTo 
 * and returns the data regarding the compression as a struct 
 * 
 * @param stream 
 * @param writeTo 
 * @return struct compressionData 
 */
struct compressionData fromCompressTo (FILE *stream, FILE *writeTo) {
    int c;
    int d;
    int i = 0; /* characters in a row*/
    int readChars = 0;
    int writtenChars = 0;
    

    c = getc(stream);
    d = c;
    while((c = fgetc(stream)) != EOF) {
        ++i;
        ++readChars;
        if (c != d) {
            writtenChars += i/10 + 2;
            fprintf(writeTo, "%c%d", (char) d, i);
            d = c;
            i = 0;
        }
    }
    if (c != d) {
        ++i;
        ++readChars;
        writtenChars += intLog10(i) + 2;
        fprintf(writeTo, "%c%d", (char) d, i);
    }

    struct compressionData data = {readChars, writtenChars, 100.0*(writtenChars)/readChars};

    return data;
}
/**
 * @brief usage message 
 * 
 */
void usage(void) {
    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

int main (int argc, char *const argv[]) {
    myprog = argv[0];
    struct compressionData overhead;
    char *outputfile = NULL;

    int c = 0;
    int os = 0;
    while ((c = getopt(argc, argv, ":o:")) != -1) {
        switch (c)
        {
        case 'o': 
            if (optarg == NULL) {
                usage();
            }
            outputfile = optarg;
            os++;
            break;
        case ':':
        case '?':
            usage();
        default:
            break;
        }
    }
    if (os > 1) usage();

    for (int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], CAT) || !strcmp(argv[i], TEMP)) {
            fprintf(stderr, "[%s] Please use files which don't have names exactly like %s or %s.\n", myprog, TEMP, CAT);
            fprintf(stderr, "Therefore either delete or rename those files.\n");
            exit(EXIT_FAILURE);
        }
    }    

    if (optind >= argc) {
        if (outputfile != NULL) {
            ofp = fopen(outputfile, "w");
            overhead = fromCompressTo(stdin, ofp);
        } else {
            tempfp = fopen(TEMP, "w");
            overhead = fromCompressTo(stdin, tempfp);
            fclose(tempfp);
        }
    } else { 
        //check if the arguments are readable
        for (int i = optind; i < argc; ++i) {
            ifp = fopen(argv[i], "r");
            if (ifp == NULL) {
                fprintf(stderr,"[%s] Error: %s\n" , myprog, strerror(errno));
                remove(TEMP);
                remove(CAT);
                exit(EXIT_FAILURE);
            }
            fclose(ifp);
        } 
        catfp = fopen(CAT, "w");
        //cats files to one file
        for (int i = optind; i < argc; i++) {
            ifp = fopen(argv[i], "r");
            while ((c = getc(ifp)) != EOF)
            {
                putc(c, catfp);
            }
            fclose(ifp);
        }
        fclose(catfp);

        catfp = fopen(CAT, "r");
        if (outputfile != NULL) {
            ofp = fopen(outputfile, "w");
            overhead = fromCompressTo(catfp, ofp);
        } else {
            tempfp = fopen(TEMP, "w");
            overhead = fromCompressTo(catfp, tempfp);
            fclose(tempfp);
        }
        fclose(catfp);
    }

    
    if (ofp == NULL) { /*write to stdout */
        printf("\n\n");
        tempfp = fopen(TEMP, "r");
        while((c = getc(tempfp)) != EOF) {
            fputc(c, stdout);
        }
        fclose(tempfp);
        printf("\n");
    } else {
        fclose(ofp);
    }

    remove(TEMP);
    remove(CAT);
    
    fprintf(stderr, "\nRead: %d\nWritten: %d\nCompression ratio: %.1f %% \n", 
        overhead.readChars, overhead.writtenChars, overhead.compressionRatio);
    
    exit(EXIT_SUCCESS);
}

/**
 * @brief floored logarithm of 10 
 * 
 * @param number 
 * @return int 
 */
int intLog10(int number) {
    int i = 0;
    while((number /= 10) > 0) i++; 
    return i;
}