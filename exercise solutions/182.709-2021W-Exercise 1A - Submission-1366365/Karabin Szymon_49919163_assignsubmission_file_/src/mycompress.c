/**
 * @file mycompress.c
 * @author Szymon Karabin 11839439
 * @date 08.11.2021
 *
 * @brief mycompress in its entirety. Programme compresses input text by replacing repeated 
 * occurences of same letter
 * 
 * @details Programme takes 0 to infinitely many input files. When given input files, 
 * programme reads and compresses those. When not given input files, programme reads 
 * from stdin. Programme has optional argument -o, which, when used, prints output to 
 * specified output file. When -o is not set, programme prints to stdout. Statsitics 
 * about characters read, characters printed and compression rate are always printed 
 * to stderr.
 **/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

/**
 * Main programme method
 * @brief Entry point of mycompress. Programme implemented in its entirety here.
 * @param argc Number of arguments in programme
 * @param argv Arguments (including optional arguments) in programme
 * @return EXIT_SUCCESS (0) on success, EXIT_FAILURE (1) if programme fails.
 *
 * @details Programme first parses arguments, starting with the optional arguments, 
 * with the help of getopt(). Returns EXIT_FAILURE if there is more than 1 optional 
 * argument or if the optional argument is not o.
 * Afterwards, programme proceeds to parse the other arguments, which are the names 
 * of the input files. Any number of input files is permitted.
 * If specified, the output file is then opened.
 * Then the programme proceeds to compressing the input. The first block compresses 
 * and prints the result to stdout, the second block compresses and prints to the 
 * out file.
 * Compression occurs by iterating through input charcter by character, first 
 * checking for EOF and then checking whether the character is beginning a streak 
 * or whether or not the character just read in is the same as the current character 
 * that has a streak. The characters read in are also counted for statistical purposes.
 * When the streak ends, ie. the charater just read in is not the same as the character 
 * with a current streak, the programme prints the compressed data for the streak in 
 * the given format, adds the amount of characters printed to the relevant counter for 
 * statistical purposes and then resets the other relevant counters.
 * When the loop is broken by on EOF, the programme also has to print the last streak 
 * to the output file/stdout.
 * This is all followed by a closing down of any open files and the printing of the 
 * statstics to stderr.
 **/
int main(int argc, char **argv) {
    int option_index = 0;
    int opt_o = 0;
    char *outFile = NULL;

    // Argument parsing: optional args (output)
    while ((option_index = getopt(argc, argv, "o:")) != -1) {
        switch (option_index) {
            case 'o':
                if (opt_o == 0) {
                    outFile = optarg;
                }
                opt_o++;
                break;
            case '?':
                // Anything else
                fprintf(stderr, "Error: Wrong optional arguments! %s\n", strerror(1));
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }

    if (opt_o > 1) {
        fprintf(stderr, "Error: Too many optional arguments! %s\n", strerror(1));
        exit(EXIT_FAILURE);
    }

    // Argument parsing: input args
    int inputFileAmt = argc - optind;
    char *inFileArray[inputFileAmt];
    for (int i = 0; i < inputFileAmt; i++) {
        inFileArray[i] = argv[optind+i];
    }

    // If out file is NULL, we print to stdout
    FILE *out = NULL;
    if (opt_o == 1) {
        // Open output file
        out = fopen(outFile, "w");
        if (out == NULL) {
            // Failed to open file
            fclose(out);
            fprintf(stderr, "Error: File opening failed! %s", strerror(2));
            exit(EXIT_FAILURE);
        }
    }

    // Read and compress
    // Counters for stats
    int inChars = 0;
    int outChars = 0;

    if (inputFileAmt == 0) {
        //Read from stdin
        int i = 0; // counter
        char current; // letter read in

        while (true) {
            char c = fgetc(stdin);

            // Leave while loop if EOF
            if (feof(stdin)) {
                break;
            }

            inChars++;
            
            // First character
            if (i == 0) {
                i++;
                current = c;
                continue;
            }

            // If charcter is the same
            if (current == c) {
                i++;
                continue;
            }

            // When streak ends, print compressed data
            int data;
            if (out == NULL) {
                data = fprintf(stdout, "%c%d", current, i);
            } else {
                data = fprintf(out, "%c%d", current, i);
            }
            if (data == -1) {
                if (out != NULL) {
                    fclose(out);
                }
                fprintf(stderr, "Error: fprintf failed! %s\n", strerror(2));
                exit(EXIT_FAILURE);
            }

            // Do stats, (re)set counters
            outChars += data;
            i = 1;
            current = c;
        }

        // Print last compression streak
        int data;
        if (out == NULL) {
            data = fprintf(stdout, "%c%d", current, i);
        } else {
            data = fprintf(out, "%c%d", current, i);
        }
        outChars += data;
        if (data == -1) {
            if (out != NULL) {
                fclose(out);
            }
            fprintf(stderr, "Error: fprintf failed! %s\n", strerror(2));
            exit(EXIT_FAILURE);
        }
    } else if (inputFileAmt > 0) {
        // Read from file
        for (int i = 0; i < inputFileAmt; i++) {
            // Open input file
            FILE *in = fopen(inFileArray[i], "r");
            if (in == NULL) {
                fclose(in);
                if (out != NULL) {
                    fclose(out);
                }
                fprintf(stderr, "Error: fopen failed %s\n", strerror(1));
                exit(EXIT_SUCCESS);
            }

            int i = 0;
            char current;

            while (true) {
                char c = fgetc(in);

                // Leave loop if EOF
                if (feof(in)) {
                    break;
                }

                inChars++;

                // First character
                if (i == 0) {
                    i++;
                    current = c;
                    continue;
                }

                // If charcter is the same
                if (current == c) {
                    i++;
                    continue;
                }

                // When streak ends, print compressed data
                int data;
                if (out == NULL) {
                    data = fprintf(stdout, "%c%d", current, i);
                } else {
                    data = fprintf(out, "%c%d", current, i);
                }
                if (data == -1) {
                    if (out != NULL) {
                        fclose(out);
                    }
                    fprintf(stderr, "Error: fprintf failed! %s\n", strerror(2));
                    exit(EXIT_FAILURE);
                }

                // Do stats, (re)set counters
                outChars += data;
                i = 1;
                current = c;
            }

            // Print last compression streak
            int data;
            if (out == NULL) {
                data = fprintf(stdout, "%c%d", current, i);
            } else {
                data = fprintf(out, "%c%d", current, i);
            }
            outChars += data;
            if (data == -1) {
                if (out != NULL) {
                    fclose(out);
                }
                fprintf(stderr, "Error: fprintf failed! %s\n", strerror(2));
                exit(EXIT_FAILURE);
            }

            // Close input file
            if (fclose(in) != 0) {
                if (out != NULL) {
                    fclose(out);
                }
                fprintf(stderr, "Error: fclose failed! %s\n", strerror(2));
                exit(EXIT_FAILURE);
            }
        }  
    } else {
        if (out != NULL) {
            fclose(out);
        }
        fprintf(stderr, "Error: Unknown error. %s\n", strerror(2));
        exit(EXIT_FAILURE);
    }
    
    // Close output file
    if (out != NULL) {
        fclose(out);
    }

    // Finishing off, print stats
    fprintf(stderr, "Read: %d characters\nWritten: %d characters\nCompression ratio: %4.1f%%\n", inChars, outChars, ((double)(outChars)/(inChars))*((double)100));
    return EXIT_SUCCESS;
}
