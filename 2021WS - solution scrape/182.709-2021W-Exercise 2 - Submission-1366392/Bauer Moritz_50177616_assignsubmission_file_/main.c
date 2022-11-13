/**
 * @brief main file - reads 2 lines from stdin and multiplies them
 * @author Moritz Bauer, 0649647
 * @date 21.12.11
 */
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define PRINT_LEADING_ZERO 0

char *program_name = "<not yet set>";

/**
 * @brief reads a line from child
 * @param pipe_to_parent_fd 
 * @param line 
 */
static void read_line_from_child(int *pipe_fd, char *line) {

    close(pipe_fd[1]);
    FILE *f_ah_bh = fdopen(pipe_fd[0], "r");
    char *l = NULL;
    size_t len_ah_bh = 0;
    if (getline(&l, &len_ah_bh, f_ah_bh) == -1) {
        fclose(f_ah_bh);
        exit(EXIT_FAILURE);
    }
    strcpy(line, l);
    free(l);
    fclose(f_ah_bh); //also does close(pipe_to_parent_fd[0]);
}

/**
 * @brief reads a long from string
 * @param line
 * @return
 */
static long readNum(char *line) {

    char *endptr;
    long val;
    val = strtol(line, &endptr, 16);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (endptr == line) {
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

/**
 * @brief takes both pipes, redirects pipes (stdin, stdout) and executes recursive
 * @param pipe_to_child_fd
 * @param pipe_to_parent_fd
 */
static void exec_child(int *pipe_to_child_fd, int *pipe_to_parent_fd) {
    //STDIN
    close(pipe_to_child_fd[1]);
    dup2(pipe_to_child_fd[0], STDIN_FILENO);

    //STOUT
    close(pipe_to_parent_fd[0]);
    dup2(pipe_to_parent_fd[1], STDOUT_FILENO);

    execlp(program_name, program_name, NULL);

    fprintf(stderr, "execlp failed\n");
    // cleanup
    close(pipe_to_parent_fd[1]);
    close(pipe_to_child_fd[0]);
    exit(EXIT_FAILURE);

}

/**
 * @brief wait for process and check exitcode
 * @param pid
 */
static void my_wait(pid_t pid) {
    int wstatus;
    waitpid(pid, &wstatus, 0);
    if (WEXITSTATUS(wstatus)) {
        exit(EXIT_FAILURE);
    }
}

/**
 * fork and write A and B part in pipes of child processes
 * @param a
 * @param b
 * @param size
 * @param pipe_to_parent_fd
 * @return
 */
static pid_t my_fork(char *a, char *b, size_t size, int *pipe_to_parent_fd) {
    int pipe_to_child_fd[2];

    if (pipe(pipe_to_child_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }


    pid_t pid = fork();


    if (pid == -1) {
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        //CHILD
        exec_child(pipe_to_child_fd, pipe_to_parent_fd);
    }


    // PARENT
    close(pipe_to_child_fd[0]);
    write(pipe_to_child_fd[1], a, size);
    write(pipe_to_child_fd[1], b, size);
    close(pipe_to_child_fd[1]);

    return pid;
}

/**
 * @brief convert a ascii char of a hex digit to a number
 * @param ch
 * @return
 */
static char hex2num(char ch) {
    if (ch >= '0' && ch <= '9') { return ch - '0'; }
    if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
    if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
    fprintf(stderr, "No hex char: %c\n", ch);
    exit(EXIT_FAILURE);
}


/**
 * @brief convert a number to a ascii char of a hex digit
 * @param ch
 * @return
 */
static char num2hex(char ch) {
    if (ch < 0) {
        fprintf(stderr, "cant convert: %d\n", ch);
        exit(EXIT_FAILURE);
    }
    if (ch < 10) { return ch + '0'; }
    if (ch < 17) { return ch + 'a' - 10; }
    fprintf(stderr, "cant convert: %d\n", ch);
    exit(EXIT_FAILURE);
}

/**
 * @brief convert a string to an array of numbers
 * @param src
 * @param dst
 * @param size
 */
static void str2HexArr(char *src, char *dst, size_t size) {
    for (int i = 0; i < size; ++i) {
        dst[i] = hex2num(src[size - i - 1]);
    }
}

/**
 * @brief convert an array of numbers to a string
 * @param num
 * @param str
 * @param size_num
 */
static void hexArr2Str(char *num, char *str, size_t size_num) {
    size_t strpos = 0;
    for (int i = 0; i < size_num; ++i) {
        char val = num[size_num - i - 1];
#if PRINT_LEADING_ZERO
        str[strpos] = num2hex(val);
        strpos++;
#else
        if (val != 0 || strpos > 0) {
            str[strpos] = num2hex(val);
            strpos++;
        }
#endif
    }
    str[strpos] = '\0';
}

/**
 * @attention this function is NOT SAVE! only use it if there is enough space in acc! (be aware of carry)
 * @brief add a value to an accumulator
 * @param acc a array of numbers (not characters)
 * @param val a array of numbers (not characters)
 * @param size of val
 * @param offset shift left of val
 */
static void add(char *acc, char *val, size_t size, size_t offset) {
    char carry = 0;
    for (int i = 0; i < size; ++i) {
        char v = acc[i + offset] + val[i] + carry;
        acc[i + offset] = (v & 0x0f);
        carry = v > 15 ? 1 : 0;
    }

    while (carry) {
        char v = acc[size + offset] + carry;
        acc[size + offset] = (v & 0x0f);
        carry = v > 15 ? 1 : 0;
        size++;
    }

}

/**
 * @brief read result from pipe and add to result
 * @param pipe
 * @param result
 * @param len
 * @param offset
 */
static void add_to_result(int *pipe, char *result, size_t len, size_t offset) {

    char *line = malloc(len + 1);
    read_line_from_child(pipe, line);
    size_t len_ah_bh = strlen(line) - 1;
    char *ah_bh = malloc(len_ah_bh);

    str2HexArr(line, ah_bh, len_ah_bh);
    free(line);

    add(result, ah_bh, len_ah_bh, offset);
    free(ah_bh);
}

/**
 * do all the stuff
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {


    program_name = argv[0];


    char *line1 = NULL;
    char *line2 = NULL;
    size_t restrict1 = 0;
    size_t restrict2 = 0;
    ssize_t len1;
    ssize_t len2;


    //read lines
    if ((len1 = getline(&line1, &restrict1, stdin)) == -1) {
        fprintf(stderr, "could not read line 1\n");
        exit(EXIT_FAILURE);
    }
    if ((len2 = getline(&line2, &restrict2, stdin)) == -1) {
        fprintf(stderr, "could not read line 2\n");
        exit(EXIT_FAILURE);
    }

    //truncate '\n'
    if (line1[len1 - 1] == '\n') {
        line1[len1 - 1] = '\0';
        len1 = len1 - 1;
    }
    if (line2[len2 - 1] == '\n') {
        line2[len2 - 1] = '\0';
        len2 = len2 - 1;
    }

    // handle not equal length
    if (len1 != len2) {
        fprintf(stderr, "lines unequal length\n");
        exit(EXIT_FAILURE);
    }

    // handle empty
    if (len1 == 0) {
        fprintf(stderr, "empty lines\n");
        exit(EXIT_FAILURE);
    }

    // handle odd length and not length 1
    if (len1 % 2 == 1 && len1 != 1) {
        fprintf(stderr, "lines have odd length\n");
        exit(EXIT_FAILURE);
    }

    // VALID INPUT

    if (len1 > 1) {
        // case length > 1

        ssize_t len = len1;
        ssize_t lenHalf = len1 / 2;
        ssize_t lenDouble = len1 * 2;


        // SPLIT A AND B
        char *ah = malloc(lenHalf + 1);
        memcpy(ah, line1, lenHalf);
        ah[lenHalf] = '\n';

        char *al = malloc(lenHalf + 1);
        memcpy(al, line1 + lenHalf, lenHalf);
        al[lenHalf] = '\n';

        char *bh = malloc(lenHalf + 1);
        memcpy(bh, line2, lenHalf);
        bh[lenHalf] = '\n';

        char *bl = malloc(lenHalf + 1);
        memcpy(bl, line2 + lenHalf, lenHalf);
        bl[lenHalf] = '\n';




        // PIPES
        int pipe_ah_bh_fd[2];
        int pipe_ah_bl_fd[2];
        int pipe_al_bh_fd[2];
        int pipe_al_bl_fd[2];


        if (pipe(pipe_ah_bh_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(pipe_ah_bl_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(pipe_al_bh_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(pipe_al_bl_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // FORK
        pid_t pid_hh = my_fork(ah, bh, lenHalf + 1, pipe_ah_bh_fd);
        pid_t pid_hl = my_fork(ah, bl, lenHalf + 1, pipe_ah_bl_fd);
        pid_t pid_lh = my_fork(al, bh, lenHalf + 1, pipe_al_bh_fd);
        pid_t pid_ll = my_fork(al, bl, lenHalf + 1, pipe_al_bl_fd);

        free(ah);
        free(al);
        free(bh);
        free(bl);

        // WAIT
        my_wait(pid_hh);
        my_wait(pid_hl);
        my_wait(pid_lh);
        my_wait(pid_ll);


        char *result = calloc(lenDouble, sizeof(char));
        char *result_str = calloc(lenDouble, sizeof(char));
        {    // SUM PARTS
            add_to_result(pipe_ah_bh_fd, result, len, len);
            add_to_result(pipe_ah_bl_fd, result, len, lenHalf);
            add_to_result(pipe_al_bh_fd, result, len, lenHalf);
            add_to_result(pipe_al_bl_fd, result, len, 0);

            // PRINT
            hexArr2Str(result, result_str, lenDouble);
            fprintf(stdout, "%s\n", result_str);
        }
        free(result);
        free(result_str);


    } else {
        // case length = 1
        long a = readNum(line1);
        long b = readNum(line2);
        long result = a * b;

        fprintf(stdout, "%lx\n", result);


    }
    fflush(stdout);
    exit(EXIT_SUCCESS);
}
