/**
 * @file main.c
 * @author Ismail Yilmaz <e12019840@student.tuwien.ac.at>
 * @date 12.1.2021
 *
 * @brief Main program module.
 * 
 * This program implements mydiff, which compares two files line by line and counts the characters that are not matching.
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#define BUF_SIZE 256

void printUsageMessage();
void skipFileToNextLine(FILE* filePointer);
size_t myMin(size_t , size_t );
unsigned int getDiffCount(char* a, char* b, size_t length, bool caseSensitive);

/** @brief Programm entry point
 *  @param argc The argument counter
 *  @param argv The argument vector
 *  @returns 
 */ 
int main(int argc, char** argv){
    //defaults
    int argcShould = 3;
    char *filePathOut = NULL;
    bool caseSensitive = 1;
    bool toOutputFile = 0;

    int c = 0;
    while((c = getopt(argc, argv, "io:")) != -1){
        switch(c){
            case 'o': {
                filePathOut = optarg;
                toOutputFile = 1;
                argcShould += 2;
            }break;
            case 'i': {
                caseSensitive = 0;
                argcShould++;
            }break;
            default: {
                printUsageMessage();
                return -1 ;
            }
        }
    }
    if(argcShould != argc){
        printf("Wrong number of Arguments (expected %d, got %d)\n",argcShould,argc);
        printUsageMessage();
        return;
    }
    
    char *filePathA = argv[argc - 2];
    char *filePathB = argv[argc - 1];
    FILE* fa,*fb, *fo;
    
    fa = fopen(filePathA,"r");
    if(fa == NULL){
        printf("Error opening %s: %s \n",filePathA,strerror(errno));
        exit(1);   
    }
    fb = fopen(filePathB,"r");
    if(fb == NULL){
        printf("Error opening file: %s (%s)\n",filePathB, strerror(errno));
        exit(1);   
    }
    if(toOutputFile){
        fo = fopen(filePathOut,"w+");
        if(fo == NULL){
            printf("Error opening file: %s (%s)\n",filePathOut, strerror(errno));
            exit(1);
        }
    }
    
    char bufferA[BUF_SIZE];
    char bufferB[BUF_SIZE];
    int i = 1;
    int diffCount = 0;
    int lineNrA = 0;
    int lineNrB = 0;    
    int (*strncmp_ptr)(const char*, const char*, size_t) = caseSensitive ? &strncmp : &strncasecmp;
    FILE* outStream = toOutputFile? fo:stdout;  //write to stdout or outPutFile
    
    while (fgets(bufferA, BUF_SIZE - 1, fa)){    //while fileA has content
        char* handler = fgets(bufferB, BUF_SIZE - 1, fb);
        if(!handler){   //fileB out of content
            break;
        }
        size_t shorterLength = myMin(strlen(bufferA),strlen(bufferB));
        bool aLineFin = strchr(bufferA,'\n') != NULL; 
        bool bLineFin = strchr(bufferB,'\n') != NULL;
        
        /*Compare read segments with selected function*/
        if((*strncmp_ptr)(bufferA, bufferB, shorterLength) != 0){   
            diffCount += getDiffCount(bufferA,bufferB,shorterLength, caseSensitive);
        }
        if(aLineFin && !bLineFin){    //fileA end of Line
            skipFileToNextLine(fb);
        }
        if(!aLineFin && bLineFin){    //fileB end of line
            skipFileToNextLine(fa);
        }
        if(aLineFin || bLineFin){
            if(diffCount > 0){        //write line in outFile or print
                fprintf(outStream,"Line %d, characters: %d\n",i,diffCount);
                diffCount = 0;
            }
            i++;
        }
    }
    exit(EXIT_SUCCESS);
    return 1;
}
void printUsageMessage(){
    printf("mydif [-i] [-o outfile] file1 file2\n");
    return;
}

/** @brief advances file-pointer till next line
    @param filePointer the pointer to the file
*/
void skipFileToNextLine(FILE* filePointer){
    char buffer[BUF_SIZE];
    while(fgets(buffer, BUF_SIZE-1, filePointer)){
        if(strchr(buffer,'\n') != NULL)
            break;
    }
    return;
}
/** @brief returns the minimum of the 2 provided numbers
 *  @param a the first number
 *  @param b the second number
 *  @returns the smaller of a and b
 */
//returns the smaller of the 2 given numbers
size_t myMin(size_t a, size_t b){
    return a > b ? b : a;  
}
/** @brief compares to strings up to length characters, and counts the characters that don't match 
 *  @param a the first string
 *  @param b the second string
 *  @param length the length up to which the comparison should be made
 *  @param caseSensitive specifies if case sensitive or not
 *  @returns the number of differing characters 
 */
unsigned int getDiffCount(char* a, char* b, size_t length, bool caseSensitive){
    unsigned int count = 0;
    char charsIgnored[] = "\n";
    for(int i = 0; i < length; i++){
        char toCompareA = caseSensitive ? a[i] : tolower(a[i]);
        char toCompareB = caseSensitive ? b[i] : tolower(b[i]);
        if(toCompareA != toCompareB){
            if( strchr(charsIgnored,toCompareA) || strchr(charsIgnored, toCompareB))    
                break;  //stop comparing when linebreak is found
            count++;
        }
    }
    return count;
}

