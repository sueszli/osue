/**
 * @file points.c
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at
 * @brief implementations of the methods declared in header file points.h
 * @date 04.12.2021
 * 
 * 
 */
#include "points.h"

int rearrangeArray(Point *points, int length)
{
    double mean = 0;

    for (int i = 0; i < length; i++)
    {
        mean += points[i].x;
    }

    mean = mean / length;

    int smaller = 0;
    char change = 0;

    for (int i = 0; i < length; i++)
    {
        if (points[i].x < mean)
        {
            swapEntry(points, i, smaller);
            smaller++;
        }
        else if (points[i].x == mean) {
            if(change) {
                swapEntry(points, i, smaller);
                smaller++;
            }
            change = !change;
        }
    }

    return smaller;
}

void swapEntry(Point *points, int i, int j)
{
    Point temp = points[i];
    points[i] = points[j];
    points[j] = temp;
}

int printPoints(Point *points, int length, FILE* stream)
{
    int error = 0;

    for (int i = 0; i < length; i++)
    {
        error = fprintf(stream, "%f %f\n", points[i].x, points[i].y) < 0 ? -1 : error;
    }
    return error;
    
}

int parsePoint(char* line, Point* point) 
{
    float x, y;

    if(isspace(line[0])) return -1;

    x = strtof(line, &line);

    if(line[0] != ' ' || line[1] == ' ') return -1;

    y = strtof(line, &line);

    if (line[0] != '\n' && line[0] != 0) return -1;

    if(isinff(x) || isinff(y) || isnanf(x) || isnanf(y)) return -1;

    point->x = x;
    point->y = y;

    return 0;
}

float distancePoints(Point p1, Point p2)
{
     return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y) * (p1.y-p2.y));
}