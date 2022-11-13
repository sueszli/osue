/**
@file mycompress.c
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 12.11.2021

@brief compresses a file
@details compresses a file- subsequent character repetitions become character plus number of repetitions

synopsis mycompress [-o outfile] [file...]
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <strings.h>

#define BUFFER_SIZE 64

int main(int argc, char *argv[]) { //argc ist anzahl der Argumente und Optionen
    char *prog_name = argv[0];

    char *outfile = NULL;
    int c;
    while ( (c = getopt(argc, argv, "o:")) != -1 ){ //argv ist aufruf als array
        switch ( c ) {
            case 'o':
                outfile = optarg;
                break;
            default: /* invalid option */
                fprintf(stderr, "[%s:%d] ERROR: wrong input-unknown option, synopsis mycompress [-o outfile] [file...]:\n", prog_name, __LINE__);
                exit(EXIT_FAILURE);
                break;
            }
    }

    FILE* out;
    if(outfile!=NULL){
        out = fopen(outfile, "w");
        if(out==NULL){
            fprintf(stderr, "[%s:%d] Could not open outfile %s: %s\n", prog_name, __LINE__, outfile, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        out = stdout;
    }

    //read from console
    FILE *in;
    char buffer[BUFFER_SIZE];
    int previousCount = -1; //parse infinite 'a' by counting in buffer 
    char previousChar;
    int readChars = 0;
    int writtenChars = 0;
    if(optind==argc) {
        in = stdin;

        //read
        while(1) {
            fgets(buffer, BUFFER_SIZE, in);
            size_t size = strlen(buffer);

            //go through each character
            for(int i=0; i<size; ++i) {
                char c = buffer[i];

                if(previousCount>=0){
                    //"append"
                    if(c==previousChar) {
                        ++previousCount;
                        continue;
                    }

                    //print previous (set new later)
                    fprintf(out, "%c%d", previousChar, previousCount);
                    writtenChars += 2;
                }

                //set new
                previousChar = c;
                previousCount = 1;
                ++readChars;
            }
        }
    }
    //read from files
    else {
        int index = optind-1;
        while(++index<argc) { //for each file
            char *filename = argv[index];
            FILE *in = fopen(filename, "r");
            if(in==NULL){
                fprintf(stderr, "[%s:%d] Could not open infile %s: %s\n", prog_name, __LINE__, filename, strerror(errno));
                exit(EXIT_FAILURE);
            }

            while (1) {
                char* line = fgets(buffer, BUFFER_SIZE, in);
                if(line==NULL) {
                    break;
                }
                size_t size = strlen(buffer);

                //go through each character
                for(int i=0; i<size; ++i) {
                    char c = buffer[i];
                    ++readChars;

                    if(previousCount>=0){
                        //"append"
                        if(c==previousChar) {
                            ++previousCount;
                            continue;
                        }

                        //print previous (set new later)
                        fprintf(out, "%c%d", previousChar, previousCount);
                        writtenChars += 2;
                        while(previousCount>9){ //numbers are characters too
                            printf("%d", previousCount);
                            ++writtenChars;
                            previousCount /= 10;
                        }
                    }

                    //set new
                    previousChar = c;
                    previousCount = 1;
                }
            }
            
            //print last
            if(previousCount>=0){
                //print previous
                fprintf(out, "%c%d", previousChar, previousCount);
                writtenChars += 2;
                while(previousCount>9){ //numbers are characters too
                    printf("%d", previousCount);
                    ++writtenChars;
                    previousCount /= 10;
                }
            }

            fclose(in);
        }
    }
    
    fprintf(stderr, "Read: %d\nWritten: %d\nCompression ratio: %.1f%%\n", readChars, writtenChars, 100.0*writtenChars/readChars);

    if(outfile!=NULL) {
        fclose(out);
    }

    return EXIT_SUCCESS;
}
