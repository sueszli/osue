/**
 * 
 * @file myexpand.c
 * @author Emil Schneider
 * @date 14.11.2021
 * @brief myexpand is a simple implementation of the linux "expand" command line program
 * 
 * @details myexpand replaces tabs with a spaces in a given file (or stdin)
 * 
 * 
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @brief Starting point of the program. 
 * @details The program arguments get parsed and the file-pointers are created. If there are no errors (eg. file does not exist) expandFile() is called
 * @param argc counter of arguments
 * @param argv the programs arguments and options.
 */
static char const shortopts[] = "t:o:";
static void usage(void);
static void expandFile(FILE *fp,int tabstop,FILE *output);

int main(int argc, char **argv){

    
    int c;
    int tabstop = 8;
    FILE *outputFile = NULL;
    
    while((c = getopt(argc, argv, shortopts)) != -1){
        
        switch(c)
        {
            case 't':
                if(isdigit(*optarg))
                {
                    tabstop = strtol(optarg, NULL, 0);

                    if(tabstop < 1){
                        
                        usage();
                        exit(EXIT_FAILURE);
                    }
                    
                }else{
                
                    usage();
                    return 1;
                }
                break;
            case 'o':
                outputFile = fopen(optarg,"w");
                if(outputFile == NULL){
                
                    usage();
                
                }
                break;
            default:
                
                usage();
                break;
        }
    }


    if(argv[optind] != NULL){
        FILE *fp = fopen(argv[optind],"r");
        if(fp == NULL){
            printf("File not found\n");
            exit(EXIT_FAILURE);
        }
        expandFile(fp,tabstop,outputFile);
    }else{
        usage();
    }

    exit(EXIT_SUCCESS);
}    

/**
 * @brief Function to iterate through a file and replacing tabs with spaces
 * @details Depending wheather output is NULL or not, the new text gets either printed to stdout or saved into output (as normal text)
 * @param fp Pointer to input file
 * @param tabstop The number of spaces to be placed instead of a tab
 * @param output Pointer to output file (Leave NULL if function should write to stdout)
 */
static void expandFile(FILE *fp, int tabstop, FILE *output){

    
    char c;
    int x = 0;
    
    if(output == NULL){
        
        while((c = fgetc(fp)) != EOF){
            if(c == '\t')
            {
                int p = tabstop * ((x / tabstop) + 1);
                while(x < p){
                    printf(" ");
                    x++;
                }
            }else{
                if(c == '\n'){
                    x = 0;
                }else{
                    x++;
                }
                printf("%c", c);
            }
        }
        
        fclose(fp);
    }else{
         while((c = fgetc(fp)) != EOF){
            if(c == '\t')
            {
                int p = tabstop * ((x / tabstop) + 1);
                while(x < p){
                    fwrite(" ",1,sizeof(" "),output);
                    x++;
                }
            }else{
                if(c == '\n'){
                    x = 0;
                }else{
                    x++;
                }
                fwrite(&c,1,sizeof(c),output);
            }

        }

        fclose(fp);
        fclose(output);
    }
    

}

/**
 * @brief Function to print basic usage of program. 
 */
static void usage(){
    fprintf(stderr, " Usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}