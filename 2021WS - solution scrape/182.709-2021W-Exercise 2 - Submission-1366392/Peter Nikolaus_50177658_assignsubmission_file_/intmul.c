/**
 * @file intmul.c
 * @author Nikolaus Peter 11919877 <e11919877@student.tuwien.ac.at>
 * @date 10.12.2021
 *
 * @brief Main program module.
 * 
 * This programm takes 2 hexadecimal integers of the same length from stdin, multiplies them and prints the result.
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char* myprog;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 * Will result in EXIT_FAILURE.
 */
static void usage(void){
    fprintf(stderr,"Usage: %s\n",myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief This function checks if the given char is a hexadecimal digit. 
 * @param c The char that is checked.
 * @return Returns 0 if the char is not a hexadecimal deigit, 1 if it is.
 */
static int isHexChar(char c){
    return (((c >= (int)'a')&&(c <= (int)'f'))  ||  ((c >= (int)'A')&&(c <= (int)'F'))  ||  ((c >= (int)'0')&&(c <= (int)'9')));
}

/**
 * @brief This function checks if the given String is a hexadecimal integer. 
 * @param x The string that is checked.
 * @return Returns 0 if the string is not a hexadecimal integer, 1 if it is.
 */
static int isHex(char* x){
    for (int i = 0; i < strlen(x); i++){
        if (!(isHexChar(x[i]))){
            return 0;
        }
    }
    return 1;
}

/**
 * @brief This function converts an integer to a hexadecimal digit. 
 * @param a The integer that is converted.
 * @pre 0 <= a <= 15
 * @return Returns a hexadecimal digit (or X if precondition is not met).
 */
static char toHex(int a){
    switch (a){
        case 0:
            return '0';
            break;
        case 1:
            return '1';
            break;
        case 2:
            return '2';
            break;
        case 3:
            return '3';
            break;
        case 4:
            return '4';
            break;
        case 5:
            return '5';
            break;
        case 6:
            return '6';
            break;
        case 7:
            return '7';
            break;
        case 8:
            return '8';
            break;
        case 9:
            return '9';
            break;
        case 10:
            return 'a';
            break;
        case 11:
            return 'b';
            break;
        case 12:
            return 'c';
            break;
        case 13:
            return 'd';
            break;
        case 14:
            return 'e';
            break;
        case 15:
            return 'f';
            break;
        default:
            return 'X';
            break;
    }
}

/**
 * @brief This function checks if the String is equal to zero. 
 * @param x The string that is checked.
 * @return Returns 0 if the string is not equal to zero, 1 if it is. (only hexadecimal digits are considered)
 */
static int isZero(char* a){
    for (int i = 0; i < strlen(a); i++){
        if (isHexChar(a[i]) && (a[i] != '0')){
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Calculates the sum of two hexadecimal integers of any size.
 * @param a The string of the first number.
 * @param b The string of the second number.
 * @return Returns the sum of the two numbers.
 */
static char* add(char* a, char* b){
    if (isZero(a)){
        char* result = malloc((strlen(b)+1)*sizeof(char));
        strcpy(result,b);
        return result;
    }
    if (isZero(b)){
        char* result = malloc((strlen(a)+1)*sizeof(char));
        strcpy(result,a);
        return result;
    }
    int minlen;
    int maxlen;

    for (int i = 0; i < strlen(a); i++){
        if (!isHexChar(a[i])){
            a[i] = '\0';
        }
    }

    for (int i = 0; i < strlen(b); i++){
        if (!isHexChar(b[i])){
            b[i] = '\0';
        }
    }

    char* ln;
    char* sn;

    if (strlen(a) < strlen(b)){
        minlen = strlen(a);
        maxlen = strlen(b);

        ln = b;
        sn = a;
    } else {
        minlen = strlen(b);
        maxlen = strlen(a);

        ln = a;
        sn = b;
    }

    char* result = malloc(sizeof(char)*maxlen + 2);
    result[0] = '0';
    for (int i = 1; i < maxlen+1; i++){
        result[i] = ln[i-1];
    }

    int carry = 0;
    for (int i = 0; i < minlen; i++){
        char *lnc = malloc(sizeof(char)*2);
        char *snc = malloc(sizeof(char)*2);
        lnc[0] = ln[(strlen(ln)-1)-i];
        lnc[1] = '\0';
        snc[0] = sn[(strlen(sn)-1)-i];
        snc[1] = '\0';

        int num1 = strtol(lnc,NULL,16);
        int num2 = strtol(snc,NULL,16);
        
        int res = num1 + num2 + carry;
        carry = 0;

        if(res > 15){
            res -= 16;
            carry++;
        }

        result[(maxlen) - i] = toHex(res);
        free(lnc);
        free(snc);
    }

    int i = minlen;
    while (carry == 1){
        
        if (i >= strlen(ln)){
            carry = 0;
            result[0] = '1';
        } else {
            char *lnc = malloc(sizeof(char)*2);
            lnc[0] = ln[(strlen(ln)-1)-i];
            lnc[1] = '\0';
            int num1 = strtol(lnc,NULL,16);
            int res = num1 + carry;
            carry = 0;
            if (res > 15){
                res -= 16;
                carry = 1;
            }
            result[(maxlen) - i] = toHex(res);
            i++;
            free(lnc);
        }
        
    }

    while (result[0] == '0'){
        memmove(result, result+1, strlen(result));
    }

    for (int i = 0; i < strlen(result); i++){
        if (!isHexChar(result[i])){
            result[i] = '\0';
            break;
        }
    }

    return result;
}

/**
 * @brief Add n zeros to the end of the String a
 * @param a The string
 * @param n the number of zeros that will be added.
 * @return Returns a new string with the added zeros. (if a is equal to 0 no further zeros will be added)
 */
static char* addZeros(char* a, int n){
    if (isZero(a)){
        char* b = malloc(2*sizeof(char));
        b[0] = '0';
        b[1] = '\0';
        return b;
    }

    char* b = malloc(sizeof(char)*(n + strlen(a) + 1));
    for (int i = 0; i < strlen(a); i++){
        b[i] = a[i];
    }
    for(int i = strlen(a); i < strlen(a)+n; i++){
        b[i] = '0';
    }
    b[strlen(a)+n] = '\0';
    return b;
}


/**
 * Program entry point.
 * @brief reads the two numbers from stdin, checks input if it is valid, sets up pipes, forks into 4 child processes that calculate a the
 * product of smaller numbers and finally adds those result back together. Prints the final result to stdout
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc,char* argv[]){
    myprog = argv[0];
    if (argc > 1){
        usage();
    }

    size_t sizeA = 0;
    char* a;
    size_t sizeB = 0;
    char* b;

    if (!feof(stdin)){
        getline(&a,&sizeA,stdin);
        if (a[strlen(a)-1] == '\n'){
            a[strlen(a)-1] = '\0';
        }
    } else {
        fprintf(stderr, "empty input\n");
        exit(EXIT_FAILURE);
    }

    if (!feof(stdin)){
        getline(&b,&sizeB,stdin);
        if (b[strlen(b)-1] == '\n'){
            b[strlen(b)-1] = '\0';
        }
        if ((b[0] == '\n') && (strlen(b) > 1)){
            memcpy(b, b+1, strlen(b)-1);
        }
    } else {
        fprintf(stderr, "no second number\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(a) != strlen(b)){
        fprintf(stderr, "\na:%s b:%s\n",a,b);
        fprintf(stderr, "The number of digits is different in A and B\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(a) == 1){
        int x = strtol(a,NULL,16);
        int y = strtol(b,NULL,16);
        fprintf(stdout,"%x\n",(x*y));
        exit(EXIT_SUCCESS);
    }

    if ((strlen(a)%2) == 1){
        fprintf(stderr, "The number of digits is not even!\n");
        exit(EXIT_FAILURE);
    }

    if (!(isHex(a) && isHex(b))){
        fprintf(stderr, "The Input is not a hexadecimal!\n");
        exit(EXIT_FAILURE);
    }

    char* a1 = malloc(((strlen(a)/2)+1) * sizeof(char));
    memcpy(a1, a, strlen(a)/2);

    char* a2 = malloc(((strlen(a)/2)+1) * sizeof(char));
    memcpy(a2, a+(strlen(a)/2), strlen(a)/2);

    char* b1 = malloc(((strlen(b)/2)+1) * sizeof(char));
    memcpy(b1, b, strlen(b)/2);

    char* b2 = malloc(((strlen(b)/2)+1) * sizeof(char));
    memcpy(b2, b+(strlen(b)/2), strlen(b)/2);

    int n = strlen(a);

    free(a);
    free(b);

    char* as[2];
    as[0] = a1;
    as[1] = a2;

    char* bs[2];
    bs[0] = b1;
    bs[1] = b2;

    int inpipefd[4][2];
    pipe(inpipefd[0]);
    pipe(inpipefd[1]);
    pipe(inpipefd[2]);
    pipe(inpipefd[3]);
    int outpipefd[4][2];
    pipe(outpipefd[0]);
    pipe(outpipefd[1]);
    pipe(outpipefd[2]);
    pipe(outpipefd[3]);

    for (int i = 0; i < 4; i++){
        pid_t pid = fork();
        if (pid < 0){
            fprintf(stderr, "ERROR!!!\n");
            exit(EXIT_FAILURE);
        }
        if (pid == 0){
            //child process
        
            close(outpipefd[i][0]);
            close(inpipefd[i][1]);
            dup2(inpipefd[i][0], STDIN_FILENO);
            dup2(outpipefd[i][1], STDOUT_FILENO);
            close(inpipefd[i][0]);
            close(outpipefd[i][1]);
            execlp("./intmul", "./intmul", NULL);
        } else {
            //parent process

            close(inpipefd[i][0]);
            close(outpipefd[i][1]);
            char* num1;
            char* num2;
            
            num1 = malloc(strlen(as[(i/2)%2])*sizeof(char));
            strcpy(num1,as[(i/2)%2]);
            num2 = malloc(sizeof(char)*(2+strlen(bs[(i%2)])));
            num2[0] = '\n';
            num2[1] = '\0';

            strcat(num2,bs[(i%2)]);

            write(inpipefd[i][1], num1, strlen(num1)+1);
            write(inpipefd[i][1], num2, strlen(num2)+1);
            close(inpipefd[i][1]);

            free(num1);
            free(num2);
        }
    }

    free(a1);
    free(a2);
    free(b1);
    free(b2);

    int status1;
    int status2;
    int status3;
    int status4;
    wait(&status1);
    wait(&status2);
    wait(&status3);
    wait(&status4);

    //checks if all child processes have terminated normally (by exit or return from main)
    if (!(WIFEXITED(status1) && WIFEXITED(status2) && WIFEXITED(status3) && WIFEXITED(status4))){
        fprintf(stderr,"not all children have terminated normally\n");
        exit(EXIT_FAILURE);
    }

    //checks if all child processes have terminated with success
    if (WEXITSTATUS(status1) || WEXITSTATUS(status2) || WEXITSTATUS(status3) || WEXITSTATUS(status4)){
        fprintf(stderr,"not all terminations were successful\n");
        fprintf(stderr,"not all terminations were successful\n");
        exit(EXIT_FAILURE);
    }

    char* buf1 = malloc(n+1);
    char* buf2 = malloc(n+1);
    char* buf3 = malloc(n+1);
    char* buf4 = malloc(n+1);

    read(outpipefd[0][0], buf1, (n+1));
    read(outpipefd[1][0], buf2, (n+1));
    read(outpipefd[2][0], buf3, (n+1));
    read(outpipefd[3][0], buf4, (n+1));

    //the \n is not part of the hexadecimal number
    for (int i = 0; i < strlen(buf1); i++){
        if (buf1[i] == '\n'){
            buf1[i] = '\0';
        }
    }
    for (int i = 0; i < strlen(buf2); i++){
        if (buf2[i] == '\n'){
            buf2[i] = '\0';
        }
    }
    for (int i = 0; i < strlen(buf3); i++){
        if (buf3[i] == '\n'){
            buf3[i] = '\0';
        }
    }
    for (int i = 0; i < strlen(buf4); i++){
        if (buf4[i] == '\n'){
            buf4[i] = '\0';
        }
    }

    char* val11 = addZeros(buf1,n);
    char* val12 = addZeros(buf2,(n/2));
    char* val21 = addZeros(buf3,(n/2));
    char* val22 = addZeros(buf4,0);

    char* result1 = add(val11,val12);
    char* result2 = add(val21, val22);
    char* result3 = add(result1,result2); 
    printf("%s\n", result3);
    
    free(result1);
    free(result2);
    free(result3);

    free(buf1);
    free(buf2);
    free(buf3);
    free(buf4);

    free(val11);
    free(val12);
    free(val21);
    free(val22);

    close(outpipefd[0][0]);
    close(outpipefd[1][0]);
    close(outpipefd[2][0]);
    close(outpipefd[3][0]);

    return EXIT_SUCCESS;
}