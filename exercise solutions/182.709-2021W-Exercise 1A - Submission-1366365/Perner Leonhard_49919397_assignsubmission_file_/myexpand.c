// @author: Leonhard Perner, 12020652 @date: 13.11.2021
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void myReadStdin(char *stemp[]);
int myReadFile(char *fname, char *stemp[]);
char* myReplace(int t, char *workstr);
char* myStringConcat(char *s1, char *s2);
int myWriteFile (char *fname, char *s);

int main(int argc, char *argv[])
{



    // Following Code Block is handeling the arguments and creates an Array pos_arg with all positional arguments
    char *t_arg = NULL;
    int tc = 0;
    int to = 0;
    char *o_arg = NULL;
    int c;
    while ( (c = getopt(argc, argv, "t:o:")) != -1){
        switch(c) {
            case 't' : t_arg = optarg; tc++;
            // printf(t_arg);
                break;
            case 'o' : o_arg = optarg; to++;
            // printf(o_arg);
                break;
            case '?' : //invalid option
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (tc > 1 || to > 1 )
    {
        fprintf(stderr, "%s: options -t and -o can only be used once\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // t...tabstop, default 8, specified using -t
    int t = 8;
    if (t_arg != NULL) 
    {       
        for (int i = 0; i < strlen(t_arg); i++)
        {
            char curr = t_arg[i];
            if ((int)curr < 48 || (int)curr > 57)
            {      
                fprintf(stderr, "%s: argument -t requires an positive int\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
        t = strtol(t_arg,NULL, 10);
    }
                
    
    // handeling positional arguments (pos_arg with pos_argc elements)
    int pos_argc = argc - optind;
    char *pos_arg[pos_argc];
    
    if (pos_argc != 0){
        for (int i = 0; i < pos_argc; i++) {
            pos_arg[i] = argv[optind + i];
        }
     }
    

    char *workstr = "";
    if (pos_argc == 0)
    {
        printf("Input: ");
        myReadStdin(&workstr);
    }else{
                    
        for(int i=0; i < pos_argc; i++){
            char *temp = "";
            if(myReadFile(pos_arg[i], &temp) == -1){
                fprintf(stderr, "%s: cant read file\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            workstr = myStringConcat(workstr, temp);
        }
    }


    char *res = myReplace(t, workstr);
    if(o_arg == NULL){
        printf("\nOutput is:\n%s\n", res);
    }else{
        if(myWriteFile(o_arg, res) == -1){
            fprintf(stderr, "%s: cant write to file\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    free(res);
    exit(EXIT_SUCCESS);


    return 0;
}


// replaces -> with spaces, t specifies amount of spaces per ->, workstr is the String wich will be modified
char* myReplace(int t, char *workstr){
    int lineStart = 0;
    int l = strlen(workstr);
    

    for (int i=0; i < l; i++){
        // printf("Reached\n");
        
        char curr = workstr[i];
        if (curr == '\n'){
            lineStart = i +1;
        }
        if (curr == '\t'){
            int p = t * (((i - lineStart) / t) + 1);
            int c = p - (i - lineStart);
            char spaces[c];
            spaces[c] = '\0';
            for(int i=0; i < c; i++){
                spaces[i] = ' ';
            }
            l = l + c -1;
            char *temp = (char*) malloc(l + 1);
            strcpy(temp, workstr);          
            strcpy(temp + i, spaces);
            strcpy(temp + i + c, workstr + i +1);
            temp[l + 1] = '\0';
            workstr = temp;           
        }
        
    }
    return workstr;
}


// uses getchar and malloc to read an String of undefined length from stdin
// end of input has to be signaled with EOF (strg + d)
void myReadStdin(char *stemp[])
{
    for(int i =0;i != -1;i++)
    {    
        if(i != 0) {
            *stemp = (char*)realloc((*stemp),i+1);
        }else{
            *stemp = (char*)malloc(i+1);
        }
        (*stemp)[i]=getchar();
        if((*stemp)[i] == EOF) {
            (*stemp)[i]= '\0';
            return;
        }
    }   
}

// reads a text file to a string
int myReadFile (char *fname, char *stemp[])
 {
   FILE *f;
   f = fopen(fname,"r");
   if(f == NULL) {
      return(-1);
   } for(int i=0; i != -1; i++) {
      if(i != 0)
            *stemp = (char*)realloc((*stemp),i+1);
        else
            *stemp = (char*)malloc(i+1);
      (*stemp)[i]= fgetc(f);
      if((*stemp)[i] == EOF){
         (*stemp)[i] = '\0';
         break;
      }
   }
   fclose(f);
   return(0);
}

// writes a string to a file
int myWriteFile (char *fname, char *s){
    FILE *f;
    f = fopen(fname, "w");
    if(f == NULL){
        return -1;
    }
    fputs(s, f);
    fclose(f);
    return 0;
}

// returns the concated string of the two input strings
char* myStringConcat(char *s1, char *s2)
{
    char *temp = (char*) malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(temp, s1);
    strcat(temp, s2);
    return temp;
}