/**
 * @file
 * @author Florian Spitzer, Student ID 00507913
 * @date 2021-12-07
 *
 * @brief FFT program.
 * 
 * This program reads a signal of complex numbers from **stdin**, and writes
 * the discrete Fourier transform of this signal to **stdout**.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * The name of this program. Will be set at the beginning of main() and shall
 * stay constant then.
 */
static char prog_name[256] = "<undefined>";

/**
 * A complex number.
 */
typedef double complex complex_t;

/**
 * @brief Check if **c** is a line delimiter.
 * @details Line delimiters are newline characters and zeroes.
 * Regarding zero a delimiter makes it possible to ommit a newline at the end
 * of the input.
 * @param c The character.
 * @return Returns 1 if c is a delimiter and 0 otherwise.
 */
static int is_delim(int c)
{
    return c == '\n' || c == 0;
}

/**
 * @brief Find out the precision of a floating point number stored in a string.
 * @details The precision is the number of digits after the decimal point.
 * @param p The string.
 * @return Return the precision or -1 if no decimal point is found in the string.
 */
static int precision(char *p)
{
    char *dot = strchr(p, '.');
    if (dot == NULL) return -1;
    dot++;
    int prec;
    for (prec = 0; dot[prec] <= '9' && dot[prec] >= '0'; prec++)
        ;
    return prec;
}

/**
 * @brief Read a complex number from **fp**.
 * @details The complex number has to be in the form "a" or "a b" or "a b*i",
 * where a and b are valid numbers in the sense of strtod().
 * After the complex number there must be a delimiter as defined by
 * **is_delim**.
 * If successful, store the complex number in *c;
 * and also increase *maxprec to the higher of the two precisions
 * encountered, if neccessary.
 * @param fp The stream to read from.
 * @param c Pointer to a complex number to store the result.
 * @param maxprec Pointer to an integer storing the highest precision
 * encountered so far.
 * @return Return zero on scuccess and non-zero on error.
 */
static int read_complex(FILE *fp, complex_t *c, int *maxprec)
{
    char *line = NULL;
    size_t size = 0;
    double real = 0, imag = 0;
    if (getline(&line, &size, fp) < 0) {
        free(line);
        return 1;
    }
    char *p;
    int realprec = precision(line);
    real = strtod(line, &p);
    if (p == line) {
        free(line);
        return 2;
    }
    if (is_delim(*p)) {
        *c = real;
        free(line);
        if (realprec > *maxprec)
            *maxprec = realprec;
        return 0;
    }
    char *p0;
    int imagprec = precision(p);
    imag = strtod(p, &p0);
    if (p0 == p) {
        free(line);
        return 3;
    }
    if (is_delim(*p0) || (*p0 == '*' && p0[1] == 'i' && is_delim(p0[2]))) {
        *c = real + imag*I;
        free(line);
        if (realprec > *maxprec)
            *maxprec = realprec;
        if (imagprec > *maxprec)
            *maxprec = imagprec;
        return 0;
    }
    free(line);
    return 4;
}

/**
 * @brief Check if stream **fp** has reached EOF.
 * @details Write a message to stderr if ungetc() failed.
 * Use the global variable **prog_name** to give the name of
 * this program.
 * @param fp The stream.
 * @return Returns 1 if **fp** has reached EOF, -1 if ungetc() failed,
 * and 0 otherwise.
 */
static int at_eof(FILE *fp)
{
    int a = fgetc(fp);
    if (a == EOF) return 1;
    if (ungetc(a, fp) == EOF) {
        fprintf(stderr, "%s: ungetc() failed\n", prog_name);
        return -1;
    }
    return 0;
}

/**
 * @brief Write a complex number to a stream, with a specified precision.
 * @param fp The stream to write to.
 * @param c The complex number to write.
 * @param precision How many digits to write after the decimal point.
 */
static void print_complex(FILE *fp, complex_t c, int precision)
{
    char fmt[256] = {0};
    sprintf(fmt, "%%.%dlf %%.%dlf*i\n", precision, precision);
    fprintf(fp, fmt, creal(c), cimag(c));
}

/**
 * @brief Close a file descriptor.
 * @details Write a message to **stderr** if the call to close() fails.
 * Use the global variable **prog_name** to give the name of
 * this program.
 * @param fd The file descriptor to close.
 */
static void close_fd(int fd)
{
    if (close(fd) < 0)
        fprintf(stderr, "%s: close(%d) failed\n", prog_name, fd);
}

/**
 * @brief Close a stream.
 * @details Write a message to **stderr** if the call to fclose() fails.
 * Use the global variable **prog_name** to give the name of
 * this program.
 * @param stream The stream to close.
 */
static void close_stream(FILE *stream)
{
    if (fclose(stream) == EOF)
        fprintf(stderr, "%s: close(%p) failed\n", prog_name, (void *)stream);
}


/**
 * @brief Create a child process.
 * @details Set up two pipes for communication between parent and child.
 * In the parent:
 * Set **fps[0]** to an input stream connected to the child process' **stdout**.
 * Set **fps[1]** to an output stream connected to the child process' **stdin**.
 * In the child:
 * Use dup2() to connect **stdin** and **stdout** to the parent's two streams.
 * In case of an error, write a message to stdout and exit with EXIT_FAILURE.
 * Use the global variable **prog_name** to give the name of
 * this program.
 * @param fps Pointer to an array of two streams to open.
 * @param toclose Pointer to an array of two streams to close if fork() fails.
 * @return Return the PID of the child process in the parent process.
 * Do not return in the child process.
 */
static int do_fork(FILE **fps, FILE **toclose) {
    int raw[2] = {0}, transformed[2] = {0};
    if (pipe(raw) < 0) {
        fprintf(stderr, "%s: pipe() failed\n", prog_name);
        exit(EXIT_FAILURE);
    }
    if (pipe(transformed) < 0) {
        fprintf(stderr, "%s: pipe() failed\n", prog_name);
        close_fd(raw[0]); close_fd(raw[1]);
        exit(EXIT_FAILURE);
    }
    pid_t childpid = 0;
    if ((childpid = fork()) == -1) {
        fprintf(stderr, "%s: fork() failed\n", prog_name);
        close_fd(raw[0]); close_fd(raw[1]);
        close_fd(transformed[0]); close_fd(transformed[1]);
        if (toclose != NULL) {
            fclose(toclose[0]); fclose(toclose[1]);
        }
        exit(EXIT_FAILURE);
    } else if (childpid == 0) {
        /* this is the child process */
        close_fd(raw[1]);
        close_fd(transformed[0]);
        if (dup2(raw[0], 0) < 0
            || dup2(transformed[1], 1) < 0) {
            fprintf(stderr, "%s: dup2() failed\n", prog_name);
            close_fd(raw[0]);
            close_fd(transformed[1]);
            exit(EXIT_FAILURE);
        }
        execlp("./forkFFT", "./forkFFT", NULL);
        fprintf(stderr, "%s: execlp() failed\n", prog_name);
        close_fd(raw[0]);
        close_fd(transformed[1]);
        exit(EXIT_FAILURE);
    } else {
        /* this is the original process */
        close_fd(raw[0]);
        close_fd(transformed[1]);
        fps[0] = fdopen(transformed[0], "r");
        fps[1] = fdopen(raw[1], "w");
        if (fps[0] == NULL || fps[1] == NULL) {
            if (fps[0] != NULL) close_stream(fps[0]);
            if (fps[1] != NULL) close_stream(fps[1]);
            fprintf(stderr, "%s: fdopen() failed\n", prog_name);
            close_fd(raw[1]);
            close_fd(transformed[0]);
            exit(EXIT_FAILURE);
        }
    }
    return childpid;
}

/**
 * @brief The program starts here. Read a signal from **stdin** and
 * recursively use child processes to gather the discrete Fourier transform
 * of this signal. Write the transform to **stdout**.
 * @details Set the global variable **prog_name** to argv[0] (and append the
 * PID in parentheses). Create two child processes and delegate the
 * even and on lines from **stdin** to them.
 * Wait for them to finish and read the Fourier transformes from their output.
 * Combine the two, and write this to **stdout**.
 * In case of an error, write a message to stderr and exit with EXIT_FAILURE.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char **argv)
{
    if (argc > 1) {
        fprintf(stderr, "SYNOPSIS\n    forkFFT\n");
        exit(EXIT_FAILURE);
    }
    sprintf(prog_name, "%s(%d)", argv[0], getpid());
    complex_t c = 0;
    int maxprec = 1;
    if (read_complex(stdin, &c, &maxprec) != 0) {
        fprintf(stderr, "%s: read_complex() failed in line 1\n", prog_name);
        return EXIT_FAILURE;
    }
    int eof = at_eof(stdin);
    if (eof == 1) {
        print_complex(stdout, c, maxprec);
        return EXIT_SUCCESS;
    } else if (eof < 0)
        return EXIT_FAILURE;
    FILE *fps[4] = {NULL}; /* [0] even child in, [1] even child out
                            * [2] odd child in, [3] odd child out */
    int child_pids[2] = {0};
    child_pids[0] = do_fork(fps, NULL);      /* even child */
    child_pids[1] = do_fork(fps + 2, fps);   /* odd child */
    print_complex(fps[1], c, maxprec);
    int count = 0;
    for (count = 1; (eof = at_eof(stdin)) == 0; count++) {
        if (read_complex(stdin, &c, &maxprec) != 0) {
            fprintf(stderr, "%s: read_complex() failed in line %d\n",
                    prog_name, count + 1);
            for (int i = 0; i < sizeof(fps) / sizeof(fps[0]); i++)
                close_stream(fps[i]);
            return EXIT_FAILURE;
        }
        print_complex(fps[2 * (count % 2) + 1], c, maxprec);
    }
    close_stream(fps[1]); close_stream(fps[3]);
    if (eof < 0) {
        close_stream(fps[0]); close_stream(fps[2]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 2; i++) {
        int wstatus = 0, r = 0;
        while ((r = waitpid(child_pids[i], &wstatus, 0)) != child_pids[i]) {
            if (r != -1) continue;
            if (errno == EINTR) continue;
            fprintf(stderr, "%s: waitpid() failed\n", prog_name);
            close_stream(fps[0]); close_stream(fps[2]);
            exit(EXIT_FAILURE);
        }
        if (!WIFEXITED(wstatus) || (WEXITSTATUS(wstatus) != EXIT_SUCCESS)) {
            close_stream(fps[0]); close_stream(fps[2]);
            exit(EXIT_FAILURE);
        }
    }
    complex_t high[count / 2];
    memset(high, 0, sizeof high);
    for (int i = 0; (eof = at_eof(fps[0])) == 0; i++) {
        complex_t cs[2] = {0};
        for (int j = 0; j < 2; j++)
            if (read_complex(fps[j * 2], cs + j, &maxprec) != 0) {
                fprintf(stderr,
                        "%s: read_complex() from child failed "
                        "(input length power of 2?)\n",
                        prog_name);
                close_stream(fps[0]); close_stream(fps[2]);
                return EXIT_FAILURE;
            }
        complex_t oterm = cexp(- 2 * M_PI * I * i / count) * cs[1];
        print_complex(stdout, cs[0] + oterm, maxprec);
        high[i] = cs[0] - oterm;
    }
    if (eof < 0) {
        close_stream(fps[0]); close_stream(fps[2]);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < count / 2; i++)
        print_complex(stdout, high[i], maxprec);
    close_stream(fps[0]); close_stream(fps[2]);
    return 0;
}
