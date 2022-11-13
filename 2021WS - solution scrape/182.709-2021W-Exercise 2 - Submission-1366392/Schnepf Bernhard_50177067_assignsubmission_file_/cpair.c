#include <stdio.h>
#include <stdlib.h> //For EXIT_SUCCESS / EXIT_FAILURE
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h> //pipes
#include <math.h>
#include <sys/wait.h> //for wait
#include <float.h>

/**
 * @author Bernhard Schnepf, 12022508
 * @brief Evalutes the points pare with the shortest distance of all points given.
 * @details
 * @date 10.12.2021
 */

typedef struct Point {
    float x, y;
} Point;

static char* program_name;
static Point *points;
//Creation of pipes
//[4] => [0] P1 read, [1] P1 write, [2] P2 read, [3] P2 write
//[2] = [0] READ, [1] Write
static int pipes[4][2];

/**
 * @brief Function that prints out the USAGE and exits
 * @details The function prints out the USAGE and a detailed EXAMPLE for it. Then it exits with EXIT_FAILURE. 
 * @return none
 */
static void input_error() {
    fprintf(stderr, "USAGE: %s\n", program_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Function to be called, when exit. Frees points object.
 * @details The function closes the shared memory and semaphores if they have not been closed yet.
 * 
 */
static void clean_up(){
    if(points != NULL){
        free(points);
    }
}

/**
 * @brief Function that checks and converts the input to points and returns the mean.
 * @details The function checks if the given input is correct by comparing it to a regex pattern.
 *          If so, then it is possible to proceeed and convert the points and add it to an array of points.
 *          Because the number of inputs is unknown, we need to reallocate the memory needed, if we exceed it.
 *          At the end we calculate the mean and return it.
 * 
 * @param points_size 
 * @param points_count
 * @return float mean
 */
static float convert_points(size_t points_size, size_t *points_count){

    char *line_buf = NULL;
    size_t line_buf_size = 0;
    float mean = 0;

    //Variable to crate regex
    regex_t regex;
    //Function call to create regex that shall be checked for
    if(regcomp(&regex, "^[+-]?([0-9]+[.]?[0-9]*|[.][0-9]+) [+-]?([0-9]+[.]?[0-9]*|[.][0-9]+)$", REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) {
        fprintf(stderr, "ERROR: %s Couldn't create regex!\n",program_name);
        exit(EXIT_FAILURE);
    }

    while(getline(&line_buf, &line_buf_size, stdin) != EOF){
        if(regexec(&regex, line_buf, 0, NULL, 0) == 0){
            if(points_size == *points_count){
                points_size *= 2;
                Point *points_temp = realloc(points, sizeof(Point) * points_size);
                    if(points_temp == NULL){
                        free(line_buf);
                        regfree(&regex);
                        fprintf(stderr, "ERROR: %s Couldn't allocate points memory!\n",program_name);
                        exit(EXIT_FAILURE);
                    }
                points = points_temp;
            }

            char *line = line_buf;
            line = strtok(line," ");
            points[*points_count].x = strtof(line, NULL);
            line = strtok(NULL," ");
            points[*points_count].y = strtof(line, NULL);
            mean += points[*points_count].x;
            (*points_count)++;

        }else {
            fprintf(stderr, "ERROR: %s Wrong input pattern used!\n",program_name);
            fprintf(stderr, "WRONG: %s \n",line_buf);
            fprintf(stderr, "EXAMPLE: -1.0 4.0\n");
            free(line_buf);
            regfree(&regex);
            exit(EXIT_FAILURE);
        }
    }
    free(line_buf);
    regfree(&regex);
    return mean / (*points_count);
}

/**
 * @brief Handles the cases for amount of points <= 2
 * @details The function exits with success, if only one point or none points are given.
 *          If there is exactly two points, then we print both points to stdout and exit the program with success. 
 * 
 * @param points_count 
 */
static void check_default_cases(size_t points_count){

    if(points_count <= 1){
        exit(EXIT_SUCCESS);
    }

    if(points_count == 2){
        fprintf(stdout, "%f %f\n%f %f\n",points[0].x,points[0].y,points[1].x,points[1].y);
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Closes all pipes
 * @details The function loops through all the pipes and closes them.
 *          If an error occurs while closing, then the program exits.
 * 
 */
void close_pipes(){
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            if(close(pipes[i][j]) < 0){
                fprintf(stdout, "ERROR: %s Couldn't close pipes properly!\n",program_name);
                exit(EXIT_FAILURE);
            }
        }
        
    }
}

/**
 * @brief Initialises the pipes for the children.
 * @details The function initialises two pipes for each child as described in the assignment.
 *         
 */
void init_pipes(){
    for (size_t i = 0; i < 4; i++)
    {
        if(pipe(pipes[i]) == -1){
            fprintf(stdout, "ERROR: %s Couldn't create pipes!\n",program_name);
            close_pipes();
            exit(EXIT_FAILURE);
        }    
    }
}


/**
 * @brief Maps the according pipes input and output accordingly.
 * @details The function maps the childX read and write accordingly to
 *          STDIN and STDOUT to be able to communicate with parent / child.
 *          Returns on success 0 and on failure -1. 
 * 
 * @param child 
 * @return int 
 */
int dup_children(int child){

    //Remapping to stdin
    if(dup2(pipes[child][0], STDIN_FILENO) == -1) {
        return -1;
    }

    //Remapping to stdout    
    if(dup2(pipes[child+1][1], STDOUT_FILENO) == -1){
        return -1;
    }
    return 0;
}

/**
 * @brief Creates two copys of this process and gives them a task.
 * @details The function creates a copy of this process and exits, if it couldnt create a copy.
 *          On success, the copied process ends up in case 0, to set the pipes for communication / get a task.
 *          The parent will go into the default case and proceed to crate the second child the same way.
 * 
 * @param children 
 * @param size 
 */
void fork_children(pid_t children[], size_t size){
    for (size_t i = 0; i < size; i++)
    {
        switch (children[i] = fork())
        {
        case -1:
            fprintf(stdout, "ERROR: %s Couldn't fork child%ld!\n",program_name, i);
            close_pipes();
            exit(EXIT_FAILURE);
            break;
        
        case 0:
            if(dup_children(i*2) == -1){
                fprintf(stdout, "ERROR: %s Couldn't dup for child%ld\n!",program_name, i);
                close_pipes();
                exit(EXIT_FAILURE);
            }
            close_pipes();
            if(execlp(program_name, program_name, NULL) == -1){
                fprintf(stdout, "ERROR: %s Couldn't execlp for child%ld\n!",program_name, i);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            break;
        }
    }
    
}

/**
 * @brief Closes the unused pipes for the parent.
 * @details The function loops through the pipes and closes the unnecessary ones.
 *          On error, it exits and returns an error message.
 * 
 */
void close_parents_pipes(){
    for (size_t i = 0; i < 4; i++)
    {
        if(close(pipes[i][i%2]) < 0){
            fprintf(stdout, "ERROR: %s Couldn't close pipes properly!\n",program_name);
            exit(EXIT_FAILURE);
        }    
    }
    
}

/**
 * @brief Writes a point to the destination in a specific way.
 * @details The function writes the given point to the given output destination.
 *          On Error, the function prints an error message and exits with EXIT_FAILURE.
 * 
 * @param out 
 * @param point 
 */
static void write_point(FILE *out, Point point){
    if(fprintf(out, "%f %f\n", point.x, point.y) < 0){
        fprintf(stdout, "ERROR: %s Couldn't write into file!\n",program_name);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Writes all points <= mean to child1 and > mean to child2.
 * @details The function opens up the the communication to its children and sends
 *          child1 all points <= mean and child2 all points > mean.
 *          After that the files are being closed and the program returns.
 *          (Because of endless recursion this, when 3 points have the same x,
 *          the function randomly distributes x values into child 1 or 2, when they have the same x)
 * 
 * @param points_count 
 * @param mean 
 */
static void write_to_children(size_t points_count, float mean){
   
    FILE *out_child1 = fdopen(pipes[0][1], "w");
    FILE *out_child2 = fdopen(pipes[2][1], "w");

    for (size_t i = 0; i < points_count; i++)
    {
        if(points[i].x < mean){
            write_point(out_child1, points[i]);
        }else if(points[i].x == mean){ //To avoid endless recursion if more than 2 points have the same x
            write_point(i%2 == 0 ? out_child1 : out_child2, points[i]);
        }else {
            write_point(out_child2, points[i]);
        }
    }
    fclose(out_child1);
    fclose(out_child2);
}

/**
 * @brief Waits for its children to finish their process and handles it accordingly.
 * @details The function waits for both children to finish their process.
 *          When waiting for the children throws an error, it is handled and exits with EXIT_FAILURE.
 *          If waiting is successful, then we check if the process exited on EXIT_SUCCESS or EXIT_FAILURE.
 *          When the exit status was not successful we throw an error and exit with EXIT_FAILURE. 
 * 
 * @param children 
 * @param children_size 
 */
static void wait_for_children(pid_t children[], size_t children_size){

    for (size_t i = 0; i < children_size; i++)
    {
        int status;
        if(waitpid(children[i], &status, 0) < 0){
            fprintf(stderr, "ERROR: %s Couldn't wait for children!\n", program_name);
            exit(EXIT_FAILURE);
        }

        if(WEXITSTATUS(status) != EXIT_SUCCESS){
            fprintf(stderr, "ERROR: %s Child%ld exited with an error!\n", program_name, i);
            exit(EXIT_FAILURE);
        }
        
    }
}

/**
 * @brief Converts a line to a point.
 * @details The function converts the given line into a Point and sets the given Points x and y value to it.
 * 
 * @param point 
 * @param line_buf 
 */
static void convert_point(Point *point, char *line_buf){
    char *line = line_buf;
    line = strtok(line," ");
    (*point).x = strtof(line, NULL);
    line = strtok(NULL," ");
    (*point).y = strtof(line, NULL);
}

/**
 * @brief Reads the output of a child, converts it to a Point and puts it into the results array.
 * @details The function opens up the output of the child and reads one line after another.
 *          Because of recursion, there can only be 2 solutions provided at max.
 *          The line is then converted into a point and set to the results array.
 *          If there is not one line that can be read, then the program exits. 
 * 
 * @param pos 
 * @param results 
 * @return size_t amount of points returned (at max 3)
 */
static size_t read_from_child(int pos, Point *results){

    FILE *in_child = fdopen(pipes[pos][0], "r");
    if(in_child == NULL){
        fprintf(stderr, "ERROR: %s Couldn't open file!\n", program_name);
        exit(EXIT_FAILURE);
    }

    char *line_buf = NULL;
    size_t line_buf_size = 0;
    size_t count = 0;

    for (size_t i = 0; i < 2; i++)
    {
        int res = getline(&line_buf, &line_buf_size, in_child);
        if(res < 0 && count > 0){
            fprintf(stderr, "ERROR: %s Couldn't read line!\n", program_name);
            exit(EXIT_FAILURE);
        }else if(res > 0){
            convert_point(&results[i], line_buf);
            count++;
        }
    }
    fclose(in_child);
    free(line_buf);
    return count;
}

/**
 * @brief Caluclates the dif between two points.
 * @details The function calculates the absolute difference by using the
 *          theorem of phythagoras and math.h. Returns the difference.
 * 
 * @param p1 
 * @param p2 
 * @return float distance
 */
static float distance(Point p1, Point p2){
    return abs(sqrt((pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2))));
}

/**
 * @brief Reads from child1 and child2 the cpair, compares them and puts the best cpair of them into the result.
 * @details The function reads the best result from child1 and child2, then it returns the best one.
 *          (If one child didn't produce a cpair result, then the half with the result is the result.)
 * 
 * @param result 
 */
static void read_from_children(Point *result){
    Point p1[2];
    Point p2[2];
    size_t p1_length = read_from_child(1,p1);
    size_t p2_length = read_from_child(3,p2);

    if(p1_length == p2_length){
        if(distance(p1[0], p1[1]) <= distance(p2[0], p2[1])){
            result[0] = p1[0];
            result[1] = p1[1];
        }else {
            result[0] = p2[0];
            result[1] = p2[1];
        }
    }else if(p1_length > p2_length) {
        result[0] = p1[0];
        result[1] = p1[1];
    }else {
        result[0] = p2[0];
        result[1] = p2[1];
    }
    
}

/**
 * @brief Evaluates the best result of the recursion and distances between two ponts of half 1 and half 2.
 * @details The function checks if a distance of a point in x <= mean and point of x > point is smaller than
 *          the distance of two points in its own half. Because there is an issue if many points have the same x,
 *          this algorithm does also compare two points with the same x, that don't have the same y!
 *          At the end the cpair with the shortest distance is printed stdout.
 * 
 * @param p1p2 
 * @param points_count 
 * @param mean 
 */
static void eval_result(Point *p1p2, size_t points_count, float mean){
    float distance_p1p2 = distance(p1p2[0], p1p2[1]);
    Point p3[2];
    float distance_p3 = FLT_MAX;

    for (size_t i = 0; i < points_count; i++)
    {
        if(points[i].x <= mean){
            for (size_t j = 0; j < points_count; j++)
            {
                if(points[j].x > mean || (points[j].x == mean && points[j].y != points[i].y)){
                    if(distance_p3 > distance(points[i],points[j])){
                        distance_p3 = distance(points[i],points[j]);
                        p3[0] = points[i];
                        p3[1] = points[j];
                    }
                }
            }
            
        }
    }

    if(distance_p3 < distance_p1p2){
        write_point(stdout, p3[0]);
        write_point(stdout, p3[1]);
    }else {
        write_point(stdout, p1p2[0]);
        write_point(stdout, p1p2[1]);
    }
}


/**
 * @brief Function that evaluates the pair of points with the shortest distance.
 * @details The function first sets the program name and checks if no input arguments are given as expected.
 *          After that it allocates the points memory and sets a cleanup function for the points if the program
 *          is being stopped beforehand. After that the points from stdin are being converted and the mean calculated.
 *          Then it checks, if the amount of points is <= 2 and exits the programm accordingly.
 *          If not, then the communication pipes are being initialized and the child process started
 *          with the according points being split up by the mean. After that it waits for the child processes
 *          to finish succesfully and evaluates the cpair with their results and by checking if two points from
 *          mean <= x and > x may have a better solution. All error handling and memory usage is handled in the methods
 *          accordingly.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    program_name = argv[0];
    float mean = 0;

    //Check if no arguments are given as expected
    if(argc != 1){
        input_error();
    }

    //Malloc points
    size_t points_size = 2;
    size_t points_count = 0;
    points = malloc(sizeof(Point) * points_size);
    
    if(points == NULL){
        fprintf(stderr, "ERROR: %s Couldn't allocate points memory!\n",program_name);
        exit(EXIT_FAILURE);
    }

    //Set cleanup for points
    if(atexit(clean_up) < 0){
        fprintf(stderr, "ERROR: %s Couldn't set up clean_up function!\n", program_name);
        exit(EXIT_FAILURE);
    }

    //Convert input to points array, set mean and check default cases, points_count <= 2
    mean = convert_points(points_size, &points_count);
    check_default_cases(points_count);

    //Creation of pipes
    init_pipes(); //Does also do error handling and close pipes if needed

    //Create child processes
    pid_t children[2];
    if(points_count > 1){
        fork_children(children, 2); 
    }
    close_parents_pipes();

    //Split the array in two parts. Child 1 gets the first half where all x values of the points are <= mean
    //and Child 2 gets the rest, where x > mean
    write_to_children(points_count, mean);

    //Wait for children
    wait_for_children(children, 2);

    //Read out data from childs and eval best
    Point p1p2[2];
    read_from_children(p1p2);

    //Evaluate best result of both halfs
    eval_result(p1p2, points_count, mean);
    exit(EXIT_SUCCESS);
}
