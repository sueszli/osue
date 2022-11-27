// @author: Péter Békési
// @program name: mydiff
// @date: 2020-11-06
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h> 
#include <errno.h>
#include <stdlib.h>
#include <signal.h>


#ifdef DEBUG
#define debug(msg, arg) \
     printf("%s: %s\n", msg, arg)

#else
#define debug(msg, arg)
#endif

char *myprog;

char *line1=NULL;
char *line2=NULL;

static void print_usage_error (void) {
    fprintf(stderr, "USAGE: %s [-i] [-o outfile] [file...]\n", myprog);
}

bool i;
bool o;
FILE* write_to;

static void set_global_options(bool _i, bool _o, FILE* _write_to){
    i=_i;
    o=_o;
    write_to=_write_to;
}

static void print_error (char* error) {
    fprintf(stderr, "%s ERROR: %s\n", error);
}

static void handle_signal(int signal) {
    if (line1 != NULL) {
        free(line1);
    }
    if (line2 != NULL) {
        free(line2);
    }
    exit(EXIT_FAILURE);
}

static int option_handler(bool _i, bool _o, FILE* _write_to, int argc, char** argv){
    int c;
    int i_count=0;
    int o_count=0;
    while ((c = getopt(argc, argv, "o:i")) != -1 )
    {
        if(c=='i'){
            if(i_count<2){
              i_count++;
              _i=true;
            }else{
                
                print_error("Synopsis violation, option -i can only occur 1 times.");
                exit(EXIT_FAILURE);
            }
        }else if(c=='o'){
            if(o_count<2){
              o_count++;
              _o=true;
              _write_to=fopen(optarg,"w");
              if(_write_to==NULL){
                  print_error("File could not be opened");
                  exit(EXIT_FAILURE);
              }
              
            }
            else{
                
                fclose(_write_to);
                print_error("Synopsis violation, option -o can only occur 1 times.");
                exit(EXIT_FAILURE);
            }
        } else{
            print_error("Synopsis violation, invalid options");
            print_usage_error();
            exit(EXIT_FAILURE);
        }

    set_global_options(_i,_o,_write_to);   
    }
}


static int count_difference(char* line1, char* line2,size_t length,bool case_sensitive){
    int diff=0;
    for(int k=0; k<length;k++){
        const char* l1=&line1[k];
        const char* l2=&line2[k];
        /*char* l11=line1[k];
        //strncpy(&l11, &line1[k],1);
        char* l22=line1[k];
        //strncpy(&l22, &line2[k],1);*/
        
        //printf("l11: %c  l22: %c \n", l11,l22);
        if(strncmp(l1,l2,1)!=0){
            if(!case_sensitive){
               diff++;
            }
     }
     if(strncasecmp(l1,l2,1)!=0){
            if(case_sensitive){
               diff++;
            }
     }
    }
    return diff;
}

static int compare_lines(char* line1, char* line2,size_t length, FILE* _write_to,bool case_sensitive){
    int cmp=0;
    if (!case_sensitive){
        if(cmp=strncmp(line1,line2,length-1)!=0){
            return count_difference(line1,line2,length-1,case_sensitive);
        }
    }
    else
    {
        if(cmp=strncasecmp(line1,line2,length-1)!=0){
            return count_difference(line1,line2,length-1,case_sensitive);
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    myprog = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);

    int erg=0;
    erg=option_handler(i,o, write_to,argc,argv);

    if (write_to==NULL){
        write_to=stdout;
    }    

    //char *line1 = NULL;
    size_t len1 = 0;
    ssize_t nread1;
    //char *line2 = NULL;
    size_t len2 = 0;
    ssize_t nread2;
    
    FILE *input1 = fopen(argv[argc-2], "r");
    if(input1==NULL){
        print_error("File could not be opened");
        exit(EXIT_FAILURE);
    }
    FILE *input2 = fopen(argv[argc-1], "r");
    if(input2==NULL){
        print_error("File could not be opened");
        exit(EXIT_FAILURE);
    }
    size_t length;
    int j=0;
    int cmp=0;
    while ((nread1 = getline(&line1, &len1, input1)) != -1) {
        if ((nread2 = getline(&line2, &len2, input2)) == -1){
            break;
        }
        else{ 
            if(strlen(line1)<=strlen(line2)){
                length=strlen(line1);
            }else{
                length=strlen(line2);
            }
            j++;
            if((cmp=compare_lines(line1,line2,length,write_to,i))!=0){
                fprintf(write_to,"Line: %d, characters: %d \n", j, cmp);
            }
        }
    }

    free(line1);
    free(line2);

    if (fclose(write_to) == EOF) {
        print_error("ERROR: Failed closing output file");
        exit(EXIT_FAILURE);
    }
    if (fclose(input1) == EOF) {
        print_error("ERROR: Failed closing output file");
        exit(EXIT_FAILURE);
    }
    if (fclose(input2) == EOF) {
        print_error("ERROR: Failed closing output file");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}



