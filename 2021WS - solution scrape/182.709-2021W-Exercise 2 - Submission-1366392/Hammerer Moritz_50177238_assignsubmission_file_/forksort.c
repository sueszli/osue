#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>


/**
 * @name: forksort
 * @author: Moritz Hammerer, 11902102
 * @date: 09.12.2021
 * 
 * @brief Sorting strings from stdin with forks and children using recursive mergesort
 *  
 */

char *myprog = "progName";


/**
 * @brief Gives out custom error message followed by errno and exits program
 * 
 * @details Takes an Char* with changable text and outputs it together with the name of the program and the corresponding errno.
 * Afterwards the program gets closed with ERROR_FAILURE
 *  
 * @param text      Char* Short text to futher explain the problem
 */
void errorOut(char* text){
    fprintf(stderr, "[%s] %s: %s\n", myprog, text, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Sorting strings from stdin with forks and children
 * 
 * @details Recursively sorts the strings alphabetically following the method of mergesort. With Pipes the
 * stdin and stdout of the children gets connected to the parents
 */
int main(int argc, char* argv[]) {
    myprog = argv[0];
    if (argc >= 2){
        errno = E2BIG;
        errorOut("Usage error. Don't start with Arguments");
    }

    char* line = NULL;
    char* linePE = NULL; 
    char* linePO = NULL;
    size_t len = 0;
    size_t lenPE = 0;
    size_t lenPO = 0;
    int counter = 0;
    char *temp = NULL;

    size_t sizeBuffer = 2;

    while (getline(&line, &len, stdin) > 0)
    {
        if (counter % 2 == 0) //EVEN indices
        {
            int newSize = strlen(line);
            if (linePE != NULL)
                newSize += strlen(linePE);

            if (newSize + sizeBuffer > lenPE) //check if realloc necessary
            {
                lenPE += len;

                if ((temp = realloc(linePE, (size_t) lenPE)) == NULL)
                {
                    free(linePE);
                    free(linePO);
                    free(line);
                    errorOut("Realloc for linePE failed");
                }
                linePE = temp;
            }

            linePE = strcat(linePE, line);
        }
        else //odd indices
        {
            int newSize = strlen(line);
            if (linePO != NULL)
                newSize += strlen(linePO);

            if (newSize + sizeBuffer > lenPO)
            {
                lenPO += len;

                if ((temp = realloc(linePO, (size_t) lenPO)) == NULL)
                {
                    free(linePE);
                    free(linePO);
                    free(line);
                    errorOut("Realloc for linePO failed");
                }
                linePO = temp;
            }

            linePO = strcat(linePO, line);
        }
        counter++;
    }
    free(line);
    
    switch (counter)
    {
    case 0:
        errorOut("No Arguments given");
    
    case 1:
        fprintf(stdout, linePE);
        exit(EXIT_SUCCESS);

    default:;
        int pipePWE[2]; //Parent Write to even indices
        int pipePRE[2]; //Parent Read from even indices
        int pipePWO[2]; //Parent Write to odd indices
        int pipePRO[2]; //Parent Read from odd indices

        //PE (even indices)
        if (pipe(pipePWE) < 0){
            errorOut("pipe pipePWE failed");
        }

        if (pipe(pipePRE) < 0){
            close(pipePWE[0]);
            close(pipePWE[1]);
            errorOut("pipe pipePWE failed");
        }

        pid_t pidPE = fork();

        switch (pidPE)
        {
        case -1: //Error
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            errorOut("PE Fork failed");
        
        case 0: //Child
            close(pipePWE[1]);
            dup2(pipePWE[0], STDIN_FILENO);
            close(pipePWE[0]);

            close(pipePRE[0]);
            dup2(pipePRE[1], STDOUT_FILENO);
            close(pipePRE[1]);            

            execlp("./forksort", strcat(myprog, "/cE"), NULL);
            errorOut("Should not be called");

        default: //Parent
            close(pipePWE[0]);
            close(pipePRE[1]);

            write(pipePWE[1], linePE, strlen(linePE));
            close(pipePWE[1]);

            break;
        }


        //PO (odd indices)
        if (pipe(pipePWO) < 0){
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            errorOut("pipe pipePWO failed");
        }

        if (pipe(pipePRO) < 0){
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            close(pipePWO[0]);
            close(pipePWO[1]);
            errorOut("pipe pipePWO failed");
        }

        pid_t pidPO = fork();

        switch (pidPO)
        {
        case -1: //Error
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            close(pipePWO[0]);
            close(pipePWO[1]);
            close(pipePRO[0]);
            close(pipePRO[1]);
            errorOut("PO Fork failed");
        
        case 0: //Child
            close(pipePWO[1]);
            dup2(pipePWO[0], STDIN_FILENO);
            close(pipePWO[0]);

            close(pipePRO[0]);
            dup2(pipePRO[1], STDOUT_FILENO);
            close(pipePRO[1]);

            execlp("./forksort", strcat(myprog, "/cO"), NULL);
            errorOut("Should not be called");

        default: //Parent
            close(pipePWO[0]);
            close(pipePRO[1]);

            write(pipePWO[1], linePO, strlen(linePE));
            close(pipePWO[1]);

            break;
        }

        //Wait for Children
        int status;
        pid_t wpid;
        while ((wpid = wait(&status)) > 0){
            //fprintf(stderr, "[%s] %s: %d ended in %d\n", myprog, "Lifenotice", (int) wpid, status);
            if (status != EXIT_SUCCESS){
                close(pipePWE[0]);
                close(pipePWE[1]);
                close(pipePRE[0]);
                close(pipePRE[1]);
                close(pipePWO[0]);
                close(pipePWO[1]);
                close(pipePRO[0]);
                close(pipePRO[1]);
                errorOut("Child failed");
            }
        }

        //Read from Children
        FILE *evenFd = fdopen(pipePRE[0], "r");
        if (evenFd == NULL){
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            close(pipePWO[0]);
            close(pipePWO[1]);
            close(pipePRO[0]);
            close(pipePRO[1]);
            errorOut("evenFd Read failed");
        }

        FILE *oddFd = fdopen(pipePRO[0], "r");
        if (oddFd == NULL){
            fclose(evenFd);
            close(pipePWE[0]);
            close(pipePWE[1]);
            close(pipePRE[0]);
            close(pipePRE[1]);
            close(pipePWO[0]);
            close(pipePWO[1]);
            close(pipePRO[0]);
            close(pipePRO[1]);
            errorOut("oddFd Read failed");
        }

        //Sorting
        char* lineEven = NULL;
        size_t lenEven = 0;
        char* lineOdd = NULL;
        size_t lenOdd = 0;

        if (getline(&lineEven, &lenEven, evenFd) > 0)
        {
            if(getline(&lineOdd, &lenOdd, oddFd) > 0)
            {
                //Compare
                while(1){
                    //Fix line breaks
                    int lastIndexEven = strlen(lineEven) - 1;
                    int lastIndexOdd = strlen(lineOdd) - 1;
                    if (lineEven[lastIndexEven] != '\n')
                    {
                        strcat(lineEven, "\n");
                    }
                    if (lineOdd[lastIndexOdd] != '\n')
                    {
                        strcat(lineOdd, "\n");
                    }


                    /*
                    if (lineEven > lineOdd) = if (strcmp(lineEven, lineOdd) > 0)
                        lineOdd
                        lineEven
                    */

                    if (strcmp(lineEven, lineOdd) > 0)
                    {
                        fprintf(stdout, "%s", lineOdd);
                        if (getline(&lineOdd, &lenOdd, oddFd) > 0)
                        {
                            //More in oddFd -> Repeat
                            continue;
                        }
                        else {
                            //No more in this Fd -> break while loop
                            fprintf(stdout, "%s", lineEven);
                            break;
                        }

                    } else
                    {
                        fprintf(stdout, "%s", lineEven);
                        if (getline(&lineEven, &lenEven, evenFd) > 0)
                        {
                            continue;
                        }
                        else {
                            fprintf(stdout, "%s", lineOdd);
                            break;
                        }
                    }
                }
            } else
            {
                //No lines in odd-part
                fprintf(stdout, "%s", lineEven);
            }  
        }


        //Write remaining lines (already sorted in children)
        while (getline(&lineEven, &lenEven, evenFd) > 0)
        {
            fprintf(stdout, "%s", lineEven);
        }
        while (getline(&lineOdd, &lenOdd, oddFd) > 0)
        {
            fprintf(stdout, "%s", lineOdd);
        }
 

        fclose(evenFd);
        fclose(oddFd);
        free(lineEven);
        free(lineOdd);

        close(pipePWE[0]);
        close(pipePWE[1]);
        close(pipePRE[0]);
        close(pipePRE[1]);
        close(pipePWO[0]);
        close(pipePWO[1]);
        close(pipePRO[0]);
        close(pipePRO[1]);

        break;
    }

    exit(EXIT_SUCCESS);
}