#include <stdlib.h>
#include <math.h>

// OWN dependencies (i.e. local implementation files)
#include "./parse_usage.h"
#include "memMngmt.h"

Input config;

int string_length (long num) {
    return floor(log10(num)) + 1;
}

double my_div (long dividend, long divisor){
    return (double)dividend / (double)divisor;
}

int main(int argc, char *argv[])
{
    initializeMemMngmt(argv[0], "[-o outfile] [file...]");
    config = parse_usage(argc, argv);

    /* algo outline
     * n_read, n_written = 0
     * prev_char = "null", streak = 0
     * loop1: for each file f
        * loop2: for each char c
            * if EOF
                * (just the first part of the else below?)
            * n_read++
            * if c == prev_char
                * streak++
            * else
                * write "c + streak"
                * n_written = n_written + 1 + (log10 streak, rounded somehow)
                * 
                * prev_char = c
                * streak = 1
    */

    int cur_char;
    int prev_char = -1;
    long streak = 0;
    long n_read = 0, n_written = 0;

    for (int j = 0; j < config.n_infiles; j++)
    {
        //for each in-file:
        bool file_done = false;
        while (!file_done)
        {
            cur_char = fgetc(config.inFILEs[j]);

            if (cur_char == EOF)
            {
                file_done = true;
            }
            else
            {
                n_read++;
                
                // special case for very first character:
                if (prev_char == -1) {
                    prev_char = cur_char;
                }

                // detect longer streak or
                // write out and start new streak
                if (cur_char == prev_char)
                {
                    streak++;
                }
                else
                {
                    fprintf(config.outFILE, "%c%li", prev_char, streak);
                    n_written = n_written + 1 + string_length(streak);

                    prev_char = cur_char;
                    streak = 1;
                }
            }
        }
    }
    fprintf(config.outFILE, "%c%li", prev_char, streak);
    fflush(config.outFILE);
    n_written = n_written + 1 + string_length(streak);

    // Diagnostics message
    fprintf(stderr,
            "Read:%9li characters\nWritten:%6li characters\nCompression ratio: %.2f%%\n",
                 n_read,                n_written,                   my_div(n_written,n_read)*100);


    cleanup();
    return EXIT_SUCCESS;
}
