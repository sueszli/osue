#include <stdio.h>
#include "datatypes.h"
#include "memMngmt.h"

void print_point_to(Point p, FILE* stream) {
    negative_guard(fprintf(stream, "%f %f\n", p.x, p.y), "printing a point failed");
}
void print_point(Point p) {
    print_point_to(p, stdout);
}