/**
 * @file mycompress.c
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 24.10.2021
 *
 * @brief my compress main module
 *
 * This program reads from various, optional files (in case no file is provided, it reads from stdin)
 * and applies a simple RLE (run length encoding) - algorithm to compress its content.
 * The compressed, encoded data is written to an optional output file, otherwise to stdout.
 * In addition further information regarding the compression, including the amount of characters
 * read/written as well as the ratio of both values, is written to stderr.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define PRG_NAME "mycompress"

/**
 * usage function.
 * @brief This function writes to stderr how the program is to be used
 */
static void usageMessage(void) {
    fprintf(stderr, "[%s] ERROR: invalid arguments, Usage: %s [-o outfile] [file...]\n", PRG_NAME, PRG_NAME);
    exit(EXIT_FAILURE);
}


/**
 * General error handling
 * @brief This function handles errors caused by the Stream I/O
 * @param description specifys error caused
 * @param filename specifys the file which caused the error
 * @param infile pointer to the current input file
 * @param outfile pointer to the output file
 * @details prints to stderr the error with further details, cleans resources before exiting with failure code
 */
static void failedWithError(char* description, char* filename, FILE* infile, FILE* outfile) {
    fprintf(stderr, "[%s] ERROR: %s: %s\n", PRG_NAME, description, filename);
    fclose(infile);
    fclose(outfile);
    exit(EXIT_FAILURE);
}


/**
 * compression algorithm
 * @brief This function applies the RLE-algorithm to the content of the infile and writes the encoded content to outfile
 * @param infile pointer to the file whose content is compressed
 * @param outfile pointer to the destination file of the encoded content
 * @param infile_name name/path of the infile
 * @param outfile_name name/path of the outfile
 * @param written_char int pointer to the amount of written characters
 * @details reads in single chars from infile and compares it with the previous one. In case they differ, the previous char followed by its amount is printed to outfile.
 * @returns the amount of read characters
 */
static int compress(FILE* infile, FILE* outfile, char* infile_name, char* outfile_name, int* written_char) {
    char cur_c = 'a';
    char c;
    int count = 0;
    int cur_count = 0;
    while((c = fgetc(infile)) != EOF) {
        //new char -> print old and its count
        if(c != cur_c) {
            if(cur_count > 0) {
                if(fprintf(outfile,"%c%d",cur_c,cur_count) < 0) {
                    failedWithError("writing to file",outfile_name,infile,outfile);
                }
                *written_char += 2;
            }
            cur_c = c;
            cur_count = 0;
        }
        cur_count++;
        count++;
    }
    if(feof(infile)) {
        if(fprintf(outfile,"%c%d",cur_c,cur_count) < 0) {
            failedWithError("writing to file",outfile_name,infile,outfile);
        }
        *written_char += 2;
    }
    if(ferror(infile) != 0) {
        failedWithError("reading file",infile_name,infile,outfile);
    }
    return count;
}

/**
 * Print compression details
 * @brief This function prints compression details to stderr
 * @param written_char amount of characters written
 * @param read_char amount of characters read
 * @details prints to stderr the amount of characters written/read and the reached compression ratio.
 */
void printCompressionDetails(int written_char, int read_char) {
    fprintf(stderr, "%-11s%d characters\n%-11s%d characters \n","Read:",read_char,"Written:", written_char);
    if(read_char > 0) {
        fprintf(stderr, "Compression ratio: %.1f%%\n", 100.0*written_char/read_char);
    }
}

/**
 * Program entry point.
 * @brief This function handles the parameters of mycompress and depending on the arguments passed,
 * applies the compression algorithm either to the specified files or to stdin/stdout
 * @details in case there is no output file specified using -o, the output is written to stdout,
 *  in case there is one or more inputfiles given, their content is compressed, otherwise stdin is used as input source
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv) {
    char *out_file_c = NULL;
    int c;
    while ((c = getopt(argc, argv, "o:")) != -1){
        switch (c) {
            case 'o': out_file_c = optarg;
                break;
            case '?':
                usageMessage();
                break;
        }
    }
    FILE *out, *in = NULL;
    
    //no outfile file provided -> stdout as output
    if(out_file_c == NULL) {
        out = stdout;
        out_file_c = "stdout";
    } else {
        out = fopen(out_file_c,"w");
        if(out == NULL) {
            failedWithError("failed to open",out_file_c,in,out);
        }
    }
    int written_char = 0;
    int read_char = 0;
    
    //no input file provided -> stdin as input
    if(argv[optind] == NULL) {
        in = stdin;
        read_char = compress(in,out, "stdin",out_file_c, &written_char);
        fclose(in);
    } else {
        while(argv[optind] != NULL) {
            in = fopen(argv[optind],"r");
            if(in == NULL) {
                failedWithError("failed to open",argv[optind],in,out);
            }
            read_char += compress(in,out,argv[optind],out_file_c, &written_char);
            optind++;
            fclose(in);
        }
    }
    printCompressionDetails(written_char,read_char);
    fclose(out);
    return EXIT_SUCCESS;
}
