/**
 * @file debugUtility.c
 * @author Simon Josef Kreuzpointner 12021358
 * @date 21 November 2021
 * 
 * @brief implementation of the debug utility module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "debugUtility.h"

void printError(char *functionName, char *msg)
{
#ifdef ERROR
    extern char *programName;

    (void)fprintf(
        stderr,
        "[%s] [ ERROR ] %s(): %s (%s)\n", programName, functionName, msg, strerror(errno));
#endif
}

void printWarning(char *functionName, char *msg)
{
#ifdef WARNING
    extern char *programName;

    (void)fprintf(
        stderr,
        "[%s] [ WARNING ] %s(): %s\n", programName, functionName, msg);
#endif
}

void printInfo(char *functionName, char *msg)
{
#ifdef INFO
    extern char *programName;

    (void)fprintf(
        stderr,
        "[%s] [ INFO ] %s(): %s\n", programName, functionName, msg);
#endif
}

void printName(char *msg)
{
    extern char *programName;

    (void)fprintf(
        stdout,
        "[%s] %s\n", programName, msg);
}

int printfName(const char *format, ...)
{
    extern char *programName;

    char *f = NULL;
    int temp = snprintf(NULL, 0, "[%s] %s", programName, format);
    if (temp < 0)
    {
        (void)printError("snprintf", "An error ocurred while trying to compile a string.");
        return -1;
    }

    f = malloc(temp + 1);
    if (f == NULL)
    {
        (void)printError("malloc", "An error ocurred while trying to allocate memory.");
        return -1;
    }

    if (snprintf(f, temp + 1, "[%s] %s", programName, format) < 0)
    {
        (void)printError("snprintf", "An error ocurred while trying to compile a string.");
        if (f != NULL)
        {
            (void)free(f);
        }
        return -1;
    }

    va_list varargs;
    va_start(varargs, format);

    (void)vfprintf(stdout, f, varargs);
    (void)fprintf(stdout, "\n");

    if (f != NULL)
    {
        (void)free(f);
    }

    va_end(varargs);

    return 0;
}