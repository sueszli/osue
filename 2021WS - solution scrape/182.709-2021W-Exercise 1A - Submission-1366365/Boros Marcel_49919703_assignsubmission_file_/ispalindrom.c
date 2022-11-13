/**
 * @file ispalindrom.c
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 3.11.2021
 *
 * 
 * @brief This program checks whether strings are palindroms or not. There are two options
 * "-s" (ignoring whitespaces) and "-i" (case insensitive) which can be specified during 
 * the check. There is also the possibility, to read from the command line and write 
 * your output into a file, which can be specified with the option "-o". 
 **/



#include <unistd.h>
#include "help.h"

/**
 * @brief 
 * These two functions are mainly used for the functionality of the program. They have
 * the program name and all options as parameters.
 * @param prog The program name
 * @param opt_s Activity of option s
 * @param opt_i Activity of option i
 * @param opt_o Activity of option o
 * @param o_arg Argument for output (output File)
 * @param path  Path to the input File
 */
void readFromCommandLine(char* prog, int opt_s, int opt_i, int opt_o, char* o_arg);
void readFromFile(char* prog, char* path, int opt_s, int opt_i, int opt_o, char* o_arg);



/**
 * @brief The main function is analyzing the options list given in the input. If a certain options is read by 
 * getopt, the counter increases. Furthermore, depending on how many positional arguments are given, the input
 * from those files will be handled. Otherwise the input is coming from the command line.
 * @param argc  Argument counter
 * @param argv  Argument vector
 * @return Returns EXIT_SUCCESS
 */

int main (int argc, char * argv[]) {
    
    int opt_s = 0, opt_i = 0, opt_o = 0;
    char* o_arg = NULL;
    
    // no command line arguments or options
    if(argc == 1) {
        readFromCommandLine(argv[0],opt_s, opt_i, opt_o, o_arg);
    }
    
    
    // more arguments, possible option-list or more files
    else {
        int c;
        
        //read in the whole option list
        while( (c = getopt(argc, argv, "sio:")) != -1) {
            switch(c) {
                case 's': ++opt_s;
                    break;
                case 'i': ++opt_i;
                    break;
                case 'o': o_arg = optarg; ++opt_o;
                    break;
                case '?' : //invalid option
                    break;
                default: 
                    break;
            }
        }
        if(opt_s > 1 || opt_i > 1 || opt_o > 1) {
            //some option has been called more than once
            fprintf(stderr, "Error in %s -> some option has been called more than once",argv[0]);
            exit(EXIT_FAILURE);
        }
        
        if((argc - optind) == 0) {
            //no positional arguments specified -> input from command line 
            readFromCommandLine(argv[0],opt_s, opt_i, opt_o, o_arg);
        }
        
        if((argc - optind) >= 1) {
            //input file(s) specified
            int file_counter = argc-optind;
            while(file_counter > 0) {
                readFromFile(argv[0],argv[optind++],opt_s,opt_i, opt_o, o_arg);
                --file_counter;
            }
            
        }
        
    }
    return EXIT_SUCCESS;
}



/**
 * @brief Read input from a file and check whether it is a palindrom. If the file was
 * successfully opened all of the input is stored in a central buffer. The allocated memory
 * for the buffer is constantly increased if there is not enough space.
 */
void readFromFile(char* prog, char* path, int opt_s, int opt_i, int opt_o, char* o_arg) {
     char* line_buffer = NULL;
     char buffer[BUFFER_MAX] = "";
     int newMemory = 2;
     FILE *inputFile = NULL;
     inputFile = fopen(path, "r");
        
    //check error for file opening
    if (inputFile == NULL) {
        fprintf(stderr, "Error in %s -> fopen failed: %s (%s)\n",prog, strerror(errno),path);
        exit(EXIT_FAILURE);
    }
    
       
    line_buffer = malloc(sizeof(char) * BUFFER_MAX);   //allocating memory for the first time
        
    while(fgets(buffer, BUFFER_MAX, inputFile) != NULL) {
            
        strcat(line_buffer,buffer);
        if(!feof(inputFile)) {
            //If there is more input, memory has to be increased
            line_buffer = realloc(line_buffer,sizeof (char) * BUFFER_MAX * newMemory++); 
        }
            
    }
    
    
    separateLines(prog ,line_buffer, opt_s, opt_i, opt_o, o_arg); //Divide strings into tokens and check their properties
    free(line_buffer);
    
    if(inputFile != NULL) {
        fclose(inputFile);    
    }
    
}

/**
 * @brief Reads a line from from command line and checks if it is a palindrom. After it was checken
 * the output is written right away into stdout or in a file. Consequently this function works line
 * by line. If the buffer for a line is not huge enough, it will still be increased. 
 */


void readFromCommandLine(char* prog, int opt_s, int opt_i, int opt_o, char* o_arg) {
    char* line = NULL;
    char buffer[BUFFER_MAX] = "";
    int newMemory = 2, res = 1;
    FILE *outputFile;
    if(o_arg != NULL) {
        outputFile = fopen(o_arg, "a");
        if(outputFile == NULL) {
            fprintf(stderr, "fopen failed in %s: %s\n", prog,strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    
    
    
    while(fgets(buffer,BUFFER_MAX,stdin) != NULL) {
            
        //check whethter this is the first time to allocate memory for this line
        if(res == 1) { 
            line = NULL;
            line = calloc(BUFFER_MAX, sizeof(char));
            res = 0;
        }
            
        strcat(line,buffer);
            
        if(line[strlen(line)-1] != '\n') {
            line = realloc(line,sizeof (char) * BUFFER_MAX * newMemory++);  //new Memory if there is still input
            continue;
        }
           
        line[strlen(line)-1] = 0;               //remove newLine as last element
        char output_string[strlen(line)];
        strcpy(output_string,line);
        
        char equal = reverse(line,opt_s, opt_i); //check if string is a palindrom
        
        if(equal == 'y') {
            if(opt_o == 1) {
                char output_text[BUFFER_MAX] = "";
                strcat(output_text, output_string);
                strcat(output_text, " is a palindrom\n");
                    
                //write to file
                if(fputs(output_text, outputFile) == EOF) {
                    fprintf(stderr, "fputs failed in %s: %s\n", prog,strerror(errno));
                    exit(EXIT_FAILURE);
                }
                fflush(outputFile);  //write buffered string into file
            } 
            else {
                printf("%s is a palindrom\n", output_string);
            }
        } else {
            if(opt_o == 1) {
                char output_text[BUFFER_MAX] = "";
                strcat(output_text, output_string);
                strcat(output_text, " is not a palindrom\n");
                //write to file
                if(fputs(output_text, outputFile) == EOF) {
                    fprintf(stderr, "fputs failed in %s: %s\n", prog,strerror(errno));
                    exit(EXIT_FAILURE);
                }
                fflush(outputFile); //write buffered string into file
            } else {
                printf("%s is not a palindrom\n", output_string);
            }
        }
        newMemory = 2;
        res = 1;
        free(line);
    }
    if(outputFile != NULL) {
        fclose(outputFile);
    }
    
}


