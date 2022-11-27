/**
 * @file main.c
 * @author Johannes Ziehengraser <e1506152@student.tuwien.ac.at>
 * @date 06.11.2021
 *
 * @brief myexpand main programm
 * 
 * This program replaces tabs with a set amount of spaces in several files or stdin.
 * It is optional to change the amount of whitespaces with -t [number]. The default is 8.
 * The result will be either in stdout or if -o [filename] is set in a new file. The default is stdout.
 **/



#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

/**
 * expand function.
 * @brief expands the input and prints to output.
 * @details This function writes for each char read from in, the appropiate output to out, either the read char or whitespace.
 * @param tabCount The amount of whitespace characters a tabstop should be replaced.
 * @param in Filepointer of the input.
 * @param out Filepointer of the output.
 */
static void expand(int tabCount, FILE *in, FILE *out){
    char c = '0'; /* the currently read character */
    int charsInLineCounter = 0; 

    while(fread(&c,1,1,in) != 0){ /* Read the whole FILE char for char */
        if (c == '\t'){ /*If a tabstop is read, whitespace is written to out. Also counting characters in line.*/
            do{
                fwrite(" ",1,1,out); 
                charsInLineCounter++;
            } while ((charsInLineCounter % tabCount) != 0); /*do-while prints whitespace at least once to out. charsInLineCounter % tabCount returns the number of whitespaces that have to be added*/
        } else if (c == '\n'){ /*If a linebreak is read, charsInLineCounter gets reset and read char gets written to out*/
            charsInLineCounter = 0;
            fwrite(&c,1,1,out);
        } else {
            charsInLineCounter++; /*The charsInLineCounter gets increased and read char gets written to out*/
            fwrite(&c,1,1,out);
        }
    }
    fclose(in);
}




/**
 * Main function.
 * @brief This function reads the arguments and defines in and out for the expand function.
 * @details Checks the arguments given and if -t or -o are present and opens the files. Calls expand on every file given else it uses the stdin.
 * @param argc arguments count.
 * @param argv arguments vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main (int argc, char *argv[]){

    FILE *ofp;
    FILE *fp;
    int t_arg = 8;
    char *o_arg = NULL;
    int opt;
    int i;

    errno = 0;

    while((opt = getopt(argc, argv, "t:o:")) != -1){ /*Reading the arguments and saving the in the relevant varables*/
        switch (opt)
        {
        case 't':
            t_arg = (int) strtol(optarg,NULL,10);
            break;
        case 'o':
            o_arg = optarg;
            break;
        default:
            break;
        }
    }

    if (t_arg < 0){ /* If tabstop count is negative the programm exits failing*/
        return EXIT_FAILURE;
    }

    if (o_arg == NULL){ /* Sets output to file or stdout */
        ofp = stdout;
    } else {
        if ((ofp = fopen(o_arg, "w+")) == NULL){
            return EXIT_FAILURE;
        }
    }

    if (optind == argc){ /* If no input files are given stdin is used as input*/
        fp = stdin;
        expand(t_arg, fp, ofp);
    }

    for (i = optind; i < argc; i++){ /* Runs expand function for each file given in the arguments*/
        if ((fp = fopen(argv[i], "r")) == NULL){ /* Checks if filepath can be opened and throws error if not.*/
            fprintf(stderr, "%s> [ERROR]: File not found (%s).\n", argv[0], argv[i]);
            continue;
        }   
        expand(t_arg, fp, ofp);
    }
    fclose(ofp);
    return EXIT_SUCCESS;
}