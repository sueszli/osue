/**
 * @file main.c
 * @author Johanna Reisinger
 * @brief Program takes two String that represent Hexadecimal numbers from stdin and mutiplies them using recusive forking.
 *        Only numbers of length 2^n are allowed and the two numbers have to be of the same length.
 *        Two empty string result in an empty output.
 * @date 2021-12-11
 * 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

FILE * fp;
char *numberA;
char *numberB;
char *program_name;
char *ah;
char *al;
char *bh;
char *bl;
char *ahbh;
char *ahbl;
char *albh;
char *albl;
char *sumresult;

/**
 * @brief converts an integer number to a hexadecimal string
 * 
 * @param i the integer to covert
 * @return char the hex string
 */
char intToHexchar(int i){
    char resString[2]; // 2 for '\0'
    sprintf(resString, "%x", i); //convert to String
    return resString[0];
}

/**
 * @brief converts a hexadecimal char to an integer eg. 'a' to 10
 * 
 * @param c the character to convert
 * @return int the integer to return
 */
int hexcharToInt(char c){
    char cc[2];
    cc[0] = c; 
    cc[1] = '\0';
    int res = strtol(&(cc[0]), NULL, 16);
    return res;
}

/**
 * @brief add two hexadecimal numbers respresented as strings 
 * 
 * @param a first number to add
 * @param b second number to add
 * @param res result pointer
 * @return int error code 0 if successful else 1
 */
int add( char *a, char *b, char **res ){
    int sizeA = strlen(a);
    int sizeB = strlen(b);
    char *min = sizeA < sizeB ? a : b;
    char *max = sizeA >= sizeB ? a : b;
    int maxSize = strlen(max);
    int minSize = strlen(min);
    int carryFlag = 0;

    char *tmpres = malloc( sizeof(char) * (maxSize+2) ); // +2 for '\0' and leading 0 or carry
    tmpres[maxSize+1] = '\0';
    if( tmpres == NULL ) {
        return 1;
    }

    for( int i = 1; i <= maxSize; i++ )
    {
        // subtract from size to get right most
        int tmp = hexcharToInt( max[maxSize - i] );
        if( i <= minSize ) {
            tmp += hexcharToInt( min[minSize - i] );
        }
        tmp += carryFlag;
        carryFlag = tmp > 15 ? 1 : 0;
        tmp = tmp & 0xF; // remove carry if present

        // +1 to consider the leading 0 or carry
        tmpres[ maxSize + 1 - i ] = intToHexchar(tmp); 
    }

    if(carryFlag) {
        tmpres[0] = '1';
    } else {
        tmpres[0] = '0';
    }

    (*res) = tmpres;

    return 0;
}

/**
 * @brief multiply two hexadecimal numbers respresented as strings 
 * 
 * @param a first number to mulitply
 * @param b second number to multiply
 * @param res result pointer
 * @return int error code 0 if successful else 1
 */
int multiply(char *a, char *b, char *res){
    int ahex = strtol(a, NULL, 16);
    int bhex = strtol(b, NULL, 16);

    unsigned int temp = ahex*bhex;
    char resString[3];
    sprintf(resString, "%x", temp); //convert to String
    res[0] = resString[0]; 
    res[1] = resString[1]; 

    return 0;
}

/**
 * @brief return a string of zeros
 * 
 * @param n how many zeros the string should consist of
 * @return char* the result string
 */
char* zeros(int n){
    char *res = malloc(sizeof(char)*(n+1));
    if( res == NULL ){
        return NULL;
    }
    for(int i = 0; i<n; i++){
        res[i] = '0';
    }
    res[n] = '\0';
    return res;
}

/**
 * @brief adds n numbers of zeros to the end of the hex string
 * 
 * @param hex a hexadeciml number in form of a string
 * @param n how many zeros shall be appended
 * @param newhex the result string
 * @return int error code 0 if successful else 1
 */
int appendZeros(char *hex, int n, char **newhex){
    char *result;
    int size = strlen(hex) + n;

    char *zerostr = zeros(n);
    if( zerostr == NULL ){
        return 1;
    }

    result = malloc(sizeof(char)*(size+1));
    if( result == NULL ){
        free(zerostr);
        return 1;
    }

    result[0] = '\0';
    result = strcat(result, hex);
    result = strcat(result, zerostr);
    
    (*newhex) = result;
    
    free(zerostr);
    return 0;
}

/**
 * @brief calculates the formula ah*bh*16^n + al*bh*16^n + ah*bl*16^n + al*bl
 * 
 * @param n the lenght of the original number strings
 * @param ahbh hex string representing ah*bh
 * @param albh hex string representing al*bh
 * @param ahbl hex string representing ah*bl
 * @param albl hex string representing al*bl
 * @param res resultpointer
 * @return int error code 0 if successful else 1
 */
int sum( int n, char *ahbh, char *albh, char *ahbl, char *albl, char **res){


    //fst calc
    char *ahbh_16n;
    if( appendZeros(ahbh,n,&ahbh_16n) ){
        fprintf(stderr, "Failed to append zeros");
        return 1;
    }

     //snd calc
    char *albh_16n;
    if( appendZeros(albh,n/2,&albh_16n) ){
        free(ahbh_16n);
        fprintf(stderr, "Failed to append zeros");
        return 1;
    }

     //third calc
    char *ahbl_16n;
    if( appendZeros(ahbl,n/2,&ahbl_16n) ){
        free(ahbh_16n);
        free(albh_16n);
        fprintf(stderr, "Failed to append zeros");
        return 1;
    }

    char *tmp1;
    char *tmp2;
    char *result;
    if( add( ahbh_16n, albh_16n, &tmp1) ){
        free(ahbh_16n);
        free(albh_16n);
        free(ahbl_16n);
        fprintf(stderr, "Failed to add");
        return 1;
    }
    if( add( ahbl_16n, tmp1, &tmp2) ){
        free(ahbh_16n);
        free(albh_16n);
        free(ahbl_16n);
        free(tmp1);
        fprintf(stderr, "Failed to add");
        return 1;
    }
    free(tmp1);
    if( add( albl, tmp2, &result) ){
        free(ahbh_16n);
        free(albh_16n);
        free(ahbl_16n);
        free(tmp2);
        fprintf(stderr, "Failed to add");
        return 1;
    }
    free(tmp2);

    free(ahbh_16n);
    free(albh_16n);
    free(ahbl_16n);

    (*res) = result;

    return 0;
}

/**
 * @brief frees recources 
 * 
 */
void freeRecources(void){
    if(numberA != NULL){
        free(numberA);
    }
    if(numberB != NULL){
        free(numberB);
    }
    if(fp != NULL){
        fclose(fp);
    }
    if(ah != NULL){
        free(ah);
    }
    if(al != NULL){
        free(al);
    }
    if(bh != NULL){
        free(bh);
    }
    if(bl != NULL){
        free(bl);
    }
    if(sumresult != NULL){
        free(sumresult);
    }
    if(ahbh != NULL){
        free(ahbh);
    }
    if(ahbl != NULL){
        free(ahbl);
    }
    if(albh != NULL){
        free(albh);
    }
    if(albl != NULL){
        free(albl);
    }  
}

/**
 * @brief exits the program, prints error message and frees recources
 * 
 * @param errmsg the message to print
 */
void errorExit(char *errmsg){
    fprintf(stderr, "%s\n", errmsg);
    freeRecources();
    exit(EXIT_FAILURE);
}

/**
 * @brief parses one line of text from a file
 * 
 * @param f the file to read from
 * @param line result pointer
 * @return int error code 0 if successful else 1
 */
int parseLine(FILE *f, char **line) {

    int chunkSize = 50;
    int chunkCount = 1;
    int size = 0; //string size
    char *buffer = malloc(sizeof(char)*chunkSize);
    if(buffer == NULL){
        fprintf(stderr, "Failed to alloc buffer");
        return 1;
    }

    char c;
    while((c=fgetc(f) ) != '\n'){
        //validierung
        if(c == EOF){
            break;
        }
        if(c == '\0'){
            break;
        }
        if(c == '\r'){
            continue;
        }
        if(c > 'F' || c < 'A'){
            if (c > '9' || c < '0'){
                if(c > 'f' || c < 'a'){
                    if(buffer != NULL){
                        free(buffer);
                    }
                    fprintf(stderr, "Char %c\n", c);
                    errorExit("Not a valid Hex num");
                }
            }
        }

        //buffer calc
        if (chunkCount*chunkSize < size+1){
            chunkCount++;
            char *tmp;
            tmp = realloc(buffer, sizeof(char)*chunkSize*chunkCount);
            if(tmp == NULL){
                fprintf(stderr, "Failed to alloc buffer\n");
                free(buffer);
                return 1;
            }
            chunkCount++;
        }

        //write
        buffer[size] = c;
        size++;
    }
    buffer[size] = '\0';
    (*line) = buffer;
    return 0;
}

/**
 * @brief forks the programm, sets up pipes and reroutes stdin and stdout. If child process the program intmul will be executed
 *        If parent process it writes the numbers to be processed to pipe.
 * 
 * @param pid process id
 * @param pfd_recv pipe fd
 * @param a first string to be processed by child
 * @param b second string to be processed by child
 * @return int error code 0 if successful else 1 
 */
int createChildren(pid_t *pid, int *pfd_recv, char *a, char *b){

    int pfd_send[2];
    pipe(pfd_send);

    *pid = fork();
    switch (*pid)
    {
    case -1:
        return 1;
        break;
    case 0:
        //pipe stuff
        dup2(pfd_send[0], STDIN_FILENO);
        close(pfd_send[1]);

        dup2(pfd_recv[1], STDOUT_FILENO);
        close(pfd_recv[0]);

        //exec
        execlp(program_name, program_name, NULL);
        break;
    default:
        close(pfd_send[0]);
        close(pfd_recv[1]);

        FILE *f = fdopen(pfd_send[1], "w");
        if(f==NULL){
            close(pfd_send[1]);
            return 1;
        }

        fprintf(f, "%s\n%s\n", a, b);
        
        fclose(f);
        close(pfd_send[1]);
        break;
    }
    return 0;
}

/**
 * @brief splits a string int to substrings down the middle
 * 
 * @param input the string to split
 * @param fst first half
 * @param snd second half
 * @return int error code 0 if successful else 1
 */
int splitString(char *input, char **fst, char **snd){
    int len = strlen(input)/2;

    char *tmpFst = malloc(sizeof(char)*len+1);
    if(tmpFst == NULL){
        return 1;
    }

    char *tmpSnd = malloc(sizeof(char)*len+1);
    if(tmpSnd == NULL){
        free(tmpFst);
        return 1;
    }

    memcpy(tmpFst, input, len);
    tmpFst[len] = '\0';
    memcpy(tmpSnd, input+len, len);
    tmpSnd[len] = '\0';

    *fst = tmpFst;
    *snd = tmpSnd;
    return 0;
}

/**
 * @brief wait for child and read output
 * 
 * @param pipefd filde descritor of pipe to read from
 * @param pid process id to wait for
 * @param result line that was read
 * @return int  error code 0 if successful else 1
 */
int readChild(int *pipefd, pid_t pid, char **result){
    int status;
    waitpid(pid, &status, 0);

    if (WEXITSTATUS(status) != EXIT_SUCCESS){
        return 1;
    }
    FILE *f = fdopen(pipefd[0], "r");
    if(f == NULL){
        fprintf(stderr, "Error open f\n");
        return 1;
    }

    char *temp;
    parseLine(f, &temp);
    *result = temp;
    
    fclose(f);
    return 0;
}

/**
 * @brief Program entry. Calls parseline(). Validates input. Splits input. Creates and sends to children. 
 *        Read subresults from children and adds these accordingly. Prints endresult to stdout. 
 * 
 * @param argc argument count
 * @param argv arguments
 * @return int error code 0 if successful else 1
 */
int main(int argc, char *argv[]) {
   
    int lenA;
    int lenB;
    program_name = argv[0];

    char result[3]; //fixed size to max length of minimal calculation
    result[2] = '\0'; //End of String

    fp = stdin;
    if (fp == NULL)
        exit(EXIT_FAILURE);
    
    //parse input
    if(parseLine(stdin, &numberA)){
        errorExit("Failed parsing line");
    }
    lenA = strlen(numberA);
     if(parseLine(stdin, &numberB)){
        errorExit("Failed parsing line");
    }
    lenB = strlen(numberB);

    if(lenA != lenB){
        errorExit("Numbers are not of same length");
    }

    if(lenA == 0){
        printf("\n");
        freeRecources();
        exit(EXIT_SUCCESS);
    }

    if(lenA == 1){
        multiply(numberA, numberB, &result[0]);
        fprintf(stdout, "%s", result);
        freeRecources();
        exit(EXIT_SUCCESS);
    }

    if((lenA%2)!=0){
        errorExit("Number of digits not even");
    }

    if(lenA == 0){
        printf(" ");
        freeRecources();
        exit(EXIT_SUCCESS);
    }
    
    splitString(numberA, &ah, &al);
    splitString(numberB, &bh, &bl);

    int pfd_recv_1[2]; //ahbh
    pid_t pid1;
    pipe(pfd_recv_1);
    if(createChildren(&pid1, &(pfd_recv_1[0]), ah, bh)){
       errorExit("Failed to create child");
    }

    int pfd_recv_2[2]; //ahbl
    pid_t pid2;
    pipe(pfd_recv_2);
    if(createChildren(&pid2, &(pfd_recv_2[0]), ah, bl)){
       errorExit("Failed to create child");
    }

    int pfd_recv_3[2]; //albh
    pid_t pid3;
    pipe(pfd_recv_3);
    if(createChildren(&pid3, &(pfd_recv_3[0]), al, bh)){
       errorExit("Failed to create child");
    }

    int pfd_recv_4[2]; //albl
    pipe(pfd_recv_4);
    pid_t pid4;
    if(createChildren(&pid4, &(pfd_recv_4[0]), al, bl)){
       errorExit("Failed to create child");
    }
    
   if(readChild(&(pfd_recv_1[0]), pid1, &ahbh)){
       errorExit("Failed to read from child");
   }

   if(readChild(&(pfd_recv_2[0]), pid2, &ahbl)){
       errorExit("Failed to read from child");
   }

   if(readChild(&(pfd_recv_3[0]), pid3, &albh)){
       errorExit("Failed to read from child");
   }

   if(readChild(&(pfd_recv_4[0]), pid4, &albl)){
       errorExit("Failed to read from child");
   }

    close(pfd_recv_1[0]);
    close(pfd_recv_2[0]);
    close(pfd_recv_3[0]);
    close(pfd_recv_4[0]);

    if(sum(lenA, ahbh, ahbl, albh, albl, &sumresult)){
        errorExit("Failed to add subresults");
    }

    char *nozeroes;
    for(int i = 0; i<strlen(sumresult); i++){
        if(sumresult[i] != '0'){
            nozeroes = &sumresult[i];
            break;
        }
    }

    fprintf(stdout, "%s\n", nozeroes);

    freeRecources();
    return EXIT_SUCCESS;
}