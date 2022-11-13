/**
 * @file intmul.c
 * @author Nico Lehegzek 12021356
 * @date 5.12.2021
 *  
 * @brief Main program module. It calculates the product of two hexadecimal numbers (recursively if necessary). 
 * 
 * @details This program calculates the product of two hexadecimal numbers written to stdout.
            If the amount of digits of the numbers are equal and the input is of correct form there are
            following two possibilities how the program solves the multiplication.
            If the length of the numbers is one it instantly writes the result to stdout.
            Otherwise it creates four child processes and gives each of them one part of
            the equation. Meaning the calculation occurs recursively.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <sys/wait.h>

/**
 * @brief This struct stores the necessary pipe information for a process
 * 
 * @details It contains the pid (identification of the pipe);
            And the pipes needed for the communication between the child
            and the parent.
            There are 2 pipes needed for each child:
            1st: Parent writes values to stdin from child
            2nd: Parent reads from stdout of child
 */
typedef struct
{
    pid_t pid;
    int p_c[2];
    int c_p[2];
}pipE;

/**
 * @brief This struct stores the result of all four childs
 * 
 * @details A1_res: Result of child one (A(h) * B(h));
            A2_res: Result of child two (A(h)*B(l));
            B1_res: Result of child three (A(l)*B(h));
            B2_res: Result of child four (A(l)*B(l));
 */
typedef struct
{
    char *A1_res;
    char *A2_res;
    char *B1_res;
    char *B2_res;
}resulT;


static char *progName; //*< this stores the program name*/

static pipE pipes[4]; //*< this stores the pipes to all four children*/
static resulT reS; //*< this stores the result of the children*/
static char*nA; //*< this stores the first number read from stdin*/
static char*nB; //*< this stores the second number read from stdin*/

/**
 * @brief This function writes the correct synopsis of this program to stderr
 * 
 * @details exits the program with the code EXIT-FAILURE after output to stderr
 *          global variable: progName - is the program name as written in argv[0]
 * 
 * @return has no return values (void) 
 */
static void usage(void){
    fprintf(stderr,"USAGE:%s\n",progName);
    exit(EXIT_FAILURE);
}

/**
 * @brief initializes all needed global variables
 * 
 * @details initializes the global variables to avoid unexpected behaviour
 *          nA - set to NULL
 *          nB - set to NULL
 *          reS - everything set to NULL
 *          pipes - pid and pipes set to -1
 *                 
 * @param void - no parameters 
 *        
 * 
 * @return void - no return value
 */
static void initializeValues(void){
    nA = NULL;
    nB = NULL;
    reS.A1_res = NULL;
    reS.A2_res = NULL;
    reS.B1_res = NULL;
    reS.B2_res = NULL;
    for(int i = 0; i < 4; i++){
        pipes[i].pid    = -1;
        pipes[i].p_c[0] = -1;
        pipes[i].p_c[1] = -1;
        pipes[i].c_p[0] = -1;
        pipes[i].c_p[1] = -1;
    }
}

/**
 * @brief reads the first two lines from stdin and checks for inconsistencies
 * 
 * @details reads from stdin using getline since input can be of infinite length
 *          after reading the numbers the function checks if they are given as hexadecimals and
 *          if both numbers have an equal amount of digits.
 *          global variable: nA - the first number read gets stored here
 *                           nB - the second number read gets stored here
 *                 
 * @param void - no parameters       
 * 
 * @return returns 0 on success
 *         returns -1 if getline failed
 *         returns -2 if the two numbers are not of equal length
 *         returns -3 if one of the numbers is not given as hexadecimal
 */
static int readNumbers(void){
    size_t line = 0;    
    ssize_t result;

    // read the first number
    result =  getline(&nA, &line, stdin);

    if(result == - 1){
        fprintf(stderr, "an error ocurred while reading from stdin\n");
        return -1;
    }

    // read the second number
    result =  getline(&nB, &line, stdin);

    if(result == - 1){
        fprintf(stderr, "an error ocurred while reading from stdin\n");
        return -1;
    }

    // check if the number of digits of both numbers are equal
    if(strlen(nA)!=strlen(nB)){
        fprintf(stderr, "the amount of digits is not equal\n");
        return -2;
    }

    // check if both numbers are in hexadecimal representation
    for (int i = 0; i < strlen(nA)-1; i++){
        if((nA[i]<'0'||nA[i]>'9')&&(nA[i]<'A'||nA[i]>'F')&&
           (nA[i]<'a'||nA[i]>'f')){
            fprintf(stderr, "one of the numbers was not a hexadecimal number\n");
            return -3;
        }

        if((nB[i]<'0'||nB[i]>'9')&&(nB[i]<'A'||nB[i]>'F')&&
           (nB[i]<'a'||nB[i]>'f')){
            fprintf(stderr, "one of the numbers was not a hexadecimal number\n");
            return -3;
        }
    }
    
    return 0;
}

/**
 * @brief splits the two numbers in four parts.
 * 
 * @details all parts contain (strlen(x)-1)/2 characters
*           global variable: nA - reads their value to copy parts into the corresponding pointers
 *                           nB - reads their value to copy parts into the corresponding pointers
 *     
 * @param **A1 - first half of nA gets at this address 
 *        **A2 - second half of nA gets at this address 
 *        **B1 - first half of nB gets at this address 
 *        **b2 - second half of nB gets at this address 
 * 
 * @return void - no return value
 */
static void splitNumbers(char**A1, char**A2, char**B1,char**B2){

    // -1 to remove \'n'
    int strLenA = strlen(nA)-1;
    int strLenB = strlen(nB)-1;

    // +1 for '\0'
    // memset to initialize the string with zeros
    *A1 = malloc(sizeof(char)*(strLenA/2)+1);
    memset(*A1, 0, (strLenA/2)+1);
    *A2 = malloc(sizeof(char)*(strLenA/2)+1);
    memset(*A2, 0, (strLenA/2)+1);

    *B1 = malloc(sizeof(char)*(strLenB/2)+1);
    memset(*B1, 0, (strLenA/2)+1);
    *B2 = malloc(sizeof(char)*(strLenB/2)+1);
    memset(*B2, 0, (strLenA/2)+1);

    // copy the corresponding values to the values
    memcpy(*A1,nA,(strLenA/2));
    memcpy(*A2,nA+(strLenA/2),(strLenA/2));

    memcpy(*B1,nB,(strLenB/2));
    memcpy(*B2,nB+(strLenB/2),(strLenB/2));
}

/**
 * @brief creates the pipes for all four children
 * 
 * @details Four pipes are created since every multiplication gets split into
 *          four parts if necessary 
 *          global variable: pipes - to store the created information
 *                 
 * @param void - no parameters 
 *        
 * 
 * @return returns 0 on success and -1 on failure
 */
static int createPipes(void){
    for (int i = 0; i < 4; i++)
    {
        // first pair of pipes to redirect to stdin of child
        if(pipe(pipes[i].p_c)==-1){
            fprintf(stderr,"Error creating pipe: %s\n",strerror(errno));
        return -1;
        }
        
        // second pair of pipes to redirect stdout of child
        if(pipe(pipes[i].c_p)==-1){
            fprintf(stderr,"Error creating pipe: %s\n",strerror(errno));
        return -1;
        }
    }
    
    return 0;
}


/**
 * @brief creates four child processes using fork(), duplicates opened fd's using dup2(), 
 *        closes not needed pipes and loads a new program into a process's mem using exec
 * 
 * @details redirects the first pipe to the stdin of the child (to share data) and
 *          the stdout of the child to the second pipe
 *          closes all unneeded pipes for children and parents
 *          specified exec:
 *          - ...l... - variable number of arguments
 *          - ...p    - searching the environment variable $PATH for the program specified
 *          global variable: pipes - to read the necessary pipe information and store created information
 *                           progName - for exec
 *                 
 * @param void - no parameters 
 *        
 * 
 * @return returns 0 on success and -1 on failure
 */
static int forkAndClose(void){
    for (int i = 0; i < 4; i++){
        // stores the pid of each child
        pipes[i].pid = fork();
        switch (pipes[i].pid)
        {
        // error
        case -1:
            fprintf(stderr,"Error forking: %s\n",strerror(errno));
            return -1;
            break;
        // child process
        case 0:
            //close not needed pipes
            if(close(pipes[i].p_c[1])==-1){
                fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
                return -1;
            }

            if(close(pipes[i].c_p[0])==-1){
                fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
                return -1;
            }

            // redirect pipes as needed

            // The parent shall write the values to be multiplied to stdin of the child (via the pipe)
            if(dup2(pipes[i].p_c[0],STDIN_FILENO)==-1){
                fprintf(stderr,"dup2-error: %s!\n",strerror(errno));
                return -1;
            }
            //The child shall write its value to it's stdout which is redirected via the pipe (to the parent)
            if(dup2(pipes[i].c_p[1],STDOUT_FILENO)==-1){
                fprintf(stderr,"dup2-error: %s!\n",strerror(errno));
                return -1;
            }

            //close not redirected pipes
            if(close(pipes[i].p_c[0])==-1){
                fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
                return -1;
            }
            if(close(pipes[i].c_p[1])==-1){
                fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
                return -1;
            }
            
            // Load a new program into the process's memory
            if(execlp(progName,progName,NULL,NULL)==-1){
                fprintf(stderr,"error using execlp: %s!\n",strerror(errno));
                return -1;
            }

            break;
        default:
            // nothing to do here
            break;
        }

        // close not needed pipes
        if(close(pipes[i].p_c[0])==-1){
            fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
            return -1;
        }
        if(close(pipes[i].c_p[1])==-1){
            fprintf(stderr,"error closing pipe: %s!\n",strerror(errno));
            return -1;
        }
    }

    return 0;
}

/**
 * @brief writes the corresponding parts of the multiplication to the stdin of the children
 * 
 * @details writes the four combinations to the four childs:
 *              1) First child gets A(h) and B(h)
 *              2) Second child gets A(h) and B(l)
 *              3) Third child gets A(l) and B(h)
 *              4) Fourth child gets A(l) and B(l)
*           global variable: pipes - to read the necessary pipe information
 *                 
 * @param *A1: first half of numberA
 *        *A2: second half of numberA
 *        *B1: second half of numberB
 *        *B2: first half of numberB
 *        
 * 
 * @return returns 0 on success and -1 on failure
 */
static int shareData(char *A1, char *A2, char *B1, char *B2){

    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        // open "stdin" of corresponding child
        FILE* out = fdopen(pipes[i].p_c[1],"w");
        if(out == NULL){
            fprintf(stderr,"error using fdopen: %s!\n",strerror(errno));
            return -1;
        }

        // first child
        if(count == 0){
            if(fprintf(out,"%s\n",A1)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
            if(fprintf(out,"%s\n",B1)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
        }else if(count == 1){
            // second child
            if(fprintf(out,"%s\n",A1)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
            if(fprintf(out,"%s\n",B2)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
        }else if(count == 2){
            // third child
            if(fprintf(out,"%s\n",A2)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
            if(fprintf(out,"%s\n",B1)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
        }else if(count == 3){
            // fourth child
            if(fprintf(out,"%s\n",A2)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
            if(fprintf(out,"%s\n",B2)<0){
                fprintf(stderr,"error writing to pipe: %s!\n",strerror(errno));
                return -1;
            }
        }else{
            //should never be reached
            assert(0);
        }
        
        count++;

        // close stdin of the correspind child
        if(fclose(out)==EOF){
            fprintf(stderr,"error closing file: %s!\n",strerror(errno));
            return -1;
        }
    }

    return 0;
    
}

/**
 * @brief reads the results produced by the child processes
 * 
 * @details reads the results written to stdout of the child using the formally created pipes.       
 *          global variable: pipes - to access necessary information
 *                           reS - the results of the children get stored here
 *                 
 * @param void - no parameters   
 * 
 * @return returns 0 on success and -1 otherwise
 */
static int readResult(void){
    
    // open the pipe corresponding to the stdout of the first child
    FILE *in1 = fdopen(pipes[0].c_p[0],"r");
    if(in1 == NULL){
        fprintf(stderr,"error using fdopen: %s!\n",strerror(errno));
        return -1;
    }
    
    // open the pipe corresponding to the stdout of the second child
    FILE *in2 = fdopen(pipes[1].c_p[0],"r");
    if(in1 == NULL){
        fprintf(stderr,"error using fdopen: %s!\n",strerror(errno));
        return -1;
    }

    // open the pipe corresponding to the stdout of the third child
    FILE *in3 = fdopen(pipes[2].c_p[0],"r");
    if(in1 == NULL){
        fprintf(stderr,"error using fdopen: %s!\n",strerror(errno));
        return -1;
    }

    // open the pipe corresponding to the stdout of the fourth child
    FILE *in4 = fdopen(pipes[3].c_p[0],"r");
    if(in1 == NULL){
        fprintf(stderr,"error using fdopen: %s!\n",strerror(errno));
        return -1;
    }


    size_t line = 0;    
    ssize_t result;

    // read the result of the first child using getline
    if((result =  getline(&reS.A1_res, &line, in1)) == -1){
        fprintf(stderr,"error reading from child: %s!\n",strerror(errno));
        return -1;
    };

    // read the result of the second child using getline
    if((result =  getline(&reS.A2_res, &line, in2)) == -1){
        fprintf(stderr,"error reading from child: %s!\n",strerror(errno));
        return -1;
    };

    // read the result of the third child using getline
    if((result =  getline(&reS.B1_res, &line, in3)) == -1){
        fprintf(stderr,"error reading from child: %s!\n",strerror(errno));
        return -1;
    };

    // read the result of the fourth child using getline
    if((result =  getline(&reS.B2_res, &line, in4)) == -1){
        fprintf(stderr,"error reading from child: %s!\n",strerror(errno));
        return -1;
    };
    
    // close the pipe corresponding to stdout of child 1
    if(fclose(in1) == EOF){
        fprintf(stderr,"error closing file: %s!\n",strerror(errno));
        return -1;
    }
    // close the pipe corresponding to stdout of child 2
    if(fclose(in2) == EOF){
        fprintf(stderr,"error closing file: %s!\n",strerror(errno));
        return -1;
    }
    // close the pipe corresponding to stdout of child 3
    if(fclose(in3) == EOF){
        fprintf(stderr,"error closing file: %s!\n",strerror(errno));
        return -1;
    }
    // close the pipe corresponding to stdout of child 4
    if(fclose(in4) == EOF){
        fprintf(stderr,"error closing file: %s!\n",strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief waits for the four child processes to terminate
 * 
 * @details waits for the four child proccesses using wait.
 *          check the corresping exit status of the children;
 *          if it was EXIT_FAILURE; the program will be terminated
 *          with code EXIT_FAILURE.
 *          In this case the order of the children finishing does not matter.
 *                 
 * @param void - no parameters   
 * 
 * @return returns 0 on success and -1 otherwise
 */
static int waitForChildren(void){
    int status, count = 0;
    
    while(count < 4){
        // wait for next children
        wait(&status);

        // continue if child got interrupted
        if(errno == EINTR)
            continue;

        // check the status of the correspinding child
        if(WEXITSTATUS(status) == EXIT_FAILURE){
            fprintf(stderr,"error occurred in child process!\n");
            return -1;
        }

        count++;
    }


    return 0;
}

/**
 * @brief add the results of the four child processes together noting offsets
 * 
 * @details the results is either n*2 or n*2-1 long. There is no other option
 *          since both numbers have to be equally long.
 *          checks the position to add offset given in forumala (16^n/2) or (16^n)
 *          calculates the hexadecimal sum from the back, since the int can be of any size
 *          and the carry needs too be noted.
*          global variable: nA - to access original length of input
 *                          reS - to read the results of the four children
 *                 
 * @param void - no parameters   
 * 
 * @return returns 0 on success and -1 otherwise
 */
static int calculate(void){
    
    //carry will be stored here
    int c = 0;
    // -2 because of '\0' and '\n'
    int lenA1 = strlen(reS.A1_res)-2;
    int lenA2 = strlen(reS.A2_res)-2;
    int lenB1 = strlen(reS.B1_res)-2;
    int lenB2 = strlen(reS.B2_res)-2;

    // create result string with max length of original length*2
    // +1 for '\0'
    char res[(strlen(nA)-1)*2+1];

    // calculate the result beginning at ones digit
    for(int i = (strlen(nA)-1)*2-1; i >= 0; i--){
        int number1 = 0, number2 = 0, number3 = 0, number4 = 0, sum = 0;

        // result at pos x will be stored in h[0]
        char h[2];
        h[1] = '\0';

        // case 1: A(l) * B(l) (Position is smaller than half)
        // only number1 will be set all other numbers will be 0
        if(lenB2 >= 0){
            h[0] = reS.B2_res[lenB2];
            number1 = (int)strtol(h,NULL,16);
            lenB2--;
        }
        
        // case 2: A(h) * B(l) * 16^(n/2) + A(l) * B(h) * 16^(n/2) + A(l) * B(l) 
        //         (since we reached the specific position -> bigger than half)
        // number4 might remain 0 - offset of 16^n/2 reached
        if(i < (strlen(nA)-1) + (strlen(nA)-1)/2){
            if(lenA2 >= 0){
                h[0] = reS.A2_res[lenA2];
                number2 = (int)strtol(h,NULL,16);
                lenA2--;
            }
            if(lenB1 >= 0){
                h[0] = reS.B1_res[lenB1];
                number3 = (int)strtol(h,NULL,16);
                lenB1--;
            }

            // case 3: A(h) * B(h) * 16^(n) + A(h) * B(l) * 16^(n/2) + A(l) * B(h) * 16^(n/2) + A(l) * B(l) 
            //(since we reached the specific position -> position that exceeds original length)
            // number 4 will be set accordingly - offset of 16^n reached
            if(i < (strlen(nA)-1)){
                if(lenA1 >= 0){
                    h[0] = reS.A1_res[lenA1];
                    number4 = (int)strtol(h,NULL,16);
                    lenA1--;
                }
            }
        }       

        //create sum of the child calculations
        sum = number1 + number2 + number3 + number4 + c;

        // turn int into char
        char help[2];
        sprintf(help,"%x",(sum%16));
        
        //store result at pos i accordingly
        res[i] = help[0];

        //store carry if necessary
        c = sum/16;
    }

    // terminate result with '\0'
    res[(strlen(nA)-1)*2] = '\0';

    // print result to stdout
    if(fprintf(stdout, "%s\n",res)<0){
        fprintf(stderr, "error printing result\n");
        return -1;
    }
    return 0;
    /*
    Note: I don't explicitly use 16^(n) or 16^(n/2) since it is not needed.
    I just start using the corresponding calculations starting at the corresponding positions.
    --> Meaning when I for instance start using A(h) * B(l) we have already reached the 16^(n/2)
    position (offset) - and therefore I implicitly considered the factor. 
    Same goes for the other parts of the equation.
    */
}


/**
 * @brief frees the memory of the given pointers
 * 
 * @details the four pointers store the results of the child processes.
 *          the function gets called before terminating the program if the
 *          function splitNumbers was already called.
 *                 
 * @param **A1 - result of first child that shall be freed
 *        **A2 - result of second child that shall be freed
 *        **B1 - result of third child that shall be freed
 *        **B2 - result of fourth child that shall be freed
 * 
 * @return void - returns nothing
 */
static void exitHelper(char **A1, char **A2, char **B1, char **B2){

    // check if the pointers were initialized before freeing them 
    if(*A1 != NULL)
        free(*A1);
    if(*B1 != NULL)
        free(*B1);
    if(*A2 != NULL)
        free(*A2);
    if(*B2 != NULL)
        free(*B2);
}


/**
 * @brief this function gets registered with atexit()
 * 
 * @details this function frees the global variables numberA, numberB and reS
 *          global variable: nA - free if intialized
 *                           nB - free if intialized
 *                           reS      - free all intialized values
 *                 
 * @param void - no parameters   
 * 
 * @return void - no return value
 */
static void exitFunction(void){
    // check if the pointers were initialized before freeing them 
    if(nA != NULL)
        free(nA);
    if(nB != NULL)
        free(nB);
    if(reS.A1_res != NULL)
        free(reS.A1_res);
    if(reS.A2_res != NULL)
        free(reS.A2_res);
    if(reS.B1_res != NULL)
        free(reS.B1_res);
    if(reS.B2_res != NULL)
        free(reS.B2_res);
}

/**
 * @brief the main entry of this module. It allocates all of the needed memory
 *        and calls all function to calculate the product of the two hexadecimal numbers
 * 
 * @details it also sets up the function called for error handling and repeats the
 *          calculation until the supervisor is closed or the generator is closed.
 *          global variables:  progName - the program name as written in argv[0]
 *                             countEdges - the edges of the last result
 *                             countNodes - the amount of nodes of the last result
 *                             freeSpace - the free space semaphore
 *                             buffer - the circular buffer
 *     
 * @param argc: contains the amount of arguments
 *        argv[]: must only contain the name of the program
 * 
 * @return Returns EXIT_FAILURE if an error ocurred somewhere during the process.
 *         Returns EXIT_SUCCESS otherwise. 
 */
int main(int argc, char*argv[])
{
    // store program name
    progName = argv[0];
    
    // check if no arguments were given
    // if some were found print usage and exit with EXIT_FAILURE
    if((argc-optind)>0){
        usage();
    }

    
    // setup exit function
    if(atexit(exitFunction)!=0){
        fprintf(stderr, "error setting up atexit function");
        exit(EXIT_FAILURE);
    }

    // initialize global variables
    initializeValues();

    // read first two lines from stdin
    int res = readNumbers();

    // error ocurred while reading from stdin or numbers do not meet the requirements
    if(res == -1 || res == -2 || res == -3){
        exit(EXIT_FAILURE);
    }
    
    // checks if the numbers only consist of one digit
    if((strlen(nA)-1)==1)
    {
        // if true the product is calculated and written to stdout
        int number1 = (int)strtol(nA,NULL,16), number2 = (int)strtol(nB,NULL,16);
        fprintf(stdout,"%x\n",(number1*number2));
    }else
    {
        // otherwise further action is needed

        // if the amount of digits is not even the program terminates with EXIT_FAILURE
        if((strlen(nA)-1)%2!=0||(strlen(nB)-1)%2!=0){
            fprintf(stderr, "amount of digits in numbers not even\n");
            exit(EXIT_FAILURE);
        }

        char* A1 = NULL;
        char* A2 = NULL;
        char* B1 = NULL;
        char* B2 = NULL;
        // determine the four parts to pass on to children
        splitNumbers(&A1, &A2, &B1, &B2);
        
        // create the pipes to communicate with children
        if(createPipes()==-1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }

        // fork respectively and close not needed ends of pipes
        if(forkAndClose()==-1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }

        // share the corresponding parts of the numbers with the children
        if(shareData(A1, A2, B1, B2)==-1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }

        // wait for all four children and check their exit code
        if(waitForChildren() == -1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }

        // read the result from the children
        if(readResult()==-1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }
        
        // calculate the result using the results from the children
        if(calculate()==-1){
            exitHelper(&A1, &A2, &B1, &B2);
            exit(EXIT_FAILURE);
        }
        
        // free everyhing
        exitHelper(&A1, &A2, &B1, &B2);
            
    }
    
    exit(EXIT_SUCCESS);
}