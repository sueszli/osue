#include <stdio.h>
#include <stdlib.h>

void ERROR_MSG(const char *error_msg, const char *prg_name)
{
    fprintf(stderr, "%s - ERROR: %s\n", prg_name, error_msg);
    exit(EXIT_FAILURE);
}
