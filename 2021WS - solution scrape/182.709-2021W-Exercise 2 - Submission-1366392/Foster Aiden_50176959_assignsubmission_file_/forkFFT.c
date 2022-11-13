/**
 * @file forkFFT.c
 * @author Aiden Foster 11910604
 * @date 08.12.2021
 *
 * @brief program module to calculate the fourier tranformation
 * 
 * This program is used to calculate the fourier transformation of floating point values given through stdin,
 *  the result ist printed to stdout
**/

#include "forkFFT.h"

/** @brief set to 1 to terminate with EXIT_FAILURE */
volatile sig_atomic_t invalidAndQuit = 0;

/** @brief quit flag for child processes */
sharedQuit* quit = NULL;

/** @brief semaphore for writing to sharedQuit*/
sem_t* sem_write = SEM_FAILED;

/** @brief name of this program */
char* programname;

/**
 * @brief signal handler
 * @details sets invalidAndQuit to 1
 * @param signal the signal to handle
**/
static void handle_signal(int signal){
    invalidAndQuit = 1;
}

/**
 * usage function
 * @brief output a usage message to stderr and terminate
**/
static void usage(){
	fprintf(stderr, "Usage: %s\n", programname);
	fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * print an error
 * @brief prints the message to stderr with according formatting
 *
 * @param error the error to print
**/
static void fprintError(char* error){
    if (error[strlen(error)-1] != '\n') {
	    fprintf(stderr, "[%s]: %s\n", programname, error);
    } else {
	    fprintf(stderr, "[%s]: %s", programname, error);
    }
    fflush(stderr);
}

/**
 * get the number of lines a tree has for a given number of lines
 * @brief calculate the number of lines a tree has
 *
 * @param resultLines the number of lines for which to calculate the number of lines of a tree
**/
static int getTreeLinesFromResultLines(int resultLines) {
    if (resultLines <= 0) {
        return 0;
    }
    int treeLines = 0;
    while(resultLines > 1) {
        treeLines++;
        resultLines >>= 1;
    }
    return 2*treeLines + 1;
}

/**
 * Parse a string containing two float values.
 * The string may continue after the two float values (e.g. "*i") - the rest is ignored
 * @brief parse a string containing two float values and return them float values
 *
 * @param string the string containing two float values
 * @param real a pointer to which to write the first float value
 * @param imag a pointer to which to write the second float value
**/
static int splitStringToValues(char* string, float* real, float* imag) {
    errno = 0;
    char *end;
    *real = strtof(string, &end);
    while(*end == ' ') {
        end++;
    }
    if(*end == '\n' || *end == '\0') {
        *imag = 0.0;
    } else {
        *imag = strtof(end, NULL);
    }
    return errno;
}

/**
 * Calculate the fourier transformation according to the butterfly method
 * @brief calculate the fourier transformation according to the butterfly method
 *
 * @param evenResult_real the real prat of the even result
 * @param evenResult_imag the imaginary part of the even result
 * @param oddResult_real the real part of the odd result
 * @param oddResult_imag the imaginary part of the odd result
 * @param result_real a pointer pointing to the location to which to write the real part of the result
 * @param result_imag a pointer pointing to the location to which to write the imaginary part of the result
 * @param result_index the index of this result (is used as offset when writing to the result pointers)
 * @param result_length the number of results
**/
static void calculateFFT(float evenResult_real, float evenResult_imag, float oddResult_real, float oddResult_imag, float *result_real, float *result_imag, int result_index, int result_length) {
    float pi = 3.141592654;
    float minus2piOverNTimesK = - (2 * pi * result_index) / result_length;
    float cosOfminus2piOverNTimesK = cos(minus2piOverNTimesK);
    float sinOfminus2piOverNTimesK = sin(minus2piOverNTimesK);
    result_real[result_index] = evenResult_real + cosOfminus2piOverNTimesK * oddResult_real - sinOfminus2piOverNTimesK * oddResult_imag;
    result_imag[result_index] = evenResult_imag + sinOfminus2piOverNTimesK * oddResult_real + cosOfminus2piOverNTimesK * oddResult_imag;
    result_real[result_index + result_length/2] = evenResult_real - cosOfminus2piOverNTimesK * oddResult_real + sinOfminus2piOverNTimesK * oddResult_imag;
    result_imag[result_index + result_length/2] = evenResult_imag - sinOfminus2piOverNTimesK * oddResult_real - cosOfminus2piOverNTimesK * oddResult_imag;
}

/**
 * Exit with an error
 * @brief prints the error message, cleans up semaphores and shared memory and then terminates with EXIT_FAILURE
 *
 * @param errorMessage the error message to print, may be NULL of "" if no message should be printed
**/
static void errorExit(char *errorMessage) {
    if (errorMessage != NULL && strlen(errorMessage) > 0) {
        fprintError(errorMessage);
    }
    while(sem_wait(sem_write) == -1) {
        if(errno != EINTR) {
            fprintError("Could not wait for semaphore");
            break;
        }
    }
    quit->quit = 1;
    if (quit->processes > 0) {
        quit->processes -= 1;
    }
    char unlink = (quit->processes <= 0);
    while(sem_post(sem_write) == -1) {
        if(errno != EINTR) {
            fprintError("Could not post to semaphore");
            break;
        }
    }
    if(sem_close(sem_write) < 0) {
        fprintError("Could not close semaphore");
    }
    if(munmap(quit, sizeof(*quit)) < 0) {
        fprintError("Could not unmap shared memory");
    }
    if(unlink) {
        if(sem_unlink(SEM_WRITE_QUIT_NAME) < 0) {
            fprintError("Failed to unlink semaphore");
        }
        if(shm_unlink(SHM_QUIT_FLAG_NAME) < 0) {
            fprintError("Failed to unlink shared memory");
        }
    }
    exit(EXIT_FAILURE);
}

/**
 * @brief Add a new process to the process count
**/
static void addToProcesses(void) {
    while(sem_wait(sem_write) == -1) {
        if(errno != EINTR) {
            errorExit("Could not wait for semaphore");
        }
    }
    quit->processes += 1;
    while(sem_post(sem_write) == -1) {
        if(errno != EINTR) {
            errorExit("Could not post to semaphore");
        }
    }
}

/**
 * Terminate the program with EXIT_SUCCESS
 * @brief terminate the program with EXIT_SUCCESS if cleanup throws no errors or prints an error and exits with EXIT_FAILURE if an error occures
**/
static void terminate(void) {
    while(sem_wait(sem_write) == -1) {
        if(errno != EINTR) {
            errorExit("Could not wait for semaphore");
        }
    }
    if (quit->processes > 0) {
        quit->processes -= 1;
    }
    char unlink = (quit->processes <= 0);
    while(sem_post(sem_write) == -1) {
        if(errno != EINTR) {
            quit->processes += 1;
            errorExit("Could not post to semaphore");
        }
    }
    if(sem_close(sem_write) < 0) {
        fprintError("Could not close semaphore");
        munmap(quit, sizeof(*quit));
        if(unlink) {
            sem_unlink(SEM_WRITE_QUIT_NAME);
            shm_unlink(SHM_QUIT_FLAG_NAME);
        }
    }
    if(munmap(quit, sizeof(*quit)) < 0) {
        fprintError("Could not unmap shared memory");
        if(unlink) {
            sem_unlink(SEM_WRITE_QUIT_NAME);
            shm_unlink(SHM_QUIT_FLAG_NAME);
        }
    }
    if(unlink) {
        if(sem_unlink(SEM_WRITE_QUIT_NAME) < 0) {
            fprintError("Could not unlink semaphore");
            shm_unlink(SHM_QUIT_FLAG_NAME);
            exit(EXIT_FAILURE);
        }
        if(shm_unlink(SHM_QUIT_FLAG_NAME) < 0) {
            fprintError("Could not unlink shared memory");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

/**
 * Adds a string to another string. The first string must be terminated with "\n\0" and either be NULL or already allocated via malloc. The operation is done in place but the pointer is reallocated if necessary.
 * @brief adds a string to another string
 * @param toAddTo a pointer to the string to add to
 * @param toAdd the string to add
 *
 * @return the reallocated pointer
**/
static void addToString(char** toAddTo, char* toAdd) {
    int index = 0;
    if(*toAddTo == NULL) {
        *toAddTo = (char *) malloc(sizeof(char) * (strlen(toAdd) + 1));
    } else {
        int length = strlen(*toAddTo);
        *toAddTo = (char *) realloc(*toAddTo, length + 1 + strlen(toAdd) + 1);
        (*toAddTo)[length-1] = ',';
        (*toAddTo)[length] = ' ';
        index = length+1;
    }
    strcpy((*toAddTo) + index, toAdd);
}

/**
 * calculate the length a string has to have for it to be able to represent any float for a given number of decimal places
 * @brief claculate the length a string has to have for a given number of decimal places
 * @param decimalPlaces number of decimal places the flout should have
 *
 * @return the length a string could have for the given number of decimal places
**/
static int stringWidthForFloat(int decimalPlaces) {
    int widthBeforeComma = 0;
    for(float f = FLT_MAX; f>=1; f /= 10.0) {
        widthBeforeComma++;
    }
    int signWidth = 1;
    int commaWidth = 1;
    return signWidth + widthBeforeComma + commaWidth + decimalPlaces;
}

/**
 * Program entry point
 * @brief forkFFT checks stdin for two integers to multiply. These integers have to be of same length and of a length that is a power of 2. The result is written to stdout
 * @details exits with EXIT_FAILURE if invalidAndQuit is set to 1
 * @param argc The amount of arguments passed
 * @param argv[] The arguments passed
 *
 * @return EXIT_FAILURE if program encountered an error, EXIT_SUCCESS otherwise
**/
int main(int argc, char * argv[]) {
    if(argc != 1) {
        usage(argv[0]);
    }
    programname = argv[0];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = handle_signal;
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //open shared memory
    int quitfd = shm_open(SHM_QUIT_FLAG_NAME, O_RDWR | O_CREAT, 0600);
    if(quitfd == -1){
        fprintError("Failed to allocate memory for shared quit flag.\n");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(quitfd, sizeof(*quit)) < 0){
        fprintError("Failed to allocate memory for shared quit flag.\n");
        close(quitfd);
        exit(EXIT_FAILURE);
    }
    quit = mmap(NULL, sizeof(*quit), PROT_READ | PROT_WRITE, MAP_SHARED, quitfd, 0);
    if(quit == MAP_FAILED){
        fprintError("Failed to map memory for shared quit flag.\n");
        close(quitfd);
        exit(EXIT_FAILURE);
    }
    if(close(quitfd) < 0) {
        fprintError("Failed to close file descriptor of shared memory\n");
        quit->quit = 1;
        munmap(quit, sizeof(*quit));
        exit(EXIT_FAILURE);
    }
    //open semaphore
    sem_write = sem_open(SEM_WRITE_QUIT_NAME, O_CREAT, 0600, 1);
    if(sem_write == SEM_FAILED) {
        fprintError("Failed to open semaphore");
        quit->quit = 1;
        char unlink = (quit->processes <= 0);
        munmap(quit, sizeof(*quit));
        if(unlink) {
            shm_unlink(SHM_QUIT_FLAG_NAME);
        }
        exit(EXIT_FAILURE);
    }
    addToProcesses();

    int eof = 0;
    char *line1 = NULL;
    size_t len1 = 0;
    ssize_t nread1;
    char *line2 = NULL;
    size_t len2 = 0;
    ssize_t nread2;
   
    errno = 0; 
    while(quit->quit == 0 && (nread1 = getline(&line1, &len1, stdin)) == -1) {
        if(errno != EINTR && errno != 0){
            errorExit("Interrupted during line read.\n");
        }
        //EOF
        if (errno == 0) {
            errorExit("Encountered EOF before any input");
        }
        errno = 0;
    }
    if (quit->quit == 1) {
        errorExit("Received quit.");
    }
    if (len1 == 0) {
        errorExit("No input given");
    }
    while(quit->quit == 0 && eof == 0 && (nread2 = getline(&line2, &len2, stdin)) == -1) {
        if(errno != EINTR && errno != 0){
            errorExit("Interrupted during line read.\n");
        }
        //EOF
        if (errno == 0) {
            char *end;
            float value = strtof(line1, &end);
            if (errno != 0 || (*end != '\n' && *end != '\0')) {
                fprintError("Failed to convert the input to float:");
                errorExit(line1);
            }
            fprintf(stdout, "%f 0.0*i\n", value); //result
            fprintf(stdout, "\n"); //empty line
            fprintf(stdout, "%f 0.0*i\n", value); //tree
            fflush(stdout);
            eof = 1;
        }
        errno = 0;
    }
    if (quit->quit == 1) {
        free(line1);
        free(line2);
        errorExit("Received quit");
    }
    if (len2 == 0) {
        if (eof != 1) {
            errno = 0;
            char *end;
            float value = strtof(line1, &end);
            if (errno != 0 || (*end != '\n' && *end != '\0')) {
                fprintError("Failed to convert the input to float:");
                free(line1);
                free(line2);
                errorExit(line1);
            }
            fprintf(stdout, "%f 0.0*i\n", value);
            fflush(stdout);
        }
        eof = 1;
    }

    if (eof == 0) {
        pid_t children[NUM_CHILDREN];
        FILE* pipesTo[NUM_CHILDREN];
        FILE* pipesFrom[NUM_CHILDREN];
        for(int i=0; i<NUM_CHILDREN; i++){
            children[i] = 0;
            pipesTo[i] = stdout;
            pipesFrom[i] = stdin;
        }
        for(int i=0; i<NUM_CHILDREN; i++){
            int pipefdTo[2];
            int pipefdFrom[2];
            if(pipe(pipefdTo) == -1){
                free(line1);
                free(line2);
                errorExit("Failed to create pipe!\n");
            }
            if(pipe(pipefdFrom) == -1){
                free(line1);
                free(line2);
                errorExit("Failed to create pipe!\n");
            }
            children[i] = fork();
            switch(children[i]) {
                case -1:
                    free(line1);
                    free(line2);
                    errorExit("Fork failed!\n");
                case 0: //child
                    while(close(pipefdTo[1]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    while(close(pipefdFrom[0]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    while(dup2(pipefdTo[0], STDIN_FILENO) == -1){
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("mapping pipe end to stdin failed!\n");
                        }
                    }
                    while(dup2(pipefdFrom[1], STDOUT_FILENO) == -1){
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("mapping pipe end to stdout failed!\n");
                        }
                    }
                    while(close(pipefdTo[0]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    while(close(pipefdFrom[1]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    execlp(argv[0], argv[0], NULL);
                    break; //should not be required
                default: //parent
                    pipesTo[i] = fdopen(pipefdTo[1], "w");
                    if(pipesTo[i] == NULL){
                        free(line1);
                        free(line2);
                        errorExit("opening pipe as FILE* failed!\n");
                    }
                    while(close(pipefdTo[0]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    pipesFrom[i] = fdopen(pipefdFrom[0], "r");
                    if(pipesFrom[i] == NULL){
                        free(line1);
                        free(line2);
                        errorExit("opening pipe as FILE* failed!\n");
                    }
                    while(close(pipefdFrom[1]) == -1) {
                        if(errno != EINTR){
                            free(line1);
                            free(line2);
                            for(int j=0; j<=i; j++){
                                fclose(pipesTo[i]);
                                fclose(pipesFrom[i]);
                            }
                            errorExit("closing pipe end failed!\n");
                        }
                    }
                    break;
            }
        }
       
        int lines = 0;
        for(; eof == 0 && quit->quit == 0; lines += 2) {
            fprintf(pipesTo[0], "%s", line1);
            fprintf(pipesTo[1], "%s", line2);
            for(int i=0; i<NUM_CHILDREN; i++){
                fflush(pipesTo[i]);
            }
            if(invalidAndQuit != 0){
                free(line1);
                free(line2);
                for(int i=0; i<NUM_CHILDREN; i++){
                    fclose(pipesTo[i]);
                    fclose(pipesFrom[i]);
                }
                errorExit("Broken Pipe!\n");
            }
    
            errno = 0; 
            while(quit->quit == 0 && eof == 0 && (nread1 = getline(&line1, &len1, stdin)) == -1) {
                if(errno != EINTR && errno != 0){
                    free(line1);
                    free(line2);
                    for(int i=0; i<NUM_CHILDREN; i++){
                        fclose(pipesTo[i]);
                        fclose(pipesFrom[i]);
                    }
                    errorExit("Interrupted during line read.\n");
                }
                //EOF
                if (errno == 0) {
                    eof = 1;
                    break;
                }
                errno = 0;
            }
            while(quit->quit == 0 && eof == 0 && (nread2 = getline(&line2, &len2, stdin)) == -1) {
                if(errno != EINTR && errno != 0){
                    free(line1);
                    free(line2);
                    for(int i=0; i<NUM_CHILDREN; i++){
                        fclose(pipesTo[i]);
                        fclose(pipesFrom[i]);
                    }
                    errorExit("Interrupted during line read.\n");
                }
                //EOF
                if (errno == 0) {
                    free(line1);
                    free(line2);
                    for(int i=0; i<NUM_CHILDREN; i++){
                        fclose(pipesTo[i]);
                        fclose(pipesFrom[i]);
                    }
                    errorExit("Number of inputs is not a power of 2.\n");
                }
                errno = 0;
            }
        }
                
        free(line1);
        free(line2);
        line1 = NULL;
        line2 = NULL;
        
        for(int i=0; i<NUM_CHILDREN; i++){
            if(fclose(pipesTo[i]) != 0){
                errorExit("Failed to close pipe!\n");
            }
        }

        float results_real[lines];
        float results_imag[lines];
        for(int k=0; k < lines/2; k++) {
            char* childResults[NUM_CHILDREN];
            size_t sizeChildResults[NUM_CHILDREN];
            for(int i=0; i<NUM_CHILDREN; i++){
                childResults[i] = NULL;
                sizeChildResults[i] = 0;
            }
            for(int i=0; i<NUM_CHILDREN; i++){
                while( getline(&childResults[i], &sizeChildResults[i], pipesFrom[i]) == -1) {
                    if(errno != EINTR){
                        for(int i=0; i<NUM_CHILDREN; i++){
                            free(childResults[i]);
                        }
                        for(int i=0; i<NUM_CHILDREN; i++){
                            fclose(pipesFrom[i]);
                        }
                        errorExit("Interrupted during line read.\n");
                    }
                }
                int len = strlen(childResults[i]);
                if(childResults[i][len-1] == '\n'){
                    childResults[i][len-1] = '\0';
                }
            }
            
            float evenResult_real;
            float evenResult_imag;
            splitStringToValues(childResults[0], &evenResult_real, &evenResult_imag);
            float oddResult_real;
            float oddResult_imag;
            splitStringToValues(childResults[1], &oddResult_real, &oddResult_imag);
            calculateFFT(evenResult_real, evenResult_imag, oddResult_real, oddResult_imag, results_real, results_imag, k, lines);
            for(int i=0; i<NUM_CHILDREN; i++){
                free(childResults[i]);
            }
        }

        if(invalidAndQuit != 0 || quit->quit != 0) {
            for(int i=0; i<NUM_CHILDREN; i++){
                fclose(pipesFrom[i]);
            }
            errorExit("Received quit.");
        }

        //print result
        for (int i=0; i<lines; i++) {
            fprintf(stdout, "%f %f*i\n", results_real[i], results_imag[i]);
        }
        fflush(stdout);
        if(invalidAndQuit != 0){
            for(int i=0; i<NUM_CHILDREN; i++){
                fclose(pipesFrom[i]);
            }
            errorExit("Broken Pipe!\n");
        }
       
        //print tree
        line1 = NULL;
        len1 = 0;
        int allHaveTrees = 1;
        int allHaveNoTrees = 1;
        for(int i=0; i<NUM_CHILDREN; i++){
            errno = 0;
            while( getline(&line1, &len1, pipesFrom[i]) == -1) {
                if(errno != EINTR && errno != 0){
                    free(line1);
                    for(int i=0; i<NUM_CHILDREN; i++){
                        fclose(pipesFrom[i]);
                    }
                    errorExit("Interrupted during empty line read for tree.\n");
                }
                if(errno == 0){
                    allHaveTrees = 0;
                }
            }
            int len = strlen(line1);
            if(len == 1 && line1[len-1] == '\n'){
                allHaveNoTrees = 0;
            } else {
                fprintError("Received additional output:");
                fprintError(line1);
                free(line1);
                for(int i=0; i<NUM_CHILDREN; i++){
                    fclose(pipesFrom[i]);
                }
                errorExit("Child has more results than expected");
            }
        }
        free(line1);
        line1 = NULL;
        if(allHaveTrees == 0 && allHaveNoTrees == 0) {
            for(int i=0; i<NUM_CHILDREN; i++){
                fclose(pipesFrom[i]);
            }
            errorExit("Some children have trees, some do not");
        }
        if(allHaveTrees == 1) {
            int treeLines = getTreeLinesFromResultLines(lines/2); //get tree lines for child
            int first = 0;
            int innerDistance = TREE_SEPERATOR_DISTANCE;
            line1 = NULL;
            len1 = 0;
            line2 = NULL;
            len2 = 0;
            for(int i=0; i<treeLines && quit->quit == 0; i++) {
                while( getline(&line1, &len1, pipesFrom[0]) == -1) {
                    if(errno != EINTR){ //also terminate if eof is encountered
                        free(line1);
                        for(int i=0; i<NUM_CHILDREN; i++){
                            fclose(pipesFrom[i]);
                        }
                        errorExit("Interrupted during line read for tree.\n");
                    }
                }
                while( getline(&line2, &len2, pipesFrom[1]) == -1) {
                    if(errno != EINTR){ //also terminate if eof is encountered
                        free(line1);
                        free(line2);
                        for(int i=0; i<NUM_CHILDREN; i++){
                            fclose(pipesFrom[i]);
                        }
                        errorExit("Interrupted during line read for tree.\n");
                    }
                }
                line1[strlen(line1)-1] = '\0'; //remove \n 
                line2[strlen(line2)-1] = '\0'; //remove \n
                if (first == 0) {
                    fprintf(stdout, "\n"); //print empty line to seperate results from tree
                    int treeLength1 = strlen(line1);
                    int treeLength2 = strlen(line2);
                    char *innerNode = NULL;
                    for (int i=0; i<lines; i++) {
                        char innerNodePart[2*stringWidthForFloat(4) + 5];
                        sprintf(innerNodePart, "%.4f %.4f*i\n", results_real[i], results_imag[i]);
                        addToString(&innerNode, innerNodePart);
                    }
                    //char *innerNode = "innerNodeStringToPrintEverywhere";
                    int nodeLength = strlen(innerNode);
                    if (innerNode[nodeLength-1] == '\n') {
                        innerNode[nodeLength-1] = '\0';
                    }
                    int treeLengthTotal = treeLength1 + treeLength2 + innerDistance;
                    if (nodeLength > treeLengthTotal) {
                        innerDistance += (nodeLength - treeLengthTotal);
                        treeLengthTotal = nodeLength;
                    }
                    //print this node
                    int i;
                    for (i=0; i<(treeLengthTotal-nodeLength+1)/2; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                    }
                    fprintf(stdout, "(%s)", innerNode);
                    free(innerNode);
                    i += nodeLength + 2;
                    for(; i<treeLengthTotal; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                    }
                    fprintf(stdout, "\n");
                    //print the connecting slashes and backslashes
                    for(i=0; i<treeLength1/2; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                    }
                    fprintf(stdout, "/");
                    i++;
                    for(; i<(treeLengthTotal-treeLength2)+treeLength2/2; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                    }
                    fprintf(stdout, "\\");
                    i++;
                    for(; i<treeLengthTotal; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                    }
                    fprintf(stdout, "\n");
                }
                first = 1;
                //output line
                fprintf(stdout, line1);
                for(int i=0; i<innerDistance; i++) {
                        fprintf(stdout, TREE_SEPERATOR);
                }
                fprintf(stdout, line2);
                fprintf(stdout, "\n");
                fflush(stdout);
            }
            free(line1);
            free(line2);
            line1 = NULL;
            line2 = NULL;
        }

        for(int i=0; i<NUM_CHILDREN; i++){
            int status;
            while(wait(&status) == -1){
                if(errno != EINTR){
                    for(int i=0; i<NUM_CHILDREN; i++){
                        fclose(pipesFrom[i]);
                    }
                    errorExit("Interrupted while waiting for children.\n");
                }
            }
            if(!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
                char errorMessage[50];
                if (status == EXIT_FAILURE) {
                    sprintf(errorMessage, "Child terminated with EXIT_FAILURE");
                } else {
                    sprintf(errorMessage, "Child terminated with %d", status);
                }
                for(int i=0; i<NUM_CHILDREN; i++){
                    fclose(pipesFrom[i]);
                }
                errorExit(errorMessage);
            }
        }

        for(int i=0; i<NUM_CHILDREN; i++){
            if(fclose(pipesFrom[i]) != 0){
                errorExit("Failed to close pipe!\n");
            }
        }
    }

    if(invalidAndQuit != 0 || quit->quit != 0){
        errorExit("Received quit.");
    }
    
    terminate();
}
