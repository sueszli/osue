/**
 * @file compare.c
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @date 27.10.2021
 *
 * @brief Implementation of the line comparison.
 */
#include "compare.h"

/**
 * @brief Converts the string to lower case
 * @param str The String
 */
static void tolower_str(char *str);

/**
 * Uses getline to read in each line of the file. The buffer ist set to NULL to allow getline to automatically
 * initialize the buffer. The buffer still has to be manually freed. If case_sensitive is 1 then each line and
 * the keyword is converted into lower case.
 * @param file File to be compared.
 * @param outfile Results are written to this file.
 * @param keyword Keyword to be compared.
 * @param case_insensitive 1 enables case-insensitive comparison (the keyword will also be converted to lower case).
 */
void compare_lines(FILE *file, FILE *outfile, char *keyword, int case_insensitive) {

    size_t bufsize = 0;
    char *line_buf = NULL;
    ssize_t linelen;

    if (case_insensitive == 1) {
        tolower_str(keyword);
    }

    while ((linelen = getline(&line_buf, &bufsize, file)) != -1) {
        char *cmp_result;
        if (case_insensitive == 1) {
            char line_lower[linelen + 1];
            strncpy(line_lower, line_buf, sizeof(line_lower));
            tolower_str(line_lower);

            cmp_result = strstr(line_lower, keyword);
        } else {
            cmp_result = strstr(line_buf, keyword);
        }

        if (cmp_result != NULL) {
            // adds newline if it doesn't exist in the line
            if (line_buf[linelen - 1] == '\n') {
                fputs(line_buf, outfile);
            } else {
                fprintf(outfile, "%s\n", line_buf);
            }
        }
    }

    free(line_buf);
}


static void tolower_str(char *str) {
    for (int i = 0; str[i]; ++i) {
        str[i] = (char) tolower(str[i]);
    }
}
