#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


#define PROTOCOL "http://"


void usage(void)
{
    (void)fprintf(stderr, "Usage: mirror [-l] [-t timeout] [-m maximum] [-f filetype] url \n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){

    int lflag, opt, timeout, tflag, max;
    char *f_arg = NULL;

    lflag = 0;
    timeout = 0;
    tflag = 0;
    max = 0;
    int mflag = 0;
    int fflag = 0;
    char *endpt;
    char *endpt2;

    while ((opt = getopt(argc, argv, "lt:m:f:")) != -1)
    {
        switch (opt)
        {
        case 'l':
            lflag++;
            break;
        case 't':
            
           
            tflag++;
            timeout = (int)(strtol(optarg, &endpt, 10));
            if(endpt == optarg){
                usage();
            }
            break;
        case 'm':
            
            mflag++;
            max = (int)(strtol(optarg, &endpt2, 10));
            if (endpt2 == optarg)
            {
                usage();
            }
            break;
        case 'f':
            fflag++;
            f_arg = optarg;

        default: /* '?' */
            usage();
        }
    }

    if(fflag>1 || mflag>1 || tflag>1 || lflag>1){
        usage();
    }
    char *url = NULL;

    if((argc-optind)!=1){
        usage();
    }else{
        url = argv[optind];
        int i = strncmp(PROTOCOL, url, 7);
        if (i != 0)
        {
            usage();
        }
    }

    exit(EXIT_SUCCESS);
}