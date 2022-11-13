/**
 * @file main.c
 * @author Nikolaus Peter 11919877 <e11919877@student.tuwien.ac.at>
 * @date 31.10.2021
 *
 * @brief Main program module.
 * 
 * This programm takes input via file or stdin and checks each line if it is a palindrom.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* myprog;

int tolower(int a);


/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 * Will result in EXIT_FAILURE.
 */
static void usage(void){
    fprintf(stderr,"Usage: %s [-s] [-i] [-o outfile] [file...]\n",myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief This function checks if the given String is a Palindrom or not. 
 * @param str The string that is checked.
 * @param ignoreWhitespaces if 0 whitespaces can change if the string is accepted as a palindrom or not, if 1 whitespaces are ignored
 * @param caseInsensitive if 0 string is checked case sensitive, if 1 string is checked case insensitive
 * @return Returns 0 if the string is not a Palindrom, 1 if it is.
 */
static int ispalindrom(char* str, int ignoreWhitespaces, int caseInsensitive){
    int skipFront = 0;
    int skipBack = 0;

    for(int i = 0; (i+skipFront)<strlen(str); i++){
        
        char a = str[i + skipFront];
        char b = str[strlen(str)-(1+i+skipBack)];

        if (caseInsensitive){
            a = tolower(a);
            b = tolower(b);
        }

        if (ignoreWhitespaces){
            //as long as one of the characters is a whitespace, it will be skiped. 
            //skipFront and skipBack keep track of the total amount skipped.
            while((a==' ')||(b==' ')){
                if (a == ' '){
                    skipFront++;
                    a = str[i + skipFront];
                    if (caseInsensitive){
                        a = tolower(a);
                    }
                }
                if (b == ' '){
                    skipBack++;
                    b = str[strlen(str)-(1+i+skipBack)];
                    if (caseInsensitive){
                        b = tolower(b);
                    }
                }
            }
        }

        if (a!=b){
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Reads one character by one by the line provided by filePointer fp and dynamicly increases memory when needed.
 * @return Returns the next line in the filePointer fp with the right amount of memory allocated.
 */
static char *inputString(FILE* fp){
    char *input;
    char c;
    int size = 8;
    int len = 0;

    input = malloc(sizeof(*input)*size);

    c = fgetc(fp);
    while((EOF!=c) && (c != '\n')){
        input[len]=c;
        len++;
        if(len==size){
            input = realloc(input, sizeof(*input)*(size*=2));
        }
        c = fgetc(fp);
    }
    input[len]='\0';
    len++;
    input = realloc(input, sizeof(*input)*len);
    return input;
}

/**
 * Program entry point.
 * @brief handles option/ argument parsing. If specified reads Inputfiles and writes Outputfiles. 
 * The Output will be each line of the Input followed by "is a palindrom" if the function ispalindrom returns 1 and "is not a palindrom" if it returns 0
 * global variables: myprog
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc,char* argv[]) {
    myprog = argv[0];
    opterr = 0;

    int ignoreWhitespaces = 0;
    int caseInsensitive = 0;
    int outfileSpecified = 0;
    int totalFiles = 0;
    char* outfile;
    int* infiles;
    infiles = (int*) malloc(0);

    int option;
    
    while ((option = getopt(argc,argv, "sio"))!=-1) {
        switch (option) {
            case 's':
                ignoreWhitespaces = 1;
                break;
            case 'i':
                caseInsensitive = 1;
                break;
            case 'o':
                outfileSpecified = 1;
                outfile = argv[optind];
                optind++;
                break;
            default:
                fprintf(stderr,"%s, Wrong option\n",myprog);
                usage();
                break;
        }
    }

    for(int i = optind; i < argc; i++){
        infiles = realloc(infiles, sizeof(infiles)+sizeof(int));
        infiles[totalFiles] = i;
        totalFiles++;
    }

    FILE * outPointer;
    if (outfileSpecified){
        outPointer = fopen(outfile, "w");
    } else {
        outPointer = stdout;
    }

    int i = 0;
    do{
        FILE * inPointer;
        if (totalFiles > 0){
            if ((inPointer = fopen(argv[infiles[i]], "r")) == NULL){
                printf("%s, %s could not be found\n",myprog,argv[infiles[i]]);
                usage();
            }
        } else {
            inPointer = stdin;
        }

        char *singleLine;

        while(!feof(inPointer)){
            singleLine = inputString(inPointer);
            if (!feof(inPointer)){
                if (ispalindrom(singleLine, ignoreWhitespaces, caseInsensitive)){
                    fprintf(outPointer,"%s is a palindrom\n", singleLine);
                } else {
                    fprintf(outPointer,"%s is not a palindrom\n", singleLine);
                }
            }
        }
        free(singleLine);
        fclose(inPointer);

        i++;
    } while (i < totalFiles);
    
    if (totalFiles > 0){
        free(infiles);
    }
    
    fclose(outPointer);

    printf("\n");
    return EXIT_SUCCESS;
}