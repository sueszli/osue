/**
*@author: Christoph Leopold 
*@date: 16.11.2020
*/


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<ctype.h>


static void free_resources(void);
static void checkIfPalindrom(void);

FILE *input_stream = NULL;
FILE *output_stream = NULL;

char *program_name= NULL;
int whitespace;
int caseInsensitive;


/*
    main: handling of options and input and output files get opend
 */
int main(int argc, char * const argv[]){

	program_name = argv[0];
	input_stream = stdin;
    output_stream = stdout;
    int opt;
    whitespace = 0, caseInsensitive = 0;
    int sCount = 0, iCount = 0; 
	

	while ((opt = getopt(argc, argv, "sio:")) != -1) {
        switch (opt) {
            case 's':
                whitespace = 1;
                sCount++;
                if (sCount > 1)
                {
                    fprintf(stderr, "Useage: %s [-s] [-i] [-o outfile] [file...]\n", argv[0]);
                    free_resources();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'i':
                caseInsensitive = 1;
                iCount++;
                if (iCount > 1)
                {
                    fprintf(stderr, "Useage: %s [-s] [-i] [-o outfile] [file...]\n", argv[0]);
                    free_resources();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                if ((output_stream = fopen(optarg, "w+")) == NULL) {
                    fprintf(stderr, "%s Error: Could not open outfile\n", argv[0]);
                    free_resources();
                    exit(EXIT_FAILURE);
                }
                break;
            default: 
                fprintf(stderr, "Useage: %s [-s] [-i] [-o outfile] [file...]\n", argv[0]);
                free_resources();
                exit(EXIT_FAILURE);
        }
    }

	int infile_index = optind;
    
    if (infile_index < argc) {

		while (infile_index < argc) {

                if ((input_stream = fopen(argv[infile_index], "r")) == NULL) {
                        fprintf(stderr, "%s Error: Could not input file\n", argv[0]);
                        free_resources();
                        exit(EXIT_FAILURE);
                }
                checkIfPalindrom();
                infile_index = infile_index + 1;
		}
	}
	else{
		checkIfPalindrom();
	}
	free_resources();
}

/*
    checkIfPalindrom: each file gets read in line by line and checked if its a palindrom, if "whitespace" is 1 then all whitespaces are ignored 
    and if "caseInsensitive" is 1 then there is no differentiation between upper and lower case characters.
 */
static void checkIfPalindrom(void) {

     char lineBuffer[1024];
     char lineBufferS[1024];
     char line[1024];



    while(fgets(lineBuffer,1024,input_stream) != NULL){
        
        int size = 0;
        int isPalindrom = 1;
       
        for (int i = 0; i < 1024; i++)
            {
                line[i] = lineBuffer[i];
            }

        if (whitespace == 1)
        {
            int count = 0;

            
            for (int i = 0; i < 1024; i++)
            {
                if (lineBuffer[i] != ' ')
                {
                    lineBufferS[count] = lineBuffer[i];
                    count++;
                }
                
            }
            
            
            for (int i = 0; i < 1024; i++)
            {
                lineBuffer[i] = lineBufferS[i];
            }

            
               
        }
        
        


        for(int i = 0; i < 1024; i++){
            if (lineBuffer[i] != '\n' && lineBuffer[i] != 0)
            {
                size++;
            }
            else {
                break;
            }
        }

        for (int i = 0; i < 1024; i++){
            if (line[i] == '\n')
            {
                line[i] = 0;
            }
        }
    
        
        for (int i = 0; i < (size)/2; ++i)
        {
            
            if (caseInsensitive == 1)
            {
                if (tolower(lineBuffer[i]) != tolower(lineBuffer[size- 1 - i])) {
                    isPalindrom = 0;
                    break;
                }
            }
            else {
                if (lineBuffer[i] != lineBuffer[size- 1 - i]){
                    isPalindrom = 0;
                    break;
                }
            }
        }
        
        if (isPalindrom == 0)
        {
            fprintf(output_stream, "%s is not a palindrom\n", line);

        }
        else{
            fprintf(output_stream, "%s is a palindrom\n", line);
        }



    }

    free_resources();
    exit(EXIT_SUCCESS);
   		
}
/*
    free_resources: Any streams that are not closed get closed
*/
static void free_resources(void) {
    if (input_stream != NULL){
        if (input_stream != stdin) {
            if (fclose(input_stream) != 0) {
                fprintf(stderr, "%sError: Could not close input stream!\n", program_name);
            }
        }
    }

    if (output_stream != NULL
        && output_stream != stdout) {
        if (fclose(output_stream) != 0) {
            fprintf(stderr, "%sError: Could not close output stream!\n", program_name);
        }
    }

}