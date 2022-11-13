/**
 * @file myexpand.c
 * @author Márton Károlyi (e12028193@student.tuwiean.ac.a)
 * @brief Module for myexpand
 * 
 * @date 14.11.2021
 * 
 * Implementation of myexpand according to the specifications
 **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define OPTSTRING "t:o:"
#define DEFAULT_TABSTOP 8


/**
 * @brief Counts the number of tabs in a string
 * 
 * @param line The line where tabs are counted
 * @details no global variables used
 * @return int The number of tabs found
 */
static int count_tabs(const char *line){
    int i = 0, count = 0;
    while(line[i] != '\0'){//goes through the whole line counting tabs 
        if(line[i] == '\t'){
            count++;
        }
        i++;
    }
    return count;
}

/**
 * @brief Replaces all tabs with an equivalent amount of spaces
 * 
 * @param oldline The line with the tabs
 * @param newline The line where the tabs are expanded, must be long enough to accomodate the extra characters
 * @param tabstop The standard amount of spaces per tab
 * @details no global variables used
 */
static void replace_tabs(const char *oldline,char *newline,int tabstop){
    int old_ind = 0,new_ind = 0;//old_ind iterates through the oldline,new_ind through the newline
    char current;
    while((current = oldline[old_ind++]) != '\0'){//steps through the oldline
        if(current == '\t'){//if tab found expand with according amount of spaces
            int p = (tabstop * ((new_ind / tabstop) + 1)) - new_ind;
            for(int i = 0;i < p;i++){
                newline[new_ind++] = ' ';
            }
        }
        else{//step through newline otherwise
            newline[new_ind++] = current;
        }
    }
    newline[new_ind] = '\0';//indicate end of array
}

/**
 * @brief Writes the contents of the "in" file to the "out" file with the tabs expanded
 * 
 * @param in The filepointer of the file to be read from
 * @param out The filepointer of the file to be written to
 * @param tabstop The standard amount of spaces per tab
 * @details no global variables used
 * @return int code to indicate succes or failure of the operation
 */
static int expand(FILE *in,FILE *out,int tabstop){
    char *buffer = NULL;
    size_t buffer_size = 0;
    ssize_t line_length;
    line_length = getline(&buffer,&buffer_size,in);
    while(line_length >= 0){//if there are lines, expand and write them
        char new_buffer[line_length+count_tabs(buffer)*tabstop];
        replace_tabs(buffer,new_buffer,tabstop);
        if(fputs(new_buffer,out) == EOF){//if encountering error, return
            free(buffer);
            return errno;
        }
        line_length = getline(&buffer,&buffer_size,in);
    } 
    fflush(out);//cleanup after done
    free(buffer);
    return EXIT_SUCCESS;
}
/**
 * @brief Writes the synopsis to stderr
 * 
 */
static void usage(){
    fprintf(stderr,"Usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Main function of the module, reads the options and performs the specified operations
 * 
 * @param argc The argument counter
 * @param argv The argument vector
* @details no global variables used
 * @return int EXIT_SUCCESS if all operations successful,EXIT_FAILURE otherwise
 */
int main(int argc, char *argv[]) {
    // return expand(stdin,stdout,DEFAULT_TABSTOP);

    char *output = NULL;
    int tabstop = DEFAULT_TABSTOP;
    int opt;
    char *program_name = argv[0];
    FILE *outfile = stdout,*infile;
    int return_value = EXIT_SUCCESS;

    while ((opt = getopt(argc, argv, OPTSTRING)) != -1) {
        switch (opt) {
            case 't':{
                char *rest;
                long input = strtol(optarg, &rest, 10);
                    if(input <= 0 || *rest){//if the argument isn't a positive number
                        fprintf(stderr,"%s: tab size contains invalid character(s): ‘%s’\n", program_name, optarg);
                        exit(EXIT_FAILURE);
                    }
                tabstop = input;
                }                
                break;
            case 'o':
                output = optarg;
                break;
            case '?':
                fprintf(stderr,"%s: invalid option -- '%s'\n", program_name, optarg);
                usage();
                break;
            default:
                assert(0);
        }
    }

    if(output){//the the o flag set
        FILE *open;
        if((open = fopen(output,"w+")) == NULL){
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        outfile = open;
    }//otherwise write to stdout

    if(optind == argc){//if no input, read from stdin
        return expand(stdin,outfile,tabstop);
    }

    for(int i = optind; i < argc;i++){//read from all argument files sequentially, handling errors
        if((infile = fopen(argv[i],"r")) == NULL){
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            return_value = EXIT_FAILURE;
            continue;
        }
        if(expand(infile,outfile,tabstop) < 0){
            fprintf(stderr, "can't write to file: %s\n", strerror(errno));
            return_value = EXIT_FAILURE;
        }
        if(fclose(infile) == EOF){
            fprintf(stderr, "fclose failed: %s\n", strerror(errno));
            return_value = EXIT_FAILURE;
        }        
    }

    fclose(outfile);//cleanup afterwards
    return return_value;
}
