#include <stdio.h>
#include "ispalindrom.h"



FILE *input = fopen("data.txt", "r");

if (input == NULL) {
	fprintf(stderr, "[%s] ERROR: fopen failed: %s\n",prog_name, strerror(errno));
	exit(EXIT_FAILURE);
}
