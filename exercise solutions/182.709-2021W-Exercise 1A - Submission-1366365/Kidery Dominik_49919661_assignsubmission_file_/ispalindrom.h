#ifndef ISPALINDROM


static void listTraversal(char *input, FILE *outputFile, int f, int b, int c, int i, int s, int o);
static void remove_whitespaces(char *input);
static int checkPalin(char *input, int f, int b, int c, int i);
static void writeToConsole(char *copied, int pal);
static void writeToFile(char *copied, FILE *outputFile, int pal);

#define ISPALINDROM
#endif