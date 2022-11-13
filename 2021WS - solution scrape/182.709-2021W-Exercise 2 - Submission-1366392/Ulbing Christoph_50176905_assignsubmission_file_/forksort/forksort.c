/**
 * @name forksort.c
 * @author Christoph Ulbing 12019872
 * @date 11.12.2021
 * @brief reads lines from stdin, and sorts them with mergesort
 * @details reads lines from stdin until an EOF is encountered. Sorts the lines with mergesort
 * by using fork to divide the input in two parts and let them sort by the childprocesses.
 * The parent process then only needs to merge the output of the childprocesses.
 * */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINE_SIZE 1024

/**
 * @brief sorts the input with mergesort
 * @details handels the input arguments, sets up the pipes to communkcate with the child process
 * creates two childprocesses with fork, writes the input into the pipes,
 * reads the output of the child processes and merges them sorted, writes the lines sorted to stdout
 * @param argc the argument count
 * @param argv the argument values, containing the input edges
 * @return reunts EXIT_SUCCESS on success and EXIT_FAILURE on failure
 * */

int main(int argc, char** argv)
{
    char* progname = argv[0];
    if(argc > 1){
        fprintf(stderr,"ERROR: %s too much arguments\n", progname);
        exit(EXIT_FAILURE);
    }

    //stdin only has one line
    char* firstline = malloc(sizeof(char*));
    fgets(firstline,MAX_LINE_SIZE,stdin);
    char* secondline = malloc(sizeof(char*));
    if(fgets(secondline,MAX_LINE_SIZE,stdin)==NULL){
        fprintf(stdout,"%s",firstline);
        exit(EXIT_SUCCESS);
    }

    //open pipes
    int fd1write[2];    //fd[0]-read from  fd[1]-write in
    if(pipe(fd1write)== -1){
        fprintf(stderr,"ERROR: %s opening pipe failed\n", progname);
        exit(EXIT_FAILURE);
    }
    int fd1read[2];
    if(pipe(fd1read)==-1){
        fprintf(stderr,"ERROR: %s opening pipe failed\n", progname);
        exit(EXIT_FAILURE);
    }
    int fd2write[2];
    if(pipe(fd2write)== -1){
        fprintf(stderr,"ERROR: %s opening pipe failed\n", progname);
        exit(EXIT_FAILURE);
    }  
    int fd2read[2];
    if(pipe(fd2read)==-1){
        fprintf(stderr,"ERROR: %s opening pipe failed\n", progname);
        exit(EXIT_FAILURE);
    }


    //open childprocess 1
    int id1 = fork();
    if(id1 == 0){

        if(dup2(fd1write[0],STDIN_FILENO)==-1){
            fprintf(stderr,"ERROR: %s dupping pipe failed\n", progname);
            exit(EXIT_FAILURE);
        }
        close(fd1write[1]);
        close(fd1write[0]); 
        close(fd2write[1]);
        close(fd2write[0]);

        if(dup2(fd1read[1],STDOUT_FILENO)==-1){
            fprintf(stderr,"ERROR: %s dupping pipe failed\n", progname);
            exit(EXIT_FAILURE);
        }
        close(fd1read[1]);  
        close(fd1read[0]);
        close(fd2read[1]);
        close(fd2read[0]);
        

        if(execlp("./forksort",argv[0],NULL)==-1){
            fprintf(stderr,"ERROR: %s child process failed execlp ./forksort\n", progname);
        }
        exit(EXIT_FAILURE);
    }

    //open childprocess 2
    int id2 = fork();
    if(id2 == 0){//ChildPorcess 2

        if(dup2(fd2write[0],STDIN_FILENO)==-1){
            fprintf(stderr,"ERROR: %s dupping pipe failed\n", progname);
            exit(EXIT_FAILURE);
        }

        close(fd1write[1]);
        close(fd1write[0]);
        close(fd2write[1]);
        close(fd2write[0]); 


        if(dup2(fd2read[1],STDOUT_FILENO)==-1){
            fprintf(stderr,"ERROR: %s dupping pipe failed\n", progname);
            exit(EXIT_FAILURE);
        }

        close(fd1read[1]);
        close(fd1read[0]);
        close(fd2read[1]); 
        close(fd2read[0]);
        
        if(execlp("./forksort",argv[0],NULL)==-1){
            fprintf(stderr,"ERROR: %s child process failed execlp ./forksort\n", progname);
        }
        exit(EXIT_FAILURE);
    }

    //close unused Pipes from parent process
    close(fd1write[0]);
    close(fd2write[0]);
    close(fd1read[1]);
    close(fd2read[1]); 



    //write first line to pipe
    write(fd1write[1],firstline,strlen(firstline));

    //write second line to pipe
    if(secondline[strlen(secondline)-1] != '\n' ){ 
            strcat(secondline,"\n");
        }
    write(fd2write[1],secondline,strlen(secondline));

    free(firstline);
    free(secondline);

    //write remaining lines alternately to the pipes
    char* line = malloc(sizeof(char*)+9);
    int writeCounter = 0;
    while ((fgets(line,MAX_LINE_SIZE,stdin)) != NULL)
    {
        if(writeCounter % 2 == 0)
        {
            write(fd1write[1],line,strlen(line));
        }
        if(writeCounter % 2 == 1)
        {

            write(fd2write[1],line,strlen(line));
        }

        writeCounter++;
    }
    free(line);

    //close write from writepipes
    close(fd1write[1]);
    close(fd2write[1]);

    
    //wait for the childprocesses
    int status = 0;
    waitpid(id1,&status,0);
    if(WEXITSTATUS(status)!= EXIT_SUCCESS){
        fprintf(stderr,"ERROR: %s waiting for child process failed", progname);
        exit(EXIT_FAILURE);
    }
    waitpid(id2,&status,0);
    if(WEXITSTATUS(status)!= EXIT_SUCCESS){
        fprintf(stderr,"ERROR: %s waiting for child process failed", progname);
        exit(EXIT_FAILURE);
    }
    

    //open files with pipe Input from the child processes
    FILE* readfile1 = fdopen(fd1read[0],"r");
    FILE* readfile2 = fdopen(fd2read[0],"r"); 
    
    char* readline = NULL;
    readline = malloc(sizeof(char*)+9);

    //read lines from readfile1    
    char** linesfile1 = malloc(sizeof(char*));
    int linenrfile1 = 0;
    while ((fgets(readline,MAX_LINE_SIZE,readfile1)) != NULL)
    {

        linesfile1 = realloc(linesfile1,sizeof(char*)*(linenrfile1+1));
        linesfile1[linenrfile1] = malloc(strlen(readline)+1);
        strcpy(linesfile1[linenrfile1],readline);
        linenrfile1++;
        

    }
    
    //read lines from readfile2
    char** linesfile2 = malloc(sizeof(char*));
    int linenrfile2 = 0;
    while ((fgets(readline,MAX_LINE_SIZE,readfile2)) != NULL)
    {
        linesfile2 = realloc(linesfile2,sizeof(char*)*(linenrfile2+1));
        linesfile2[linenrfile2] = malloc(strlen(readline)+1);
        strcpy(linesfile2[linenrfile2],readline);
        linenrfile2++;
    }


    int index1 = 0;
    int index2 = 0;
    //merge the output from the two childprocesses and write it to stdout
    while(index1 < linenrfile1 && index2 < linenrfile2){
            if(strcmp(linesfile1[index1],linesfile2[index2])<0){
                fprintf(stdout,"%s",linesfile1[index1]);
                free(linesfile1[index1]);
                index1++;
            }else{
                fprintf(stdout,"%s",linesfile2[index2]);
                free(linesfile2[index2]);
                index2++;
            }
    }
    while (index1 != linenrfile1)
    {
        fprintf(stdout,"%s", linesfile1[index1]);
        free(linesfile1[index1]);
        index1++;
    }
    while(index2 != linenrfile2){
        fprintf(stdout,"%s", linesfile2[index2]);
        free(linesfile2[index2]);
        index2++;
    }

    //closing ressources
    free(linesfile1);
    free(linesfile2);
    free(readline);
    fclose(readfile1);
    fclose(readfile2);

    //close read from readpipes
    close(fd1read[0]);
    close(fd1read[0]);
    
    
    exit(EXIT_SUCCESS);
}