/**
 * @file mygrep.c
 * @author 12021354_Leon Rischar
 * @brief Read from File or Stdin and search for a specified keyword, print string to file or stdout if it contains keyword
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>

int terminate;

void handle_signal(int signal) {
    //fprintf(stdout,"End program\n");
    terminate = 1; 
}

/**
 * @brief turns every char in str to the coresponding char in lower case
 * 
 * @param str 
 * @return char* 
 */
char * makeLower(char str[]){
    for (size_t i = 0; i < strlen(str); ++i) {
        str[i] = tolower(str[i]);
    }
    return str;
}

/**
 * @brief reads file at path line by line and searches for keyword in casesensitive or insesitive, then write result to stdout or to output
 * 
 * @param path 
 * @param output 
 * @param keyword 
 * @param case_in 
 * @return int 
 */
int readFile(char *path,char *output, char *keyword, bool case_in){
    char buffer[1024];
    char *lowerKey = makeLower(keyword);
    FILE *in, *out;
    char *is_in_String;

    if ((in = fopen(path, "r")) == NULL){ // fopen failed
        fprintf(stderr,"Failed to open %s",path);
        exit(EXIT_FAILURE);
    }
    if(output != NULL){
        if ((out = fopen(output, "a")) == NULL){ // fopen failed
            fprintf(stderr,"Failed to open %s",output);
            exit(EXIT_FAILURE);
        }
    }
    
    while (fgets(buffer, sizeof(buffer), in) != NULL) { 
        
        if(case_in == false){ 
            is_in_String = strstr(buffer,keyword);
        }
        else{
            char bufferCopy[1024];
            strcpy(bufferCopy,buffer);
            is_in_String = strstr(makeLower(bufferCopy),lowerKey);
        }
        if (is_in_String != NULL)
        {
            if (output != NULL)
            {
                if (fputs(buffer, out) == EOF){
                    fprintf(stderr,"Failed to write to %s",output);
                }
            }
            else{
                fprintf(stdout,"%s","Output: ");
                fprintf(stdout,"%s",buffer);
            }
        }
    }
    if (ferror(in)){
        fprintf(stderr,"Failed to read from %s",output);
    }
    fclose(in);
    if (output != NULL){
        fclose(out);
    }
    
    return 0;
}

/**
 * @brief reads from stdin and searches for keyword in casesensitive or insesitive, then writes result to stdout or to output
 * 
 * @param output 
 * @param keyword 
 * @param case_in 
 * @return int 
 */
int readStdin(char *output,char *keyword, bool case_in){
    char buffer[1024];
    FILE *out;
    char *is_in_String;
    char *lowerKey = makeLower(keyword);

    if(output != NULL){
        if ((out = fopen(output, "a")) == NULL){ // fopen failed
            fprintf(stderr,"Failed to open %s",output);
            exit(EXIT_FAILURE);
        }
    }

    while(fgets(buffer,sizeof(buffer),stdin) != NULL){
        if (terminate == 1)
        {
            break;
        }

        //fprintf(stdout,"READ INPUT: %s",buffer);
        if(case_in == false){ 
            is_in_String =  strstr(buffer,keyword);
        }
        else{
            char bufferCopy[1024];
            strcpy(bufferCopy,buffer);
            is_in_String = strstr(makeLower(bufferCopy),lowerKey);
        }
        if (is_in_String != NULL)
        {
            //fprintf(stdout,"is in string: %s",buffer);
            if (output != NULL)
            {
                //fprintf(stdout,"has output: %s",buffer);
                if (fputs(buffer, out) == EOF){
                    fprintf(stderr,"Failed to write to %s",output);
                }
            }
            else{
                fprintf(stdout,"%s","Output: ");
                fprintf(stdout,"%s",buffer);
            }
        }
    }
    if (output != NULL){
        fclose(out);
    }
    return 0;
}

/**
 * @brief main function reads arguments and calls readStdin or readFile
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]){

    bool read_stdin = true;
    bool case_in = false;
    char *o_arg = NULL; 
    char *keyword;
    int opt_i = 0;
    int c;
    
    terminate = 0;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0 
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    while ( (c = getopt(argc, argv, "o:i")) != -1 ){
        switch ( c ) {
            case 'o': o_arg = optarg;
                break;
            case 'i': opt_i++;
                break;
            case '?': /* invalid option */
                break; 
        }
    }
    if((argc - optind) < 1){ //No keyword given
        fprintf(stderr,"Usage: %s [-i] [-o outfile] keyword [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if ( opt_i > 1 ){ // Option '-i' can be there once or not at all
        fprintf(stderr,"Usage: %s [-i] [-o outfile] keyword [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else{
        if (opt_i == 1)
        {
            case_in = true;
        }
    }

    keyword = argv[optind];
    char *files[argc - optind - 1];
    int counter = 0;
    for (int i = optind+1; i < argc; i++)
    {
        files[counter] = argv[i];
        counter++;
    }
    if (counter != 0 ) //There are files to read from
    {
        read_stdin = false;
    }
    
    FILE *out;
    if(o_arg != NULL){
        if ((out = fopen(o_arg, "w")) == NULL){ // fopen failed
            fprintf(stderr,"Failed to open %s",o_arg);
            exit(EXIT_FAILURE);
        }
        fclose(out);
    }
    

    /*
    printf("Parameter: \n");
    printf("Option i: %i\n",opt_i);
    printf("Output File: %s\n",o_arg);
    for (int i = 0; i < counter; i++)
    {
        printf("Input File: %s\n",files[i]);
    }
    */

    if (read_stdin == true)
    {
        readStdin(o_arg,keyword,case_in);
    }
    else{
        for (int i = 0; i < counter; i++)
        {
            readFile(files[i],o_arg,keyword,case_in);
        }
    }
    exit(EXIT_SUCCESS);
}

