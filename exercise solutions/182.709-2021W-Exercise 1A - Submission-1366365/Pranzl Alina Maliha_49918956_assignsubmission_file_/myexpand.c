/**
 * @file myexpand.c
 * @author Alina Maliha Pranzl (12024022)
 * @brief myexpand: reads in several files and replaces tabs with spaces  
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

char* myprog;

static void usage(void) { 
    fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief prints a certain amout of spaces after tab-character in input file using a formula calculated with an input integer
 * @param in: input file where the method reads from
 * @param out: output file where the changed lines get written
 * @param tabstop: positive integer that changes how many spaces get written
 */
static void expand(FILE* in, FILE* out, int tabstop){
    char* line_buf = NULL;
    size_t line_buf_size;
    ssize_t line_length;
    int i;
    int lineexp;
    int p;
    while ((line_length = getline(&line_buf, &line_buf_size, in)) != -1)
    {
        lineexp = 0;
        for (i = 0; i < line_length; i++)
        {
            if(line_buf[i] == '\t')
            {
                p = tabstop * (((i+lineexp) / tabstop) + 1);
                fprintf(out,"%*s", p, " ");
                lineexp += p;
            }
            else
            {
                fprintf(out, "%c", line_buf[i]);
            }
        }
    }

    free(line_buf);
    
}


int main(int argc, char** argv) {
    myprog = argv[0];
    int c;
    int opt_t = 0;
    int opt_o = 0;
    long tabstop = 8;
    char* t_arg = NULL;
    char* o_arg = NULL;
    char* endptr = NULL;
    FILE* out, * in;

    while((c = getopt(argc, argv, "t:o:")) != -1)
    { 
        switch(c)
        {
            case 't':
                opt_t++;
                t_arg = optarg;
                break;
            case 'o':
                o_arg = optarg;
                opt_o++;
                break;
            default:
                usage();
                break;
        }
    }


    if(opt_o > 1)
    {
        fprintf(stderr, "-o provided more than once");
        usage();
    }

    if(opt_t > 1)
    {
        fprintf(stderr, "-t provided more than once");
        usage();
    }

    if(opt_t == 1)
    {
        errno = 0;
        tabstop = strtol(t_arg, &endptr, 10);
        if((errno == ERANGE &&(tabstop == LONG_MAX || tabstop == LONG_MIN)) || (errno != 0 && tabstop == 0) || tabstop < 0){
            fprintf(stderr, "expected tabstop to be int > 0, got %ld", tabstop);
            usage(); 
        }
    } 

    if(opt_o == 1)
    {
        if((out = fopen(o_arg,"w")) == NULL)
        { 
            fprintf(stderr, "could not open output file %s", o_arg);
            usage();
        } 
    }
    else 
    {
        out = stdout;
    }

    if((argc - optind) == 0)
    { 
        expand(stdin, out, tabstop);
    } 
    else 
    {
        int i;
        for ( i = optind; i < argc && errno == 0; i++)
        {
            if((in = fopen(argv[i],"r")) == NULL) 
            { 
                fprintf(stderr, "could not open input file %s", argv[i]);
                usage();
            } 
            expand(in, out, tabstop);
            fclose(in);  
        }
    }

    if(opt_o == 1) 
    {
        fclose(out);
    }

    if(errno != 0)
    {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);

}