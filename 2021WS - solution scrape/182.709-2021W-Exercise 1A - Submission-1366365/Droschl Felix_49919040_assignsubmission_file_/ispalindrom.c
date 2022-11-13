/**
 * @file ispalindrom.c
 * @author Felix Droschl 11840546
 * @date 14.11.2021
 *
 * @brief ispalindrom
 *
 * This program decides for each of a given inputs lines, if they are a palindrom.
 **/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



//types

//structure used for saving command line arguments
typedef struct options{
  int s;
  int i;
  FILE * output;
  int n;

}options;
 

//functions


//removes all blank spaces of the given line
static void remove_blank(char* s){
    char* temp = s;
    do {
      while (*temp == ' ') {
	++temp;
        }
    } while (*s++ = *temp++);
	
}
//makes the line lowercase
static void lowercase(char * s){
  while(*s != '\0' ){
    *s = tolower(*s);
    s++;
  }
}
//decides if a string is a palindrom
static int palindrom_check(char* s){
  
  int first = 0;
  int last = strlen(s)-1; 
  while(last > 0){
    
    char  b = s[first];
    char  c = s[last];

    if(c!=b){
      return 0;
    }
    first++;
    last--;
  }
    
  return 1;
}


//changes the line according to the options and then checks if the changed line is a palindrom. After that this procedure prints the original line together with a string that tells if it is a palindrom.
static void  change_line(const char* line,int s, int i, options o){
       char* copy = strdup(line);
       if(line == NULL){
	 fprintf(stderr,"ispalindrom: ERROR: change_line cant work with line  NULL as input");
	 exit(EXIT_FAILURE);
       }
       
       if(i==1){
	 lowercase(copy);
       }
       if(s==1){
	 remove_blank(copy);
       }
       
      
       if(palindrom_check(copy)==1){
	 fprintf(o.output,"%s is a palindrom\n", line);
       }
       else{
	 fprintf(o.output,"%s is no palindrom\n", line);
       }
       
      free(copy);

}
//removes \n from lines, is needed for stdin
static char*  remove_invisible_char(char*s){
  if(s[strlen(s)-1]=='\n'){
    s[strlen(s)-1]='\0';
  }
  return s;

}
//reads all the lines from a given input file and runs change_line with each of them, for stdin this procedure runs until the program is stopped 
static void file_reader(FILE * input,int s, int i, options o){
	char * line = NULL;
	size_t len = 0;
	while ((getline(&line, &len, input)) != -1) {
	  
	  if(strlen(line)==0){
	    continue;
	  }
	  line = remove_invisible_char(line);
  
	  change_line(line, s, i,o);
	  
	
	}
	free(line);
	
	
}

//checks the command line for given options 
static void  set_opt(options* options, int argc, char ** argv){
  int specific_option;
   while((specific_option=getopt(argc,argv,"sio:"))!=-1){
    if(specific_option=='s'){
      options->s=1;
    }
    if(specific_option=='i'){
     options->i=1;
    }
    if(specific_option=='o'){
      options->output = fopen(optarg,"w");
      if(options->output==NULL){
	fprintf(stderr, "ispalindrom: ERROR: NULL is no valid output");
	exit(EXIT_FAILURE);
      }
     }
  }
  options->n=optind;
}

//main

//the main function running read file for all the given inputs, after setting the options 
int  main(int argc, char ** argv){
  options  options = {0,0,stdout,0};
  set_opt(&options, argc, argv);
  int s = options.s;
  int i = options.i;

  
  if(options.n<argc){
    int t = options.n;

    //run through all input files 
    for(;t<argc;t++){
      FILE*input = fopen(argv[t],"r");
      if(input== NULL){
	fprintf(stderr, "ERROR: cant read from NULL");
	continue;
      }
      
      file_reader(input,s,i,options);
      fclose(input);
    }
  }
  //no input files
  else{
    file_reader(stdin,s, i, options);
  }
  fclose(options.output);
  exit(EXIT_SUCCESS);		

}


