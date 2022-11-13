#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

char* programName = "Not set";
typedef struct{
    int fd[2];
    FILE* access;
} pipe_t;

typedef struct {
    pid_t pid;
    pipe_t inputP;
    pipe_t outputP;

} child_t;

child_t child1;
child_t child2;

//FUNCTION DECLARATIONS
static int readLine(FILE* file, char **line);
static void startChildProcess(child_t* child);
static inline void error_exit(char* message);
static void splitLinesToChildren(char **line);
static void outputRemainingLines(FILE *output, char** line);
static void outputChildrenResult(char** line, char** line2);
static inline void tryWaitForChildCompletion(pid_t pid_child);
static inline void check(int returnValue, char* errMessage);

/**
 * @brief Marks entry point of the program, parses arguments, reads in lines from stdin with the goal of alphabetically ordering said lines
 * 
 * @details Continuously reads in lines from stdin and initializes two children for each process, delegates the read in lines equally to both children, 
 * program communication functions via two pipes, inputPipe and outputPipe, whereas the inputPipe serves the purpose of writing to the child processes and the output pipe
 * functions as a means to read out the ordered results of the children. When both children finish writing their output, the parent process compares each line from the two 
 * output pipes and outputs the ordered result.
 * 
 * @param argc number of program arguments should be one
 * @param argv Should only contain program name in argv[0]
 * @return int Returns EXIT_SUCCESS on proper program exit and on error EXIT_FAILURE otherwise
 */
int main(int argc, char *argv[])
{
    programName = argv[0];
   
    if(argc != 1)
    {
        error_exit("Too many arguments Usage:  [./forksort < filename]");
    }
    char* line = malloc(10*sizeof(char));
    int terminatedOnEOF = readLine(stdin, &line);
    //If there is only one line, exit program otherwise split lines
    if(terminatedOnEOF)
    {
        fprintf(stdout,"%s",line);
        free(line);
        exit(EXIT_SUCCESS);
    }
    //Initialize children
    startChildProcess(&child1);
    startChildProcess(&child2);

    //Open pipe access Input
    child1.inputP.access = fdopen(child1.inputP.fd[1],"w");
    child2.inputP.access = fdopen(child2.inputP.fd[1],"w");

    //Split lines to children
    splitLinesToChildren(&line);
   
    //Close input pipes
    fclose(child1.inputP.access);
    fclose(child2.inputP.access);

    //Read output from children
    child1.outputP.access = fdopen(child1.outputP.fd[0], "r");
    child2.outputP.access = fdopen(child2.outputP.fd[0], "r");

    char* output_line = malloc(sizeof(line) * 10);

    //Write result
    outputChildrenResult(&line,&output_line);
    
    //Close read access
    fclose(child1.outputP.access);
    fclose(child2.outputP.access);
    
    //Wait for children to complete
    tryWaitForChildCompletion(child1.pid);
    tryWaitForChildCompletion(child2.pid);

    free(line);
    free(output_line);
    return EXIT_SUCCESS;
}

/**
 * @brief Compares the outputs of child 1 and child 2 line-wise and outputs the smaller string of the two, continuing until no strings are left in either end of the pipes
 * 
 * @details Uses pointers line and line2 to store the output of the output pipe of both children and compares them, outputting the smaller of the two first and continuing to do
 * so until the buffer is empty. If one buffer reaches its end before the other, the remaining strings in the yet to be emptied buffer is written to stdout.
 * 
 * @param line char pointer, pointing to output of the output pipe of child 1
 * @param line2 char pointer, pointing to output of the output pipe of child 2
 */
static void outputChildrenResult(char** line, char** line2)
{
    int c1finished = 0;
    int c2finished = 0;
  
    c1finished = readLine(child1.outputP.access, line);
    c2finished = readLine(child2.outputP.access, line2);
    
    (*line)[strcspn(*line,"\n")] = '\0';
    (*line2)[strcspn(*line2,"\n")] = '\0';
  
    do
    { 
        //fprintf(stderr,"Comparing: %s, %s  \n", *line, *line2);
        if(strcmp(*line,*line2) <= 0)
        {
            //fprintf(stderr,"Branch 1: line:%s, line2:%s  \n", *line, *line2);
            if(c1finished == 1)
            {
                if((*line)[0] != 0)
                {
                    check(fprintf(stdout,"%s%c", *line,'\n'),"Failed to write to stdout");  
                }
                outputRemainingLines(child2.outputP.access,line2);
                break;
            }
            else
            {
                if((*line)[0] != 0)
                {
                    check(fprintf(stdout,"%s%c", *line, '\n'),"Failed to write to stdout");
                }
            }
           c1finished = readLine(child1.outputP.access, line);
           if(c1finished == 1 && c2finished == 1)
           {
               outputRemainingLines(child2.outputP.access,line2);
           }
           (*line)[strcspn(*line,"\n")] = '\0';
        }
        else
        {
            if(c2finished == 1)
             {
                if((*line2)[0] != 0)
                {
                    check(fprintf(stdout,"%s%c", *line2,'\n'),"Failed to write to stdout");  
                }
                outputRemainingLines(child1.outputP.access,line);
                break;
            }
            else
            {
                if((*line2)[0] != 0)
                {
                    check(fprintf(stdout,"%s%c", *line2, '\n'),"Failed to write to stdout");
                }
            }
            c2finished = readLine(child2.outputP.access, line2);
            if(c1finished == 1 && c2finished == 1)
            {
               outputRemainingLines(child1.outputP.access,line);
            }
            (*line2)[strcspn(*line2,"\n")] = '\0';
        }
        
    } while (!(c1finished && c2finished) );
    
}
/**
 * @brief Outputs remaining lines of the specified file descriptor (buffer)
 * 
 * @param output File descriptor of buffer containing lines yet to be output
 * @param line Char pointer holding the current line
 */
static void outputRemainingLines(FILE *output, char** line)
{
    int endedOnEOF = 0;
    do
    {
        if(endedOnEOF == 1)
        {
            break;
        }
        if(*line[0] != 0)
        {
            check(fprintf(stdout, "%s%c", *line,'\n'),"Failed to write to stdout!");
        }
        endedOnEOF = readLine(output, line);
        
    } while (!endedOnEOF);
    
    
}
/**
 * @brief Splits lines read in through stdin to the children of the current process
 * 
 * @details The alternator switches between 0 and 1 and distributes the lines alternatingly between the first and second child, Empty lines are not forwarded
 * And when passing on the last line of the current input the newline is removed indicating the end of an input. This process is repeated until an EOF is reached.
 * 
 * @param line Char pointer, pointing to the most recent line read in.
 */
static void splitLinesToChildren(char **line)
{
    int alternator = 0;
    int endedOnEOF = 0;
   
    char* currline = malloc(sizeof(*line)* 10);

    do
    {       
        endedOnEOF = readLine(stdin, &currline);
        
        if(endedOnEOF == 1)
        {
            currline[strcspn(currline,"\n")] = '\0';
            (*line)[strcspn(*line,"\n")] = '\0';
            if(currline[0] != 0)
            {
                check(fprintf(child1.inputP.access,"%s",currline),"Error writing line");
            }
            if((*line)[0] != 0)
            {
                check(fprintf(child2.inputP.access,"%s",*line),"Error writing line");
            }
            break;
        }
        if(alternator == 0)
        {
            if((*line)[0] != 0)
            {
                check(fprintf(child1.inputP.access,"%s", *line),"Error writing line");
            }
            alternator = 1;
        }else{
            if((*line)[0] != 0)
            {
                check(fprintf(child2.inputP.access,"%s", *line),"Error writing line");
            }
            alternator = 0;
        }
        strcpy(*line, currline);
        
    } while (!endedOnEOF);
    
    free(currline);
    
}

/**
 * @brief Checks the return value of volatile method and in case of an error, exits program and returns error message
 * 
 * @param returnValue Value returned by the called method 
 * @param errMessage Message detailing what type of error occured
 */
static inline void check(int returnValue, char* errMessage)
{
    if(returnValue < 0)
    {
        error_exit(errMessage);
    }
}

/**
 * @brief Creates pipes, Redirects pipes and forks followed by an exec, creating a new process image
 * 
 * @param child Specifies child to start process
 */
static void startChildProcess(child_t* child)
{
    //Create Pipes
    check(pipe(child->inputP.fd),"Could not init pipe!");
    check(pipe(child->outputP.fd),"Could not init pipe!");

    check(child->pid = fork(),"Fork failed!");
    
    if(child->pid == 0)
    {
        check(close(child->inputP.fd[1]),"Failed to close write end of input Pipe!");
        check(dup2(child->inputP.fd[0],STDIN_FILENO),"Failure duplicating file descriptor");
        check(close(child->inputP.fd[0]),"Failed to close read end of input Pipe!");

        check(close(child->outputP.fd[0]),"Failed to close read end of output Pipe!");
        check(dup2(child->outputP.fd[1],STDOUT_FILENO),"Failure duplicating file descriptor");
        check(close(child->outputP.fd[1]),"Failed to close write end of output Pipe!");
        
        if(child == &child2)
        {
            check(close(child1.inputP.fd[1]),"Failed to close write end of input pipe!");
            check(close(child1.outputP.fd[0]),"Failed to close write end of input pipe!");
        }
        
        if(execlp(programName,programName, NULL) == -1)
        {
            error_exit("Could not create new process image");
        }
        //Cannot be reached
    }
    check(close(child->inputP.fd[0]),"Failed to close read end of input Pipe!");
    check(close(child->outputP.fd[1]),"Failed to close write end of input Pipe");

}
/**
 * @brief Reads in line from specified file pointer and stores reference in line pointer
 * 
 * @param file File descriptor to read string from
 * @param line char pointer referencing the line read in
 * @return int Returns 0 upon reading in a line that ends on EOF, otherwise returns 1 indicating that there are more lines to read in
 */
static int readLine(FILE* file, char **line)
{
    int terminatedOnEOF = 0;
    int curr_char;
    int i = 0;
    do
    {
        *line = realloc(*line, (i+1 )* sizeof(char));
        curr_char = fgetc(file);
        if(curr_char == EOF)
        {
            terminatedOnEOF = 1;
            break;
        }
        (*line)[i] = (char) curr_char;
        i++;

    } while (curr_char != '\n');
    
    (*line)[i] = '\0';
    if(terminatedOnEOF)
    {
        return 1;
    }else
    {
        return 0;
    }
}

/**
 * @brief Exit program upon error and output error message specified by the function argument
 * 
 * @param message Short string detailing the specifics of the error occured
 */
static inline void error_exit(char* message)
{
    fprintf(stderr,"Error message: %s, Process ID: %d, ERRNO: %s", message, getpid(),strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Waits for child process to be finished. In case the child doesnt terminate with "EXIT_SUCCESS", an error message is returned and the program exits
 * 
 * @param pid_child Child process to await completion
 */
static inline void tryWaitForChildCompletion(pid_t pid_child)
{
    int status;
    while (waitpid(pid_child, &status, 0) == -1) {
        if (errno == EINTR)
        {
            continue;
        }
        error_exit("Waiting for child process failed!");
    }
    if(WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        error_exit("Exit status of child not EXIT_SUCCESS!");
    }

  
}