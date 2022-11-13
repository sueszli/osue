/**
 * @file cpair.c
 * @author Jakob Roithinger 52009269
 * @date 8.12.2021
 *
 * @brief Reads in points and returns a pair with the shortest distance to one another
 * @details This program read in two dimensional points from stdin, the x and y coordinates are floats, to be separated by a single
 * space, and each point is to separated by a new line. Then the program calculates the shortest distance between two points
 * by forking two children recursively, and feeding them points over a pipe. To decide which points are given to which child
 * the mean of all x-values is calculated, and one child gets all points higher than the mean, the other one all lower.
 *
 **/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <mcheck.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/// MAX_IN is the maximum lenght of a line and is composed of maximum realistic 
/// lentgh of a float (=46 found empirically) times 2, plus 1 (space) plus 1 (\n)
#define MAX_IN 94 
#define INT_MAX 2147483647
#define tfflush fflush(stderr)
#define ISCHILD 1
#define ISPARENT 0
#define NO_POINTS_STR "np"
#define IGNORE_COMPILER_WARNING -1

#define errormsg fprintf
#define tempfpf nop
#define tempfpf2 nop
/**
 * @brief Takes in variable amount of arguments and does nothing
 * @details Used for turning debug messages on and off, i.e. changing define of tempfpf
 * @param a Any arguments usually passed to fprintf
 **/
static void nop(void *a, ...) { return; }

/// positions of different pointers in pointers_to_free
#define LINEBUFFER_POS 0
#define POINTS_POS 1
#define POINTSLEFT_POS 2
#define POINTSRIGHT_POS 3
#define POINTSCHILD1_POS 4
#define POINTSCHILD2_POS 5
#define POINTERS_TO_FREE_SIZE 6
char *program_name;       ///< name of the file
void **pointers_to_free;  ///< array of pointers which need to be freed at exit
int alloced_bit_map = 0;  ///< bitmap of which pointers have been malloced

/// struct to encapsulate a two dimensional point
typedef struct point_2d {
    float x;
    float y;
} point_2d_t;

/**
 * @brief Prints usage to stderr and exits.
 * @details Global variables: program_name.
 **/
static void usage(void);

/**
 * @brief Iteratively calculates mean x value of point array.
 * @details Iterative approach is to avoid overflow. Global variables: program_name.
 * @param points Array of points.
 * @param size_of_points_array Size of the array points.
 * @return The mean of the x-values of the point array.
 **/
static float iterative_mean_xs(point_2d_t points[], int size_of_points_array);

/**
 * @brief Forks the current program once.
 * @details In order to split the work up, this program forks itself twice. This function only performs one fork,
 * and sets up all the necessary pipes for the child and parent process to communicate with one another.
 * Global variables: program_name.
 * @param pipefd_child_to_parent "Pipe" array for communication used for child to write to parent.
 * @param pipefd_parent_to_child "Pipe" array for communication used for parents to write to child.
 * @return The pid of the child process, in case this process is the parent.
 **/
static pid_t my_fork(int pipefd_child_to_parent[2], int pipefd_parent_to_child[2]);

/**
 * @brief Write all points in array to specified file.
 * @details Global variables: program_name.
 * @param file_out File that is to be written to.
 * @param points Array of points that are to be written in the format specified.
 * @param total_points The amount of entries in points.
 **/
static void write_points_to_file(FILE *file_out, point_2d_t points[], int total_points);

/**
 * @brief Reads in two points from filed.
 * @details Reads two points in string format from file and returns them, if the file is closed, then the string.
 * represented by the macro NO_POINTS_STR is returned. Global variables: program_name.
 * @param in_pipe_file The file to be read from.
 * @return A string containing two points.
 **/
static char *read_two_points(FILE *in_pipe_file);

/**
 * @brief Converts a string to a point_2d_t struct.
 * @details  Global variables: program_name.
 * @param str The string to be converted.
 * @return A two dimensional point with coordinates according to the string.
 **/
static point_2d_t string_to_point2d(char *str);

/**
 * @brief Parses a string to a float.
 * @details Parameters end_pointer is changed for use in string_to_point2d. Global variables: program_name.
 * @param cur_point_str_from_stdin The string to be converted.
 * @param end_pointer A pointer to the first character that could not be converted to part of the float.
 * @return A float with corresponding to value displayed in the string given.
 **/
static float parse_float(char *cur_point_str_from_stdin, char **end_pointer);

/**
 * @brief Calculates the Euclidian distance between two points.
 * @details Global variables: program_name.
 * @param p1 The first point.
 * @param p2 The second point.
 * @return The distance between these two points.
 **/
static float get_distance_between_points(point_2d_t p1, point_2d_t p2);

/**
 * @brief Closes the file with fd and writes error messages.
 * @details Global variables: program_name.
 * @param fd File descriptor of the file to be closed.
 * @param child_to_parent Either ISCHILD or ISPARENT to make error messages have more meaning. Purely informational.
 * @return Any error code thrown while closing.
 **/
static int close_with_stderr(int fd, int child_to_parent);
/**
 * @brief Performs dup2 and writes error messages.
 * @details Global variables: program_name.
 * @param old_fd File descriptor as first argument of dup2.
 * @param new_fd File descriptor as second argument of dup2.
 * @param child_to_parent Either ISCHILD or ISPARENT to make error messages have more meaning. Purely informational.
 * @return Any error code thrown while closing.
 **/
static int dup2_with_stderr(int old_fd, int new_fd, int child_to_parent);

/**
 * @brief Writes points to child process in format specified.
 * @details Global variables: program_name.
 * @param pipe_fd File descriptor of the write-end of a pipe that is to be written to.
 * @param points Array of points to be written.
 * @param total_points Total amount of points.
 **/
static void write_points_to_child(int pipe_fd, point_2d_t *points, int total_points);

/**
 * @brief Writes points to child process in format specified.
 * @details Wait for the child process to write points to pipe and reads them. Global variables: program_name.
 * @param child_pid PID of child
 * @param pipefd File descriptor of the read-end of a pipe that is to be written to.
 * @param safe_malloc_val Either POINTSCHILD1_POS or POINTSCHILD2_POS, used to malloc for string that is read.
 * @return Two points read from child in string format.
 **/
static char *read_from_child(pid_t child_pid, int pipefd, int safe_malloc_val);

/**
 * @brief Calculates which pair of points is closest and writes them to parent.
 * @details Takes in the closest pair of points as string from child processes, converts them to point_2d_t format
 * calculates the distance between them, then calculates distances between all points given to child1
 * and all points given to child2. Then check which pair of points, is closest and writes them to parent.
 * Global variables: program_name.
 * @param points_child_1 Points as string returned by child1.
 * @param points_child_2 Points as string returned by child2.
 * @param points_left Points array given to child1.
 * @param total_left_points Amount of points given to child1.
 * @param points_right Points array given to child2.
 * @param total_right_points Amount of points given to child2.
 **/
static void write_closest_points_to_parent(char *points_child_1, char *points_child_2, point_2d_t *points_left, int total_left_points, point_2d_t *points_right, int total_right_points);

/**
 * @brief Safely reallocs a pointer, stores the pointer in global variable
 * @details Reallocs and manages resources and marks them to be freed at exit. Global variables: program_name, pointers_to_free.
 * @param old_pointer Pointer to be realloced, use NULL is malloc behaviour intended.
 * @param size Size to be malloced.
 * @param pos Position of pointer in pointers_to_free, of of the macros ending in _POS
 * @return Pointer to realloced address.
 **/
static void *safe_realloc(void *old_pointer, size_t size, int pos);

/**
 * @brief Safely frees a resources malloced with safe_realloc.
 * @details Frees rescource at pointers_to_free[pos]. Global variables: program_name, pointers_to_free.
 * @param pos Position of pointer in pointers_to_free to be freed, of of the macros ending in _POS
 **/
static void safe_free(int pos);

/**
 * @brief Safely frees all resources malloced with safe_realloc.
 * @details Global variables: program_name, pointers_to_free.
 **/
static void freeall();


int main(int argc, char *argv[]) {
    program_name = argv[0];
    pointers_to_free = malloc(sizeof(void(*)) * POINTERS_TO_FREE_SIZE);
    if (pointers_to_free == NULL) {
        errormsg(stderr, "\n%s: Failed to allocate memory: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (atexit(freeall)){
        errormsg(stderr, "\n%s %d: could not assign freeall to at exit\n", program_name, getpid());
        exit(EXIT_FAILURE);
    }

    char option_char;  ///< used to temporarily store option chars

    /// standard use of getopt
    while ((option_char = getopt(argc, argv, "")) != -1) {
        switch (option_char) {
            case '?':
                usage();
            default:
                assert(0);
        }
    }

    if (argc > 1) {
        usage();
    }

    /// pipes for communication between parent and child
    int pipefd_c1_child_to_parent[2];
    int pipefd_c1_parent_to_child[2];
    int pipefd_c2_parent_to_child[2];
    int pipefd_c2_child_to_parent[2];
    pid_t child1_pid = IGNORE_COMPILER_WARNING;
    pid_t child2_pid = IGNORE_COMPILER_WARNING;

    size_t max_line_length = MAX_IN;
    int total_points = 0;
    char *line_buffer = safe_realloc(NULL, MAX_IN * sizeof(char), LINEBUFFER_POS);
    int point_arr_max_len = 2;
    point_2d_t *points = safe_realloc(NULL, point_arr_max_len * sizeof(point_2d_t), POINTS_POS);

    tempfpf(stderr, "\n%s %d:launched\n", program_name, getpid());
    while (getline(&line_buffer, &max_line_length, stdin) != EOF) {
        if (total_points > point_arr_max_len) {
            point_arr_max_len *= 2;
            points = safe_realloc(points, point_arr_max_len * sizeof(point_2d_t), POINTS_POS);
            if (points == NULL) {
                errormsg(stderr, "%s: Failed to reallocate memory to points array with size %d: %s\n", program_name, point_arr_max_len, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        if (total_points == 2) {
            /// fork first child
            if (my_fork(pipefd_c1_child_to_parent, pipefd_c1_parent_to_child) == -1) {
                errormsg(stderr, "%s: Failed to fork child: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            /// fork second child
            if (my_fork(pipefd_c2_child_to_parent, pipefd_c2_parent_to_child) == -1) {
                errormsg(stderr, "%s: Failed to fork child: %s\n", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        tempfpf(stderr, "\n%s %d ______in_______: %s", program_name, getpid(), line_buffer);

        *(points + total_points++) = string_to_point2d(line_buffer);
    }
    safe_free(LINEBUFFER_POS);

    if (total_points <= 1) {  ///< could be 0 if interrupted
        tempfpf(stderr, "\n%s %d: only %d points, exiting:\n", program_name, getpid(), total_points);
        fclose(stdout);
        exit(EXIT_SUCCESS);
    }
    if (total_points == 2) {
        tempfpf(stderr, "\n%s %d: only %d points, writing:\n", program_name, getpid(), total_points);
        write_points_to_file(stdout, points, 2);
        fclose(stdout);
        exit(EXIT_SUCCESS);
    }

    /// more than two points => calculate mean...
    float xs_mean = iterative_mean_xs(points, total_points);
    /// ...and split array 
    point_2d_t *points_left = safe_realloc(NULL, total_points * sizeof(point_2d_t), POINTSLEFT_POS);
    point_2d_t *points_right = safe_realloc(NULL, total_points * sizeof(point_2d_t), POINTSRIGHT_POS);
    int total_left_points = 0, total_right_points = 0;
    if (points_left == NULL || points_right == NULL) {
        errormsg(stderr, "%s: Failed to allocate memory to split points array: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < total_points; i++) {
        if ((*(points + i)).x <= xs_mean) {
            *(points_left + total_left_points++) = *(points + i);
        } else {
            *(points_right + total_right_points++) = *(points + i);
        }
    }
    /// in case all points have same x value, then total right points will be empty
    if (total_right_points == 0) {
        total_left_points--;
        *points_right = *(points_left + total_left_points);
        total_right_points++;
    }
    safe_free(POINTS_POS);

    /// write to children
    write_points_to_child(pipefd_c1_parent_to_child[1], points_left, total_left_points);
    write_points_to_child(pipefd_c2_parent_to_child[1], points_right, total_right_points);

    tempfpf(stderr, "%s %d: waiting on children\n", program_name, getpid());

    /// read from children
    char *points_child_1 = read_from_child(child1_pid, pipefd_c1_child_to_parent[0], POINTSCHILD1_POS);
    char *points_child_2 = read_from_child(child2_pid, pipefd_c2_child_to_parent[0], POINTSCHILD2_POS);

    /// calculate p1, p2, p3 and write to parents
    write_closest_points_to_parent(points_child_1, points_child_2, points_left, total_left_points, points_right, total_right_points);

    tempfpf(stderr, "\n%s %d: DONE", program_name, getpid());
    exit(EXIT_SUCCESS);
}

static void usage(void) {
    errormsg(stderr, "Usage: %s \n", program_name);
    exit(EXIT_FAILURE);
}

static float iterative_mean_xs(point_2d_t points[], int size_of_points_array) {
    float avg = 0;
    int t = 1;
    for (int i = 0; i < size_of_points_array; i++) {
        avg += ((*(points + i)).x - avg) / t++;
    }
    return avg;
}

static pid_t my_fork(int pipefd_child_to_parent[2], int pipefd_parent_to_child[2]) {
    /// Initilize pipes before forking
    int err = 0;
    if ((err |= pipe(pipefd_child_to_parent)) != 0 || (err |= pipe(pipefd_parent_to_child)) != 0) {
        errormsg(stderr, "%s %d: Error initializing pipes: %s", program_name, getpid(), strerror(errno));
    }

    pid_t pid = fork();
    switch (pid) {
        case -1:
            errormsg(stderr, "%s %d: Error forking: %s", program_name, getpid(), strerror(errno));
        case 0:
            /// child process
            err |= close_with_stderr(pipefd_child_to_parent[0], ISCHILD);  ///< close unused read end of child to parent
            err |= close_with_stderr(pipefd_parent_to_child[1], ISCHILD);  ///< close unused write end of parent to child

            err |= dup2_with_stderr(pipefd_child_to_parent[1], STDOUT_FILENO, ISCHILD);
            err |= dup2_with_stderr(pipefd_parent_to_child[0], STDIN_FILENO, ISCHILD);

            err |= close_with_stderr(pipefd_child_to_parent[1], ISCHILD);  ///< close old write end
            err |= close_with_stderr(pipefd_parent_to_child[0], ISCHILD);  ///< close old read end
            if (err == -1) {
                /// error message is taken care of in close_with_stderr and dup2_with_stderr
                exit(EXIT_FAILURE);
            }
            execlp(program_name, program_name, NULL);
            errormsg(stderr, "%s %d: Cannot exec : %s\n", program_name, getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        default:
            /// parent process
            err |= close_with_stderr(pipefd_child_to_parent[1], ISCHILD);  ///< close unused write end of child to parent
            err |= close_with_stderr(pipefd_parent_to_child[0], ISCHILD);  ///< close unused read end of parent to child
            if (err == -1) {
                exit(EXIT_FAILURE);
            }
    }
    return pid;  ///< can only ever return child pid or -1
}

static void write_points_to_file(FILE *file_out, point_2d_t points[], int total_points) {
    for (int i = 0; i < total_points; i++) {
        float x = (*(points + i)).x;
        float y = (*(points + i)).y;
        tempfpf(stderr, "\n%s %d: writing point i = %d, (x, y) = %f %f:", program_name, getpid(), i, x, y);
        fflush(stderr);
        if (fprintf(file_out, "%f %f\n", x, y) == -1) {
            errormsg(stderr, "%s %d: Failed to print points through pipe: %s\n", program_name, getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    tempfpf(stderr, "\n%s %d: wrote", program_name, getpid());
    fflush(stderr);
}

static char *read_two_points(FILE *in_pipe_file) {
    size_t max_line_length = MAX_IN;  
    char *line1 = malloc(sizeof(char) * MAX_IN);
    char *line2 = malloc(sizeof(char) * MAX_IN);
    int chars_read = 0;
    chars_read = getline(&line1, &max_line_length, in_pipe_file);
    if (chars_read == EOF) {
        /// errormsg(stderr, "\n%s %d: Failed to read points through pipe after i = 0 points: %s. Probably no points given.\n", program_name, getpid(), strerror(errno));
        /// assume child closed output, therefore no points are given
        return NO_POINTS_STR;
    }
    chars_read = getline(&line2, &max_line_length, in_pipe_file);
    if (chars_read == EOF) {
        errormsg(stderr, "\n%s %d: Failed to read points through pipe after i = 1 points: %s\n", program_name, getpid(), strerror(errno));
        return NO_POINTS_STR;
    }
    tempfpf(stderr, "\n%s %d: read(ed) from child", program_name, getpid());
    fclose(in_pipe_file);
    return strcat(line1, line2);
}

static point_2d_t string_to_point2d(char *str) {
    char *end_pointer1;
    char *end_pointer2;

    float x = parse_float(str, &end_pointer1);
    float y = parse_float(end_pointer1, &end_pointer2);
    if (*end_pointer2 != 13 && *end_pointer2 != '\n') {  ///< 13 is line feed
        errormsg(stderr, "\n%s %d: Error parsing float: \"%s\"", program_name, getpid(), str);
        exit(EXIT_FAILURE);
    }
    point_2d_t ret = {x, y};
    return ret;
}

static float parse_float(char *std_buffer, char **end_pointer) {
    float x = strtof(std_buffer, end_pointer);

    if (x == HUGE_VALF) {
        errormsg(stderr, "\n%s: Floating point number in %s is too big", program_name, std_buffer);
        exit(EXIT_FAILURE);
    }
    if (!isspace(**(end_pointer)) || (std_buffer == *end_pointer)) {
        errormsg(stderr, "%s %d: Error parsing floating point number in %s because of character \"%c\"", program_name, getpid(), std_buffer, **(end_pointer));
        exit(EXIT_FAILURE);
    }
    return x;
}

static float get_distance_between_points(point_2d_t p1, point_2d_t p2) {
    float delta_x = (p1.x - p2.x);
    float delta_y = (p1.y - p2.y);
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

static int close_with_stderr(int fd, int child_to_parent) {
    int err = close(fd);
    if (err == -1) {
        if (child_to_parent == ISCHILD) {
            errormsg(stderr, "%s %d: error closing read from child to parent pipe: %s\n", program_name, getpid(), strerror(errno));
        } else {
            errormsg(stderr, "%s %d: error closing read from parent to child pipe: %s\n", program_name, getpid(), strerror(errno));
        }
    }
    return err;
}

static int dup2_with_stderr(int old_fd, int new_fd, int child_to_parent) {
    int err_or_fd = dup2(old_fd, new_fd);
    if (err_or_fd == -1) {
        if (child_to_parent == ISCHILD) {
            errormsg(stderr, "%s %d: error with dup child to parent pipe: %s\n", program_name, getpid(), strerror(errno));
        } else {
            errormsg(stderr, "%s %d: error with dup parent to child pipe: %s\n", program_name, getpid(), strerror(errno));
        }
        return err_or_fd;
    }
    return 0;
}

static char *read_from_child(pid_t child_pid, int pipefd, int safe_malloc_val) {
    int child_status = 0;
    char *points_child = safe_realloc(NULL, sizeof(char) * MAX_IN * 2, safe_malloc_val);
    int wait_err = waitpid(child_pid, &child_status, 0);
    tempfpf(stderr, "\n%s %d: child done\n", program_name, getpid());
    if (wait_err == -1 || child_status != EXIT_SUCCESS) {
        errormsg(stderr, "\n%s %d: failed to wait for child: %s\n", program_name, getpid(), strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *in_pipe_file = fdopen(pipefd, "r");
    if (in_pipe_file == NULL) {
        errormsg(stderr, "\n%s %d: Failed to fdopen pipe read: %s\n", program_name, getpid(), strerror(errno));
        exit(EXIT_FAILURE);
    }
    tempfpf2(stderr, "\n%s %d: child 1 points: %s\n", program_name, getpid(), points_child);
    return read_two_points(in_pipe_file);
}

static void write_closest_points_to_parent(char *points_child_1, char *points_child_2, point_2d_t *points_left, int total_left_points, point_2d_t *points_right, int total_right_points) {
    /// calculate p3
    float pair3_dist = INT_MAX;
    int p3_i, p3_j;
    for (int i = 0; i < total_left_points; i++) {
        for (int j = 0; j < total_right_points; j++) {
            float dist = get_distance_between_points(points_left[i], points_right[j]);
            if (dist < pair3_dist) {
                pair3_dist = dist;
                p3_i = i;
                p3_j = j;
            }
        }
    }

    /// convert string from children to points
    float pair1_dist = INFINITY;
    point_2d_t pair1_p1;
    point_2d_t pair1_p2;
    tempfpf2(stderr, "\n%s %d: parsing points from child1, string:%s\n", program_name, getpid(), points_child_1);
    fflush(stderr);
    if (strcmp(points_child_1, NO_POINTS_STR) != 0) {
        pair1_p1 = string_to_point2d(points_child_1);
        pair1_p2 = string_to_point2d(index(points_child_1, '\n'));
        pair1_dist = get_distance_between_points(pair1_p1, pair1_p2);
    }

    float pair2_dist = INFINITY;
    point_2d_t pair2_p1;
    point_2d_t pair2_p2;
    tempfpf2(stderr, "\n%s %d: parsing points from child2, string:%s\n", program_name, getpid(), points_child_2);
    fflush(stderr);
    if (strcmp(points_child_2, NO_POINTS_STR) != 0) {
        pair2_p1 = string_to_point2d(points_child_2);
        pair2_p2 = string_to_point2d(index(points_child_2, '\n'));
        pair2_dist = get_distance_between_points(pair2_p1, pair2_p2);
    }

    /// get minimum distance of P1, P2, P3
    float min_dist;
    point_2d_t min_pair_p1, min_pair_p2;
    if (pair1_dist < pair2_dist) {
        min_pair_p1 = pair1_p1;
        min_pair_p2 = pair1_p2;
        min_dist = pair1_dist;
    } else {
        min_pair_p1 = pair2_p1;
        min_pair_p2 = pair2_p2;
        min_dist = pair2_dist;
    }
    if (pair3_dist < min_dist) {
        min_pair_p1 = points_left[p3_i];
        min_pair_p2 = points_right[p3_j];
        min_dist = pair3_dist;
    }

    /// write closest points to stdout
    write_points_to_file(stdout, &min_pair_p1, 1);
    write_points_to_file(stdout, &min_pair_p2, 1);
    fclose(stdout);
}

static void *safe_realloc(void *old_pointer, size_t size, int pos) {
    void *pointer = realloc(old_pointer, size);
    if (pointer == NULL) {
        errormsg(stderr, "%s: Failed to allocate memory: %s\n", program_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    alloced_bit_map |= (1 << pos);
    pointers_to_free[pos] = pointer;
    return pointer;
}

static void safe_free(int pos) {
    if (!(alloced_bit_map & (1 << pos))) {
        tempfpf(stderr, "\n%s %d: Can not free at pos %d\n", program_name, getpid(), pos);
        exit(EXIT_FAILURE);
    }
    alloced_bit_map ^= (1 << pos);
    free(pointers_to_free[pos]);
}

static void freeall() {
    for (int i = 0; i < POINTERS_TO_FREE_SIZE; i++) {
        if ((alloced_bit_map & (1 << i))) {
            safe_free(i);
        }
    }
}


static void write_points_to_child(int pipe_fd, point_2d_t *points, int total_points) {
    FILE *file_parent_to_child = fdopen(pipe_fd, "w");
    if (file_parent_to_child == NULL) {
        errormsg(stderr, "\n%s %d: failed to fdopen fd=%d: %s", program_name, getpid(), pipe_fd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    write_points_to_file(file_parent_to_child, points, total_points);
    fclose(file_parent_to_child);
}
