/**
*@author Daniel Atour Nissan 11810773    
*@brief replaces tab values in text files
*@date 29.10.2021
*
*/
#include<stdio.h>
#include <stdlib.h>    
#include <getopt.h>  
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define USAGE()	fprintf(stdout,"Usage: %s [-t tabstop] [-o destination] [file...]", "myExpand");		
										
const char *fileType=".txt";
static const struct option long_options[] =
    {
        { "tab", required_argument, 0, 't' },
        { "output", required_argument,0, 'o' }
    };
extern char *optarg;
extern int optind;
int main(int argc, char **argv)
{
 
    int tabValue=8;
    int index = -1;
    bool isFileGiven=false;
    char fileName[200];
    char readFileName[200];
    while (1)
    {
   int result = getopt_long(argc, argv,"o:t:",
            long_options, &index);
        if (result == -1) break; 
        switch(result){
            case 't':
                tabValue=strtol(optarg,NULL,10);
                if(tabValue<0){
                    fprintf(stderr,"%s error:tab value must be positive \n",argv[0]);
                   exit(EXIT_FAILURE);
                }
               // printf("%d",tabValue);
               // printf("'t' was specified. Arg: `<%s>`\n", optarg);
                break;
            case 'o':
                isFileGiven=true;
                strcpy(fileName,optarg);
               // printf("'o' was specified. Arg: `<%s>`\n", optarg);
                break;
            default:
                USAGE();
                exit(EXIT_FAILURE);
                break;
            }
    }
    //in case no input file is passed calls the fuction with stdin
    if(optind>= argc){

        if(isFileGiven){
             printf("no input passed, enter value:");
            FILE * file = fopen(fileName, "a+");
            openFile(file,stdin,tabValue);
                   fclose(file);
        }
        else {
           // printf("no input passed, enter value:");
            openFile(stdout,stdin,tabValue);
        }   
       
    }
    //iterating over multiple input files
      for(; optind < argc; optind++){     
    
        strcpy(readFileName,argv[optind]);
        //printf("parsing arguments: %s\n", argv[optind]); 
   
    if(!isFileGiven){
         FILE * readFile=fopen(readFileName,"r");
         openFile(stdout,readFile,tabValue);
         fclose(readFile);
  
     }
    else{
        FILE *readFile=fopen(readFileName,"r");
        FILE *inputFile=fopen(fileName,"a+");
        openFile(inputFile,readFile,tabValue );
        fclose(readFile);
        fclose(inputFile);
        }
    }
 
exit(EXIT_SUCCESS);

} 
/**
*@brief function reads readFile charwise and replaces the char in case of tab according to the given formula.
*@param file: file into which is to be written
*@param readFile: file from which is being read.
*@param tabValue: tab Value used to calculate the amount of spaces.
*@return void
*/
void openFile(FILE * file, FILE * readFile,int tabValue){
    if(readFile == NULL){
        fprintf(stderr,"myExpand error:Input File can not be read."); 
        exit(EXIT_FAILURE);
    }
    char c;
    if (file != NULL)  {
        int position=0;
         while ((c = getc(readFile)) != EOF){
            if(c=='\t'){
                int newPosition=tabValue * ((position / tabValue) + 1);
                for(;position< newPosition;position=position+1){
                fprintf(file,"%c",' ');      
                }
            }
        position++;   
        fprintf(file,"%c",c);
        }
    }
    else{
        fprintf(stderr,"myExpand error: File can not be opend."); 
        exit(EXIT_FAILURE);
     }
}
