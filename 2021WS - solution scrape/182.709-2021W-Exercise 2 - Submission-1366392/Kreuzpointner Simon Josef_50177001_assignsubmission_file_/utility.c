/**
 * @file utility.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 15 November 2021
 * 
 * @brief implementation of the utility module
 */

#include <math.h>

int floatEquals(float a, float b)
{
    if (fabs(a - b) < 0.0001)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int floatCmp(float a, float b)
{
    if (floatEquals(a, b) == 1)
    {
        return 0;
    }
    else if (a < b)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

float floatMin(float a, float b)
{
    if (a < b || floatEquals(a, b) == 1)
    {
        return a;
    }
    else
    {
        return b;
    }
}

float floatMin3(float a, float b, float c)
{
    return floatMin(a, floatMin(b, c));
}