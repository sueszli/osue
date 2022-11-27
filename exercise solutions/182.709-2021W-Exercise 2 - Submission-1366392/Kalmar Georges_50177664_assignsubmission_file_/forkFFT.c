/**
 * @file main.c
 * @author Georges Kalmar 9803393
 * @date 10.12.2021
 *
 * @brief forkFFT program
 * 
 * This program collects the float inputs and calculate the spectral coefficients using the FFT Algorithm.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#define PI (3.141592654)
#define SIZE_BUFFER (1024)

static char* program_name = "notyetset";

/** 
 * @brief Function that exits the program.
 * @details This function is called if an error occured, in this case the program cannot continue the tasks properly.
 * Therefore it prints the process ID, name of the program, error message and exits the program with EXIT_FAILURE.
 * @param msg is used to give the function the error message
 **/
static void abortProg(char* msg){
	fprintf(stderr,"pid: %d: Error: %s: %s\n", getpid(),program_name,msg);
	exit(EXIT_FAILURE);
}

/** 
 * @brief Function that closes all open file descriptors and file pointers and exits the program.
 * @details This function is called if an error occured, in this case the program cannot continue the tasks properly.
 * Therefore prints the process ID, name of the program, error message and closes all open file descriptors and 
 * file pointers. Then exits the program with EXIT_FAILURE.
 * @param msg is used to give the function the error message
 **/
static void abortProg_arg(char* msg,int fd1,int fd2,int fd3,int fd4,FILE* fp1,FILE* fp2,FILE* fp3,FILE* fp4){
    fprintf(stderr,"pid: %d: Error: %s: %s\n", getpid(),program_name,msg);
	if(fd1 != 0){if((close(fd1)) == -1){fprintf(stderr,"couldn't close fd1\n");}}
	if(fd2 != 0){if((close(fd2)) == -1){fprintf(stderr,"couldn't close fd2\n");}}
	if(fd3 != 0){if((close(fd3)) == -1){fprintf(stderr,"couldn't close fd3\n");}}
	if(fd4 != 0){if((close(fd4)) == -1){fprintf(stderr,"couldn't close fd4\n");}}
	if(fp1 != 0){if((fclose(fp1)) == EOF){fprintf(stderr,"couldn't close fp1\n");}}
	if(fp2 != 0){if((fclose(fp2)) == EOF){fprintf(stderr,"couldn't close fp2\n");}}
	if(fp3 != 0){if((fclose(fp3)) == EOF){fprintf(stderr,"couldn't close fp3\n");}}
	if(fp4 != 0){if((fclose(fp4)) == EOF){fprintf(stderr,"couldn't close fp4\n");}}
	exit(EXIT_FAILURE);
}

/** 
 * @brief Function that calculates and returns the R[k] real part
 * @details This function calculates the R[k] real part following the rules of complex numbers:
 * (a1+b1i) + (a2+b2i)*cos() + (a2+b2i)*i*sin() = [a1 + a2*cos()-b2*sin] +[b1+b2*cos()+a2*sin()]*i
 * real part: [a1 + a2*cos()-b2*sin]
 * @param a1 is the real part of the first complex number
 * @param b1 is the imag part of the first complex number
 * @param a2 is the real part of the second complex number
 * @param b2 is the imag part of the second complex number
 * @param k is the integer that describes the position in the array 
 * @param n is the integer that describes the size of the array
 * @return The functions returns the calculated R[k] real part
 **/
static double fft_real_left (double a1, double b1, double a2, double b2, int k, int n){
    return (a1 + a2*cos(-2*PI*k/n)-b2*sin(-2*PI*k/n));
}

/** 
 * @brief Function that calculates and returns the R[k] imag part
 * @details This function calculates the R[k] imag part following the rules of complex numbers:
 * (a1+b1i) + (a2+b2i)*cos() + (a2+b2i)*i*sin() = [a1 + a2*cos()-b2*sin] +[b1+b2*cos()+a2*sin()]*i
 * imag part: [b1+b2*cos()+a2*sin()]
 * @param a1 is the real part of the first complex number
 * @param b1 is the imag part of the first complex number
 * @param a2 is the real part of the second complex number
 * @param b2 is the imag part of the second complex number
 * @param k is the integer that describes the position in the array 
 * @param n is the integer that describes the size of the array
 * @return The functions returns the calculated R[k] imag part
 **/
static double fft_imag_left (double a1, double b1, double a2, double b2, int k, int n){
    return (b1 + b2*cos(-2*PI*k/n)+a2*sin(-2*PI*k/n));
}

/** 
 * @brief Function that calculates and returns the R[k+n/2] real part
 * @details This function calculates the R[k] real part following the rules of complex numbers:
 * (a1+b1i) - (a2+b2i)*cos() - (a2+b2i)*i*sin() = [a1 - a2*cos()+b2*sin] +[b1-b2*cos()-a2*sin()]*i
 * real part: [a1 - a2*cos()+b2*sin]
 * @param a1 is the real part of the first complex number
 * @param b1 is the imag part of the first complex number
 * @param a2 is the real part of the second complex number
 * @param b2 is the imag part of the second complex number
 * @param k is the integer that describes the position in the array 
 * @param n is the integer that describes the size of the array
 * @return The functions returns the calculated R[k+n/2] real part
 **/
static double fft_real_right (double a1, double b1, double a2, double b2, int k, int n){
    return (a1 - a2*cos(-2*PI*k/n)+b2*sin(-2*PI*k/n));
}

/** 
 * @brief Function that calculates and returns the R[k+n/2] imag part
 * @details This function calculates the R[k] imag part following the rules of complex numbers:
 * (a1+b1i) - (a2+b2i)*cos() - (a2+b2i)*i*sin() = [a1 - a2*cos()+b2*sin] +[b1-b2*cos()-a2*sin()]*i
 * imag part: [b1-b2*cos()-a2*sin()]
 * @param a1 is the real part of the first complex number
 * @param b1 is the imag part of the first complex number
 * @param a2 is the real part of the second complex number
 * @param b2 is the imag part of the second complex number
 * @param k is the integer that describes the position in the array 
 * @param n is the integer that describes the size of the array
 * @return The functions returns the calculated R[k+n/2] imag part
 **/
static double fft_imag_right (double a1, double b1, double a2, double b2, int k, int n){
    return (b1 - b2*cos(-2*PI*k/n)-a2*sin(-2*PI*k/n));
}

/** 
 * @brief Function that converts a string into a complex number.
 * @details This function converts a string into a and b of a complex number (a+b*i).
 * The real and imag parts are stored in two separate double variables. No error checking as the client ensures
 * the structure of the string, in the form "a b". 
 * @param str is string that is converted
 * @param res1 is the double pointer where the real part is stored
 * @param res2 is the double pointer where the imag part is stored
 **/
static void string_to_complex(char*str,double*res1, double*res2){
    char* endptr1;
    char* endptr2;
    *res1 = strtod(str,&endptr1);
    endptr1 = (endptr1 + 1);
    *res2 = strtod(endptr1,&endptr2);
}

/** 
 * @brief closes the pipe ends and sets the open pipe ends accordingly
 * @details This function closes the pipe ends in the children processes and sets the open pipe ends accordingly.
 * If one of the tasks fail the function returns -1 and the whole program must be aborted.
 * @param pipe1 is the first pipe
 * @param pipe2 is the second pipe
 * @return The function returns 0 on success and -1 on error
 **/
static int close_and_dup2(int* pipe1,int* pipe2){
    if(close(pipe1[1]) == -1){fprintf(stderr,"close pipe end didn't work\n");return -1;}
    if(close(pipe2[0]) == -1){fprintf(stderr,"close pipe end didn't work\n");return -1;}
    if(dup2(pipe1[0],0) == -1){fprintf(stderr,"dup2 didn't work\n");return -1;}
    if(dup2(pipe2[1],1) == -1){fprintf(stderr,"dup2 didn't work\n");return -1;}
    return 0;
}

/** 
 * @brief Function that checks if n is in the form 2^k
 * @details This function checks if n is in the form 2^k and if so returns 0. If not -1 is returned. In this
 * case the whole program must be aborted as the FFT Algorithm doesn't work.
 * It checks all valid options of 2^k that exist using an integer varialbe.
 * @param n is the integer that describes the size of the array
 * @return The function returns 0 on success and -1 on error
 **/
static int check_n(int n){
    int i = 2;
    while(1){
        if (n == i){
            return 0;
        }
        int newi = i*2;
        if(newi<i){return -1;}    
        i = newi;
    }
}
/** 
 * @brief Main gets the inputs from stdin and starts to fork recursively and calculate the spectral coefficients using FFT.
 * @details Main gets the input floats from stdin and forks into two children even and odd, sending each of them half of the data via stdin.
 * The children execute main after the fork, and so they create new children and become parents too. This is done recursively until only one
 * input is received. Then the FFT Algorithm is used to calculate the complex numbers and send them back to their parents until only one
 * parent process has collected the final values. These complex spectral coefficents are the printed via stdout. 
 * @param argc Stores the amount of arguments
 * @param argv Stores the text string of the arguments in an array
 * @return The program return EXIT_SUCCESS on success or returns EXIT_FAILURE in case of errors 
 **/
int main(int argc, char*argv[]){
    program_name = argv[0];
    char buffer[SIZE_BUFFER];
    char* endptr1;
    double inp1;
    int c;
    
    if(argc>1){abortProg("No options and arguments allowed, Synopsis: forkFFT");}
    if(fgets(buffer,sizeof(buffer),stdin) != NULL) {
        inp1 =strtod(buffer,&endptr1);
        if(*endptr1 !='\n'){abortProg("invalid input: only floats separated by next line are allowed");}
    }
    if((c=fgetc(stdin)) == EOF){
         printf("%f 0.0*i\n",inp1);
         exit(EXIT_SUCCESS);
    }
    else{
         ungetc(c,stdin);
    }

    int pipe_p_c_even[2];
    int pipe_c_p_even[2];
    int pipe_p_c_odd[2];
    int pipe_c_p_odd[2];
    //open the pipes to the even child
    if(pipe(pipe_p_c_even) == -1){abortProg("could not create pipe");}
    if(pipe(pipe_c_p_even) == -1){abortProg_arg("could not create pipe",pipe_p_c_even[0],pipe_p_c_even[1],0,0,0,0,0,0);}
    int status_e,status_o;
    pid_t pid_e,pid_o,w_pid_e,w_pid_o;
    pid_e = fork();   //here I get the PID of the child in the parent process and 0 in the child process 
    switch(pid_e){
    case -1: 
        abortProg_arg("could not fork",pipe_p_c_even[0],pipe_p_c_even[1],pipe_c_p_even[0],pipe_c_p_even[1],0,0,0,0); 
        break;
    case 0:
        //close the unused ends of the two pipes and assign stdin and stdout
        if((close_and_dup2(pipe_p_c_even,pipe_c_p_even)) == -1){
            close(0);
            close(1);
            abortProg_arg("could not close_and_dup2",pipe_p_c_even[0],pipe_p_c_even[1],pipe_c_p_even[0],pipe_c_p_even[1],0,0,0,0);
        }
        execlp("./forkFFT","./forkFFT",NULL); 
        //if this line is reached execlp failed, therefore abort program
        close(0);
        close(1);
        abortProg_arg("execlp didn't work",pipe_p_c_even[0],pipe_p_c_even[1],pipe_c_p_even[0],pipe_c_p_even[1],0,0,0,0);
        break;
    default:
        //close the unused ends of the two pipes 
        if(close(pipe_p_c_even[0]) == -1){abortProg_arg("close pipe end didn't work",pipe_p_c_even[1],pipe_c_p_even[0],pipe_c_p_even[1],0,0,0,0,0);}
        if(close(pipe_c_p_even[1]) == -1){abortProg_arg("close pipe end didn't work",pipe_p_c_even[1],pipe_c_p_even[0],0,0,0,0,0,0);}
        FILE* fp_out_even;
        if((fp_out_even= fdopen(pipe_p_c_even[1],"w")) == NULL){abortProg_arg("fdopen didn't work",pipe_p_c_even[1],pipe_c_p_even[0],0,0,0,0,0,0);}
        FILE* fp_in_even;
        if((fp_in_even= fdopen(pipe_c_p_even[0],"r")) == NULL){abortProg_arg("fdopen didn't work",pipe_c_p_even[0],0,0,0,fp_out_even,0,0,0);}

        if(pipe(pipe_p_c_odd) == -1){abortProg_arg("could not create pipe",0,0,0,0,fp_out_even,fp_in_even,0,0);}
        if(pipe(pipe_c_p_odd) == -1){abortProg_arg("could not create pipe",pipe_p_c_odd[0],pipe_p_c_odd[1],0,0,fp_out_even,fp_in_even,0,0);}

        pid_o = fork();
        switch(pid_o){
        case -1: 
            abortProg_arg("could not fork",pipe_p_c_odd[0],pipe_p_c_odd[1],pipe_c_p_odd[0],pipe_c_p_odd[1],fp_out_even,fp_in_even,0,0);
            break;
        case 0:
            //close the unused ends of the two pipes and assign stdin and stdout
            if((close_and_dup2(pipe_p_c_odd,pipe_c_p_odd)) == -1){
                close(0);
                close(1);
                abortProg_arg("could not close_and_dup2",pipe_p_c_odd[0],pipe_p_c_odd[1],pipe_c_p_odd[0],pipe_c_p_odd[1],fp_out_even,fp_in_even,0,0);
            }
            execlp("./forkFFT","./forkFFT",NULL);
            //if this line is reached execlp failed, therefore abort program
            close(0);
            close(1);
            abortProg_arg("could not close_and_dup2",pipe_p_c_odd[0],pipe_p_c_odd[1],pipe_c_p_odd[0],pipe_c_p_odd[1],fp_out_even,fp_in_even,0,0);
            break;
        default:
            //close the unused ends of the two pipes 
            if(close(pipe_p_c_odd[0]) == -1){abortProg_arg("close pipe end didn't work",pipe_p_c_odd[1],pipe_c_p_odd[0],pipe_c_p_odd[1],0,fp_out_even,fp_in_even,0,0);}
            if(close(pipe_c_p_odd[1]) == -1){abortProg_arg("close pipe end didn't work",pipe_p_c_odd[1],pipe_c_p_odd[0],0,0,fp_out_even,fp_in_even,0,0);}
            FILE* fp_out_odd;
            if((fp_out_odd= fdopen(pipe_p_c_odd[1],"w")) == NULL){abortProg_arg("fdopen didn't work",pipe_p_c_odd[1],pipe_c_p_odd[0],0,0,fp_out_even,fp_in_even,0,0);}
            FILE* fp_in_odd;
            if((fp_in_odd= fdopen(pipe_c_p_odd[0],"r")) == NULL){abortProg_arg("fdopen didn't work",pipe_c_p_odd[0],0,0,0,fp_out_even,fp_in_even,fp_out_odd,0);}
            //first input that has been stored at beginning of main is sent to even
            fprintf(fp_out_even,"%f\n",inp1);   
            int n = 1;
            //the rest of the inputs are sent to the child odd and even
            while(fgets(buffer,sizeof(buffer),stdin) != NULL) {
                inp1 =strtod(buffer,&endptr1);
                if(*endptr1 !='\n'){abortProg_arg("invalid input: only floats separated by next line are allowed",0,0,0,0,fp_out_even,fp_in_even,fp_out_odd,fp_in_odd);}
                if(n%2 ==0){
                    fprintf(fp_out_even,"%f\n",inp1);
                }
                else{
                    fprintf(fp_out_odd,"%f\n",inp1);
                }
                n++;
            }

            if(fclose(fp_out_odd) == EOF){abortProg_arg("fclose didn't work",0,0,0,0,fp_out_even,fp_in_even,fp_in_odd,0);}
            if(fclose(fp_out_even) == EOF){abortProg_arg("fclose didn't work",0,0,0,0,fp_in_even,fp_in_odd,0,0);}

            // n has to be 2^k = 2,4,8,16...
            if(check_n(n) == -1){abortProg_arg("n has not the right size, must be 2^k",0,0,0,0,fp_in_even,fp_in_odd,0,0);}         
           
            while((w_pid_o = waitpid(pid_o,&status_o,0)) !=pid_o){       
                if(w_pid_o != -1){continue;}
                if(errno == EINTR){continue;}
                abortProg_arg("child odd not terminated correctly",0,0,0,0,fp_in_even,fp_in_odd,0,0);
            }
            if(WEXITSTATUS(status_o) != EXIT_SUCCESS){abortProg_arg("child odd not terminated correctly",0,0,0,0,fp_in_even,fp_in_odd,0,0);}

            while((w_pid_e = waitpid(pid_e,&status_e,0)) !=pid_e){       
                if(w_pid_e != -1){continue;}
                if(errno == EINTR){continue;}
                abortProg_arg("child even not terminated correctly",0,0,0,0,fp_in_even,fp_in_odd,0,0);
            }
            if(WEXITSTATUS(status_e) != EXIT_SUCCESS){abortProg_arg("child even not terminated correctly",0,0,0,0,fp_in_even,fp_in_odd,0,0);}

            //arrays for (k+n/2) 
            double real_arr[n/2];
            double imag_arr[n/2];
            //here the parent get the info of the children, calculate the results and send them to their parent
            for(int k=0;k < n/2;k++){
                char buffer_p_e[SIZE_BUFFER];
                if(fgets(buffer_p_e,sizeof(buffer_p_e),fp_in_even) == NULL){
                    abortProg_arg("n has not the right size, must be 2^k",0,0,0,0,fp_in_even,fp_in_odd,0,0);
                }
                char buffer_p_o[SIZE_BUFFER];
                if(fgets(buffer_p_o,sizeof(buffer_p_o),fp_in_odd) == NULL){
                    abortProg_arg("n has not the right size, must be 2^k",0,0,0,0,fp_in_even,fp_in_odd,0,0);
                }
                double real_1;
                double imag_1;
                double real_2;
                double imag_2;
                //strings from children are parsed to real and imag doubles,calculated accordingly and sent
                string_to_complex(buffer_p_e,&real_1,&imag_1);
                string_to_complex(buffer_p_o,&real_2,&imag_2);
                printf("%f %f*i\n",fft_real_left(real_1,imag_1,real_2,imag_2,k,n),fft_imag_left(real_1,imag_1,real_2,imag_2,k,n));
                //(k+n/2) entries are stored in an area and sent afterwards
                real_arr[k]= fft_real_right(real_1,imag_1,real_2,imag_2,k,n);
                imag_arr[k]= fft_imag_right(real_1,imag_1,real_2,imag_2,k,n);
            }
            for(int i = 0; i< n/2; i++){
                printf("%f %f*i\n",real_arr[i],imag_arr[i]);
            }
            if(fclose(fp_in_even) == EOF){abortProg_arg("fclose didn't work",0,0,0,0,fp_in_odd,0,0,0);}
            if(fclose(fp_in_odd) == EOF){abortProg("fclose didn't work");}
            break;
        }
    }
    return EXIT_SUCCESS;
}