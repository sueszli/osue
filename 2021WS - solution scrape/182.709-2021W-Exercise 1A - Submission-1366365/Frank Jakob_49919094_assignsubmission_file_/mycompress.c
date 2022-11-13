/*
* @file mycompress.c
* @author Jakob Frank (11837319)
* @details this file reads the input of an amount of files (0 - n)
*          and compresses them to a chosen output file. If there is no
*          input file, then the file takes the input from stdin and if
*          there is no output, it writes to stdout.
*          Furthermore the file prints a the amount of characters read
*          and written to stderr along with a compression ratio.
* @date 07/11/2021
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>



/*
* @brief The function fileCompress reads an input file and writes a compressed version to output
* 
* @details  The function read one char at a time and updates a counter each time the same char
*           gets read. If a different char gets read, the function writes the char and the counter
*           to the output file, updates the comp char and resets the counter. Furthermore, the function
*           increments the read ptr each time a char gets read and the written ptr each time it writes
*           to output. When the reader reaches EOF, it writes the remaining char and counter to output
*           one last time.
*
* @param input The input file to be read (or stdin if none given)
* @param output The output file to write into (or stdout if none given)
* @param read A reference to the integer where the count of read characters is stored
* @param written A reference to the integer where the count of written characters is stored
*/

static void fileCompress(FILE * input, FILE* output, size_t *read, size_t* written);



/*
@brief: the function "usage" gets called in case of an error that is traced to a
        flawed command line input. The function then prints the syntax to use the
        program to stderr and exits the executable in failure.
*/

static void usage(void);



static char *myprog;



int main(int argc, char **argv)
{
    myprog = argv[0];
    char* outputName;

    int c;
    int outbool = 0; //informs if an output argument has already been called
    

    while ((c = getopt(argc , argv , "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            if (outbool == 0)
            {
                --outbool;
                outputName = optarg;
            }
            else
            {
                usage();
            }
            break;
        
        default:
            usage();
            break;
        }
    }

      
    FILE * output;
    FILE * input;

    if (outbool == 0)
    {
        output = stdout;
    } 
    else
    {
        if ((output = fopen(outputName,"w")) == NULL)
        {
            fprintf(stderr, "%s does not exist, please enter a valid file", outputName);
            exit(EXIT_FAILURE);
        }  
    }

    size_t write = 0;
    size_t read = 0;
    size_t *writePtr = &write;
    size_t *readPtr = &read;


    if ((argc - (optind + outbool)) == 0) //no input files given
    {
        input = stdin;
    }
    else
    {
        for (size_t i = optind; i < argc; i++)
        {
            if ((input = fopen(argv[i], "r")) == NULL) //error handling (returns error in case of missing file)
            {
                fprintf(stderr, "%s can't be opened\n", argv[i]);
                fclose(output);
                exit(EXIT_FAILURE);
            }
              
            fileCompress(input, output, readPtr, writePtr);
            fclose(input);
        }
    }

    fclose(output);
    double ratio = ((double)(*writePtr)/(double)(*readPtr))*100;

    fprintf(stderr,"\nRead:     %ld\nCharacters written: %ld \nCompression ratio: %.1f%%\n", *readPtr, *writePtr, ratio);

    return 0;
}


static void fileCompress(FILE * input, FILE* output, size_t *read, size_t* written)
{
    int character;
    char write;
    

    int count = 1;
    int comp = fgetc(input);
    char countDigits[16];

    *read += 1;

    while ((character = fgetc(input)) != EOF)
    {   
        *read += 1;

        if(character != comp)
        {   
            write = (char)comp;
            fprintf(output, "%c%d", write, count);

            sprintf(countDigits, "%d", count); 
            *written += (strlen(countDigits) + 1);
        
            count = 0;
            comp = character;
        } 
        ++count;   
    }

    write = (char)comp;
    fprintf(output, "%c%d", write, count);
 
    sprintf(countDigits, "%d", count);
    *written += (strlen(countDigits) + 1);

}


void usage(void)
{
    fprintf(stderr,"Usage: %s [-o outfile] [file...]", myprog);
    exit(EXIT_FAILURE);
}