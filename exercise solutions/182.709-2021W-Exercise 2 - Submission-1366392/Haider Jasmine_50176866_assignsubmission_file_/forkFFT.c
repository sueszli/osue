/**
*@file mygrep.c
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 11.12.2021
*
*@brief implements the Cooley-Tukey Fast Fourier Transform algorithm.
*
* TODO! The mygrep module. Checks if a keyword is contained in lines of a file or stdin.
**/

#include "forkFFT.h"
#include <float.h>
#include <errno.h>
#include <regex.h>
#include <string.h>

/** program name.
 * @brief this name will be used for error messages.
 **/
char *prog_name;

pid_t pid_even = 0;
pid_t pid_odd = 0;
int even_read = 0;
int even_write = 0;
int odd_read = 0;
int odd_write = 0;
char *nextline = NULL;
FILE *even_stream = NULL;
FILE *odd_stream = NULL;


/**
 *  usage function.
 * @brief In case the arguments when calling the program were invalid, this function prints what they should be.
 * @details global variables: prog_name
 **/
static void
usage (void)
{
  fprintf (stderr, "Usage %s does not take any arguments or positional parameters \n",
	   prog_name);
  exit (EXIT_FAILURE);
}

static void fork_exec(int oddeven){

    int pipefd_ptoc[2]; //parent to child
    int pipefd_ctop[2]; //child to parent
    pipe(pipefd_ptoc);
    pipe(pipefd_ctop);

    pid_t pid = fork();

    switch (pid) {
        case -1:
            //fork failed
            exit(EXIT_FAILURE);
        case 0: //child
            close(pipefd_ptoc[1]); //close unused write end
            dup2(pipefd_ptoc[0], STDIN_FILENO); //map stdin to read end
            close(pipefd_ctop[0]); //close unused read end
            dup2(pipefd_ctop[1], STDOUT_FILENO); //map stdout to write end
            //execlp
            execlp(prog_name, prog_name, NULL);
            fprintf(stderr, "%s something went wrong with execlp, this should never be reached!\n", prog_name);
            break;
        default: //parent
            close(pipefd_ptoc[0]); //close unused read end
            close(pipefd_ctop[1]); //close unused write end
            if(oddeven == 0){
                even_write = pipefd_ptoc[1];
                even_read = pipefd_ctop[0];
            }
            else{
                odd_write = pipefd_ptoc[1];
                odd_read = pipefd_ctop[0];
            }
            break;
    }

    if(oddeven == 0){
        pid_even = pid;
    }
    else{
        pid_odd = pid;
    }

}

static void cleanup (void){

    if(nextline != NULL){
        free(nextline);
    }

    close(even_read);
    close(even_write);
    close(odd_read);
    close(odd_write);
    if(even_stream != NULL){
        fclose(even_stream);
    }
    if(odd_stream != NULL){
        fclose(odd_stream);
    }

}

static void forfkFFT_write(void){}
static void forfkFFT_read(void){}

int main (int argc, char *argv[]){
    prog_name = argv[0];
    size_t len = 0;
    ssize_t read;

    float oddval = 0.0;
    float evenval = 0.0;
    int counter = 0;

    char *endpointer = NULL;

    if (atexit(cleanup) != 0) {
       fprintf(stderr, "cannot set exit function\n");
       exit(EXIT_FAILURE);
    }


    if(argc != 1){
        usage();
    }

    pid_t my_pid = getpid();

    while((read = getline(&nextline, &len, stdin)) != -1){
        if(*nextline == EOF){ //end of file reached, stop reading
            break;
        }

        evenval = strtof(nextline, &endpointer);
        if(strcmp(endpointer, "\n") != 0){
            fprintf(stderr, "%s exit on error - invalid input!\n", prog_name);
            exit(EXIT_FAILURE);
        }
        counter++;
        if((read = getline(&nextline, &len, stdin)) != -1){
            oddval = strtof(nextline, &endpointer);
            if(strcmp(endpointer, "\n") != 0){
                fprintf(stderr, "%s exit on error - invalid input!\n", prog_name);
                exit(EXIT_FAILURE);
            }
            counter++;
        }

        else{
            if (feof(stdin)){
                if(counter == 1){//If the array consists of only 1 number, write that number to stdout and exit with exit status EXIT_SUCCESS.
                    fprintf(stdout, "%f 0.0*i\n", evenval);
                    exit(EXIT_SUCCESS);
                }
                else{//Terminate the program with exit status EXIT_FAILURE if the length of the array is not even.
                    fprintf(stderr, "%s exit on error: number of input values not even\n", prog_name);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                fprintf(stderr, "%s exit on error: getline failed %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        //fork and execlp
        if(pid_even == 0){//no fork has been executed yet
            fork_exec(0);
            fork_exec(1);

            even_stream = fdopen(even_write, "w");
            if(even_stream == NULL){
                fprintf(stderr, "%s exit on error: fdopen failed %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            odd_stream = fdopen(odd_write, "w");
            if(odd_stream == NULL){
                fprintf(stderr, "%s exit on error: fdopen failed %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

        //send floats to the children
        fprintf(odd_stream, "%f\n", oddval);
        fprintf(even_stream, "%f\n", evenval);

    }
    if(counter == 0){
        fprintf(stderr, "$%s process %d exit on error - no input received\n", prog_name, my_pid);
        exit(EXIT_FAILURE);
    }

    //close write pipes so children know there will not be any more input
    fclose(even_stream);
    fclose(odd_stream);

    //read lines from children
    even_stream = fdopen(even_read, "r");
    if(even_stream == NULL){
        fprintf(stderr, "%s exit on error: fdopen failed %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    odd_stream = fdopen(odd_read, "r");
    if(odd_stream == NULL){
        fprintf(stderr, "%s exit on error: fdopen failed %s\n", prog_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    float oddval_r = 0.0;
    float oddval_i = 0.0;
    float evenval_r = 0.0;
    float evenval_i = 0.0;
    float evil_array[counter];

    for(int i = 0; i < counter/2; i++){
        if((read = getline(&nextline, &len, even_stream)) != -1){
            if(*nextline == EOF){ //end of file reached, stop reading
                break;
            }
            evenval_r = strtof(nextline, &endpointer);
            evenval_i = strtof(endpointer, &endpointer);
        }

        if((read = getline(&nextline, &len, odd_stream)) != -1){
            if(*nextline == EOF){ //end of file reached, stop reading
                fprintf(stderr, "%d EOF from odd received in %d\n", my_pid, i);
                break;
            }
            oddval_r = strtof(nextline, &endpointer);
            oddval_i = strtof(endpointer, &endpointer);
        }

        float rk_r = evenval_r + cos(-2 * 3.141592654 / counter * i) * oddval_r - sin(-2 * 3.141592654 / counter * i) * oddval_i;
        float rk_i = evenval_i + cos(-2 * 3.141592654 / counter * i) * oddval_i + sin(-2 * 3.141592654 / counter * i) * oddval_r;
        float rkn2_r = evenval_r - (cos(-2 * 3.141592654 / counter * i) * oddval_r - sin(-2 * 3.141592654 / counter * i) * oddval_i);
        float rkn2_i = evenval_i - (cos(-2 * 3.141592654 / counter * i) * oddval_i + sin(-2 * 3.141592654 / counter * i) * oddval_r);

        //print k value directly, save k+n/2 value to return in correct order
        fprintf(stdout, "%f %f*i\n", rk_r, rk_i);
        evil_array[i*2] = rkn2_r;
        evil_array[i*2+1] = rkn2_i;

    }
    //print k+n/2 values now
    for(int i = 0; i < counter/2; i++){
        fprintf(stdout, "%f %f*i\n", evil_array[i*2], evil_array[i*2+1]);
    }

    //wait
    int status1 = 0;
    int status2 = 0;
    pid_t pid1 = wait(&status1);
    pid_t pid2 = wait(&status2);
    if((status1 != 0) || (status2 != 0)){
        fprintf(stderr, "%s exit on error - child status failure\n", prog_name);
        exit(EXIT_FAILURE);
    }
    else if(!((pid1 == pid_even)||(pid1 == pid_odd))&&((pid2 == pid_even) || (pid2 == pid_odd))){
        fprintf(stderr, "%s exit on error - wait returned pid that was not a child.\n", prog_name);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}