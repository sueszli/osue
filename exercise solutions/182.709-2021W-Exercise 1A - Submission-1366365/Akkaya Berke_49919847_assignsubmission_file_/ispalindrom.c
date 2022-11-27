/**
 * @file ispalindrom.c
 * @author Berke Akkaya 11904656
 * @brief looks if the input is a palindrom
 * @date 14.11.2021
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/**
 * @brief removes all the white spaces if the user used the -s option and places an \0 so that the new string end exists
 * 
 * @param word 
 */

static void errorHandling(const char *errorMessage)
{
    fprintf(stderr, "[./ispalindrom]: %s\n ", errorMessage);
    exit(EXIT_FAILURE);
}

static void removewhitespaces(char* word) {
 for(char* pointer=word; *pointer!='\0'; pointer++) {
            if(*pointer!=' ') {
                *word=*pointer;
                ++word;
            }
        }
 *word='\0';
}
/**
 * @brief transforms all the character to lower case with the lower case method if the user used the -i option in the input
 * 
 * @param word string that was parsed onto this method
 */
static void tolowerCase(char* word) {
     while(*word!='\0') {
            *word=tolower(*word);
            word++;
        }
}
/**
 * @brief uses two pointers and looks if the values are same until they reach the middle
 * 
 * @param word string that was parsed onto this method
 * @return int returns 0 if the string is not a palindrom; returns 1 if the string is a palindrom
 */
static int palindrom(char* word) {
    char * endpointer=word+strlen(word)-1;
    for(char * startpointer=word;startpointer<=endpointer; startpointer++) {
        if(*startpointer!=*endpointer) {
            return 0;
        }
        endpointer--;
    }
    return 1;
}
/**
 * @brief checks if the line is a palindrom or not; the string is manipulated in regard to flags which have been set. After that the string is controlled
 * 
 * @param word the line that was parsed to this method
 * @param sflag sflag is 1 if the user used the -s option in the input
 * @param iflag iflag is 1 if the user used the -i option in the input
 * @param out 
 */
static void ispalindrom(char* word, int sflag, int iflag, FILE * out){
    char *checkword=strdup(word);
    if(sflag) {
        removewhitespaces(checkword);
        }
    if(iflag) {
        tolowerCase(checkword);
    }
    int i=palindrom(checkword);
    if(i==1) {
        fprintf(out,"%s is a palindrom\n", word);
    }
    else {
         fprintf(out,"%s is not a palindrom\n", word);
    }

}
/**
 * @brief extracts a line from the input and looks via the ispalindrom method if the line is a palindrome or not
 * 
 * @param input the input which the user chose(if not specified: standard=stdin)
 * @param sflag if -s has been set 1 else 0
 * @param iflag if -i has been set 1 else 0
 * @param output the ouput file which the user chose(if not specified: standard=stdout)
 */
static void  File_Extracline(FILE * input, int sflag, int iflag, FILE * output) {
    char * line=NULL;
    size_t size=0;
    while(getline(&line, &size, input)!=-1) {
        if(line[strlen(line)-1]=='\n') {
            line[strlen(line)-1]='\0';
        }
        //when a empty line is read it jumps to the next line
        if (strlen(line)==0) {
            continue;
        }
        else {
            ispalindrom(line,sflag,iflag, output);
        }
    }
}
/**
 * @brief processes the input and determines with methods above if the input has palindrome(s) or not
 * 
 * @param argc argument counter
 * @param argv argument array
 * @return int 
 */
int main(int argc, char ** argv) {
    int sflag=0;
    int iflag=0;
    FILE* out=stdout;
    int opt;
    //gets the options that are in the argument array
    while((opt=getopt(argc, argv, "sio:"))!=-1) {
        switch(opt) {
            case 's':
                sflag=1;
                break;
            case 'i': 
            iflag=1;
            case 'o':
                if(out!=stdout)
                 fclose(out);
                out=fopen(optarg, "w");
                break;
            default:
            exit(EXIT_FAILURE);
        }
    }
    if(out==NULL) {
        errorHandling("Can not open out-file");
    }
    if(optind==argc) {
        File_Extracline(stdin, sflag, iflag, out);
    }
    else{
		for(int i = optind;i<argc;i++){

			FILE * input = fopen(argv[i],"r");
			if(input == NULL){
               errorHandling("Can not read in file");
            }
						
			
			File_Extracline(input, sflag, iflag, out);
			fclose(input);
            }

			
		}
        exit(EXIT_SUCCESS);
	}



