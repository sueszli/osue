#define PROGRAM_NAME "isPalindrom"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>



//Signatur
static void usage(void);



int main(int argc, char *const *argv) {

    bool whitespacesIgnored = false;
    bool notCaseSensitive = false;
    FILE *output = stdout;
    FILE *input = stdin;
    char *outputName = NULL;
	int c;






    while((c = getopt(argc,argv, "sio")) != EOF){
        switch (c)
        {
        case 's': //ignore whitespaces
            /* code */
            whitespacesIgnored = true;
            break;
        
        case 'i': //ignore lower/uppercases, so not case sensitive
            notCaseSensitive  = true; 
            break;

        case 'o':
            outputName = optarg;
            break;
            
        case '?': //invalid option
            usage();
            break;    
       
            
        }
    }

    if (outputName != NULL)
    {
        output = fopen(outputName, "w+");
        if(output == NULL){
            fprintf(stderr, "Error opening output File");
        }
    }

    if (optind >= argc) {
        isPalindrom(output, input,whitespacesIgnored,notCaseSensitive);

        if (fclose(input) < 0){
            fprintf(stderr, "Error closing input file");
            return EXIT_FAILURE;
        }
              
    } 
    else{
        for(int i = optind; i < argc; i++){
            input = fopen(argv[i],"r");
            if (input == NULL)
            {
                fprintf(stderr, "Error opening input file \n");
                return EXIT_FAILURE;
            }
            isPalindrom(output,input,whitespacesIgnored,notCaseSensitive);

            if (fclose(input) < 0)
            {
                fprintf(stderr, "Error closing input file \n");
                return EXIT_FAILURE;
            }
            
            
        }
    }
    return EXIT_SUCCESS;
  
}

void isPalindrom(FILE *output, FILE *input, bool whitespacesIgnored, bool notCaseSensitive){
    
    char *inputLine;
    size_t inputLineLen = 0;
    int line = 1;
    line = getline(&inputLine, &inputLineLen, input);

    while (line > 0) {
        int start = 0;
        int end = strlen(inputLine) - 1;
        bool isPalindrom = true;

         while(inputLine[end] == '\r' || inputLine[end] == '\n'){
             
             end--;
         }

         while (start < end) {
             char left; 
             char right;

             if ( whitespacesIgnored ) {
                 if(isspace(left)){
                     start++;
                     continue;
                 }
                 if (isspace(right)){
                     end--;
                     continue;
                 }               
                 
             }

             if(notCaseSensitive){
                 left = tolower(inputLine[start]);
                 right = tolower(inputLine[end]);
             }
			 else
			 {
				 left = inputLine[start];
				 right = inputLine[end];
			 }

             if (left != right){
               isPalindrom = false;
               break;
             }

             start++;
             end--;
         }
         if(isPalindrom){
             fprintf(output, "%s is a palindrom\n", inputLine);
         }
         else{
             fprintf(output,"%s is not a palindrom\n", inputLine);
         }
		 line = getline(&inputLine, &inputLineLen, input);
    }
	free(inputLine);
}

void usage(void) {
(void)fprintf(stderr,"Usage: [-s] [-i] [-o outfile] [file...]\n");
exit(EXIT_FAILURE);
}
