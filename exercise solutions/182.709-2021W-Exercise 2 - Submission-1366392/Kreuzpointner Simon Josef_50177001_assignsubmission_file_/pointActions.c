/**
 * @file pointActions.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 14 November 2021
 * 
 * @brief implementation of the point actions module
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include "pointActions.h"
#include "utility.h"

int parsePoint(char *pointString, point_t *dest)
{
    debug("parsePoint()");

    char *subStringA = NULL;
    char *subStringB = NULL;

    float a;
    float b;

    subStringA = strtok(pointString, " ");
    if (subStringA == NULL)
    {
        (void)printError("strtok", "An error ocurred while trying to tokenise a string.");
        return -1;
    }
    else
    {
        subStringB = strtok(NULL, " ");
        if (subStringB == NULL)
        {
            (void)printError("strtok", "An error ocurred while trying to tokenise a string.");
            return -1;
        }
    }

    if (parseCoordinate(subStringA, &a) < 0)
    {
        return -1;
    }

    if (parseCoordinate(subStringB, &b) < 0)
    {
        return -1;
    }

    *dest = (point_t){
        .x = a,
        .y = b};

    debug("Parsed point: %f %f", dest->x, dest->y);

    return 0;
}

int parseCoordinate(char *coordString, float *dest)
{
    debug("parseCoordinate()");

    char *endPointer = NULL;
    float result = 0.0;
    result = strtof(coordString, &endPointer);

    if (result == 0.0 && endPointer == coordString)
    {
        (void)printError("strtof", "An error ocurred while trying to convert a string to a float. No conversion completed.");
        return -1;
    }

    if (errno == ERANGE)
    {
        (void)printError("strtof", "An error ocurred while trying to convert a string to a float.");
        return -1;
    }

    *dest = result;
    return 0;
}

void printPoints(point_t **points, int pointCount, int fd)
{
    debug("printPoints()");

    for (int i = 0; i < pointCount; i++)
    {
        (void)printPoint((*points)[i], fd);
    }
}

void printPointPair(pointPair_t *pair, int fd)
{
    (void)printPoint(pair->a, fd);
    (void)printPoint(pair->b, fd);
}

void printPoint(point_t point, int fd)
{
    (void)dprintf(fd, "%f %f\n", point.x, point.y);
}

int computeXMean(point_t **points, int pointCount, float *dest)
{
    debug("computeXMean()");

    if (pointCount <= 0)
    {
        (void)printError("computeXMean", "Point count was to small.");
        return -1;
    }

    float sum = 0.0;

    for (int i = 0; i < pointCount; i++)
    {
        sum += (*points)[i].x;
    }

    *dest = (sum / (float)pointCount);

    return 0;
}

int dividePointArray(point_t **src, int pointCount, point_t **destLessThan, int *destLessThanCount, point_t **destGreaterThan, int *destGreaterThanCount, float xm)
{
    debug("dividePointArray()");

    *destLessThanCount = 0;
    *destGreaterThanCount = 0;

    for (int i = 0; i < pointCount; i++)
    {
        if ((*src)[i].x < xm || floatEquals((*src)[i].x, xm) == 1)
        {
            if (appendToPointList(destLessThan, destLessThanCount, &((*src)[i])) < 0)
            {
                (void)printError("appendToPointList", "An error ocurred while trying to appent to a point list.");
                return -1;
            }
        }
        else
        {
            if (appendToPointList(destGreaterThan, destGreaterThanCount, &((*src)[i])) < 0)
            {
                (void)printError("appendToPointList", "An error ocurred while trying to appent to a point list.");
                return -1;
            }
        }
    }

    return 0;
}

int getClosestPair(point_t **a, int aCount, point_t **b, int bCount, pointPair_t *dest)
{
    debug("computeShortestDistance()");

    float d = 0;
    float record = FLT_MAX;

    for (int i = 0; i < aCount; i++)
    {
        for (int j = 0; j < bCount; j++)
        {
            if ((d = relativeDistance((*a)[i], (*b)[j])) < 0)
            {
                (void)printError("relativeDistance", "An error ocurred while trying to calculate the relative distance between two points.");
                return -1;
            }

            if (d < record)
            {
                record = d;

                *dest = (pointPair_t){
                    .a = (*a)[i],
                    .b = (*b)[j]};
            }
        }
    }

    debug("the shortest relative distance found was: %f", record);

    return 0;
}

float relativeDistance(point_t a, point_t b)
{
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

float relativePairDistance(pointPair_t p)
{
    float result = 0;

    if ((result = relativeDistance(p.a, p.b)) < 0)
    {
        return -1;
    }

    return result;
}

int getPoints(point_t **dest, int *pointCount, int *fd)
{
    debug("getPoints()");

    FILE *stream = NULL;

    char *line = NULL;
    size_t lineSize = 0;

    int pointsSize = 0;

    if (*fd != STDIN_FILENO && *fd != STDOUT_FILENO && *fd != STDERR_FILENO)
    {
        stream = fdopen(*fd, "r");
        if (stream == NULL)
        {
            (void)printError("fdopen", "An error ocurred while trying to open a file descriptor.");
            return -1;
        }
    }
    else
    {
        switch (*fd)
        {
        case STDIN_FILENO:
            stream = stdin;
            break;
        case STDOUT_FILENO:
            stream = stdout;
            break;
        case STDERR_FILENO:
            stream = stderr;
            break;
        default:
            (void)assert(0);
            break;
        }
    }

    int i = 0;
    while (getline(&line, &lineSize, stream) > 0)
    {
        // if the end of the line was found
        if (strchr(line, '\n') != NULL)
        {
            // replace the new line symbol
            line[strlen(line) - 1] = '\0';
        }

        debug("Found a line: '%s'", line);

        pointsSize += sizeof(point_t);
        if (*dest == NULL)
        {
            *dest = malloc(pointsSize);
            if (*dest == NULL)
            {
                (void)printError("malloc", "An error ocurred while trying to allocate memory.");

                if (line != NULL)
                {
                    (void)free(line);
                    line = NULL;
                }
                return -1;
            }
        }
        else
        {
            *dest = realloc(*dest, pointsSize);
            if (*dest == NULL)
            {
                (void)printError("realloc", "An error ocurred while trying to reallocate memory.");

                if (line != NULL)
                {
                    (void)free(line);
                    line = NULL;
                }
                return -1;
            }
        }

        if (parsePoint(line, &((*dest)[i])) < 0)
        {
            if (line != NULL)
            {
                (void)free(line);
                line = NULL;
            }
            return -1;
        }

        i++;
    }

    if (line != NULL)
    {
        (void)free(line);
        line = NULL;
    }

    if (ferror(stream) != 0)
    {
        (void)printError("ferror", "An error ocurred while trying to read from a file.");

        if (line != NULL)
        {
            (void)free(line);
            line = NULL;
        }
        return -1;
    }

    // closing the file stream without closing the file descriptor
    if (stream != NULL && stream != stdin && stream != stdout && stream != stderr)
    {
        int newfd = dup(*fd);
        if (newfd < 0)
        {
            (void)printError("dup", "An error ocurred while trying to duplicate a file descriptor.");
            return -1;
        }

        *fd = newfd;

        if (fclose(stream) == EOF)
        {
            (void)printError("fclose", "An error ocurred while trying to close a file stream.");
            return -1;
        }
    }

    *pointCount = i;

    return 0;
}

int appendToPointList(point_t **list, int *curPointCount, point_t *point)
{
    if (*list == NULL)
    {
        *list = malloc(sizeof(point_t) * (++(*curPointCount)));
        if (*list == NULL)
        {
            (void)printError("malloc", "An error ocurred while trying to allocate memory.");
            return -1;
        }
    }
    else
    {
        *list = realloc(*list, sizeof(point_t) * (++(*curPointCount)));
        if (*list == NULL)
        {
            (void)printError("realloc", "An error ocurred while trying to reallocate memory.");
            return -1;
        }
    }

    //TODO: Do i need memcpy here or can i do it with an equal sign aswell?
    (void)memcpy(&((*list)[(*curPointCount) - 1]), point, sizeof(point_t));

    return 0;
}

void convertPointPairToPoints(pointPair_t **pair, point_t *dest[2])
{
    (*dest)[0] = (*pair)->a;
    (*dest)[1] = (*pair)->b;
}

int convertPointsToPointPair(point_t **points, int pointCount, pointPair_t *dest)
{
    if (pointCount == 0)
    {
        (void)printWarning("convertPointsToPointPair", "Could not convert points to point pair, because the point count was 0.");

        dest->a = (point_t){.x = INFINITY, .y = INFINITY};
        dest->b = (point_t){.x = INFINITY, .y = INFINITY};

        return 1;
    }

    if (pointCount != 2)
    {
        (void)printError("convertPointsToPointPair", "Could not convert points to point pair, because the point count was not 2.");
        return -1;
    }

    dest->a = (*points)[0];
    dest->b = (*points)[1];

    return 0;
}

int getMinDistancePointPair(pointPair_t p1, pointPair_t p2, pointPair_t p3, pointPair_t *dest)
{
    float p1Dist;
    float p2Dist;
    float p3Dist;

    if ((p1Dist = relativePairDistance(p1)) < 0)
    {
        return -1;
    }

    if ((p2Dist = relativePairDistance(p2)) < 0)
    {
        return -1;
    }

    if ((p3Dist = relativePairDistance(p3)) < 0)
    {
        return -1;
    }

    float record = floatMin3(p1Dist, p2Dist, p3Dist);

    debug("getMinDistancePointPair record: %f", record);

    if (floatEquals(p1Dist, record) == 1)
    {
        *dest = p1;
        return 0;
    }
    else if (floatEquals(p2Dist, record) == 1)
    {
        *dest = p2;
        return 0;
    }
    else if (floatEquals(p3Dist, record) == 1)
    {
        *dest = p3;
        return 0;
    }
    else
    {
        (void)printError("getMinDistancePointPair", "An error ocurred while trying to find the minimum distance between point pairs.");
        return -1;
    }
}