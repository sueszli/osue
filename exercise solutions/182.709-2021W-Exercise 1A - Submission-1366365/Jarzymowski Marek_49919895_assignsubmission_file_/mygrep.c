#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

static char* thisProgramName;

void greping(char* keyword, FILE* inputFile, FILE* outputFile, bool case_sensitive);

int main(int argc, char **argv){
    thisProgramName = argv[0];
    
    int opt;
    char* outFile = NULL;
    int i_flag = 0;
    int o_flag = 0;
    bool case_sensitive = true;
    bool sthWentWrong = false;

    //We read our arguments untill there's none left.
    while((opt = getopt(argc,argv, "io:")) != -1){
        
        switch(opt){ 
            //We write to outfile
            case 'o':
                if(outFile != NULL){
                    fprintf(stderr, "Wrong use of: mygrep [i] [-o filename] word [filename...]\n");
                    sthWentWrong = true;
                    break;
                }
                o_flag++;
                outFile = optarg;
                break;
            case 'i':
                i_flag++;
                case_sensitive = false;
                break;
            default:
                fprintf(stderr, "Wrong use of: mygrep [i] [-o filename] word [filename...]\n");
                sthWentWrong = true;
                break;
        }
    }

    if(sthWentWrong == true){
        fprintf(stderr, "Sth went wrong");
        exit(EXIT_FAILURE);
    }

    //There can be max 1 -i and -o option.
    if(i_flag > 1 || o_flag > 1){
        fprintf(stderr, "Wrong use of: mygrep [i] [-o filename] word [filename...]\n");
        exit(EXIT_FAILURE);
    }

    char* keyword = NULL;
    int inputFileIndex = 0;

    //int argumentIndex = optind;
    if(i_flag == 1 && o_flag == 1){
        keyword = argv[4];
        inputFileIndex = 5;
    } else if (i_flag == 1){
        keyword = argv[2];
        inputFileIndex = 3;
    } else if (o_flag == 1){
        keyword = argv[3];
        inputFileIndex = 4;
    } else if (i_flag == 0 && o_flag == 0){
        keyword = argv[1];
        inputFileIndex = 2;
    }

    FILE* inputFile = stdin;
    FILE* outputFile = stdout;

    if(inputFileIndex == argc || argc == 1){
        if(o_flag == 1){
                fprintf(stderr, "There can't be an output file if there's no input file. Try again.\n");
                exit(EXIT_FAILURE);
        }
        greping(keyword, stdin, outputFile, case_sensitive);
    } else {
        if(o_flag == 1){
            if(outputFile != NULL){
                outputFile = fopen(outFile, "w");
                if(outputFile == NULL){
                    fprintf(stderr, "mygrep [i] [-o filename] word [filename...]\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        while(inputFileIndex < argc){
            inputFile = fopen(argv[inputFileIndex], "r");
            if(inputFile == NULL){
                fprintf(stderr, "Couldn't open inputFile on index %i", inputFileIndex);
                exit(EXIT_FAILURE);
            }
            greping(keyword, inputFile, outputFile, case_sensitive);
            if(fclose(inputFile) != 0){
                fprintf(stderr, "Sth went wrong with fclose");
                exit(EXIT_FAILURE);
            }
            inputFileIndex++;
        }
    }
    exit(EXIT_SUCCESS);
}


void greping(char* keyword, FILE* inputFile, FILE* outputFile, bool case_sensitive){
    char* buffer = NULL;
    size_t buffer_len = 0, read_len = 0;
    while((read_len = getline(&buffer, &buffer_len,inputFile))!= -1){
        char* theLine = strdup(buffer);
        char* theKeyword = strdup(keyword);
        bool wordIsContained = false;
        if(theLine == NULL || theKeyword == NULL){
            fclose(inputFile);
            fclose(outputFile);
            exit(EXIT_FAILURE);
        }
        if(case_sensitive == true){
            wordIsContained = (strstr(theLine, theKeyword)) != NULL;
        } else {
            for(size_t i = 0; theLine[i]; i++){
                theLine[i] = tolower(theLine[i]);
            }
            for(size_t i = 0; theKeyword[i]; i++){
                theKeyword[i] = tolower(theKeyword[i]);
            }
            wordIsContained = (strstr(theLine, theKeyword)) != NULL;
        }

        if(wordIsContained == true){
            if(fputs(buffer, outputFile) == EOF){
                free(theLine);
                free(buffer);
                free(theKeyword);
                fclose(inputFile);
                fclose(outputFile);
                exit(EXIT_FAILURE);
            }
        }

        free(theLine);
        free(buffer);
        free(theKeyword);
        buffer_len = 0;
    }

    free(buffer);

}



