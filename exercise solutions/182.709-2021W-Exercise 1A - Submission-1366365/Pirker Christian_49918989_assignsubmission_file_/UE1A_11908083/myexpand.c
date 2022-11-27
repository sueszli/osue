/** 
 *  Name of module: myexpand.c
 *  @author Christian Pirker
 *  Student_ID: 11908083
 *  @date 13.11.2021
 *   
 *  @brief a reduced variation of the Unix-command myexpand, which reads in
 *  several files and replaces tabs with spaces.
 *
 *  SYNOPSIS: myexpand [-t tabstop] [-o outfile] [file...]
 * 
 *  @details: With the option -t a positive integer can be given which specifies the amount of spaces a tab
 * character is to b replaced with. By default it is set to 8 spaces. With the option -o a outfile can be given in which the output is written to.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <ctype.h>

char *name; ///< the name of the program used for error messages

static void usage(void);

static void expand(int tabstop, char *line, char *outfile);

int main (int argc, char *argv[]) {
    name = argv[0];
    int tabstop = 8; ///< the default amount of spaces a tab character is replaced with. 
    char *outfile = NULL; 
    int opt_t = 0;
    char *arg_t = NULL;
    int opt_o = 0;
    int opt;

    while((opt = getopt(argc, argv, "t:o:")) != -1){
        switch (opt) {
            case 't':
                arg_t = optarg;
                opt_t++;
                break;
            case 'o':
                outfile = optarg;
                opt_o++;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
                break;
            }
    }

/** check if the options t and o do not occure more than once and if the t argument is a positive integer */
    if((opt_t > 1) || (opt_o > 1)){
        usage();
    }

    if(arg_t != NULL){
        for(int i = 0; i < strlen(arg_t); i++){
                if(!isdigit(arg_t[i])){
                    fprintf(stderr, "%s: tabstop argument has to be an positive integer.\n", name);
                    exit(EXIT_FAILURE);
                }
        }
        tabstop = atoi(arg_t);
    }

/** The section below is for reading in lines and expanding them.
 *
 * @brief Reads in lines from stdin or from Input files and replaces all tab character with spaces. 
 * Lines then are either written to stdout or to the specified output file.
 *
 * @details If there were no input files given, lines are read from stdin. If input 
 * files are given, a loop goes through the files and reads lines from them. For each line
 * the function expand is called..
 */
    
    char *line = NULL; 
    size_t length = 0;

/* no input files are given **/
    if(argv[optind] == NULL){
        while (getline(&line, &length, stdin) != -1){
            expand(tabstop, line, outfile);
        }
        free(line);
        exit(EXIT_SUCCESS);
    }
/* input files are given **/
    else{
        int i = optind;
        while (i < argc){
            FILE *file = fopen(argv[i], "r");
            if(file == NULL){
                fprintf(stderr, " %s: fopen failed \n", name);
                exit(EXIT_FAILURE);
            }
            while (getline(&line, &length, file) != -1){
                expand(tabstop, line, outfile);
            }
            fclose(file);
            i++;  
        }
        free(line);
        exit(EXIT_SUCCESS);
    }
}

/** 
 * @brief prints a usage message
 *
 * @details prints a message conatining the program name and the correct Synopsis
 * 
 */

static void usage(void){
    fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n", name);
    exit(EXIT_FAILURE);
}

/** 
 * @brief Replaces all occurences of tab characters in a line with the amount of spaces 
 * specified in tabstop and writes the line in stdout or in a given file.
 *
 * @details a loop goes through the original line and writes the characters to a new one. 
 * If a tab character is encountered the size of the new line is adjusted with realloc
 * in order to fit the spaces in.
 *
 * @param tabstop: the amount of spaces a tab character is to be replaced with. 
 * 
 * @param line: the line where tab characters are to be replaced.
 *
 * @param outfile: a file where the changed line is written to.
 *
 */
static void expand(int tabstop, char *line, char *outfile){
    int newLength = strlen(line)+1; ///< strlen +1 to leave room for \0
    int currentIndex = 0; ///< the index in the new line where the next character in line is written to
    int p; 
    char *expandedLine = (char*) malloc(sizeof(char) * newLength); ///< for storing the expanded line
    if(expandedLine == NULL){
        fprintf(stderr, " %s: malloc failed \n", name);
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < strlen(line); i++){
        if(line[i] == '\t'){
            if(tabstop != 0){ 
                p = tabstop * ((currentIndex/tabstop) +1); ///< the next character is placed at the next multiple of the tabstop distance within the line
                if(tabstop > 1){
                    newLength += (p - currentIndex); ///< the new lenght is determined by the amount of spaces which are to be written
                    expandedLine = (char*) realloc(expandedLine, sizeof(char) * newLength);
                    if(expandedLine == NULL){
                        fprintf(stderr, " %s: realloc failed \n", name);
                        exit(EXIT_FAILURE);
                    }
                    for (int j = currentIndex; j < p; j++){
                    expandedLine[j] = ' '; ///< replace the tab character with spaces in the new line
                    }
                    currentIndex = p; 
                }else if (tabstop == 1){ ///< the size of the line has not to be adjusted because the overall amount of characters stays the same
                    expandedLine[currentIndex] = ' ';
                    currentIndex = p;
                }
            }else{ 
                newLength -= 1; ///< if tabstop = 0 the line gets smaller
            }
        }else{ 
            expandedLine[currentIndex] = line[i]; 
            currentIndex++;
        }
    }
    expandedLine[newLength-1] = '\0';

    if(outfile == NULL){
        fprintf(stdout, "%s", expandedLine);
    }else{
        FILE *output = fopen(outfile, "a");
            if(output == NULL) {
                fprintf(stderr, " %s: failed to open output file. \n",name); 
                exit(EXIT_FAILURE);
            }
            fprintf(output, "%s", expandedLine);
            fclose(output);
    }
    free(expandedLine);
}
