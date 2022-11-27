/**
 * @file forkFFT.c
 * @author Francesca Diana Fodor, 11808223
 * @brief FFT, Fast Fourier Transformation, Algorithm, Pipes, Files
 * @details Implementation of an FFT (Fast Fourier Transformation) Algorithm with complexity of O(nlogn).
 *        Two children are forked from the parent process and the dataflow happen using pipes with redirected
 *        file descriptors.
 * @date 2021-12-11
 *
 */
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


#define ERROR_EXIT(...) { fprintf(stderr,"ERROR: "__VA_ARGS__" ---> %s\n",strerror(errno)); exit(EXIT_FAILURE); }

#define PI 3.141592654

typedef struct {
  float rl;
  float img;
} complexNum;

typedef struct {
  complexNum *array;
  size_t used;
  size_t size;
} Array;


static void initArray(Array *a,int size);
static void freeArray(Array *a);
static complexNum convertIntoComplex(char *str);
static Array calc(complexNum c1, complexNum c2, int size, int k);


/**
 * @brief Main function for the program implementing FFT
 * @param argc argument length
 * @param argv argument char array
 * @return returns EXIT_SUCCESS if program was executed successfully
 *         otherwise EXIT_FAILURE is returned
 */
int main (int argc, char *argv[]) {

  int pipeWRITE_PE[2];
  int pipeREAD_PE[2];
  int pipeWRITE_PO[2];
  int pipeREAD_PO[2];

  if ((pipe(pipeREAD_PE) < 0) || (pipe(pipeWRITE_PE) < 0)) {  //0 read end 1 write end
    fprintf(stderr,"[%s] ",argv[0]);
    ERROR_EXIT("(pipe) - creation failed.");
  }
  if ((pipe(pipeREAD_PO) < 0) || (pipe(pipeWRITE_PO) < 0)) {  //0 read end 1 write end
    fprintf(stderr,"[%s] ",argv[0]);
    ERROR_EXIT("(pipe) - creation failed.");
  }

  //-- READ values from STDIN and write the values with even index to child_PE //
  //    and uneven index to child_PO -- //
  
  FILE *wPE;
  FILE *wPO;
  if (((wPE = fdopen(pipeWRITE_PE[1],"w")) == NULL ) ||
      ((wPO = fdopen(pipeWRITE_PO[1],"w")) == NULL)){
        fprintf(stderr,"[%s] ",argv[0]);
        ERROR_EXIT("(fdopen) - failed.");
  }

  int         inptSz = 0;
  char       *line = NULL;
  complexNum  cNum;
  size_t      bufsz = 0; 

  while (getline(&line,&bufsz,stdin) > 0 ) { 
    cNum = convertIntoComplex(line);
    if((inptSz % 2) == 0) {
      fprintf(wPE,"%f %f*i\n",cNum.rl,cNum.img);
    } else {
      fprintf(wPO,"%f %f*i\n",cNum.rl,cNum.img);
    }
    inptSz++;
  }
  free(line);
  fclose(wPE);
  fclose(wPO);

  //basecase return exit Success no more forks are applied
  if (inptSz == 1) {
    fprintf(stdout,"%f %f*i\n",cNum.rl,cNum.img);
    exit(EXIT_SUCCESS);
  }
  //array length is uneven no more forks can be applied
  if ((inptSz % 2) == 1) {
    fprintf(stderr,"[%s] usage: input length for child is uneven.\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  
  //-- Fork children process --//
  pid_t child_PE;
  pid_t child_PO;

  if ((child_PE = fork()) == 0){
    //close pipes from other child
    close(pipeREAD_PO[0]);
    close(pipeREAD_PO[1]);
    close(pipeWRITE_PO[0]);
    close(pipeWRITE_PO[1]);

    close(pipeWRITE_PE[1]);
    while ((dup2(pipeWRITE_PE[0], STDIN_FILENO) == -1)) {}
    close(pipeWRITE_PE[0]); //
    close(pipeREAD_PO[0]);
    while ((dup2(pipeREAD_PE[1], STDOUT_FILENO) == -1)) {}
    close(pipeREAD_PO[1]);
    
    //rewrite program code
    execlp(argv[0],argv[0],NULL);

  } else if (child_PE == -1) {
      fprintf(stderr,"[%s] ",argv[0]);
      ERROR_EXIT("(fork) child_PE - failed.");
    
    } else {
      if ((child_PO = fork()) == 0){
        //closing pipes from other child
        close(pipeREAD_PE[0]);
        close(pipeREAD_PE[1]);
        close(pipeWRITE_PE[0]);
        close(pipeWRITE_PE[1]);

        close(pipeWRITE_PO[1]);
        while ((dup2(pipeWRITE_PO[0], STDIN_FILENO) == -1)) {}
        close(pipeWRITE_PO[0]);
        close(pipeREAD_PO[0]);
        while ((dup2(pipeREAD_PO[1], STDOUT_FILENO) == -1)) {}
        close(pipeREAD_PO[1]);
        //rewrite program code
        execlp(argv[0],argv[0],NULL);
      
      } else if (child_PO == -1) {
        fprintf(stderr,"[%s] ",argv[0]);
        ERROR_EXIT("(fork) child_PO - failed.");
      
      } else {
        //close unused pipes in parent process
        close(pipeREAD_PE[1]);
        close(pipeWRITE_PE[0]);
        close(pipeREAD_PO[1]);
        close(pipeWRITE_PO[0]);

        int status[2];
        if (waitpid(child_PE,&status[0],0) == -1 || waitpid(child_PO,&status[1],0) == -1) {
          fprintf(stderr,"[%s] ",argv[0]);
          ERROR_EXIT("(waitpid) - failed");
        }
        if (WEXITSTATUS(status[0]) == EXIT_FAILURE || WEXITSTATUS(status[1]) == EXIT_FAILURE) {
          fprintf(stderr,"[%s] ",argv[0]);
          ERROR_EXIT("(child exit) - failed");
        }

        //read from child process stdout
        FILE  *rPE;
        FILE  *rPO;
        if ((rPE = fdopen(pipeREAD_PE[0],"r")) == NULL ||
            (rPO = fdopen(pipeREAD_PO[0],"r")) == NULL ){
              fprintf(stderr,"[%s] ",argv[0]);
              ERROR_EXIT("(fdopen) - failed");
            }


        char *line1 = NULL;
        size_t bufsz1 = 0;
        char *line2 = NULL;
        size_t bufsz2 = 0;

        int k = 0;
        Array arr;
        initArray(&arr,inptSz);
        while (getline(&line1,&bufsz1,rPE) > 0 ) { //delimiter \n
               getline(&line2,&bufsz2,rPO);

        Array results = calc(convertIntoComplex(line1),convertIntoComplex(line2),inptSz,k);
        arr.array[k] = results.array[0];
        arr.array[k + inptSz/2] = results.array[1];
        freeArray(&results);
        k++;
        }
        free(line1);
        free(line2);
        fclose(rPE);
        fclose(rPO);
        for (size_t i = 0; i < inptSz; i++){
          fprintf(stdout,"%f %f*i\n",arr.array[i].rl,arr.array[i].img);
        }
        freeArray(&arr);
      }
    exit(EXIT_SUCCESS);
    }
}

/**
 * @brief calc performs the calculations needed for the FFT algorithm 
 * @param c1 result from the child_PE
 * @param c2 result from the child_PO
 * @param size of the array where results are saved
 * @param k is the current count of the calculation performed
 * @return Results are returned in an array consisting of two complexNum structs
 */
static Array calc(complexNum c1, complexNum c2, int size, int k){
  float constant =  (-(2 * PI) / size ) * k;
  float cosine   = cos((constant));
  float sinus    = sin((constant));

  Array a;
  initArray(&a,2);
  a.array[0].rl = c1.rl + (cosine * c2.rl - sinus * c2.img);
  a.array[0].img = c1.img + (cosine * c2.img + sinus * c2.rl);
  a.array[1].rl = c1.rl - (cosine * c2.rl - sinus * c2.img);
  a.array[1].img = c1.img - (cosine * c2.img + sinus * c2.rl);

  return a;
}
/**
 * @brief complexNum takes a char array in the form of "0.0 0.0*i" and converts it into a complexNum struct 
 * @param *str in the form of "0.0 0.0*i"
 * @return complexNum struct
 */
static complexNum convertIntoComplex(char *str){
  complexNum tmp;
  tmp.rl = strtof(strtok(str," "),NULL);
  char *sv;
  if ((sv = strtok(NULL," ")) == NULL) {
    tmp.img = 0;
  } else {
    tmp.img = strtof(sv,NULL);
  }
  return tmp;
}

/**
 * @brief InitArray initialize an array of type float
 * @param a is a pointer to the array struct
 */
static void initArray(Array *a, int size){
     if ((a->array = (complexNum *)calloc(size,sizeof(complexNum))) == NULL) {
         fprintf(stderr,"[%s] ", "forkFFT");
         ERROR_EXIT("(calloc) - initArray - failed.")
     }
     a->used = 0;
     a->size = size;
}



/**
 * @brief freeArray frees all the memory allocated and sets sizes to 0
 * @param a is a pointer to the array struct which is to be freed.
 */
static void freeArray(Array *a){
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}
