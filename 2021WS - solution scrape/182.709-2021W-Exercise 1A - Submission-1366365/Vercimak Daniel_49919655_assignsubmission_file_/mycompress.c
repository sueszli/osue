/**
 *
 * @file mycompress.c
 * @author Daniel Vercimak Matrikelnummer:12023989
 * @date 10 Nov 2021
 * @brief mycompress reads strings, compresses them and writes them to an output. Statistic will be printed to sterr.
 *
 * @details mycompress takes -o outputfile, where outputfile is a  file, if no output file is given, it will  print to stdout.
 * Furthermore, the programm takes an arbitrarily number of input files as arguments. If no input files are given, mycompress reads from stdin.
 * Foreach inputfile given, mycompress reads them and compresses them , after compressing, they are written to the given output-file (or stdout).
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include<stdbool.h>
#include <sys/stat.h>

//the Output-File, globaly defined so it can be used where needed without the need to give in param
FILE *fp;
int charWriten=0;

/**
 * @brief compresses the input by given algorithm
 * @details the input is compressed by replacing the repeating chars with the char and a count of how often in a row it
 * is inputed, futher counts the number of chars it produces, if outputfile is set it prints there, if not, it prints to the console
 * @param input[] a line of input
 */
void mycompress(char input[]){
    int a = strlen(input);
    char *erg=malloc(a);
    int counter = 0;
    char setter[3];
    setter[1]='\0';
    char selected=input[0];
        for (int i = 0; i <= a; i++) { //
            if (selected != input[i] && (isalpha(selected)||isdigit(selected))) {
                setter[0] = selected;
                selected = input[i];
                strcat(erg, setter);
                sprintf(setter, "%d", counter);
                strcat(erg, setter);
                counter = 0;
            }
            counter++;
        }

        if(fp != NULL){
            fprintf(fp,"%s",erg);
        }else{
            fprintf( stdout,"%s", erg);
        }

        charWriten+=strlen(erg);
}
/**
 * @brief the file is read line by line and send to mycompress
 * @details the file is read line by line and send to mycompress additionaly the number of read Chars and some written Chars
 * is counted. Further there are some prints to count the new lines and empty lines. Those are printed into the Output file if given
 * or alternatively to the console.
 * @param filename[] is the name of the Input-File with ending
 * @return
 */
int compressFile(char filename[]){
    FILE *in_file = fopen(filename,"r");
    if (in_file == NULL)
    {
        printf("Error! Could not open file\n");
        exit(-1);
    }
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    int umb=1;
    char *string= malloc(size);
    int counter=1;
    int countChar=0;
    //while there are rows to read
    while(fgets(string, size, in_file)) {
        countChar+=strlen(string)-1;
        if (counter > 1) {
            if (strcmp(string, "\n") != 0 && *string!=13) {
                if(fp != NULL) {
                    fprintf(fp, "\n%d", umb);
                }else {
                    fprintf(stdout, "\n%d", umb);
                }
                mycompress(string);
                umb = 1;
                charWriten+=2;
            }else {
            umb = umb + 1;
        }
        }else{
            mycompress(string);
        }
        counter++;
    }
    //if end of file reached write the last new line into the output
    if(fgets(string, size, in_file)== NULL){
        if(fp != NULL) {
            countChar+=2;
            fprintf(fp, "\n%d\n", umb);
        }else{
            fprintf(stdout, "\n%d\n", umb);
            countChar+=2;
        }
        charWriten+=2;
    }
    fclose(in_file);
    return countChar;
}

/**
 * @brief the main methode is used to read the args that were given to the user,initiating some variables and printing statistics
 * @details the main methode is used to read the args that were given to the user,initiating some variables and printing statistics.
 * Further it is handled, what to do with those args.
 * @param argc counter of arguments
 * @param argv holding the program's arguments and options.
 * @return 0 if no exception thrown
 */
int main(int argc, char **argv) {
    int opt;
    // check if output file is given, and can be accessed
    while((opt = getopt(argc, argv, "o:")) != -1)  {
        switch(opt) {
            case 'o':
                fp = fopen(optarg, "w");
                if (fp == NULL)
                {
                    printf("Error opening the file %s", fp);
                    return -1;
                }
                //printf("Files: %s\n",optarg);
                //outputFile(optarg);
                break;
        }
    }
    int countChar=0;
    bool inputf=false;
    //check if input files were given
    for(; optind < argc; optind++){
        inputf=true;
        countChar+=compressFile(argv[optind]);
    }
    //if no input files were given, read from console
    if(!inputf) {
        //in order to re-use my compressFile Methode, a temporary file is created to save the userinput and deleted after the compression


        FILE *temp;
        temp = fopen("temp.txt", "w");
        char string[255];
        while (fgets(string, 255, stdin)) {
            if (strcmp(string, "\n") == 0) {
                fclose(temp);
                countChar += compressFile("temp.txt");
                remove("temp.txt");
                break;
            }
            fprintf(temp, "%s", string);
        }
    }
    //if a output file was given, close it after writing
    if(fp!=NULL){
        fclose(fp);
    }
    double comp=charWriten/(double)countChar*100;
    //printing the statistics
    fprintf( stderr, "\nRead: %d characters", countChar);
    fprintf( stderr, "\nWritten: %d characters", charWriten);
    fprintf( stderr, "\nCompression: %0.1f%%", comp);
    return 0;
}

