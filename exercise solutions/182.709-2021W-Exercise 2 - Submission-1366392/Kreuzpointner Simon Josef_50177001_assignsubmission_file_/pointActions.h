/**
 * @file pointActions.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 14 November 2021
 *
 * @brief the point actions module
 * 
 * @details This module provides functionality to work with 2D points,
 * aswell as usefull types
 */

#ifndef POINTACTIONS_H
#define POINTACTIONS_H

#include <stdio.h>

#include "debugUtility.h"

/**
 * @struct Point
 * @brief a 2D Point
 * 
 * @details This struct holds two float values
 * representing the x and y coordinate of the point.
 */

/**
 * @typedef point_t
 * @brief Point type
 * 
 * @details This is the implementation of the struct Point typedef.
 */
typedef struct Point
{
    float x; ///< x coordinate
    float y; ///< y coordinate
} point_t;

/**
 * @struct PointPair
 * @brief a pair of 2D Points
 * 
 * @details This struct holds two 2D Points of type point_t.
 * 
 * @see point_t
 */

/**
 * @typedef pointPair_t
 * @brief Point pair type
 * 
 * @details This is the implementation of the struct PointPair typedef. 
 */
typedef struct PointPair
{
    point_t a; ///< Point a
    point_t b; ///< Point b
} pointPair_t;

/**
 * @brief parses a point from a string
 * 
 * @details This function parses a string to a point_t.
 * This function retuns 0 on success and -1 otherwise.
 * 
 * The format of the string must be a float, which will be the x value,
 * followed by a whitespace and another float, which will be the y value.
 * 
 * The result will be written to dest.
 * 
 * Internally this function calls parseCoordinate() twice, once for each
 * coordinate given.
 * 
 * @param pointString string to be parsed
 * @param dest result of the parse
 * @return 0 on success and -1 otherwise
 */
int parsePoint(char *pointString, point_t *dest);

/**
 * @brief parses a string to a float
 * 
 * @details This function parses the given string to a float and saves
 * the parsed value in dest.
 * 
 * If an error ocurres within this function -1 is returned else 0.
 * 
 * @param coordString string to be parsed
 * @param dest destination for the result
 * @return 0 on success and -1 otherwise.
 */
int parseCoordinate(char *coordString, float *dest);

/**
 * @brief prints a list of points
 * 
 * @details This function calls printPoint() on every point in the 
 * given list. The format of the printing is such, that it can be processed
 * again using parsePoint().
 * 
 * @param points a list of points
 * @param pointCount the number of points
 * @param fd a file descriptor of the destination file to be printed to
 */
void printPoints(point_t **points, int pointCount, int fd);

/**
 * @brief prints a point pair
 * 
 * @details This function calls printPoint() on each point in the point pair.
 * The format of the printing is such, that it can be processed
 * again using parsePoint().
 * 
 * @param pair point pair to be printed
 * @param fd a file descriptor that specifies the file to be printed to
 */
void printPointPair(pointPair_t *pair, int fd);

/**
 * @brief prints the given point to fd
 * 
 * @details This function prints the given point to the location specified
 * via the file descriptor.
 * 
 * @param point the point to be printed
 * @param fd the file descriptor of the destination file
 */
void printPoint(point_t point, int fd);

/**
 * @brief computes the mean of the x coordinates over all given points
 * 
 * @details This function sums up all the x coordinates of the given points
 * and divied them by point count. The result will be written to dest.
 * 
 * @param points a list of points
 * @param pointCount number of points
 * @param dest result destination
 * @return 0 on success and -1 otherwise 
 */
int computeXMean(point_t **points, int pointCount, float *dest);

/**
 * @brief divides the given point list into two separate lists based on a
 * decision value
 * 
 * @details This function compares every x coordinate of the given points
 * to the decision value xm. If the x coordinate of a point is smaller or 
 * equal to xm it will be appended to the destLessThan list. If the x coordinate
 * is greater than xm the corresponding point will be appended to the destGreaterThan
 * list.
 * 
 * The destLessThan and destGreaterThan list have to be freed, even in case
 * of an error.
 * 
 * @param src the full list of points to be splitted
 * @param pointCount number of given points
 * @param destLessThan result destination for the less than part
 * @param destLessThanCount number of points in destLessThan
 * @param destGreaterThan result destination for the greater than part
 * @param destGreaterThanCount number of points in the destGreaterThan part
 * @param xm the decision value (preferrable x-mean)
 * @return 0 on success and -1 otherwise
 */
int dividePointArray(point_t **src, int pointCount, point_t **destLessThan, int *destLessThanCount, point_t **destGreaterThan, int *destGreaterThanCount, float xm);

/**
 * @brief get the closest pair of points from two point lists
 * 
 * @details This function compares the distance between every given point
 * in the two lists and returns the point pair with the shortest distance
 * between them.
 * 
 * The distance between two points is calculated useing the relativeDistance()
 * function.
 * 
 * @param a first list of points
 * @param aCount number of points in the first list
 * @param b second list of points
 * @param bCount number of points in the second list
 * @param dest result destination
 * @return 0 on success and -1 otherwise
 */
int getClosestPair(point_t **a, int aCount, point_t **b, int bCount, pointPair_t *dest);

/**
 * @brief computes the relative distance between two points
 * 
 * @details This function computes the relative euclidean distance between
 * the two given points. The distance returned is not the absolute measurement
 * but rather a relative value. The result can be compared with othe relative
 * measurements but will not be the exact distance.
 * 
 * This is because this is the unrooted distance between a and b. The square root
 * is not performed on the sum of squares of differences. This allows for a
 * performance boost and is still a viable measurement of distance if the 
 * value is only used in comparison with other realtive distances.
 * 
 * @param a first point
 * @param b second point
 * @return the realtive distance between the two given points
 */
float relativeDistance(point_t a, point_t b);

/**
 * @brief computes the relative distance of the given point pair
 * 
 * @details This function returns the relative distance of the pair of points
 * given by calling realtiveDistance() passing in the points in the given pair.
 * 
 * @param p point pair
 * @return the relative distance of the points in p
 */
float relativePairDistance(pointPair_t p);

/**
 * @brief parses an input file to a list of points
 * 
 * @details This function reads the content of the given input file, which is
 * specified via the given file description fd. Each line is the parsed using
 * parsePoint() and stored in dest.
 * 
 * The destination for the resulting points will be allocated inside this function
 * to fit all the points. This destination should be a NULL pointer, otherwise
 * errors may ocur down the line and the allocation process may allocate more memory
 * than actually needed. Dest must be freed even after an error.
 * 
 * If the input file specified by fd is either the standard in, standard out or 
 * standard error no filestream will be opened, instead the standard filestream
 * stdin, stdout and stderr will be used. Otherwise a filestream of the given
 * file descriptor will be opened and used to read its content. When the content
 * is read, the file descriptor will be duplicated and replaced in the given
 * file descriptor argument befor the stream will be closed. This has the effect, 
 * that the stream can be closed without closing the file descriptor.
 * 
 * The file will not be closed if the stream is either stdin, stdout or stderr.
 * 
 * The format of the contents is as follows: each line consists of two floats,
 * the x coordinate and y coordinate of a point. Each line can only accommodate one
 * point.
 * 
 * This function reads until EOF. 
 * 
 * @param dest result destination
 * @param pointCount number of points parsed
 * @param fd file descriptor of the input file
 * @return 0 on success and -1 otherwise
 */
int getPoints(point_t **dest, int *pointCount, int *fd);

/**
 * @brief appends a point to a given point list
 * 
 * @details This function reallocates memory to fit another point to the end 
 * of it. Ideally the given list should be null before first calling this
 * function. Otherwise the behaviour of of the list will be undefined an may lead
 * to errors. If the given list is not null on the first call, the allocated memory
 * will be more than the needed memory.
 * 
 * The list must be freed even after an error.
 * 
 * curPointCount must reflect the number of points in the given list before this
 * function was called. It will update inside of the function accordingly, so there
 * is no need to increment it manually after this function call.
 * 
 * The given point will be copied into the point list. 
 * 
 * @param list the list to append a point to
 * @param curPointCount the number of points currently in the list
 * @param point the point to be appended
 * @return 0 on success, -1 otherwise
 */
int appendToPointList(point_t **list, int *curPointCount, point_t *point);

/**
 * @brief converts a pair of points to an array of points
 * 
 * @details This function returns an array of points, that are inside of 
 * the given point pair. The given point pair must not be null.
 * 
 * @param pair the given point pair
 * @param dest result destination
 */
void convertPointPairToPoints(pointPair_t **pair, point_t *dest[2]);

/**
 * @brief Converts a given list of points to a point pair
 * 
 * @details This function converts a given list of points to a point pair.
 * The number of points in the given list must be 2 to get an accurate
 * point pair back. If the number of points equals to 0 the result will be a point
 * pair with two points with x and y coordinate equal to infinity.
 * 
 * If the number of points equals to 0 a warning will be printed using printWarning(),
 * the destination will be filles as mentioned above and 1 will be returned.
 * 
 * If the point count is not 2 and not 0 an error will ocur and -1 will be returned.
 * The result destination will stay untouched.
 * 
 * If the number of points is exactly 2 the result destination will be accordingly
 * filled and 0 will be returned.
 * 
 * @param points a list of points
 * @param pointCount number of points in the list
 * @param dest result destination
 * @return 0 on success, 1 if the number of points was 0 and an alternative result was
 * returned and -1 on error.
 */
int convertPointsToPointPair(point_t **points, int pointCount, pointPair_t *dest);

/**
 * @brief gets the point pair with the minimum distance
 * 
 * @details This function compares the distance of each point pair given and
 * returns the point pair with the smallest distance. The distance will only be
 * measured inside of the point pairs, no pairs will be measured against each other.
 * The result will be the point pair with the smallest distance in itself.
 * 
 * @param p1 first point pair 
 * @param p2 second point pair
 * @param p3 third point pair
 * @param dest result destination
 * @return 0 on success -1 otherwise
 */
int getMinDistancePointPair(pointPair_t p1, pointPair_t p2, pointPair_t p3, pointPair_t *dest);

#endif