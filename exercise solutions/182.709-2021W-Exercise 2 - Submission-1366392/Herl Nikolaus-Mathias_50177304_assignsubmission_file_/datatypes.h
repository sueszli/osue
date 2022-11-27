#include <stdio.h>

#ifndef TYPE_DEFINITION_DONE
#define TYPE_DEFINITION_DONE

// type Point = (Float, Float)
typedef struct Point_Str {
    float x;
    float y;
} Point;

struct EdgecaseInfo {
    int   n_low;
    int   n_high;
    Point single;
    int   ind_last_low;
    int   ind_last_high;
};

#endif

void print_point_to(Point p, FILE* stream);
void print_point(Point p);
