/**
 * @file mycompress.c
 * @author Ivana Kukic 11832468
 * @date 13.11.2021.
 *
 * @brief Implementation of an algorithm which reads and compresses input files.
 *
 * @details The program reads the content of any files given as positional
 *  arguments one after the other, compress it, and write the compressed content to
 *  an output file given by the option -o. If no input files are given, the
 *  program reads from stdin. If no output file is given, the program writes to stdout.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

/**
 * Variables for the number of read and written characters
*/
int read_chars=0;
int written_chars=0;


/**
 * @brief funtion for writing characters to output file
 * @param output file, char and the number of its repeating
 * @output returns 2 if character were written to output file (e.g a2), otherwise 0.
*/
  static int character_writing(FILE *output,char a,int count) {
     if(a!=-1) {
       fprintf(output,"%c%d",a,count);
       return 2;
     }
     return 0;
   }

/**
 * @brief funtion for compressing input file to output file
 * @details The input is compressed by substituting subsequent identical characters
 *  by only one occurence of the character followed by the number of characters.
 *  If a character appears only once in sequence, then it is printed followed by a 1.
 * @param input file and the output file
*/
  static void compress(FILE *input,FILE *output) {
    char c,a=-1;
    int repeating_char=1;

    while((c=fgetc(input))!=EOF)  {
      read_chars++;
      if(c==a) {
        repeating_char++;
      }
      else {
        written_chars+=character_writing(output,a,repeating_char);
        repeating_char=1;
      }
      a=c;
    }
    written_chars += character_writing(output,a,repeating_char);

  }

/**
 * @brief Implements the main functionality of the program.
 * @details Checks the number of arguments. If the input files and output file are given, the program will compress input file(s) and write the solution to output file.
 *  If only an output file is given, it takes arguments from stdin, compresses them and writes to output file. In case neither input file(s) nor output file are given,
 *  it takes arguments from stdin, compresses them, and writes to stdout.
 * @param argc The count of the arguments provided to the program
 * @param argv A vector containing the provided arguments
 * @returns EXIT_SUCCESS upon successfull operation, EXIT_FAILURE otherwise
 */
  int main(int argc,char* argv[]) {
    FILE *input;
    FILE *output;
    char option;
    char* output_file_name;
    char** input_file_names;
    int output_flag=0;
    int input_flag=0;
    int size = 1;

    input_file_names = (char**)malloc(size*sizeof(char*));

    if(argc>1) {
        while((option = getopt(argc,argv,"o:"))!=-1) {  //o requires an argument
            switch(option) {
              case('o'): {
                  output_file_name=optarg;
                  output_flag=1;
                  break;
               }

              case('?'): {
                  exit(EXIT_FAILURE);
               }

              default:
              assert(0);
           }
        }

       if(optind == argc) {
         input=stdin;
       }
       else {
         int j = 0;
         for (int i = optind; i < argc; i++) {
            input_file_names[j] = argv[i];
            j++;
            if ((i+1) < argc) {
              input_file_names = (char**)realloc(input_file_names, (++size)*sizeof(char*));
            }
         }
         input_flag = 1;
       }
    }

    else {
      output=stdout;
      input=stdin;
    }

    if(output_flag==1) {
      if (size > 1) {
        if((output=fopen(output_file_name,"a"))==NULL){
          fprintf(stderr, "Cannot open output file\n");
          exit(EXIT_FAILURE);
        }
      }else{
       if((output=fopen(output_file_name,"w+"))==NULL){
         fprintf(stderr, "Cannot open output file\n");
         exit(EXIT_FAILURE);
        }
      }
    }

    if(input_flag==1) {
      for (int i = 0; i < size; i++){
        if((input = fopen(input_file_names[i],"r"))==NULL){
          fprintf(stderr, "Cannot open input file\n");
         }
         else {
           compress(input, output);
           if(fclose(input) == EOF) {
             fprintf(stderr, "Cannot close input file\n");
           }
         }
      }
    }else{
      compress(input,output);
    }

    if(output_flag==1) {
      if(fclose(output) == EOF){
        fprintf(stderr, "Cannot close output file\n");
        exit(EXIT_FAILURE);
      }
    }

    fprintf(stderr,"Read:  %d characters\nWritten: %d characters\nCompression ratio: %0.1f%% ",read_chars,written_chars,((double)written_chars/read_chars)*100);

    free(input_file_names);
    exit(EXIT_SUCCESS);

  }
