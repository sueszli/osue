
/**
* @file cpair.c
* @author Valentin Schnabl, 11848108
* @date 02.12.2021
* @brief cpair calculates the nearest point pair out of many points the user can input
* @details First, the input is beeing handled from stdin, then it will be checked if the programm calls itself, so it can operate with recursion. Then it will fork and calculate the nearest pair
* out of the result of the two children.
* 
**/
#include "cpair.h"

/**
* @brief this function informs the user about the correct usage of the program. It is beeing called if the user fails to enter the correct amount of arguments or puts in too many.
* @details prints "Usage: cpair" in stderr
**/
static void usage(void){
    fprintf(stderr, "Usage: %s\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}
/**
* @brief this is being called first when calling the program. It is evaluating if correct arguments are being passed. Then it is calling spawnChild if more than 2 points are beeing passed. 
* Then it writes and reades into the child pipes and clalculates the end result with calcClosestPairMulti.
* @param argc argument counter
* @param argv arguments and options in an array
* @return EXIT_SUCCESS or EXIT FAILURE
**/
int main(int argc, char* argv[]){
    if(argc != 1){
        fprintf(stderr, "Program takes no arguments\n");
        usage();
    }
    programName = argv[0];
    Point *points;
    size_t len = parseInput(stdin, &points);
    
    if(len == 0){
        free(points);
        usage();
    }
    if(len == 1){
        free(points);
        exit(EXIT_SUCCESS);
    }
    else if(len == 2){
        for(int i = 0; i < len; ++i){
            fprintf(stdout,"%f %f\n", points[i].a,points[i].b);
        }
        fflush(stdout);
        free(points);
        exit(EXIT_SUCCESS);
    }
    //pipes and child creation
    Pipes *pipes1;
    spawnChild(&pipes1);
    FILE *child1Write = fdopen(pipes1->write, "w");
    if (child1Write == NULL){
        fprintf(stderr, "Couldn't open file");
    }
    FILE *child1Read = fdopen(pipes1->read, "r");
    if (child1Write == NULL){
        fprintf(stderr, "Couldn't open file");
    }
    if (child1Read == NULL || child1Write == NULL)
    {
        fprintf(stderr, "Cannot open stdin/stdout");
        close(pipes1->read);
        close(pipes1->write);
        free(points);
        exit(EXIT_FAILURE);
    }
    Pipes *pipes2;
    spawnChild(&pipes2);
    FILE *child2Write = fdopen(pipes2->write, "w");
    if(child2Write == NULL){
        fprintf(stderr, "Couldn't open file");
    }
    FILE *child2Read = fdopen(pipes2->read, "r");
    if (child2Write == NULL){
        fprintf(stderr, "Couldn't open file");
    }
    if (child2Read == NULL || child2Write == NULL)
    {
        fprintf(stderr, "Cannot open stdin/stdout");
        close(pipes2->read);
        close(pipes2->write);
        free(points);
        exit(EXIT_FAILURE);
    }
    //splitting array
    Point* lessThan = malloc(sizeof(Point) * len);
    for(int i = 0; i < len; ++i){
        lessThan[i].a = -1;
        lessThan[i].b = -1;
    }
    Point* greaterThan = malloc(sizeof(Point) * len);
    for(int i = 0; i < len; ++i){
        greaterThan[i].a = -1;
        greaterThan[i].b = -1;
    }
    int lessThanCounter = 0;
    int greaterThanCounter = 0;
    float m = mean(points, len);
    for(int i = 0; i < len; ++i){
        if(points[i].a <= m){
            lessThan[lessThanCounter].a = points[i].a;
            lessThan[lessThanCounter].b = points[i].b;
            lessThanCounter++;
        }
        else{
            greaterThan[greaterThanCounter].a = points[i].a;
            greaterThan[greaterThanCounter].b = points[i].b;
            greaterThanCounter++;
        }
    }
    Point *tmp = realloc(greaterThan, sizeof(Point) * greaterThanCounter);
    greaterThan = tmp;
    Point *tmp1 = realloc(lessThan, sizeof(Point) * lessThanCounter);
    lessThan = tmp1;

    if(writeToStdIn(child1Write, lessThan, lessThanCounter) == -1){
        fprintf(stderr, "Writing to child1 has failed.\n");
        free(points);
        free(pipes2);
        free(pipes1);
        free(lessThan);
        free(greaterThan);
        exit(EXIT_FAILURE);
    }
    if(writeToStdIn(child2Write, greaterThan, greaterThanCounter) == -1){
        fprintf(stderr, "Writing to child1 has failed.\n");
        free(points);
        free(pipes2);
        free(pipes1);
        free(lessThan);
        free(greaterThan);
        exit(EXIT_FAILURE);
    }

    //waiting for children
    int status;
    int count = 0;
    while(wait(&status) == -1); //todo mal 2
    if(WEXITSTATUS(status) != EXIT_SUCCESS){
        count++;
    }
    while (wait(&status) == -1)
        ; //todo mal 2
    if (WEXITSTATUS(status) != EXIT_SUCCESS)
    {
        count++;
    }

    if(count == 1){
        fprintf(stderr, "A child died with an error\n");
        free(lessThan);
        free(greaterThan);
        exit(EXIT_FAILURE);
    }
    if (count == 2)
    {
        fprintf(stderr, "Both children died with an error\n");
        free(lessThan);
        free(greaterThan);
        exit(EXIT_FAILURE);
    }

    Point* resChild1;
    int length1 = parseInput(child1Read, &resChild1);
    Point *resChild2;
    int length2 = parseInput(child2Read, &resChild2);
    ResultSet p1;
    p1.a = resChild1[0];
    p1.b = resChild1[1];
    if(length1 ==2){
        p1.dist = dist(p1.a, p1.b);
    }
    else{
        p1.dist = FLT_MAX;
    }
    ResultSet p2;
    p2.a = resChild2[0];
    p2.b = resChild2[1];
    if (length2 == 2)
    {
        p2.dist = dist(p2.a, p2.b);
    }
    else
    {
        p2.dist = FLT_MAX;
    }
    ResultSet p3 = p3 = calcClosestPairMulti(resChild1, resChild2, length1, length2);
    printSmallestDist(p1, p2, p3);

    free(resChild1);
    free(resChild2);
    free(points);
    free(pipes2);
    free(pipes1);
    free(lessThan);
    free(greaterThan);

    exit(EXIT_SUCCESS);
}

/**
* @brief Prints the neaerst result set to stdout. The nearest pair, is the pair which distance is the smallest.
* @param a,b,c the three result sets.
**/
static void printSmallestDist(ResultSet a, ResultSet b, ResultSet c){
    ResultSet cur = a;

    if(b.dist < cur.dist){
        cur = b;
    }
    if(c.dist < cur.dist){
        cur = c;
    }
    fprintf(stdout, "%f %f\n%f %f\n", cur.a.a, cur.a.b, cur.b.a, cur.b.b);
}

/**
* @brief Creates a new Child and the pipes for reading and writing into and from the child.
* @param pipes The pipes array where the pipes are stored
* @return a pid_t of the child just created
**/
static pid_t spawnChild(Pipes** pipes){
    *pipes = malloc(sizeof(Pipes));
    int childpipes[2][2];
    if(pipe(childpipes[0]) == -1 || pipe(childpipes[1]) == -1){ //creating pipes
        fprintf(stderr, "Couldn't create pipes\n");
        exit(EXIT_FAILURE);
    }
    pid_t child = fork(); //child creation
        switch(child){
            case -1: //error
                fprintf(stderr, "Couldn't fork!\n");
                close(childpipes[0][1]);
                close(childpipes[1][0]);
                close(childpipes[0][0]);
                close(childpipes[1][1]);
                exit(EXIT_FAILURE);
            case 0: //child
                close(childpipes[0][1]);
                close(childpipes[1][0]);
                if(dup2(childpipes[0][0], STDIN_FILENO) == -1){
                    fprintf(stderr, "Couldn't dup2\n");
                    close(childpipes[1][1]);
                }
                if (dup2(childpipes[1][1], STDOUT_FILENO) == -1){
                    fprintf(stderr, "Couldn't dup2\n");
                    close(childpipes[1][1]);
                    exit(EXIT_FAILURE);
                }
                if(execlp(programName, programName, NULL) == -1){
                    fprintf(stderr, "Couldn't exec %s", PROGRAM_NAME);
                    exit(EXIT_FAILURE);
                }
            default: //parent
                close(childpipes[1][1]);
                close(childpipes[0][0]);
                (*pipes) -> read = childpipes[1][0];
                (*pipes) -> write = childpipes[0][1];
                break;
        }
        return child;
}

/**
* @brief calculates the mean out of an array of points.
* @param points the array of points
* @param size the size of the array
* @return the mean.
**/
static float mean(Point* points, size_t size){
    float sum = 0;
    for(int i = 0; i < size; ++i){
        sum += points[i].a;
    }
    sum /= size;
    return sum;
}

/**
* @brief calculates the distance of two points
* @param x,y the two points
* @return the distance.
**/
static float dist(Point x, Point y){
    return sqrt((pow(y.a - x.a, 2) + pow(y.b - x.b, 2)));
}

/**
* @brief Parses the input from a file and validates it with regex. It must be a valid floating point number. It will be passed into a point array which is dynamically allocated.
* @param file the file pointer from which is beeing read
* @param points the point array which is being filled
* @return the size of the point array, 0 if validation or somthing else failed
**/
static size_t parseInput(FILE* file, Point** points){
    *points = malloc(sizeof(Point));
    size_t len = 0;
    int counter = -1;
    size_t count = 0;
    char* line = NULL;
    regex_t regex;
    regcomp(&regex, "^[+-]?([0-9]*[.])?[0-9]+ [+-]?([0-9]*[.])?[0-9]+(\n)?$", REG_EXTENDED | REG_NOSUB);
    while(getline(&line, &len, file) != -1){
        counter ++;
        if (strcmp(line, "\n") == 0){
            break;
        }
        if (regexec(&regex, line, 0, NULL, 0)){
            regfree(&regex);
            free(line);
            fprintf(stderr, "Format is not correct, point must be [float float]\n");
            return count;
        }
        else{
            Point *tmp = realloc(*points, sizeof(Point) * len);
            if(tmp == NULL){
                fprintf(stderr, "Couldn't allocate memory!");
                exit(EXIT_FAILURE);
            }
            *points = tmp;
            
            char* begin = line;
            char* end;
            (*points)[counter].a = strtof(begin, &end);
            begin = end + 1;
            (*points)[counter].b = strtof(begin, &end);
            count++;
        }
    }
    fclose(file);
    regfree(&regex);
    free(line);
    return count;
}

/**
* @brief Writes a point array to a file, seperated by blanks.
* @param file the file pointer where it will be written to
* @param points the point array
* @param size the size of the point array
* @return 1 if the writing process was successful, -1 else
**/
static int writeToStdIn(FILE* file, Point* points, int size){
    for(int i = 0; i < size; ++i){
        if(fprintf(file, "%f %f\n", points[i].a, points[i].b) < 0){
            return -1;
        }
    }
    fflush(file);
    fclose(file);
    return 1;
}

/**
* @brief Calculate the nearest pair out of two point arrays. 
* @param points the point array from child1
* @param size the size from points
* @param points1 the point array from child2
* @param size1 the size from points1
* @return p3, a ResultSet, containing the nearest pair and the distance
**/
static ResultSet calcClosestPairMulti(Point* points, Point* points1, int size, int size1){
    float cur = FLT_MAX;
    ResultSet res;

    if(size == 0){
        res.a = points1[0];
        res.b = points1[1];
        res.dist = dist(res.a, res.b);
    }
    if(size1 == 0){
        res.a = points[0];
        res.b = points[1];
        res.dist = dist(res.a, res.b);
    }
    else{
        for (int i = 0; i < size; ++i)
        {
            Point temp1 = points[i];
            for (int j = 0; j < size1; ++j)
            {
                Point temp2 = points1[j];
                float d = dist(temp1, temp2);
                if (d <= cur)
                {
                    res.a = temp1;
                    res.b = temp2;
                    res.dist = dist(temp1, temp2);
                    cur = res.dist;
                }
            }
        }
    }
    return res;
}