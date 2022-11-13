/**
 * @file forksort.c
 * @author Nadejda Capova 11923550
 * @date 11.12.2021
 *
 * @brief Main program module.
 * 
 * Implementation of algorithm which sorts lines alphabetically. 
 **/

 #include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <sys/wait.h>


/**
* @details constants: SIZE
*/
#define SIZE 1024
/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 */
void usage(void)
{
    fprintf(stderr, "forksort\n");
    exit(EXIT_FAILURE);
}


/**
 * Program entry point.
 * @brief The program starts here. The input of the program are multiple lines which should be read from stdin. 
 * The sequence ends when an EOF (End Of File) is encountered.
 * @details The program use a recursive variant of merge sort.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on success and EXIT_FAILURE on error
 */

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        usage();
    }

    pid_t child_first, child_second; //forked process
    FILE *outFirst;
    FILE *outSecond;
    FILE *inFirst;
    FILE *inSecond;

        /*pipefd_*****[0] - read end; pipefd_*****[1] - write end */
    int pipefd_firstChild[2];  // Used to store the two ends of "first" pipe child to parent
    int pipefd_firstParent[2]; // Used to store the two ends of "first" pipe parent to child
    int pipefd_secondChild[2];  // Used to store the two ends of "second" pipe child to parent
    int pipefd_secondParent[2]; // Used to store the two ends of "second" pipe parent to child

    int countLines = 0;
    char *buffer = NULL;
    size_t buflen = 0;
    char inputLine[SIZE][SIZE];
    //take the first two lines; if there are no lines -> terminates; if here is only one line-> no child process
    for (size_t i = 0; i < 2; i++)
    {
        if ((getline(&buffer, &buflen, stdin)) != -1)
        {
            strcpy(inputLine[countLines], buffer);
            countLines++;
        }
    }

     if (countLines == 1) //there is only one element in the input
    {
        fprintf(stdout, "%s", buffer);
        exit(EXIT_SUCCESS);
    }
    if (countLines == 0)
    {
        fprintf(stderr, "No elements in input array\n");
        exit(EXIT_FAILURE);
    }

     /* There are at least two input numbers*/
    if (pipe(pipefd_firstChild) == -1)
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd_firstParent) == -1)
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

     /* Create child process */
    child_first = fork();

    if (child_first == 0){
 /* Fisrt child  */

        if (close(pipefd_firstChild[1]) == -1) /* Close unused write end of the child pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }
        if (close(pipefd_firstParent[0]) == -1) /* Close unused read end of the parent pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }

        if (dup2(pipefd_firstChild[0], STDIN_FILENO) == -1) /* old descriptor - read end, new descriptor */
        {
            fprintf(stderr, "dup2 failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(pipefd_firstChild[0]) == -1) /* Close unused read end of the child pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }

        if (dup2(pipefd_firstParent[1], STDOUT_FILENO) == -1) // old descriptor - write end, new descriptor
        {
            fprintf(stderr, "dup2 failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(pipefd_firstParent[1]) == -1) /* Close unused write end of the parent pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }
        /* Recursive call */
        if (execlp("./forksort", argv[0], NULL) == -1)
        {
            fprintf(stderr, "execlp failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if (child_first == -1) /* Error handling */
    {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(pipefd_firstChild[0]) == -1) /* Close read end of the child pipe */
    {
        fprintf(stderr, "close failed: %s\n", strerror(errno));
    }

    if (close(pipefd_firstParent[1]) == -1) /* Close write end of the parent pipe */
    {
        fprintf(stderr, "close failed: %s\n", strerror(errno));
    }

    if (pipe(pipefd_secondChild) == -1)
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd_secondParent) == -1)
    {
        fprintf(stderr, "pipe failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

       child_second = fork();
    if (child_second == 0)
    {
        /* Child Odd */
        if (close(pipefd_firstChild[1]) == -1) /* Close unused write end of the child pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }
        if (close(pipefd_firstParent[0]) == -1) /* Close unused read end of the parent pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }
        if (close(pipefd_secondChild[1]) == -1) /* Close unused write end for the child pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }
        if (close(pipefd_secondParent[0]) == -1) /* Close unused read end for the parent pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }

        if (dup2(pipefd_secondChild[0], STDIN_FILENO) == -1) /* old descriptor - read end, new descriptor */
        {
            fprintf(stderr, "dup2 failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(pipefd_secondChild[0]) == -1) /* Close read end for the child pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }

        if (dup2(pipefd_secondParent[1], STDOUT_FILENO) == -1) /* old descriptor - write end, new descriptor */
        {
            fprintf(stderr, "dup2 failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(pipefd_secondParent[1]) == -1) /* Close write end of the parent pipe */
        {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
        }

        /* Recursive call */
        if (execlp("./forksort", argv[0], NULL) == -1)
        {
            fprintf(stderr, "execlp failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    else if (child_second == -1) /* Error handling */
    {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(pipefd_secondChild[0]) == -1) /* Close read end of the child pipe */
    {
        fprintf(stderr, "close failed: %s\n", strerror(errno));
    }
    if (close(pipefd_secondParent[1]) == -1) /* Close write end of the parent pipe */
    {
        fprintf(stderr, "close failed: %s\n", strerror(errno));
    }

    /* Parent Code */
    outFirst = fdopen(pipefd_firstChild[1], "w");
    outSecond = fdopen(pipefd_secondChild[1], "w");

    if (outFirst == NULL)
    {
        fprintf(stderr, "fdopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (outSecond == NULL)
    {
        fprintf(stderr, "fdopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Puts the first two read lines in in outFirst or outSecond depending on the counter value*/
    for (int i = 0; i < countLines; i += 1){
        if (i % 2 == 0)
        {
            if (fputs(inputLine[i], outFirst) == EOF)
            {
                fprintf(stderr, "fputs failed\n");
                exit(EXIT_FAILURE);
            }
        }
        if (i % 2 == 1)
        {
            if (fputs(inputLine[i], outSecond) == EOF)
            {
                fprintf(stderr, "fputs failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    //reads the rest of the lines and writes them in outFirst or outSecond
    while ((getline(&buffer, &buflen, stdin)) != -1)
    {
        if (countLines % 2 == 0)
        {
            if (fputs(buffer, outFirst) == EOF)
            {
                fprintf(stderr, "fputs failed\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (fputs(buffer, outSecond) == EOF)
            {
                fprintf(stderr, "fputs failed\n");
                exit(EXIT_FAILURE);
            }
        }
        countLines++;
    }

     if (fclose(outFirst) == EOF) //error handling
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (fclose(outSecond) == EOF) //error handling
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
 
    /* Waiting of the both processes*/
    int waitFirstChild;
    int waitSecondChild;

    if ((waitpid(child_first, &waitFirstChild, 0)) == -1)
    {
        fprintf(stderr, "waitpid failed\n");
        exit(EXIT_FAILURE);
    }

    if ((waitpid(child_second, &waitSecondChild, 0)) == -1)
    {
        fprintf(stderr, "waitpid failed\n");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(waitFirstChild)) /* Waiting for first child to exit succesfully */
    {
        int statusFirst = WEXITSTATUS(waitFirstChild);
        if (statusFirst != 0)
        {
            fprintf(stderr, "Failure with status code: %d\n", statusFirst);
            exit(EXIT_FAILURE);
        }
    }
    if (WIFEXITED(waitSecondChild)) /* Waiting for second child to exit succesfully */
    {
        int statusSecond = WEXITSTATUS(waitSecondChild);
        if (statusSecond != 0)
        {
            fprintf(stderr, "Failure with status code: %d\n", statusSecond);
            exit(EXIT_FAILURE);
        }
    }

    //open the pipe ends for reading
    inFirst = fdopen(pipefd_firstParent[0], "r");
    inSecond = fdopen(pipefd_secondParent[0], "r");
    if (inFirst == NULL || inSecond == NULL)
    {
        fprintf(stderr, "fdopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int i=0;
    int add=countLines%2;
    char firstValues[(countLines/2)+add][SIZE];
    char secondValues[(countLines/2) ][SIZE];

    int count = 0;
    ssize_t nread;
    char *line = NULL;
    size_t len = 0;
    while ((nread = getline(&line, &len, inFirst)) != -1) 
    {
       strcpy(firstValues[i],line);
       i++;
       count++;
    }

    i=0;
 
    while ((nread = getline(&line, &len, inSecond)) != -1)  
    {
       strcpy(secondValues[i],line);
       i++;
       count++;
    }

     if (fclose(inSecond) == EOF)
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
    }

    free(line);
    char result[count][SIZE];
    int f=0;
    int s=0;
    
    i=0;
    for (size_t i = 0; i < count; i++)
    {
        if(f==((countLines/2)+add)){
            //copy all values from second to result
            while(s!=(countLines/2)){
            strcpy(result[i],secondValues[s]);
                i++;
                s++;
            }
            
        }
         else if(s==(countLines/2)){
            //copy all values from first to result
            while(f!=((countLines/2)+add)){
            strcpy(result[i],firstValues[f]);
                i++;
                f++;
            }
        }
        else{ //compare both lines
            int str = strcmp(firstValues[f],secondValues[s]);
            if(str<0){
            strcpy(result[i],firstValues[f]);
            f++;
            }else if(str>0){
            strcpy(result[i],secondValues[s]);
            s++;
            }else{
                strcpy(result[i],firstValues[f]);
                f++;
                i++;
                strcpy(result[i],secondValues[s]);
                s++;
            }
        }
    }
    //prints the sorted lines
    for (size_t i = 0; i < count; i++)
    {
        fprintf(stdout, "%s", result[i]);
    }

     if (fclose(inFirst) == EOF)
    {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
    }
    free(buffer);

    exit(EXIT_SUCCESS);
}