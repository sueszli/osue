/**
 * @file forkFFT.c
 * @author Oliver Peter <e11915885@student.tuwien.ac.at>
 * @date 17.11.2021
 * @brief Main program module.
 * 
 * This program implements the Cooley-Tukey Fast Fourier Transform algorithm by forking and recursively calling the 
 * program again. 
 * Communication between processes works with pipes.
 * 
 * SYNOPSIS: ./forkFFT
 *  example: ./forkFFT < sine.txt
 * 
 **/


#include "forkFFT.h"

/**
 * @brief this is a usage function which is called if wrong arguments are given and exits with EXIT_FAILURE
 * 
 * @param argv the command line arguments
 */
static void usage(char **argv){
    fprintf(stderr,"[%s] ERROR: Wrong arguments\nSYNOPSIS: ./forkFFT \n",argv[0]);
    exit(EXIT_FAILURE);   
}

/**
 * @brief this is a error function which writes a error message with errno and exits with EXIT_FAILURE
 * 
 * @param argv the command line arguments
 * @param e the given error message
 */
static void myerror(char **argv, char* e){
    fprintf(stderr,"[%s] ERROR: %s: %s\n",argv[0],e,strerror(errno));
    exit(EXIT_FAILURE);   
}

/**
 * @brief converts given string to a complex number 
 * 
 * @param s complex number as a string with '\n' at the end in format 1.00000 1.00000*i
 * @param complex a float array of size 2 where complex[0] is the real part and complex[1] is the imaginary part
 * @return int returns -1 on error 0 otherwise
 */
static int convertStringToComplex(char *s, float *complex){

    
    char *endptr; 

    complex[RE] = 0;
    complex[IM] = 0;

    
    complex[RE] = strtof(s,&endptr);
        
    
    complex[IM]= strtof(endptr+1,&endptr);
    return 0;
}


/**
 * @brief applies the FFT Butterfly operation on the given Arrays
 * 
 * @param num_Lines is the number of lines
 * @param R is the Array of lenth num_lines where the Result will be stored
 * @param P_O is the two dimensional float array (2nd dimension is real/imaginary part) with all the odd values of size num_lines/2
 * @param P_E is the two dimensional float array (2nd dimension is real/imaginary part) with all the even values of size num_lines/2
 */
 static void doButterfly(int num_Lines, float (*R)[2],float (*P_O)[2],float (*P_E)[2]  ){
    for(int i = 0; i<num_Lines/2;i++){
            
        R[i][RE] = P_E[i][RE] + P_O[i][RE]*cosf((-2*M_PI*i)/num_Lines) - P_O[i][IM]*sinf((-2*M_PI*i)/num_Lines);
        R[i][IM] = P_E[i][IM] + cosf((-2*M_PI*i)/num_Lines)*P_O[i][IM] + sinf((-2*M_PI*i)/num_Lines)*P_O[i][RE];

        R[i + num_Lines/2][RE] = P_E[i][RE] - P_O[i][RE]*cosf((-2*M_PI*i)/num_Lines) + P_O[i][IM]*sinf((-2*M_PI*i)/num_Lines);
        R[i + num_Lines/2][IM] = P_E[i][IM] - cosf((-2*M_PI*i)/num_Lines)*P_O[i][IM] - sinf((-2*M_PI*i)/num_Lines)*P_O[i][RE];
    }
 }


/**
 * @brief this is the main module
 * 
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @return int EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char **argv){


    if(argc>1){
        usage(argv);
    }
    // child to parent or parent to child [EVEN|ODD][READ|WRITE]
    int c_p[2][2];
    int p_c[2][2];

    if(pipe(c_p[EVEN])==-1)     myerror(argv,"pipe c_p[EVEN] failed");
    if(pipe(c_p[ODD])==-1)      myerror(argv,"pipe c_p[ODD] failed");
    if(pipe(p_c[EVEN])==-1)     myerror(argv,"pipe p_c[EVEN] failed");
    if(pipe(p_c[ODD])==-1)      myerror(argv,"pipe p_c[ODD] failed");
    
    //Parent ID EVEN
    pid_t pid_E;
    //Parent ID ODD
    pid_t pid_O;

    char *buf = NULL;
    size_t bufsize = 0;
    int num_Lines=0;

    //[EVEN|ODD]
    FILE* readFromChild[2];
    FILE* writeToChild[2];

    //[EVEN/ODD][RE/IM]
    float complex[2][2]; 
    
    while(getline(&buf,&bufsize,stdin)!=-1){

        

        //save in complex
        if(convertStringToComplex(buf,complex[num_Lines%2])==-1)        myerror(argv,"converting string failed");

        
       //already forked -> send values to children
       if(num_Lines>1){
           fprintf(writeToChild[num_Lines%2], "%f %f*i\n",complex[num_Lines%2][RE],complex[num_Lines%2][IM]);
           fflush(writeToChild[num_Lines%2]);
       }

        
        //if we have 2 lines then fork
        if(num_Lines==1){
            //fprintf(stderr,"%i: FORK1\n",getpid());
            pid_E = fork();
            if(pid_E==-1)       myerror(argv,"Fork EVEN failed");

            //EVEN Child
            else if(pid_E==0){

                if(close(p_c[ODD][WRITE]) == -1)       myerror(argv,"close(p_c[ODD][WRITE]) failed");
                if(close(c_p[ODD][READ]) == -1)        myerror(argv,"close(c_p[ODD][READ]) failed");
                if(close(c_p[ODD][WRITE]) == -1)       myerror(argv,"close(c_p[ODD][WRITE]) failed");
                if(close(p_c[ODD][READ]) == -1)        myerror(argv,"close(p_c[ODD][READ]) failed");

                if(close(p_c[EVEN][WRITE]) == -1)       myerror(argv,"close(p_c[EVEN][WRITE]) failed");
                if(close(c_p[EVEN][READ]) == -1)        myerror(argv,"close(c_p[EVEN][READ]) failed");

                if(dup2(p_c[EVEN][READ],STDIN_FILENO) == -1)        myerror(argv,"dup2(p_c[EVEN][READ],STDIN_FILENO) failed");
                if(dup2(c_p[EVEN][WRITE],STDOUT_FILENO) == -1)        myerror(argv,"dup2(c_p[EVEN][WRITE],STDOUT_FILENO) failed");

                if(close(p_c[EVEN][READ]) == -1)       myerror(argv,"close(p_c[EVEN][READ]) failed");
                if(close(c_p[EVEN][WRITE]) == -1)        myerror(argv,"close(c_p[EVEN][WRITE]) failed");

                execlp("./forkFFT","forkFFT",NULL);

                myerror(argv, "execlp in even child failed");

            }//EVEN Child
            //EVEN Parent
            else{

                pid_O = fork();

                if(pid_E==-1)       myerror(argv,"Fork ODD failed");

                //ODD Child
                else if(pid_O==0){


                    if(close(p_c[EVEN][WRITE]) == -1)       myerror(argv,"close(p_c[EVEN][WRITE]) failed");
                    if(close(c_p[EVEN][READ]) == -1)        myerror(argv,"close(c_p[EVEN][READ]) failed");
                    if(close(c_p[EVEN][WRITE]) == -1)       myerror(argv,"close(c_p[EVEN][WRITE]) failed");
                    if(close(p_c[EVEN][READ]) == -1)        myerror(argv,"close(p_c[EVEN][READ]) failed");
                   
                    if(close(p_c[ODD][WRITE]) == -1)       myerror(argv,"close(p_c[ODD][WRITE]) failed");
                    if(close(c_p[ODD][READ]) == -1)        myerror(argv,"close(c_p[ODD][READ]) failed");

                    if(dup2(p_c[ODD][READ],STDIN_FILENO) == -1)        myerror(argv,"dup2(p_c[ODD][READ],STDIN_FILENO) failed");
                    if(dup2(c_p[ODD][WRITE],STDOUT_FILENO) == -1)        myerror(argv,"dup2(c_p[ODD][WRITE],STDOUT_FILENO) failed");

                    if(close(p_c[ODD][READ]) == -1)       myerror(argv,"close(p_c[ODD][READ]) failed");
                    if(close(c_p[ODD][WRITE]) == -1)        myerror(argv,"close(c_p[ODD][WRITE]) failed");

                    execlp("./forkFFT","forkFFT",NULL);

                    myerror(argv, "execlp in odd child failed");

                }//ODD Child
                
    


                if(close(p_c[ODD][READ]) == -1)       myerror(argv,"close(p_c[ODD][READ]) failed");
                if(close(c_p[ODD][WRITE]) == -1)        myerror(argv,"close(c_p[ODD][WRITE]) failed");
                if((writeToChild[ODD] = fdopen(p_c[ODD][WRITE],"w"))==NULL)     myerror(argv,"fdopen writeToChild[ODD] failed");
                if((readFromChild[ODD] = fdopen(c_p[ODD][READ],"r"))==NULL)     myerror(argv,"fdopen readFromChild[ODD] failed");
                
                //write first odd value to odd child
                fprintf(writeToChild[ODD], "%f %f*i\n",complex[ODD][RE],complex[ODD][IM]);
                fflush(writeToChild[ODD]);

                
                if(close(p_c[EVEN][READ]) == -1)       myerror(argv,"close(p_c[EVEN][READ]) failed");
                if(close(c_p[EVEN][WRITE]) == -1)        myerror(argv,"close(c_p[EVEN][WRITE]) failed");
                if((writeToChild[EVEN] = fdopen(p_c[EVEN][WRITE],"w"))==NULL)     myerror(argv,"fdopen writeToChild[EVEN] failed");
                if((readFromChild[EVEN] = fdopen(c_p[EVEN][READ],"r"))==NULL)     myerror(argv,"fdopen readFromChild[EVEN] failed");
                
                //write first even value to even child
                fprintf(writeToChild[EVEN], "%f %f*i\n",complex[EVEN][RE],complex[EVEN][IM]);
                fflush(writeToChild[EVEN]);

            }//EVEN Parent


        }

      
       num_Lines++;
        
    }


    if(num_Lines==1){
        //fprintf(stderr,"%i GOT HERE NUM 1\n",getpid());
        fprintf(stdout,"%f %f*i\n",complex[EVEN][RE],complex[EVEN][IM]);
        fflush(stdout);
        if(fclose(stdout)==-1)        myerror(argv,"close of stdout failed");
        if(fclose(stdin)==-1)        myerror(argv,"close of stdout failed");
        if(close(p_c[EVEN][READ]) == -1)       myerror(argv,"close(p_c[EVEN][READ]) failed");
        if(close(p_c[EVEN][WRITE]) == -1)        myerror(argv,"close(p_c[EVEN][WRITE]) failed");
        if(close(p_c[ODD][READ]) == -1)       myerror(argv,"close(p_c[ODD][READ]) failed");
        if(close(p_c[ODD][WRITE]) == -1)        myerror(argv,"close(p_c[ODD][WRITE]) failed");
        if(close(c_p[EVEN][READ]) == -1)       myerror(argv,"close(c_p[EVEN][READ]) failed");
        if(close(c_p[EVEN][WRITE]) == -1)        myerror(argv,"close(c_p[EVEN][WRITE]) failed");
        if(close(c_p[ODD][READ]) == -1)       myerror(argv,"close(c_p[ODD][READ]) failed");
        if(close(c_p[ODD][WRITE]) == -1)        myerror(argv,"close(c_p[ODD][WRITE]) failed");       
        
        exit(EXIT_SUCCESS);
    }
    if(num_Lines%2!=0||num_Lines==0){
        myerror(argv,"Uneven number of lines or no lines given");
    }


    if(fclose(writeToChild[EVEN]) == -1)     myerror(argv,"fclose(writeToChild[EVEN]) failed");
    if(fclose(writeToChild[ODD]) == -1)      myerror(argv,"fclose(writeToChild[ODD]) failed");



    int status_E;
    while(waitpid(pid_E,&status_E,0)==-1){
        if (errno == EINTR) continue;

        myerror(argv,"wait failed");
    }
    if(status_E!=EXIT_SUCCESS)      myerror(argv,"Exit of child even failed");

    float (*P_E)[2];
    if((P_E = malloc(sizeof(float)*num_Lines))==NULL) myerror(argv,"malloc failed");

    int c =0;
    while((getline(&buf,&bufsize,readFromChild[EVEN])!=-1)){
        if(convertStringToComplex(buf,P_E[c])==-1)        myerror(argv,"converting string failed");
        c++;
    }

    if(fclose(readFromChild[EVEN]) == -1)    myerror(argv,"fclose(readFromChild[EVEN]) failed");

    int status_O;
    while(waitpid(pid_O,&status_O,0)==-1){
        if (errno == EINTR) continue;

        myerror(argv,"wait failed");
    }
    if(status_E!=EXIT_SUCCESS)      myerror(argv,"Exit of child odd failed");



    float (*P_O)[2];
    if((P_O = malloc(sizeof(float)*(num_Lines)))==NULL) myerror(argv,"malloc failed");


    c=0;
    while((getline(&buf,&bufsize,readFromChild[ODD])!=-1)){
        if(convertStringToComplex(buf,P_O[c])==-1)        myerror(argv,"converting string failed");
        c++;
    }



    if(fclose(readFromChild[ODD]) == -1)     myerror(argv,"fclose(readFromChild[ODD]) failed");


    //The resulting Array
    float (*R)[2]; 
    if((R = malloc(sizeof(float)*num_Lines*2))==NULL)     myerror(argv,"malloc failed");
   
    doButterfly(num_Lines ,R,P_O,P_E);

    for(int i = 0; i < num_Lines; i++){
        fprintf(stdout,"%f %f*i\n",R[i][RE],R[i][IM]);
        fflush(stdout);
    }

    if(fclose(stdout)==-1)        myerror(argv,"close of stdout faile");
    if(fclose(stdin)==-1)        myerror(argv,"close of stdout faile");
    
    free(P_E);
    free(R);
    free(P_O);
    free(buf);

    return(EXIT_SUCCESS);

}