/**
 * @file forksort.c
 * @author Felix Olszewski <12022502@student.tuwien.ac.at>
 * @date 09.12.2021
 *
 * @brief Forksorts main program module.
 * @details 
 * This program reads any number of lines from stdin until an EOF is encountered. 
 * It sorts them alphabetically similar to merge-sort, 
 * where forked child-processes execute substeps and communication between processes happens 
 * via stdin which is redirected to pipes
 * The program takes no arguments.
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdbool.h>

char *program_name = NULL; /**< The program name. */


/** 
 * The struct that includes two pipe-endings for communication between a parent and child process. 
 * "in" is the ending to write into. "out" is the ending to read from. 
 * Also includes the process-id of the child, that was forked, which is used by waitpid.
 */
typedef struct {
    FILE *in;
    FILE *out;
    pid_t id;
} myPipe_t; 


static myPipe_t initializePipes(void);
void error_exit(void);

/**
 * Program entry point.
 * @brief main function, which: reads input, creates child-processes, passes on input to them, outputs child-output in a sorted manner
 * @details exits with usage and EXIT_FAILURE if something goes wrong
 * takes any number of lines as input (from stdin)
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS if successful, else EXIT_FAILURE
 */
int main(int argc, char **argv){
    program_name = argv[0];

    if (argc > 1) error_exit();

    size_t len = 0;
    char *line = NULL;

    if ((getline(&line, &len, stdin)) == -1) {
        error_exit();
    }
    char c;
    if ((c = fgetc(stdin)) == EOF){
        if (ferror(stdin) != 0) error_exit();
        fprintf(stdout, "%s", line);
        free(line);
        exit(EXIT_SUCCESS);
    } else {
        if (ungetc(c,stdin) != c) error_exit(); // returns c on success
    }

    // now initialize the children, such that the input can be distributed into the tree
    myPipe_t childA = initializePipes();
    myPipe_t childB = initializePipes();
    myPipe_t children[2] = {childA,childB};
    
    int alt_index = 0;
    // do while, cause getline was already called when doing the base-case check
    do { // der Parent-Prozess kann schonmal reinschreiben, obwohl die children die pipes noch garnicht (fertig) redirected haben müssten, es wird gepuffert
        if (ferror(stdin) != 0) error_exit();
        fprintf(children[alt_index].in, line); 
        alt_index++;
        alt_index %= 2;
    } while (getline(&line, &len, stdin) != -1);
    free(line);

    if (fclose(childA.in) != 0) error_exit(); // Upon successful completion 0 is returned.
    if (fclose(childB.in) != 0) error_exit();
    
    // wait for the child processes to have written their solution to stdout and exited
    int checkA = 0;
    int checkB = 0;
    if(waitpid(childA.id, &checkA, 0) == -1) error_exit(); // see manpage for what checkA is used for
    if(waitpid(childB.id, &checkB, 0) == -1) error_exit();
    if (checkA != EXIT_SUCCESS || checkB != EXIT_SUCCESS){
        error_exit();
    }
    
    char *lineA = NULL;
    char *lineB = NULL;
    size_t lenA = 0;
    size_t lenB = 0; 
    int a_has = getline(&lineA, &lenA, childA.out);
    int b_has = getline(&lineB, &lenB, childB.out);
    assert(a_has != -1); // condition: childB.out & childA.out for sure have >= 1 inside, cause base-case didn't trigger when reading!
    assert(b_has != -1);
    while ( (a_has != -1) || (b_has != -1) ){
        if ((a_has != -1) && (b_has != -1)){
            if (strcmp(lineA,lineB) <= 0) {
                fprintf(stdout, lineA);
                a_has = getline(&lineA, &lenA, childA.out);
            } else {
                fprintf(stdout, lineB);
                b_has = getline(&lineB, &lenB, childB.out);
            }
        } else if (a_has != -1) { // all of b printed already
            fprintf(stdout, lineA);
            a_has = getline(&lineA, &lenA, childA.out);
        } else if (b_has != -1) {
            fprintf(stdout, lineB);
            b_has = getline(&lineB, &lenB, childB.out);
        } else {
            assert(false); // defensive programming
        }
    }

    free(lineA);
    free(lineB);
    if(fclose(childA.out) == -1) error_exit(); // does not *have to be* checked because
    if(fclose(childB.out) == -1) error_exit(); // further program execution does not depend on its successful execution
    return EXIT_SUCCESS; // this is what triggers wait-pid, when in a childprocess (except with leaf-nodes)
}

/**
 * Pipe-initialization.
 * @brief initializes two pipes, forks a process, executes main() again in child-process
 * @details initializes one pipe for writing to child-proceses (stdinpipe) and one to read from child-processes (stdoutpipe)
 * the forked child-process uses execl to execute the main method (instead of its to be executed program-lines)
 * @return Returns a myPipe_t struct, which contains both pipes and the process id of the child
 */
static myPipe_t initializePipes(void){
    int stdinpipe[2];
    int stdoutpipe[2];

    if ((pipe(stdinpipe)) == -1) error_exit();
    if ((pipe(stdoutpipe)) == -1) error_exit();

    pid_t pid = fork(); // ---------------------------FORKING-----------------------------
    if (pid == -1) error_exit();
    myPipe_t pipe_info = {0};

    switch (pid) {
        case 0: // child process
            if (close(stdinpipe[1]) == -1) error_exit();
            if (dup2(stdinpipe[0], STDIN_FILENO) == -1) error_exit(); // replace the file-descriptor of STDIN_FILENO with stdinpipe[0], which is the read-end
            // so now instead of inut being read from stdin, input will be read from the parent-process-pipe

            if(close(stdoutpipe[0]) == -1) error_exit();
            if(dup2(stdoutpipe[1], STDOUT_FILENO) == -1) error_exit(); // replace the file-descriptor of STDOUT_FILENO with stdoutpipe[1], which is the write-end
            if(execl(program_name, program_name, NULL) == -1) error_exit(); // main gets executed again
            break;
        default: // parent process
            if(close(stdinpipe[0]) == -1) error_exit();
            if(close(stdoutpipe[1]) == -1) error_exit();
            pipe_info.in = fdopen(stdinpipe[1], "w");
            pipe_info.out = fdopen(stdoutpipe[0], "r");
            pipe_info.id = pid;
            if (pipe_info.in == NULL || pipe_info.out == NULL) error_exit();
            break;
    }
    return pipe_info;
}

/**
 * Print usage, print errno, exit.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: program_name
 */
void error_exit(void){
    fprintf(stderr,"USAGE: %s (add \" < <filename>.txt \" to replace stdin with <filename>.txt)\n", program_name);
    fprintf(stderr,"Error Code: %d\n",errno);
    exit(EXIT_FAILURE);
}