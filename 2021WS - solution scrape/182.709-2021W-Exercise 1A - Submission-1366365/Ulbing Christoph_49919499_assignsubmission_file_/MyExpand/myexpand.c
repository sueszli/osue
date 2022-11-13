/**
 * @name myexpand.c
 * @author Christoph Ulbing 12019872
 * @date 10.11.2021
 * 
 * @brief Main module, implements the myexpand program
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * @brief This function writes a breif description of the program to stderr and exits the program with EXIT_FAILURE
 * @param progname The name of the program
 **/
static void usage(const char* progname){
    fprintf(stderr,"%s USAGE: %s [-t tabstop] [-o outfile] [file...]\n",progname, progname);
    exit(EXIT_FAILURE);
}

/**
 * @briefThis function reads the in file and replaces each \t with 
 * a number of space characters, such that the next character is placed 
 * at the next tabstop distance and writtes the the new text in the out file
 * @param in The input file
 * @param tabstop The new tabstop distance
 * @param out The output file
 * */
static void readFile(FILE *in, int tabstop, FILE *out){
    int curpos; //the position of the current written char
    int p;

    char* linebuffer;
    size_t bufsize = 1024;
    linebuffer = (char *)malloc(bufsize*sizeof(char));

    while(getline(&linebuffer,&bufsize,in)!=-1){

        curpos = 0;
        int i = 0;
        
        while(linebuffer[i] != '\0'){
        
            if(linebuffer[i] == '\t'){
            
                p = tabstop*((curpos/tabstop)+1); //calculates the next tabstop position
        
                while(curpos < p){
                    fputc(32,out);
                    curpos++;
                }
            }else{
                fputc(linebuffer[i],out);
                curpos++;
            }
            i++;
        } 
    }
}
/**
 * @brief The main function processes the flags, opens and closes the output file 
 * and the input files
 * @param argc
 * @param argv
 * @return On success returns EXIT_SUCCESS, on failure returns EXIT_FAILURE
 **/
int main(int argc, char** argv)
{
    const char* NAME = argv[0];

    int opt;
    int tabstop = 8;
    char* opt_o = NULL;
    
    int tflag = 0;
    int oflag = 0;
    //reads the flags with getopt
    while((opt = getopt(argc, argv, "t:o:")) != -1){
        switch (opt)
        {
        case 't':   
            if(tflag == 0){
                tabstop = strtol(optarg,(char**) NULL,10);
                tflag++;
            }else{
                fprintf(stderr,"%s ERROR: flag -t can only appear once\n", NAME);
                usage(NAME);
            }            
            break;
        case 'o':
            if(oflag == 0){
                opt_o = optarg;
                oflag++;
            }else{
                fprintf(stderr,"%s ERROR: flag -o can only appear once\n", NAME);
                usage(NAME);
            }
            
            break;
        default:
            usage(NAME);
        }
    }

    if(tabstop <= 0){
        fprintf(stderr,"%s ERROR: invalid value of -t\n", NAME);
        exit(EXIT_FAILURE);
    }
    
    FILE *in, *out;
    int argvpointer = optind;

    //creates an empty output file
    if(((out = fopen(opt_o,"w")) == NULL)&& opt_o != NULL){
        fprintf(stderr,"%s ERROR: creating output file %s failed\n", NAME, opt_o);
        exit(EXIT_FAILURE);
    }

    
    //iterrates over the input files
    do{
        if(optind == argc){
            in = stdin;
        }
        else if((in = fopen(argv[argvpointer],"r")) == NULL){
            fprintf(stderr,"%s ERROR: opening input file %s failed\n", NAME, argv[argvpointer]);
            exit(EXIT_FAILURE);     
        }
        
        //if no -o option is given writtes the output in stdout
        if(opt_o == NULL ){
            readFile(in,tabstop, stdout);
            fprintf(stdout,"\n");
        }else{
            if((out = fopen(opt_o,"a")) == NULL){
                fprintf(stderr,"%s ERROR: opening output file %s failed\n", NAME, opt_o);
                exit(EXIT_FAILURE);
            }else{
                readFile(in,tabstop,out);
                //if there are more then one input files writes a \n after each file except the last file
                if(argvpointer < argc-1 ){
                    fputc(10,out);
                }
                fclose(out);
            }
        }
        if(optind != argc){
            fclose(in);
        }
        
        argvpointer++;
    }while(argvpointer<argc);

    exit(EXIT_SUCCESS);
}

