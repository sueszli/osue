#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <ctype.h>
#include <math.h>

//global variables
static char *myname; /** <Programm name */
int term = 0;

static void usage(){
    fprintf(stderr, "%s: Usage: forkFFT\n" ,myname);
    exit(EXIT_FAILURE);
}

typedef struct {
  float *array;
  size_t used;
  size_t size;
} DynArray;

void initArray(DynArray *a, size_t initialSize) {
  a->array = malloc(initialSize * sizeof(float));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(DynArray *a, float element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = realloc(a->array, a->size * sizeof(float));
  }
  a->array[a->used++] = element;
}

void freeArray(DynArray *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

void forkswitch(pid_t pid, int pipefda[], int pipefdb[], DynArray *sendArrayReal, DynArray *sendArrayIm){

    switch(pid){
        case -1:
            fprintf(stderr, "Cannot fork\n");
            exit(EXIT_FAILURE);
        case 0:
            close(pipefda[1]);
            dup2(pipefda[0], STDIN_FILENO);
            close(pipefda[0]);

            close(pipefdb[0]);
            dup2(pipefdb[1], STDOUT_FILENO);
            close(pipefdb[1]);

            execl("./forkFFT", "forkFFT", NULL);
            exit(EXIT_FAILURE);
        default:
        {
            close(pipefdb[1]);
            close(pipefda[0]);
            FILE *child = fdopen(pipefda[1], "w");
            if (child == NULL)
            {
                fprintf(stderr, "fdopen failed!\n");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < sendArrayReal->used; i++){
                //fprintf(stderr, "DEBUG: %f %f*i\n", sendArrayReal->array[i], sendArrayIm->array[i]);
                fprintf(child, "%f %f*i\n", sendArrayReal->array[i], sendArrayIm->array[i]);
            }
            fflush(child);
            fclose(child);
            close(pipefda[1]);
            break;
        }
    }
}

void parse(DynArray *evenReal, DynArray *evenIm, DynArray *oddReal, DynArray *oddIm, FILE *target){
    char *line = NULL;
    size_t size = 0;
    ssize_t line_size;
    int index = 0;
    float help;
    char* pend;
    //int debug = 0;
    while ((line_size = getline(&line, &size, target)) != -1) {
        help = strtof(line, &pend);
        //fprintf(stderr, "%iDEBUG LINE: %s\n", debug, line);
        if (strlen(line) < 1){
            fprintf(stderr, "%s is empty!\n", line);
            exit(EXIT_FAILURE);
        }
        if ((*pend == '\0') || (isspace(*pend) != 0)){
            if (index == 0){
                insertArray(evenReal, help);
            }else{
                insertArray(oddReal, help);
            }  
            help = strtof(pend, NULL);
            if (index == 0){
                insertArray(evenIm, help);
            }else{
                insertArray(oddIm, help);
            }           
            index++;
            index = index%2;
        } else{
            fprintf(stderr, "%s is not a float!\n", line);
            exit(EXIT_FAILURE);
        }
        //debug++;            
    }
}

void readFromChild(pid_t pid, int pipefd[], DynArray *real, DynArray *im){
    int status;
    if(term == 0){
        waitpid(pid, &status, 0);
        if(WEXITSTATUS(status) == EXIT_FAILURE){
            term = 1;
        }else{

            FILE *in;
            in = fdopen(pipefd[0], "r");
            if(in == NULL){
              fprintf(stderr, "%s: fopen failed: %s\n", myname, strerror(errno));
              exit(EXIT_FAILURE);
            }
            parse(real, im, real, im, in);
            fclose(in);
            close(pipefd[0]);
        }
    }else{
        waitpid(pid, &status, 0);
    }
}

int main(int argc, char **argv){

    myname = argv[0];

    if (argc > 1){
        usage();
    }


    DynArray evenReal, evenIm;
    DynArray oddReal, oddIm;
    initArray(&evenReal, 8);
    initArray(&evenIm, 8);
    initArray(&oddReal, 8);
    initArray(&oddIm, 8);

    parse(&evenReal, &evenIm, &oddReal, &oddIm, stdin);

    //fprintf(stderr, "DEBUG: even size = %li, odd size = %li\n", evenReal.used, oddReal.used);
    if (evenReal.used == 1 && oddReal.used == 0){
        fprintf(stdout,"%f %f*i\n" ,evenReal.array[0], evenIm.array[0]);
        exit(EXIT_SUCCESS);
    }
    

    if (evenReal.used != oddReal.used){
        fprintf(stderr, "array size is odd!\n");
        exit(EXIT_FAILURE);
    }
    
    int pipefd1a[2], pipefd1b[2], pipefd2a[2], pipefd2b[2];
    //child1
    if(pipe(pipefd1a) == -1){
        fprintf(stderr, "pipe1a error");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefd1b) == -1){
        fprintf(stderr, "pipe1b error");
        exit(EXIT_FAILURE);
    }
    pid_t pid1 = fork();
    forkswitch(pid1, pipefd1a, pipefd1b, &evenReal, &evenIm);


    //child2
    if(pipe(pipefd2a) == -1){
        fprintf(stderr, "pipe2a error");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefd2b) == -1){
        fprintf(stderr, "pipe2b error");
        exit(EXIT_FAILURE);
    }
    pid_t pid2 = fork();
    forkswitch(pid2, pipefd2a, pipefd2b, &oddReal, &oddIm);

    DynArray resEvenReal, resEvenIm;
    DynArray resOddReal, resOddIm;
    initArray(&resEvenReal, 8);
    initArray(&resEvenIm, 8);
    initArray(&resOddReal, 8);
    initArray(&resOddIm, 8);

    readFromChild(pid1, pipefd1b, &resEvenReal, &resEvenIm);
    readFromChild(pid2, pipefd2b, &resOddReal, &resOddIm);

    if (term == 1){
        fprintf(stderr, "term == 1\n");
        exit(EXIT_FAILURE);
    }

    //(a + i · b)(c + i · d) = a·c − b·d + i · (a · d + b · c)
    float k = resEvenReal.used;
    float n = 2*k;
    float pi = 3.141592654;
    float a,b,c,d;
    //fprintf(stderr, "DEBUG: k = %f, n = %f\n", k, n);
    for (int i = 0; i < k; i++){
        float helper = (-2*pi*i)/n;
        a = cosf(helper);
        b = sinf(helper);
        c = resOddReal.array[i];
        d = resOddIm.array[i];
        //fprintf(stderr, "DEBUG: RE = %f %f*i, RO = %f %f*i, i = %i\n", resEvenReal.array[i], resEvenIm.array[i], resOddReal.array[i], resOddIm.array[i], i);
        //fprintf(stderr, "DEBUG:a = %f, b = %f, c = %f, d = %f\n", a, b, c, d);
        //fprintf(stderr, "DEBUG RESULT:%f %f*i\n" , resEvenReal.array[i] + (a*c)-(b*d), resEvenIm.array[i] + (a*d)+(b*c));
        fprintf(stdout, "%f %f*i\n" , resEvenReal.array[i] + ((a*c)-(b*d)), resEvenIm.array[i] + ((a*d)+(b*c)));
    }
    for (int i = 0; i < k; i++){
        float helper = (-2*pi*i)/n;
        a = cosf(helper);
        b = sinf(helper);
        c = resOddReal.array[i];
        d = resOddIm.array[i];
        //fprintf(stderr, "DEBUG: RE = %f %f*i, RO = %f %f*i, i = %i\n", resEvenReal.array[i], resEvenIm.array[i], resOddReal.array[i], resOddIm.array[i], i);
        //fprintf(stderr, "DEBUG:a = %f, b = %f, c = %f, d = %f\n", a, b, c, d);
        //fprintf(stderr, "DEBUG RESULT:%f %f*i\n" , resEvenReal.array[i] - (a*c)-(b*d), resEvenIm.array[i] - (a*d)+(b*c));
        fprintf(stdout, "%f %f*i\n" , resEvenReal.array[i] - ((a*c)-(b*d)), resEvenIm.array[i] - ((a*d)+(b*c)));
    }
    exit(EXIT_SUCCESS);
}