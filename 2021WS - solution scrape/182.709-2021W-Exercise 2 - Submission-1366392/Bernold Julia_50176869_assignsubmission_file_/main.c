#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>
#include <sys/types.h>
#include <math.h>

/**
 * @brief the amount of children that are to be created
 */
#define CHILD_COUNT 4

//FLAG: Work in progress 10.12.18:00 --abgabe

/**
 * @brief the first string we read from stdin
 */
char *a;
/**
 * @brief the second string we read from stdin 
 */
char *b;
/**
 * @brief the length of a and b
 */
int length;
/**
 * @brief the name of the program, used for error output purposes
 */
char *name = "Not yet set";
/**
 * @brief contains the file descriptors for the pipe
 * there are 8 pipes total, each has a read and a write end
 */
int pipefd[8][2];
/**
 * @brief signifies whether the corresponding file descriptor
 * in pipefd is open or not
 */
int open[8][2];
/**
 * @brief the process ids of the child processes
 */
pid_t pids[4];

/**
 * @brief the mandatory usage function
 * 
 * @param message a string defining what triggered the usage function
 * gobal variable name: the name of the program
 */
static void usage(char message[])
{
    fprintf(stderr, "Usage: %s %s\n", name, message);
    exit(EXIT_FAILURE);
}

static void check_validity(void);
static void forking_process(void);
static void open_pipes(void);
static void close_unused_ends(int x, int y);
static void before_exit(void);
static void close_pipes_parent(void);
static void calc_and_print(char one[], char two[], char three[], char four[]);
static void redirect(int from, int to);
static void write_to_children(char a1[], char a2[], char b1[], char b2[]);
static void waiting_for_children(void);
static void write_at(int fd, char a[], char b[]);
static char *readfrom(int fd);

/**
 * @brief the main function of the program
 * 
 * @param argc the argument counter
 * @param argv the argument vector
 * @return EXIT_SUCCESS on successful termination, and EXIT_FAILURE otherwise
 * name: the name of the program
 * a: the first string we read
 * b: the second string we read
 * length: the length of a and b
 * pipefd: the file descriptors for the pipes
 * open: shows whether or not the file descriptor at the
 * corresponding position in pipefd has been closed yet
 */
int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        usage("this program does not take any additional options");
    }
    name = argv[0];
    size_t len = 0;
    a = NULL;
    b = NULL;
    if (getline(&a, &len, stdin) < 0)
    {
        fprintf(stderr, "%s A: Getline Failed %s\n", name, strerror(errno));
        free(a);
        exit(EXIT_FAILURE);
    }

    if (getline(&b, &len, stdin) < 0)
    {
        fprintf(stderr, "%s B: Getline Failed %s %d\n", name, strerror(errno), STDIN_FILENO);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }
    a[strlen(a) - 1] = '\0';
    b[strlen(b) - 1] = '\0';

    //a and b are now dynamically allocated and take the values of the two hex integers
    //check if a and b are valid hex integers, but don't convert them yet
    check_validity();

    length = strlen(a);

    //if the length is one, multiply
    if (length == 1)
    {
        int c = strtol(a, NULL, 16);
        int d = strtol(b, NULL, 16);
        int res = c * d;
        fprintf(stdout, "%X\n", res);
        free(a);
        free(b);
        exit(EXIT_SUCCESS);
    }

    //split a and b into halves, exit if number of digits is not even
    if (length != 2)
    {
        if (((length / 2) % 2) != 0)
        {
            free(a);
            free(b);
            usage("The two integers need to have an even amount of numbers");
        }
    }

    //opening pipes
    open_pipes();

    //splitting a and b in two parts each
    char a1[length / 2];
    a1[0] = '\0';
    strncat(a1, a, (length / 2));
    a1[strlen(a1)] = '\0';

    char a2[(length / 2) + 1];
    a2[0] = '\0';
    for (int i = 0; i < (length / 2); i++)
    {
        a2[i] = a[i + (length / 2)];
        a2[i + 1] = '\0';
    }

    char b1[length / 2];
    b1[0] = '\0';
    strncat(b1, b, (length / 2));
    b1[strlen(b1)] = '\0';

    char b2[(length / 2) + 1];
    b2[0] = '\0';
    for (int i = 0; i < (length / 2); i++)
    {
        b2[i] = b[i + (length / 2)];
        b2[i + 1] = '\0';
    }

    //create four child processes
    forking_process();

    //closing unused parent ends
    close_pipes_parent();

    //send info to children
    write_to_children(a1, a2, b1, b2);

    //wait for the children
    waiting_for_children();

    char *one = NULL;
    char *two = NULL;
    char *three = NULL;
    char *four = NULL;

    one = readfrom(pipefd[1][0]);
    one[strlen(one) - 1] = '\0';
    open[1][0] = 0;

    two = readfrom(pipefd[3][0]);
    two[strlen(two) - 1] = '\0';
    open[3][0] = 0;

    three = readfrom(pipefd[5][0]);
    three[strlen(three) - 1] = '\0';
    open[5][0] = 0;

    four = readfrom(pipefd[7][0]);
    four[strlen(four) - 1] = '\0';
    open[7][0] = 0;

    calc_and_print(one, two, three, four);
    before_exit();
    free(one);
    free(two);
    free(three);
    free(four);
    return EXIT_SUCCESS;
}

/**
 * @brief checks if the input was valid
 * @details this function takes the two strings a and b
 * and checks if they have the same length and contain only
 * characters that are expected in a hexadecimal number
 * if a newline character is encountered, it is changed to \0
 * a: the first string that is read
 * b: the second string that is read
 */
static void check_validity(void)
{
    //check for same length
    if (strlen(a) != strlen(b))
    {
        fprintf(stderr, "%s the two arguments need to have the same length\n", name);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < strlen(a); i++)
    {
        a[i] = toupper(a[i]);
        b[i] = toupper(b[i]);

        switch (a[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            break;
        case '\n':
            a[i] = '\0';
            break;
        default:
            free(a);
            free(b);
            usage("A: Invalid Character");
            break;
        }

        switch (b[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            break;
        case '\n':
            b[i] = '\0';
            break;
        default:
            free(a);
            free(b);
            usage("B: Invalid Character");
            break;
        }
    }
}

/**
 * @brief forks 4 child processes
 * @details creates four child processes, calls the
 * function which closes the unnecessary pipe ends,
 * calls the function which redirects the pipes
 * and executes them
 * pids[] array containing the process ids of processes
 * created by fork
 * pipefd[][] array containing the file descriptors for the pipes
 * name the program name
 * 
 */
static void forking_process(void)
{
    fflush(stdin);
    fflush(stdout);
    int j = 0;
    for (int i = 0; i < CHILD_COUNT; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            fprintf(stderr, "%s: An error has occurred while creating child processes 341%s\n", name, strerror(errno));
            before_exit();
            exit(EXIT_FAILURE);
        }
        else if (pids[i] == 0)
        {
            close_unused_ends(j, j + 1);

            redirect(pipefd[j + 1][1], STDOUT_FILENO);
            redirect(pipefd[j][0], STDIN_FILENO);

            execlp(name, name, NULL);
            fprintf(stderr, "%s: This statement should not be reached! %s\n", name, strerror(errno));
            before_exit();
            exit(EXIT_FAILURE);
        }
        j += 2;
    }
}

/**
 * @brief opens the pipes
 * @details opens 8 pipes and sets the 
 * corresponding position in open to 1,
 * signifying an open file descriptor
 * name the program name
 * pipefd array containing the pipe file descriptors
 * open array signifying open file descriptors in pipefd
 * 
 */
static void open_pipes(void)
{
    for (int i = 0; i < (2 * CHILD_COUNT); i++)
    {
        if (pipe(pipefd[i]) < 0)
        {
            fprintf(stderr, "%s: An error has occurred while opening the pipe 402 %s\n", name, strerror(errno));
            exit(EXIT_FAILURE);
        }

        open[i][0] = 1;
        open[i][1] = 1;
    }
}

/**
 * @brief closes unused ends
 * @details closes the ends that are not used by the
 * child process, which calls it
 * @param x the read end to remain open
 * @param y the write end to remain open
 * name the program name
 * open array signifying open file descriptors in pipefd
 * pipefd array containing the pipe file descriptors
 */
static void close_unused_ends(int x, int y)
{
    for (int i = 0; i < (2 * CHILD_COUNT); i++)
    {
        if (i == x)
        {
            open[i][1] = 0;
            if (close(pipefd[i][1]) < 0)
            {
                fprintf(stderr, "%s: An error has occurred while trying to close the file descriptor 424 %s\n", name, strerror(errno));
                before_exit();
                exit(EXIT_FAILURE);
            }
        }
        else if (i == y)
        {
            open[i][0] = 0;
            if (close(pipefd[i][0]) < 0)
            {
                fprintf(stderr, "%s: An error has occurred while trying to close the file descriptor 434 %s\n", name, strerror(errno));
                before_exit();
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            open[i][0] = 0;
            if (close(pipefd[i][0]) < 0)
            {
                exit(EXIT_FAILURE);
            }
            open[i][1] = 0;
            if (close(pipefd[i][1]) < 0)
            {
                exit(EXIT_FAILURE);
            }
        }
    }
}

/**
 * @brief exit handlers
 * @details this function is called before exiting
 * the program. It frees a and b and closes all 
 * open file descriptors
 * name the program name
 * a the first string
 * b the second string
 * open signifies which pipe file descriptors are open
 * pipefd the pipe file descriptors
 */
static void before_exit(void)
{
    free(a);
    free(b);
    for (int i = 0; i < (2 * CHILD_COUNT); i++)
    {
        if (open[i][0] == 1)
        {
            if (close(pipefd[i][0]) < 0)
            {
                fprintf(stderr, "%s An error has occurred while cleaning up ressources 467 %s\n", name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (open[i][1] == 1)
        {
            if (close(pipefd[i][1]) < 0)
            {
                fprintf(stderr, "%s An error has occurred while cleaning up ressources 474 %s\n", name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
}
/**
 * @brief closes unused pipe ends
 * @details this function closes the pipe ends
 * that are not used by the parent process
 * name the program name
 * pipefd an array containing the pipe file descriptors
 * open an array signifying which file descriptors are open
 */
static void close_pipes_parent(void)
{
    for (int i = 0; i < ((2 * CHILD_COUNT) - 1); i += 2)
    {
        if (close(pipefd[i][0]) < 0)
        {
            fprintf(stderr, "%s: An error has occurred while closing pipes 488 %s\n", name, strerror(errno));
            before_exit();
            exit(EXIT_FAILURE);
        }
        open[i][0] = 0;

        if (close(pipefd[i + 1][1]) < 0)
        {
            fprintf(stderr, "%s: An error has occurred while closing pipes 495 %s\n", name, strerror(errno));
            before_exit();
            exit(EXIT_FAILURE);
        }
        open[i + 1][1] = 0;
    }
}

/**
 * @brief redirects pipe ends
 * @details this function replaces the file descriptor 'from' 
 * with the file descriptor 'to', by closing 'to', calling dup2
 * and closing 'from'
 * 
 * @param from the file descriptor to be redirected
 * @param to the target file descriptor
 * name the program name
 */
static void redirect(int from, int to)
{
    if (close(to) < 0)
    {
        fprintf(stderr, "%s: An error has occurred while redirecting %s\n", name, strerror(errno));
        before_exit();
        exit(EXIT_FAILURE);
    }
    if (dup2(from, to) < 0)
    {
        fprintf(stderr, "%s: An error has occurred while redirecting %s\n", name, strerror(errno));
        before_exit();
        exit(EXIT_FAILURE);
    }
    if (close(from) < 0)
    {
        fprintf(stderr, "%s: An error has occurred while redirecting %s\n", name, strerror(errno));
        before_exit();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief writes to the child processes
 * @details calls the write_at method with the correct parameters
 * @param a1 first half of string a
 * @param a2 second half of string a
 * @param b1 first half of string b
 * @param b2 second half of string b
 * pipefd the pipe file descriptors
 * open signifies which file descriptors are open
 * 
 */
static void write_to_children(char a1[], char a2[], char b1[], char b2[])
{
    write_at(pipefd[0][1], a1, b1);
    open[0][1] = 0;

    write_at(pipefd[2][1], a1, b2);
    open[2][1] = 0;

    write_at(pipefd[4][1], a2, b1);
    open[4][1] = 0;

    write_at(pipefd[6][1], a2, b2);
    open[6][1] = 0;
}

/**
 * @brief writes to the child processes
 * @details this function writes the two strings
 * a and b to the file, to which the file descriptor points
 * the file stream is opened and immediately closed after using
 * @param fd the file descriptor we want to write to
 * @param a the first input string
 * @param b the second input string
 */
static void write_at(int fd, char a[], char b[])
{
    FILE *dest = fdopen(fd, "w");
    fprintf(dest, "%s\n%s\n", a, b);
    fflush(dest);
    fclose(dest);
}

/**
 * @brief waits for the child processes to terminate
 * name the program name
 */
static void waiting_for_children(void)
{
    pid_t id;
    int status = 0;
    int failed = 0;
    int i = 0;
    while (i < CHILD_COUNT)
    {
        id = wait(&status);
        if (id < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                fprintf(stderr, "%s: 681 An error has occurred while waiting for the child processes%s %d\n", name, strerror(errno), status);
                before_exit();
                exit(EXIT_FAILURE);
            }
        }
        i++;

        if (WEXITSTATUS(status) != EXIT_SUCCESS)
        {
            fprintf(stderr, "%s: 690 An error has occurred while waiting for the child processes%s %d\n", name, strerror(errno), errno);
            fflush(stderr);
            failed++;
        }
    }
    if (failed > 0)
    {
        fprintf(stderr, "%s: 696 One or multiple child processes failed\n", name);
        before_exit();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief reads a string from a file
 * @details reads a string from the file referenced 
 * by the file descriptor, adds \0 and returns it
 * 
 * @param fd the file descriptor of the file to be read from
 * @return char* the string that was read
 * name the program name
 */
static char *readfrom(int fd)
{
    FILE *from = fdopen(fd, "r");
    char *res = NULL;

    size_t len = 0;
    if (getline(&res, &len, from) < 0)
    {
        fprintf(stderr, "%s: Reading failed %s\n", name, strerror(errno));
    }
    res[strlen(res)] = '\0';

    fflush(from);
    fclose(from);
    return res;
}

/**
 * @brief calculates and prints the result
 * @details this function writes length/2 characters from
 * four, writes length/2 chars from the rest of four combined
 * with three and two and finally writes at most length chars
 * from the rest of two, three and four combined with one
 * the function prints the result of the operations performed to stdout
 * @param one the first partial result from child process 1
 * @param two the middle partial result from child process 2
 * @param three the middle partial result from child process 3
 * @param four the last partial result from child process 4
 */
static void calc_and_print(char one[], char two[], char three[], char four[])
{
    int half_len = length / 2;
    int double_len = 2 * length;

    char res[double_len + 1];
    int ind = double_len - 1;
    res[double_len] = '\0';

    int one_ind = strlen(one) - 1;
    int two_ind = strlen(two) - 1;
    int three_ind = strlen(three) - 1;
    int four_ind = strlen(four) - 1;

    char helper[] = "10";
    int divisor = strtol(helper, NULL, 16);
    int carry = 0;

    for (; ind >= 0; ind--)
    {

        if (ind >= (length + half_len))
        {
            char four_val = '\0';
            //we only use four
            if (four_ind < 0)
            {
                four_val = '0';
            }
            else
            {
                four_val = four[four_ind];
            }
            strncpy(&res[ind], &four_val, 1);
            four_ind--;
        }
        else if (ind >= length)
        {
            int four_val = 0X0;
            int two_val = 0X0;
            int three_val = 0X0;
            //we use four, three and two
            if (four_ind < 0)
            {
                four_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &four[four_ind], 1);
                help[1] = '\0';
                four_val = strtol(help, NULL, 16);
            }
            if (three_ind < 0)
            {
                three_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &three[three_ind], 1);
                help[1] = '\0';
                three_val = strtol(help, NULL, 16);
            }
            if (two_ind < 0)
            {
                two_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &two[two_ind], 1);
                help[1] = '\0';
                two_val = strtol(help, NULL, 16);
            }
            int sum = 0X0;
            sum = two_val + three_val + four_val + carry;
            carry = sum / divisor;
            sum %= divisor;
            char help[2];
            help[0] = '\0';
            sprintf(help, "%X", sum);
            help[1] = '\0';
            strncpy(&res[ind], help, 1);
            four_ind--;
            three_ind--;
            two_ind--;
        }
        else
        {
            int four_val = 0X0;
            int two_val = 0X0;
            int three_val = 0X0;
            int one_val = 0X0;
            //all arrays need to be taken into account
            if (four_ind < 0)
            {
                four_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &four[four_ind], 1);
                help[1] = '\0';
                four_val = strtol(help, NULL, 16);
            }
            if (three_ind < 0)
            {
                three_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &three[three_ind], 1);
                help[1] = '\0';
                three_val = strtol(help, NULL, 16);
            }
            if (two_ind < 0)
            {
                two_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &two[two_ind], 1);
                help[1] = '\0';
                two_val = strtol(help, NULL, 16);
            }
            if (one_ind < 0)
            {
                one_val = 0X0;
            }
            else
            {
                char help[2];
                help[0] = '\0';
                strncpy(help, &one[one_ind], 1);
                help[1] = '\0';
                one_val = strtol(help, NULL, 16);
            }
            int sum = one_val + two_val + three_val + four_val + carry;
            carry = sum / divisor;
            sum %= divisor;
            char help[2];
            help[0] = '\0';
            sprintf(help, "%X", sum);
            help[1] = '\0';
            res[ind] = help[0];
            four_ind--;
            three_ind--;
            two_ind--;
            one_ind--;
        }
    }

    fprintf(stdout, "%s\n", res);
}