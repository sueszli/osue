#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <ctype.h>

const char* prog_name;
/**
 * @brief when user inputs the wrong inputs for this programm
 * 
 */
void usage(void){
    fprintf(stderr, "USAGE: %s \n", prog_name);
    exit(EXIT_FAILURE);
} 


/**
 * @brief sets stdin to read end and stdout to write end of pipe of two different pipes
 * 
 * @param pipes 2 pipes that have the right write and read end
 */
static void redirect_pipes(int pipes[2][2]){

    //stdout to write end of pipes
    if (dup2(pipes[0][1], STDOUT_FILENO) == -1)
		{
            fprintf(stderr, "Error in %s: %s \n", prog_name,strerror((long)ferror));
            exit(EXIT_FAILURE);
	}
    if (dup2(pipes[1][0], STDIN_FILENO) == -1)
        {
            fprintf(stderr, "Error in %s: %s \n", prog_name,strerror((long)ferror));
            exit(EXIT_FAILURE);
    }
}

/**
 * @brief converts int to hex char
 * 
 * @param i int that needs to be converted to hex char (0-15)
 */
static char int_to_char(int i){
    if(i < 10){
        return '0'+i;
    }
    return 'a'+i-10;
}

/**
 * @brief converts hex char to int
 * 
 * @param c char that needs to be converted ( 0 - 9 or a - f)
 */
static int char_to_int(char c){
    if(c >= '0' && c <= '9'){
        return c - '0';
    } else {
        return tolower(c) - 'a' +10;
    }
}

/**
 * @brief closes write and read ends of 4 pipes
 * 
 * @param pipehh pipe that needs to be closed
 * @param pipehl pipe that needs to be closed
 * @param pipelh pipe that needs to be closed
 * @param pipell pipe that needs to be closed
 */
static void close_pipes(int pipehh[2][2],int pipehl[2][2],int pipelh[2][2],int pipell[2][2]){
    for(int i = 0; i <2; i++){
        for(int j= 0; j <2; j++){
            close(pipehh[i][j]);
            close(pipehl[i][j]);
            close(pipelh[i][j]);
            close(pipell[i][j]);
        }
    }
}


/**
 * @brief add hex chars
 * 
 * @param a first char and where result is stored
 * @param b second char
 * @param overflow from previous adds
 */
static void add_chars(char *a, const char *b, char * overflow){
    int value = char_to_int(*a) + char_to_int(*b) + char_to_int(*overflow);
	*a = int_to_char(value % 16);
	*overflow = int_to_char(value / 16);
}

/**
 * @brief add hex number in a string a + b
 * 
 * @param a first array and where result is stored
 * @param b second array
 * @param offset off set indicates the 16^offset of the b array
 */
static void add_string_hex(char *a, const char *b,int offset){
    char overflow = '0';
    int lengtha = strlen(a);
    int lengthb = strlen(b);
    for (int i = 0; i < lengthb; i++){
        add_chars(&a[lengtha-1-offset-i], &b[lengthb-1-i], &overflow);
    }
    int k = 0;
    k= 0;
    while(overflow != '0'){
        char zero = '0';
        add_chars(&a[lengtha-1-offset-lengthb-k],&zero, &overflow);
        k++;
    }

}

/**
 * @brief used to remove leading zeros of a char array
 * 
 * @param input array that needs leading zeros removed
 */
static void ignore_leading_zeros(char * input){
    char * copy = (char *)strdup(input);
    int i = 0;
    while(copy[i] == '0'){
        i++;
    }
    for(; copy[i] != '\0'; i++){
        *input = copy[i];
        input++;
    }
    free(copy);
    *input = '\0';
}


/**
 * @brief 
 * 
 * @param argc size of argv
 * @param argv Array of terminal inputs
 * @return int never happens, there is always an Exit() before
 */
int main(int argc, char *argv[]){
    prog_name = argv[0];

    if(argc != 1) {
        usage();
    }

    size_t t;
    char *first = NULL;
    char *second = NULL;

    getline(&first, &t, stdin);
    getline(&second,&t, stdin);
    if(first[strlen(first)-1] == '\n'){
        first[strlen(first)-1] = '\0';
    }
    if(second[strlen(second)-1] == '\n'){
        second[strlen(second)-1] = '\0';
    }
    if(strspn(first, "0123456789abcdefABCDEF") != strlen(first) ||  strspn(second, "0123456789abcdefABCDEF") != strlen(second)){
        fprintf(stderr, "Error in %s: input one or two is not a hex number\n", prog_name);
        free(first);
        free(second);
        exit(EXIT_FAILURE);
    }
    if(strlen(first) != strlen(second)){
        fprintf(stderr, "input one and two have different size\n");
        free(first);
        free(second);
        exit(EXIT_FAILURE);
    }

    if(strlen(first) == 1){
        //multhex
        int result = (int)strtol(first,NULL,16) * (int)strtol(second,NULL,16);
        fprintf(stdout, "%x\n", result);
        free(first);
        free(second);
        exit(EXIT_SUCCESS);
    }

    if(strlen(first) % 2 !=0){
        fprintf(stderr, "length of input not even\n");
        free(first);
        free(second);
        exit(EXIT_FAILURE);
    }

    int length = strlen(first)/2;
    char Al[length+2];
    char Ah[length+2];
    char Bl[length+2];
    char Bh[length+2];

    for(int i=0; i< length; i++) {
        Al[i] = first[i+length];
        Ah[i] = first[i];
        Bl[i] = second[i+length];
        Bh[i] = second[i];
    }
    free(first);
    free(second);

    Al[length] = '\n';
    Ah[length] = '\n';
    Bl[length] = '\n';
    Bh[length] = '\n';

    Al[length+1] = '\0';
    Ah[length+1] = '\0';
    Bl[length+1] = '\0';
    Bh[length+1] = '\0';


    // 0 read pipe for parent
    // 1 write pipe for parent
    int pipeshh[2][2];
    int pipeshl[2][2];
    int pipeslh[2][2];
    int pipesll[2][2];

    for(int i = 0; i <2; i++){
        if(pipe(pipeshh[i]) == -1 || pipe(pipeshl[i]) == -1 || pipe(pipeslh[i]) == -1 || pipe(pipesll[i]) == -1){
            fprintf(stderr, "%s: couldn't create Pipes %s\n", prog_name ,strerror((long)ferror));
            exit(EXIT_FAILURE);
        }
    }

    //create child processes
    pid_t pidhh;

    pidhh = fork();
    switch(pidhh){
        case -1:
            fprintf(stderr, "%s: couldn't fork hh %s\n", prog_name ,strerror((long)ferror));
            exit(EXIT_FAILURE);

        //child
        case 0:
            redirect_pipes(pipeshh);
            close_pipes(pipeshh,pipeshl,pipeslh,pipesll);
            execlp(argv[0], argv[0], NULL);
            //sould not reach
            fprintf(stderr, "%s: couldn't exec hh\n", prog_name);
            exit(EXIT_FAILURE);
        default:
            ;
    }

    pid_t pidhl;

    pidhl = fork();
    switch(pidhl){
        case -1:
            fprintf(stderr, "%s: couldn't fork hl %s\n", prog_name ,strerror((long)ferror));
            exit(EXIT_FAILURE);

        case 0:
            redirect_pipes(pipeshl);
            close_pipes(pipeshh,pipeshl,pipeslh,pipesll);
            execlp(argv[0], argv[0], NULL);
            //sould not reach
            fprintf(stderr, "%s: couldn't exec hl\n", prog_name);
            exit(EXIT_FAILURE);
        default:
            ;
    }

    pid_t pidlh;

    pidlh = fork();
    switch(pidlh){
        case -1:
            fprintf(stderr, "%s: couldn't fork lh %s\n", prog_name ,strerror((long)ferror));
            exit(EXIT_FAILURE);

        case 0:
            redirect_pipes(pipeslh);
            close_pipes(pipeshh,pipeshl,pipeslh,pipesll);
            execlp(argv[0], argv[0], NULL);
            //sould not reach
            fprintf(stderr, "%s: couldn't exec lh\n", prog_name);
            exit(EXIT_FAILURE);
        default:
            ;
    }

    pid_t pidll;

    pidll = fork();
    switch(pidll){
        case -1:
            fprintf(stderr, "%s: couldn't fork ll %s\n", prog_name ,strerror((long)ferror));
            exit(EXIT_FAILURE);

        case 0:
            redirect_pipes(pipesll);
            close_pipes(pipeshh,pipeshl,pipeslh,pipesll);
            execlp(argv[0], argv[0], NULL);
            //sould not reach
            fprintf(stderr, "%s: couldn't exec ll\n", prog_name);
            exit(EXIT_FAILURE);
        default:
            ;
    }

    //close for parent
    close(pipeshh[0][1]);
    close(pipeshh[1][0]);
    close(pipeshl[0][1]);
    close(pipeshl[1][0]);
    close(pipeslh[0][1]);
    close(pipeslh[1][0]);
    close(pipesll[0][1]);
    close(pipesll[1][0]);

    //write to childs 
    write(pipeshh[1][1], Ah, strlen(Ah));
    write(pipeshh[1][1], Bh, strlen(Bh));
    close(pipeshh[1][1]);
    
    write(pipeshl[1][1], Ah, strlen(Ah));
    write(pipeshl[1][1], Bl, strlen(Bl));
    close(pipeshl[1][1]);

    write(pipeslh[1][1], Al, strlen(Al));
    write(pipeslh[1][1], Bh, strlen(Bh));
    close(pipeslh[1][1]);

    write(pipesll[1][1], Al, strlen(Al));
    write(pipesll[1][1], Bl, strlen(Bl));
    close(pipesll[1][1]);


    //wait
    int statushh;
    waitpid(pidhh, &statushh,0);
    int statushl;
    waitpid(pidhl, &statushl,0);
    int statuslh;
    waitpid(pidlh, &statuslh,0);
    int statusll;
    waitpid(pidll, &statusll,0);

    if(WEXITSTATUS(statushh) == EXIT_FAILURE ||  WEXITSTATUS(statushl) == EXIT_FAILURE || WEXITSTATUS(statuslh) == EXIT_FAILURE|| WEXITSTATUS(statusll) == EXIT_FAILURE){
        fprintf(stderr, "%s: a child terminated with failure \n", prog_name);
        exit(EXIT_FAILURE);
    }

    char returnhh[length*2+2];
    char returnhl[length*2+2];
    char returnlh[length*2+2];
    char returnll[length*2+2];
    char endresult[length*4+2];

    for(int i = 0; i < length*4+1; i++){
        endresult[i] = '0';
    }
    endresult[length*4+1] = '\0';

    int readlength = read(pipeshh[0][0], returnhh, length *2 +1);
    if(returnhh[readlength-1] == '\n'){
        returnhh[readlength-1] = '\0';
    }
    close(pipeshh[0][0]);

    readlength = read(pipeshl[0][0], returnhl, length *2 +1);
    if(returnhl[readlength-1] == '\n'){
        returnhl[readlength-1] = '\0';
    }
    close(pipeshl[0][0]);

    readlength = read(pipeslh[0][0], returnlh, length *2 +1);
    if(returnlh[readlength-1] == '\n'){
        returnlh[readlength-1] = '\0';
    }
    close(pipeslh[0][0]);

    readlength = read(pipesll[0][0], returnll, length *2 +1);
    if(returnll[readlength-1] == '\n'){
        returnll[readlength-1] = '\0';
    }
    close(pipesll[0][0]);
    add_string_hex(endresult, returnll, 0);
    add_string_hex(endresult, returnhl, length);
    add_string_hex(endresult, returnlh, length);
    add_string_hex(endresult, returnhh, 2*length);

    ignore_leading_zeros(endresult);

    fprintf(stdout, "%s\n", endresult);

    /*
    int hh = 1;
    for(int i = 0; i< length*2 ; i++){
        hh = hh *16;
    }

    int lhhl = 1;
    for(int i = 0; i< (length) ; i++){
        lhhl = lhhl *16;
    }

    int endresult = (int)strtol(returnhh,NULL,16) * hh + (int)strtol(returnhl,NULL,16) * lhhl + (int)strtol(returnlh,NULL,16) * lhhl + (int)strtol(returnll,NULL,16);
        
    fprintf(stdout, "%x\n", endresult);
    */
    

    exit(EXIT_SUCCESS);
}