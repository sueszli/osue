#include "forkFFT.h"

static const char *program_name;
static dual_buffer_t buffer = {0, 0, 0, 0};
static pipes_t pipes;


// static void usage(){
//     fprintf(stderr,"Usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
//     exit(EXIT_FAILURE);
// }

static void cleanup(void){
    free(buffer.buffer1);
    free(buffer.buffer2);
    // if(pipes.child1.read != NULL) fclose(pipes.child1.read);
    // if(pipes.child1.write != NULL) fclose(pipes.child1.write);
    // if(pipes.child2.read != NULL) fclose(pipes.child2.read);
    // if(pipes.child2.write != NULL) fclose(pipes.child2.write);
}

static int startChildProcess(dual_pipe_t *pipes){
    close(pipes->in[1]);
    close(pipes->out[0]);
    if(dup2(pipes->in[0], STDIN_FILENO) < 0) return -1;
    if(dup2(pipes->out[1], STDOUT_FILENO) < 0) return -1;
    close(pipes->in[0]);
    close(pipes->out[1]);
    return execlp(program_name,program_name,NULL);
}

static int setupParentProcess(dual_pipe_t *pipes){
    close(pipes->in[0]);
    close(pipes->out[1]);
    if((pipes->read = fdopen(pipes->out[0],"r")) == NULL) return -1;
    return ((pipes->write = fdopen(pipes->in[1],"w")) == NULL);
}

static pid_t createChild(dual_pipe_t *pipes){
    if(pipe(pipes->in) == -1) return -1;
    if(pipe(pipes->out) == -1) return -1;
    pid_t pid = fork();
    switch(pid){
        case -1:
            fprintf(stderr, "Error: cannot fork!\n");
            return -1;
        case 0:
            if (startChildProcess(pipes) == -1) {
                fprintf(stderr, "Error: cannot exec!\n");
                return -1;
            }
        default:
            if(setupParentProcess(pipes) == 1) return -1;
            return pid;
    }
}

static int inputPair(void){
    if(getline(&buffer.buffer1,&buffer.size1,stdin) == -1) return -1;
    if(getline(&buffer.buffer2,&buffer.size2,stdin) == -1) return 0;
    return 1;
}

static int firstPair(int *counter, pid_pair_t *pid_pair){
    int read = inputPair();
    if(read == -1 || read == 0) return read;
    pid_t child1, child2;
    if((child1 = createChild(&pipes.child1)) == -1){
        fprintf(stderr, "Error: cannot create first child!\n");
        return -2;
    }
    if((child2 = createChild(&pipes.child2)) == -1){
        fprintf(stderr, "Error: cannot create second child!\n");
        return -3;
    }
    if(fputs(buffer.buffer1, pipes.child1.write) == EOF) return -1;
    if(fputs(buffer.buffer2, pipes.child2.write) == EOF) return -1;
    fflush(pipes.child1.write);
    fflush(pipes.child2.write);
    (*counter)++;
    pid_pair->child1 = child1;
    pid_pair->child2 = child2;
    return 1;
}

static int readRest(int *counter){
    int read;
    while((read = inputPair()) == 1){
        if(fputs(buffer.buffer1, pipes.child1.write) == EOF) return -1;
        if(fputs(buffer.buffer2, pipes.child2.write) == EOF) return -1;
        (*counter)++;
    }
    fflush(pipes.child1.write);
    fflush(pipes.child2.write);
    if(read != -1){        
        return -1;
    }
    return 0;    
}

static int parse_complex(char *string, complex_t *complex){
    float a, b;
    char rest;
    if(string[strlen(string) - 1] == '\n') string[strlen(string) - 1] =  '\0';
    if(sscanf(string, "%f%c", &a, &rest) == 1){
        complex->real = a;
        complex->imag = 0.0;
        return 0;
    }
    else if(sscanf(string, "%f %f * i%c", &a, &b, &rest) == 2){
        complex->real = a;
        complex->imag = b;
        return 1;
    }
    return -1; 
}

static int readFromChild(dual_pipe_t *pipes, complex_t out[], int size){
    complex_t c;    
    for(int i = 0; i < size; i++ ){        
        if(getline(&buffer.buffer1,&buffer.size1,pipes->read) == -1){
            return -1;        
        }
        if(parse_complex(buffer.buffer1,&c) == -1) return -1;
        out[i] = c;
    }    
    return 0;
}

static void print_complex(complex_t complex){
    fprintf(stdout,"%f %f * i\n",complex.real,complex.imag);
}

complex_t add(complex_t a, complex_t b){
    complex_t c = {.real = a.real + b.real, .imag = a.imag + b.imag};
    return c;
}

complex_t subtract(complex_t a, complex_t b){
    complex_t c = {.real = a.real - b.real, .imag = a.imag - b.imag};
    return c;
}

complex_t mult(complex_t a, complex_t b){
    complex_t c;
    c.real = a.real * b.real - a.imag * b.imag;
    c.imag = a.real * b.imag + a.imag * b.real;
    return c;
}

void FFT(complex_t even[], complex_t odd[], complex_t out[], int len){
    for(int k = 0; k < len; k++){
        float phi = -2 * PI / (2 * len) * k ;
        complex_t omega = {.real = cosf(phi), .imag = sinf(phi)};
        out[k] = add(even[k], mult(omega, odd[k]));
        out[k+len] = subtract(even[k], mult(omega, odd[k]));
    }
}

int main(int argc, char *argv[]) {
    program_name = argv[0];
    atexit(cleanup);

    dual_pipe_t child = {.read = NULL, .write = NULL};
    pipes.child1 = child;
    pipes.child2 = child;

    pid_pair_t pids;
    int count = 0;
    int status;
    complex_t c;    

    switch(firstPair(&count, &pids)){
        case 0:
            if(parse_complex(buffer.buffer1,&c) != -1){
                print_complex(c);
                exit(EXIT_SUCCESS);
            }        
            else{
                fprintf(stderr, "Error: wrong input!\n");
                exit(EXIT_FAILURE);
            }
        case -1:
            fprintf(stderr, "Error: no Input!\n");
        case -2:
        case -3:
            exit(EXIT_FAILURE);
        default:
            break;   
    }       

    if(readRest(&count) == -1){
        fprintf(stderr,"Error: odd input!\n");
        exit(EXIT_FAILURE);
    }

    fclose(pipes.child1.write);
    fclose(pipes.child2.write);

    complex_t even[count];
    complex_t odd[count];

    waitpid(pids.child1,&status,0);
    if(status != EXIT_SUCCESS){
        exit(EXIT_FAILURE);
    }
    readFromChild(&pipes.child1, even, count);
    waitpid(pids.child2,&status,0);
    if(status != EXIT_SUCCESS){
        exit(EXIT_FAILURE);
    }
    readFromChild(&pipes.child2, odd, count);

    complex_t complex[2*count];

    FFT(even, odd, complex, count);

    for(int i = 0; i < 2 * count; i++){
        print_complex(complex[i]); 
    }
    fflush(stdout);

    fclose(pipes.child1.read);    
    fclose(pipes.child2.read);      
    
    exit(EXIT_SUCCESS);
}