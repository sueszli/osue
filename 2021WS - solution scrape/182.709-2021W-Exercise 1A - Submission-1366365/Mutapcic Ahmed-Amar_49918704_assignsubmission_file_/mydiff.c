/**
 *@file mydiff.c
 *@author Ahmed-Amar Mutapcic <e01428451@student.tuwien.ac.at>
 *@date 2021-11-14 
 *@brief Mydiff for OSUE exercise 1A  
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>



/**
 *@param argc The argument counter
 *@param argv The argument vecoter
 *@return EXIT_FAILURE in case of some error
 */ 


//Programm 
static char* pgm_name = "mydiff";
char buffer[1024];
char buffer1[1024];

/**
 * @brief prints out program usage text
 * @details prints to stderr
 * @return
 */
 
static void usage(void){
  fprintf(stderr, "Usage : %s [-i]  [-o outfile] file1 file2\n", pgm_name);
  exit(EXIT_FAILURE);
}

/**
 * @brief Checks whether strings from two different text files contain the same value or string
 @details global variables buffer[1024], buffer1[1024], string that contain the text from given files
 * then outputs results to the output stream.
  * @param argc The argument counter
 * @param argv The argument vector
 */
int main(int argc, char **argv){

    int c = 0, opt_i = 0, opt_o = 0; //Tracks the argument -i (case sensitive) and argument -o (output file)
    
    /*Standard input  output is given*/
    FILE *outFile = stdout;
    FILE *inFile1 = stdin;
    FILE *inFile2 = stdin;

//Iterating throug given Arguments "io:" that is is not accepting an argument 
     while ((c = getopt(argc, argv, "io:" )) != -1){
	
         switch(c) {
              /*Case when casesensitive Flag is set*/ 
             case 'i':
             
             if (opt_i>0){
                usage();
                exit(EXIT_FAILURE);

             }else{
                opt_i++;

             }

             printf("%d", opt_i );
             break;
             /*Case when the output file is given*/
            case 'o':
                opt_o++;
                printf("%d",opt_o);
                if (opt_o > 1){
                usage();
                exit(EXIT_FAILURE);
                }else{
                    opt_o++;
                     if ((outFile = fopen(optarg, "w")) == NULL ){
                     fprintf(stderr, "[%s] ERROR: fopen failed: \n", pgm_name);
                     exit(EXIT_FAILURE);
                }

            }
                break;

            case '?' : //unknown option
            break;

            case ':': //option requires an argument, but argument missing
            break;

            default :
            break;

        }
        
     }
    

          if (optind < argc) {
         //open input file1
         if ((inFile1 = fopen(argv[optind++],"r")) == NULL){
           fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", pgm_name, strerror(errno));
         }
         //open input file2
         if ((inFile2 = fopen(argv[optind++],"r")) == NULL){
           fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", pgm_name, strerror(errno));
         }

        if (optind+1 == argc) {
                     usage();
       exit(EXIT_FAILURE);
        }

            int column = 1;
    
            /**
         * for each input file do:
         *  for each line in current input file do:
         *      - consume new line character
         *      - if option i is given, turn string to lowercase
         *      - if option o is given, write to output file
         *      - check which string has the smaller length and interate over the string

         */     
            while(fgets(buffer,1024,inFile1) != NULL){
                if(buffer[strlen(buffer)-1] == '\n') {
                        buffer[strlen(buffer) - 1] = '\0';


                        if(fgets(buffer1,1024,inFile2) != NULL){
                            if(buffer1[strlen(buffer1)-1] == '\n') {
                               buffer1[strlen(buffer1) - 1] = '\0';

                                int counter = 0;
                                int characters = 0;
                                int min = 0;
                                if (strlen(buffer) <= strlen(buffer1)){
                                    min = strlen(buffer);
                                }else{
                                    min = strlen(buffer1);
                                }
                        
                                    while (counter < min)
                                
                                    {

                                        if(buffer[counter]-buffer1[counter] != 0 && opt_i==0){
                                            characters++;
                                        }
                                        if (tolower(buffer[counter]) != tolower(buffer1[counter]) && opt_i==1)
                                        {
                                            characters++;
                                        }
                                        
                                        
                                        counter++;
                                    }

                                    if(characters > 0){
                                         if (opt_o == 1)
                                         {
                                            fprintf(outFile,"Line: %d, characters: %d\n",column,characters);

                                         }
                                         //print Lines and number of characters that dont mach
                                        printf("Line: %d, characters: %d\n",column,characters);
                                    }

                            }
                        }
                }
            column++;
            }

    }
    //close the file
     if((fclose(outFile))== EOF){
        fprintf(stderr, "%s\n","Fclose input file failed");
          exit(EXIT_FAILURE);
     }
exit(EXIT_SUCCESS);
}
