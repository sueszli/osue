/**
 * @file datatypes.c
 * @author Lukas Fink 11911069
 * @date 03.12.2021
 *  
 * @brief datatypes Module implementation
 *
 * contains functions for working with the datatype struct coord
 **/

#include "datatypes.h"

static double computeDistance(struct coord first, struct coord second);

static void copyCoordPair(struct coord dest[2], struct coord src[2]);

/**
 * print function
 * @brief print a coord array to a stream
 * @details prints the coord array in the form of [[x][space][y][\n]]*
 * @param dest destination for the data to be copied
 * @param stream stream to write to
 * @param arr array of coordinates to print
 * @param size size of the array
 **/
void printCoordArray(FILE *stream, struct coord *arr, int size) {
    int n;
    for (n = 0; n < size; n++) {
        fprintf(stream, "%f %f\n", arr[n].x, arr[n].y);
    }
}

/**
 * read function
 * @brief read from a child process
 * @details reads two coordinates from input into arr
 * @param arr array to be written to
 * @param input FILE stream to be read from
 * @return 0 on success (=two coordinates have been read), +1 if only one coordinate has been read and -1 on failure.
 **/
int readFromChild(struct coord arr[2], FILE *input) {
    char *buffer = NULL;
    size_t len = 0;
    char rest = '\0';
    int count = 0;
    while(getline(&buffer, &len, input) != -1) {
        if (count > 1) {    // more than two lines/points
            free(buffer);
            return -1;
        }
        char *restX = NULL;
        arr[count].x = strtof(buffer, &restX);
        if (restX == NULL) {        // only one float
            free(buffer);
            return -1;
        }
        char *restY = NULL;
        arr[count].y = strtof(restX, &restY);
        rest = restY[0];
        if (rest != '\n' && rest != '\0') {     // too much input
            free(buffer);
            return -1;
        }
        count++;
    }
    free(buffer);
    if (count == 0) {   // child had only one point
        return 1;
    }
    return 0;
}

/**
 * mean function
 * @brief calculates the arithmetic mean
 * @details calculates the arithmetic mean of the x-coordinates of the given array
 * @param arr array of coordinates
 * @param size size of the array
 * @return the arithmetic mean of the x-coordinates
 **/
double arithmeticMeanX(struct coord *arr, int size) {
    double sum = 0.0;
    int n;
    for (n = 0; n < size; n++) {
        sum += arr[n].x;
    }
    double sizeF = (double) size;
    return sum/sizeF;
}

/**
 * count function
 * @brief counts how many x coordinates are less/equal/greater than the given mean
 * @details inputSize has to be the size of the input array otherwise something bad can happen
 * @param input array of coordinates
 * @param inputSize size of the array
 * @param mean mean of the input array
 * @param l_e_g char specifying which characteristic should be count
 * @return number of coords less, equal or greater than the mean specified by 'l', 'e' or 'g', otherwise return -1.
 **/
int countLEG(struct coord *input, int inputSize, double mean, char l_e_g) {
    int less = -1;
    int equal = -1;
    int greater = -1;
    if (l_e_g == 'l') {
        less = 0;
    } else if (l_e_g == 'e') {
        equal = 0;
    } else if (l_e_g == 'g') {
        greater = 0;
    } else {
        return -1;
    }
    int count = 0;
    int n;
    for(n = 0; n < inputSize; n++) {
        if ((less == 0) && (input[n].x < mean)) {
            count++;
        }
        if ((equal == 0) && (input[n].x == mean)) {
            count++;
        }
        if ((greater == 0) && (input[n].x > mean)) {
            count++;
        }
    }
    return count;
}

/**
 * divide function
 * @brief divides the input array into two seperate arrays
 * @details every pair with x < mean goes to lessEq, every pair with mean < x goes to greater
 * The ones where x == mean go first to lessEq, the next to greater, then again to lessEq etc.
 * The arrays have to have the correct size.
 * @param lessEq coordinate array to write the x-values less and some of the equal x.values to.
 * Size: Nless + (Nequal + 1)/2
 * @param greater coordinate array to write the x-values greater and some of the equal x-values to.
 * Size: Ngreater + Nequal/2
 * @param input input coordinate array which contains the coordinates to being split.
 * @param inputSize size of the input array
 * @param mean mean of the input array
 * @return -1 if the size of the arrays is to small, on success 0.
 **/
int divide(struct coord *lessEq, struct coord *greater, struct coord *input, int inputSize, double mean) {
    if (sizeof(lessEq) + sizeof(greater) < sizeof(input)) {
        return -1;
    }
    int countLE = 0;
    int countGr = 0;
    int lessOrGr = -1;
    int n;
    for (n = 0; n < inputSize; n++) {
        if (input[n].x < mean) {
            lessEq[countLE].x = input[n].x;
            lessEq[countLE].y = input[n].y;
            countLE++;
        } else if (mean < input[n].x) {
            greater[countGr].x = input[n].x;
            greater[countGr].y = input[n].y;
            countGr++;
        } else {
            if (lessOrGr == -1) {
                // put into less
                lessEq[countLE].x = input[n].x;
                lessEq[countLE].y = input[n].y;
                countLE++;
                lessOrGr = +1;
            } else {
                // put into greater
                greater[countGr].x = input[n].x;
                greater[countGr].y = input[n].y;
                countGr++;
                lessOrGr = -1;
            }
        }
    }
    return 0;
}

/**
 * shortest distance between two points function
 * @brief calculates the shortest distance between two points of two seperate arrays
 * @details compares every distance measured and stores the points with the shortest ones in the given array
 * @param closestBetween coordinate array of size 2 to store the two closest points
 * @param first first input coordinate array
 * @param sizeFir size of the first array
 * @param second second input coordinate array
 * @param sizeSec size of the second array
 * @return on success 0, on failure -1
 **/
int shortestDistanceBetween(struct coord closestBetween[2], struct coord *first, int sizeFir, struct coord *second, int sizeSec) {
    double closestDistance = computeDistance(first[0], second[0]);
    closestBetween[0] = first[0];
    closestBetween[1] = second[0];

    int n_first;
    int n_second;
    for (n_first = 0; n_first < sizeFir; n_first++) {
        for (n_second = 0; n_second < sizeSec; n_second++) {
            double distance = computeDistance(first[n_first], second[n_second]);
            if (distance < 0) {     // something bad happend!
                return -1;
            }
            if (distance < closestDistance) {
                closestDistance = distance;
                closestBetween[0] = first[n_first];
                closestBetween[1] = second[n_second];
            }
        }
    }
    return 0;
}

/**
 * distance between two points
 * @brief calculates the distance between two points
 * @details calculates sqrt((x-x1)²+(y-y1)²)
 * @param first first coordinate
 * @param second second coordinate
 * @return the distance between the two points
 **/
static double computeDistance(struct coord first, struct coord second) {
    double x1 = (double) first.x;
    double y1 = (double) first.y;
    double x2 = (double) second.x;
    double y2 = (double) second.y;
    double dx = x1-x2;
    double dy = y1-y2;
    return sqrt(pow(dx, 2.0) + pow(dy, 2.0));
}

/**
 * writes the closest points to stdout
 * @brief writes the two closest points to stdout
 * @details compares three coordinate-pairs to determine which are the closest ones
 * if the ret-value of a coordinate-pair equals 1, then it is ignored
 * @param c1 first coordinate-pair (coordinate-array of size 2)
 * @param ret1 return value of the first coordinate pair
 * @param c2 second coordinate-pair (coordinate-array of size 2)
 * @param ret2 return value of the second coordinate pair
 * @param c3 third coordinate-pair (coordinate-array of size 2)
 * @param ret3 return value of the third coordinate pair
 * @return on success 0, when there is no input (all ret-values equal 1) -1
 **/
int writeClosestToStdout(struct coord c1[2], int ret1, struct coord c2[2], int ret2, struct coord c3[2], int ret3) {
    struct coord closest[2];
    if (ret1 != 1 && ret2 != 1 && ret3 != 1) {      // input from everything
        copyCoordPair(closest, c1);
        if (computeDistance(c2[0], c2[1]) < computeDistance(c1[0], c1[1])) {    // c2 < c1
            copyCoordPair(closest, c2);
        }
        if (computeDistance(c3[0], c3[1]) < computeDistance(closest[0], closest[1])) {  // c3 < closest
            copyCoordPair(closest, c3);
        }
    } else if (ret1 == 1 && ret2 != 1 && ret3 != 1) {   // 1 has no input
        copyCoordPair(closest, c2);
        if (computeDistance(c3[0], c3[1]) < computeDistance(closest[0], closest[1])) {  // c3 < c2
            copyCoordPair(closest, c3);
        }
    } else if (ret1 != 1 && ret2 == 1 && ret3 != 1) {   // 2 has no input
        copyCoordPair(closest, c1);
        if (computeDistance(c3[0], c3[1]) < computeDistance(closest[0], closest[1])) {  // c3 < c1
            copyCoordPair(closest, c3);
        }
    } else if (ret1 != 1 && ret2 != 1 && ret3 == 1) {   // 3 has no input
        copyCoordPair(closest, c1);
        if (computeDistance(c2[0], c2[1]) < computeDistance(closest[0], closest[1])) {  // c2 < c1
            copyCoordPair(closest, c2);
        }
    } else if (ret1 == 1 && ret2 == 1 && ret3 != 1) {   // 1 and 2 has no input
        copyCoordPair(closest, c3);
    } else if (ret1 != 1 && ret2 == 1 && ret3 == 1) {   // 2 and 3 has no input
        copyCoordPair(closest, c1);
    } else if (ret1 == 1 && ret2 != 1 && ret3 == 1) {   // 1 and 3 has no input
        copyCoordPair(closest, c2);
    } else if (ret1 == 1 && ret2 == 1 && ret3 == 1) {   // no input at all -> failure
        return -1;
    } else {                                            // should not be reachable
        return -1;
    }
    
    printCoordArray(stdout, closest, 2);    // print to stdout (parent)
    return 0;
}

/**
 * copy coordinate function
 * @brief copy values from one coordinate pair to another
 * @details copies the float values from src to dest
 * @param dest coordinate pair (coord-array of size 2) to be copied to
 * @param src coordinate pair (coord-array of size 2) to be copied from
 **/
static void copyCoordPair(struct coord dest[2], struct coord src[2]) {
    dest[0].x = src[0].x;
    dest[0].y = src[0].y;
    dest[1].x = src[1].x;
    dest[1].y = src[1].y;
}