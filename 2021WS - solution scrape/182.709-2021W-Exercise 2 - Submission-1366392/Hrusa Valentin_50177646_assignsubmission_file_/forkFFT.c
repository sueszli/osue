/**
 * @file forkFFT.c
 * @author Valentin Hrusa 11808205
 * @brief reads input complex numbers and computes a Fourier-Transformation according to the Cooley-Tukey algorithm. 
 * @details if there is just one number put in, the number is returned. Otherwise the input is split and fed to child-processes. 
 *          The Output of those processes is read with pipes and then used for computing a intermediate result for a 
 *          Fourier-Transformation via the algorithm of Cooley-Tukey. This result is written to stdout and possibly 
 *          routed to a parent-process via pipe.
 * 
 * @date 2021-12-05
 * 
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include "forkFFT.h"

//data-structures which hold information about the input-list and the children created.
//these are global because every method needs their information
struct Communicator *p_e = NULL;
struct Communicator *p_o = NULL;
struct FPListHead *head = NULL;

/**
 * @brief waits for both the child-processes to finish
 * @details uses the global Communicator-structs to wait for both children. Ignores status information
 *          since this method is only called when an error is encountered.
 *          Before calling waitpid this method closes every pipe, so an EOF is delivered
 * 
 * @param name of the program for error-message
 */
static void mywait(char *name){
    int status;
    if(close(p_e->read) != 0) fprintf(stderr, "ERROR in %s, failed to close pipe: %s\n", name, strerror(errno));
    if(close(p_o->read) != 0) fprintf(stderr, "ERROR in %s, failed to close pipe: %s\n", name, strerror(errno));;
    if(close(p_e->write) != 0) fprintf(stderr, "ERROR in %s, failed to close pipe: %s\n", name, strerror(errno));;
    if(close(p_o->write) != 0) fprintf(stderr, "ERROR in %s, failed to close pipe: %s\n", name, strerror(errno));;
    if(waitpid(p_e->pid, &status, 0) == -1) fprintf(stderr, "ERROR in %s, failed to wait for child-process\n", name);
    if(waitpid(p_o->pid, &status, 0) == -1) fprintf(stderr, "ERROR in %s, failed to wait for child-process\n", name);
}

/**
 * @brief recursivley traverses input-list and frees the previously allocated ressources
 * 
 * @param entry of the inputnumber-list
 */
static void clearListRec(struct FPList *entry){
    if(entry->next != NULL) clearListRec(entry->next);
    free(entry->str);
    free(entry);
}

/**
 * @brief is called when ressources need to be freed
 * @details calls clearListRec for recursivley freeing the input-list and 
 *          frees communicators if they were already allocated
 * 
 */
static void clearRes(void){
    if(p_e != NULL) free(p_e);
    if(p_o != NULL) free(p_o);
    if(head->next != NULL) clearListRec(head->next);
    free(head);
}

/**
 * @brief closes two pipes
 * 
 * @param pipe arrey which has to be closed
 * @return int return-values of the close operation used for error handling
 */
int myclose(int pipe[]){
    return (close(pipe[0]) + close(pipe[1]));
}

/**
 * @brief writes a complex number to a child
 * @details one entry of the number-list and one child-communicator is specified to write the string-
 *          version of the entry complex-number to the child
 * 
 * @param entry list-entry which holds the complex number to send
 * @param comm communicator-struct of child to send the complex number to
 * @param name name of program used for error handling
 */
void mywrite(struct FPList *entry, struct Communicator *comm, char *name){
    size_t written = 0;
    ssize_t ret;
    while((written < strlen(entry->str))){
        ret = write(comm->write, entry->str, strlen(entry->str));
        if(ret == -1){
            if(errno != EINTR){
                mywait(name);
                clearRes();
                fprintf(stderr, "ERROR in %s, failed to write to child process!\n", name);
            }
        }else{
            written += ret;
        }
    }
}

/**
 * @brief calls mywrite for every list-entry starting from a list-entry
 * @details mywrite is called with appropriate communicator depending on if the current list-entry
 *          is even or odd, traverses the entire list starting from the list-entry the method was called with
 * 
 * @param list entry to start the write-process with
 * @param name name of program used for error handling
 */
void feed_children(struct FPList *list, char *name){
    struct FPList *pointer = list;
    while(pointer != NULL){
        if((pointer->index % 2) == 0){
            mywrite(pointer, p_e, name);
        }else{
            mywrite(pointer, p_o, name);
        }
        pointer = pointer->next;
    }
}

/**
 * @brief creates a child process which information is stored in Communicator comm
 * @details sets up two pipes to communicate with the child process this method creates.
 *          The pipe-ends which are not used are closed instantly and the used ends are stored in the corresponding Communicator.
 *          The pid of the child is also stored in the Communicator.
 *          The child-process created by this method duplicates STDIN and STDOUT to the correct pipe ends, then closes unused ends and recursivley
 *          calls this program anew.
 * 
 * @param comm Communicator-srtuct used to hold information about child-process which is created here
 * @param name name of program used for error handling
 */
void myfork(struct Communicator *comm, char *name){
    int writepipe[2];
    int readpipe[2];
    if(pipe(writepipe) != 0){
        clearRes();
        fprintf(stderr, "ERROR in %s, failed to open pipe: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(pipe(readpipe) != 0){
        clearRes();
        if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        fprintf(stderr, "ERROR in %s, failed to open pipe: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    //fork process
    if((comm->pid = fork()) == -1){
        if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        clearRes();
        exit(EXIT_FAILURE);
    }
    //we are in child process
    if(comm->pid == 0){
        if(dup2(writepipe[0], STDIN_FILENO) == -1){
            if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            clearRes();
            exit(EXIT_FAILURE);
        }
        if(dup2(readpipe[1], STDOUT_FILENO) == -1){
            if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            clearRes();
            exit(EXIT_FAILURE);
        }
        if(close(writepipe[1]) != 0){
            if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            clearRes();
            exit(EXIT_FAILURE);
        }
        if(close(readpipe[0]) != 0){
            if(close(writepipe[0]) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
            clearRes();
            exit(EXIT_FAILURE);
        }
        execlp(name, name, NULL);
        //should not be able to execute this code
        fprintf(stderr, "ERROR in %s, failed execlp\n", name);
        if(close(writepipe[0]) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        if(close(readpipe[1]) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        clearRes();
        exit(EXIT_FAILURE);   
    }
    //we are in parent process
    if(close(writepipe[0]) != 0){
        fprintf(stderr, "ERROR in %s, while closing pipe: %s", name, strerror(errno));
        if(myclose(readpipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        if(close(writepipe[1]) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        mywait(name);
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(close(readpipe[1]) != 0){
        fprintf(stderr, "ERROR in %s, while closing pipe: %s", name, strerror(errno));
        if(myclose(writepipe) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        if(close(readpipe[0]) != 0) fprintf(stderr, "ERROR in %s, while closing pipe: %s\n", name, strerror(errno));
        mywait(name);
        clearRes();
        exit(EXIT_FAILURE);
    }
    comm->write = writepipe[1];
    comm->read = readpipe[0];
}

/**
 * @brief Error method called when the usage of the program is wrong
 * 
 * @param name name of program used for error handling
 */
void usage_error(char *name){
    fprintf(stderr, "ERROR in %s, wrong usage! Usage: %s\n", name, name);
    exit(EXIT_FAILURE);
}

/**
 * @brief converts a string to a complex number
 * @details calls strtof two times to convert a string to a complex number.
 *          If an error with strtof or the string this method was called with is encountered, an error-message is printed.
 * 
 * @param f_r real part of complex number
 * @param f_i imaginary part of complex number
 * @param line string which contains the whole complex number
 * @param nread number of chars in line, used for error handling
 * @param name name of program used for error handling
 */
void mystrtof(float *f_r, float *f_i, char *line, ssize_t nread, char *name){
    char *endptr = line;
    *f_r = strtof(line, &endptr);
    if(endptr == line || nread == -1){
        mywait(name);
        clearRes();
        fprintf(stderr, "ERROR in %s, specify at least one valid floating-point input\n", name);
        exit(EXIT_FAILURE);
    }
    *f_i = strtof(endptr, NULL);
}

/**
 * @brief constructs a new entry for the FloatingPointList and links it 
 * @details calls strtof twice to convert line to a complex number. At least a rational part of the number must be specified.
 *          The linked-list datastructure which holds all input-numbers will be linked with a new entry created and filled with
 *          the converted numbers by this method.
 * 
 * @param line string which has to be converted to a complex number
 * @param name name of program used for error handling
 * @return struct FPList* pointer to the FPList entry this method creates and links to global variable head
 */
struct FPList *addLine(char *line, char *name){
    struct FPList *nextEntry = malloc(sizeof(struct FPList));
    nextEntry->str = malloc(sizeof(char) * (strlen(line)+1));
    if(nextEntry->str == NULL){
        if(p_e != NULL) mywait(name);
        fprintf(stderr, "ERROR in %s, malloc failed to allocate memory\n", name);
        free(nextEntry);
        clearRes();
        exit(EXIT_FAILURE);
    }
    memcpy(nextEntry->str, line, strlen(line)+1);
    char *endptr = line;
    nextEntry->index = head->length;
    nextEntry->next = NULL;
    nextEntry->ifpn = 0.0f;
    nextEntry->rfpn = 0.0f;
    nextEntry->rfpn = strtof(line, &endptr);
    if(*endptr != '\0' && endptr[0] != ' ' && endptr[0] != '\n'){
        if(p_e != NULL) mywait(name);
        fprintf(stderr, "ERROR in %s, specify valid floating-point numbers1\n", name);
        free(nextEntry->str);
        free(nextEntry);
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(endptr == line){
        if(p_e != NULL) mywait(name);
        fprintf(stderr, "ERROR in %s, specify at least one valid floating-point number as input\n", name);
        free(nextEntry->str);
        free(nextEntry);
        clearRes();
        exit(EXIT_FAILURE);
    }
    char *endptr2 = endptr;
    nextEntry->ifpn = strtof(endptr, &endptr2);
    if(*endptr2 != '\0' && endptr2[0] != '\n'){
        if(p_e != NULL) mywait(name);
        fprintf(stderr, "ERROR in %s, specify valid floating-point numbers2\n", name);
        free(nextEntry->str);
        free(nextEntry);
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(errno == ERANGE){
        if(p_e != NULL) mywait(name);
        fprintf(stderr, "ERROR in %s, specified floating-point number was too large: %s\n", name, strerror(errno));
        free(nextEntry->str);
        free(nextEntry);
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(head->next == NULL){
        head->next = nextEntry;
    }else{
        struct FPList *pointer = head->next;
        while(pointer->next != NULL) pointer = pointer->next;
        pointer->next = nextEntry;
    }
    head->length++;
    return nextEntry;
}

/**
 * @brief opens a filepointer via fdopen and handles error
 * 
 * @param fd filedescriptor of pipe-end to open with fdopen
 * @param m mode to use fdopen with
 * @param name name of program used for error handling
 * @return FILE* returned by the fdopen call
 */
FILE *myfdopen(int fd, char *m, char *name){
    FILE *val;
    if((val = fdopen(fd, m)) == NULL){
        if(p_e != NULL) mywait(name);
        clearRes();
        fprintf(stderr, "ERROR in %s, failed to open pipe-filedescriptor: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return val;
}

/**
 * @brief uses global variable head to print entire linked-list of complex numbers to stdout
 * 
 */
void print_result(void){
    size_t i;
    struct FPList *pointer = head->next;
    for (i = 0; i < head->length; i++)
    {
        fprintf(stdout, "%f %f*i\n", pointer->rfpn, pointer->ifpn);
        pointer = pointer->next;
    }
}

/**
 * @brief reads numbers from children and performs the Cooley-Tukey algorithm to calculate the Fourier-Transform 
 * @details calls myfdopen and mystrtof to read numbers from children.
 *          Then, two pointers to the global variable head are used to travers the input-number linked-list
 *          according to the algorithm.
 *          in every step of the main loop the correct constant is calculated and stored in temporary variables,
 *          then the constants are multiplied with the odd output of the child-processes,
 *          then the even and the updated odd outputs from the children will be added or subtracted according
 *          to the place in the linked-list where they will be stored.
 * 
 * @param name name of program used for error handling
 */
void cooley_tukey_calc(char *name){
    size_t k;
    size_t n = head->length;
    struct FPList *first_half_pointer = head->next;
    struct FPList *second_half_pointer = head->next;
    while(second_half_pointer->index != (n/2)) second_half_pointer = second_half_pointer->next;
    float rfloat_e, rfloat_o, ifloat_e, ifloat_o, rfconst, ifconst, temp;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    FILE *r_e = myfdopen(p_e->read, "r", name);
    FILE *r_o = myfdopen(p_o->read, "r", name);
    for (k = 0; k < (n/2); k++)
    {     
        //calculation of the constant complex number
        rfconst = (float)cos((-2.0f*PI*k)/n);
        ifconst = (float)sin((-2.0f*PI*k)/n); 
        //read corresponding complex numbers from child-processes
        nread = getline(&line, &len, r_e);
        mystrtof(&rfloat_e, &ifloat_e, line, nread, name);
        nread = getline(&line, &len, r_o);
        mystrtof(&rfloat_o, &ifloat_o, line, nread, name);
        //multiply 
        temp = rfloat_o * rfconst - ifloat_o * ifconst;
        ifloat_o = rfloat_o * ifconst + ifloat_o * rfconst;
        rfloat_o = temp;
        //add
        first_half_pointer->rfpn = rfloat_e + rfloat_o;
        first_half_pointer->ifpn = ifloat_e + ifloat_o;
        second_half_pointer->rfpn = rfloat_e - rfloat_o;
        second_half_pointer->ifpn = ifloat_e - ifloat_o;

        first_half_pointer = first_half_pointer->next;
        second_half_pointer = second_half_pointer->next;
    }
    free(line);
    fclose(r_e);
    fclose(r_o);
    close(p_e->read);
    close(p_o->read);
}

/**
 * @brief main function sets up data-structures and calls the appropriate functions when they are needed
 * @details sets up head of the floating-point list, reads first two lines and if the input is valid, the children
 *          are created and written into while still reading input numbers.
 *          If theres just one number put in, it is returned. 
 *          After the parallel I/O-process the cooley-tukey algorithm is computed and the result is returned.
 * 
 * @param argc argument-counter, must be 1
 * @param argv argument-variables, must only hold the program name
 * @return int exit-status
 */
int main(int argc, char** argv) {
    char *name = argv[0];
    if(argc != 1) usage_error(name);
        
    head = malloc(sizeof(struct FPListHead));
    struct FPList *first_two_lines;
    head->next = NULL;
    head->length = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
	
    while((nread = getline(&line, &len, stdin)) <= 1){
        if(nread == -1){
            fprintf(stderr, "ERROR in %s, specify at least one floating-point number\n", name);
            free(head);
            exit(EXIT_FAILURE);
        }
        if(nread == 1){
            if(line[0] == '\n') continue;
            else break;
        }
    }
    first_two_lines = addLine(line, name); 
    while((nread = getline(&line, &len, stdin)) <= 1){
        if(nread == -1){
            //only one complex number was specified
            fprintf(stdout, "%s", head->next->str);
            clearRes();
            exit(EXIT_SUCCESS);
        }
        if(nread == 1){
            if(line[0] == '\n') continue;
            else break;
        }
    }
    addLine(line, name);
    //fork to new processes
    if((p_e = malloc(sizeof(struct Communicator))) == NULL){
        clearRes();
        fprintf(stderr, "ERROR in %s, failed memory-allocation\n", name);
        exit(EXIT_FAILURE);
    }
    if((p_o = malloc(sizeof(struct Communicator))) == NULL){
        clearRes();
        fprintf(stderr, "ERROR in %s, failed memory-allocation\n", name);
        exit(EXIT_FAILURE);
    }
    myfork(p_e, name);
    myfork(p_o, name);
    feed_children(first_two_lines, name);

    while((nread = getline(&line, &len, stdin)) != -1){
        if(nread == 1){
            if(line[0] == '\n') continue;
        }
        feed_children(addLine(line, name), name);
    }

    if((head->length % 2) != 0){
        mywait(name);
        fprintf(stderr, "ERROR in %s, the input amount of complex numbers must be even!\n", name);
        clearRes();
        exit(EXIT_FAILURE);
    }

    if(close(p_e->write) != 0){
        fprintf(stderr, "ERROR in %s, failed to close pipe: %s", name, strerror(errno));
        if(close(p_o->write) != 0) fprintf(stderr, "ERROR in %s, failed to close pipe: %s", name, strerror(errno));
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(close(p_o->write) != 0){
        fprintf(stderr, "ERROR in %s, failed to close pipe: %s", name, strerror(errno));
        clearRes();
        exit(EXIT_FAILURE);
    }

    //read from children and compute Cooley-Tukey
    cooley_tukey_calc(name);
    print_result();

    int status1, status2;
    if(waitpid(p_e->pid, &status1, 0) == -1){
        waitpid(p_o->pid, &status2, 0);
        fprintf(stderr, "ERROR in %s, failed to wait for child-process\n", name);
        clearRes();
        exit(EXIT_FAILURE);
    }
    if(waitpid(p_o->pid, &status2, 0) == -1){
        fprintf(stderr, "ERROR in %s, failed to wait for child-process\n", name);
        clearRes();
        exit(EXIT_FAILURE);
    }
    clearRes();
    if(!WIFEXITED(status1) || !WIFEXITED(status2)){
        fprintf(stderr, "ERROR in %s, child-processes failed to exit\n", name);
        exit(EXIT_FAILURE);
    }
    if((WEXITSTATUS(status1) != EXIT_SUCCESS) || (WEXITSTATUS(status2) != EXIT_SUCCESS)){
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
