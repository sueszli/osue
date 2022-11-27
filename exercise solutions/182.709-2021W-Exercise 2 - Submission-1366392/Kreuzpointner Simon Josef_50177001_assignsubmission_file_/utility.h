/**
 * @file utility.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 15 November 2021
 * 
 * @brief the utility module
 * 
 * @details This module provides usefull functions to work with floats.
 */

#ifndef UTILITY_H
#define UTILITY_H

/**
 * @brief equal check on float
 * 
 * @details This function checks if the difference between two floats are
 * in between a given threshold.
 * If the two given floats are in the threshold, this function returns 1, otherwise 0.
 * 
 * @param a first float
 * @param b second float
 * @return 1 if the two given floats are in between the threshold, 0 otherwise.
 */
int floatEquals(float a, float b);

/**
 * @brief compares two floats
 * 
 * @details This function compares the two given floats.
 * If the two given floats are equal 0 is returned. To check for equality
 * floatEquals() is used.
 * Otherwise if a is a is smaller than b, -1 is returned, else 1.
 * 
 * @see floatEquals()
 * 
 * @param a first float
 * @param b second float
 * @return 0 if a and b are equal, -1 if a < 0 and 1 if a > b
 */
int floatCmp(float a, float b);

/**
 * @brief minimum of the given floats
 * 
 * @details This function returns the minimum of the two given floats.
 * If a is smaller or equal to b, a is returned, else b is returned.
 * 
 * @param a first float
 * @param b second float
 * @return a if a is smaller or equal to b, b otherwise.
 */
float floatMin(float a, float b);

/**
 * @brief minimum of the three given floats
 * 
 * @details This function returns the minimum of the three given
 * floats by calling floatMin() behind the scenes.
 * 
 * @see floatMin()
 * 
 * @param a first float
 * @param b second float
 * @param c third float
 * @return the minimum of the three floats
 */
float floatMin3(float a, float b, float c);

#endif