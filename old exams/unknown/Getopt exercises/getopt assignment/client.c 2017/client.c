#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

/** default port number */
#define DEFAULT_PORTNUM     (2017)
#define DEFAULT_PORTNUM_STR "2017"

/** name of the executable (for printing messages) */
char *program_name = "<not yet set>";

/** Defines the command type. */
typedef enum {
  GET = 0,
  SET = 1,
  UNDEF = 2
} cmd_t;

/** Structure for the arguments. */
struct args {
  uint16_t portnum;   /**< port number */
  const char *portstr;/**< port number as string */
  cmd_t cmd;          /**< command (GET, SET) */
  uint8_t value;      /**< set value */
  uint8_t id;         /**< device id */
};

void usage(const char *message);

int main(int argc, char **argv)
{
    struct args arguments =
        { DEFAULT_PORTNUM, DEFAULT_PORTNUM_STR, UNDEF, 0, 0 };

    /* set program_name */
    if (argc > 0) {
        program_name = argv[0];
    }
    
    /***********************************************************************
     * Task 1
     * ------
     * Implement argument parsing for the client. Synopsis:
     *   ./client [-p PORT] {-g|-s VALUE} ID
     *
     * Call usage() if invalid options or arguments are given (there is no
     * need to print a description of the problem).
     *
     * Hints: getopt(3), UINT16_MAX, parse_number (client.h),
     *        struct args (client.h)
     ***********************************************************************/

    /* COMPLETE AND EXTEND THE FOLLOWING CODE */
    int c;
    int opt_p = 0;
    long num;
    char *endptr;

    bool gs_appeared = false;
    bool id_appeared = false;

    if (argc > 6) usage("More argv than possible.");

    while ((c = getopt(argc, argv, "-gs:p:")) != -1) {
        switch (c) {
            case 'p':
                if (opt_p != 0) usage("Every option character only once.");
                
                num = strtol(optarg,&endptr,10);
                if (endptr == optarg) usage("port not parsable.");
                if (num < 1024 || num > UINT16_MAX) usage("PORT ∈ [1024,UINT16_MAX]");

                arguments.portnum = (uint16_t) num;
                arguments.portstr = optarg;
                opt_p = 1;
                break;
            case 'g':
                if (gs_appeared) usage("Only one of g or s as option.");
                gs_appeared = true;
                arguments.cmd = GET;
                break;
            case 's':
                if (gs_appeared) usage("Only one of g or s as option.");
                gs_appeared = true;
                arguments.cmd = SET;
                int value = strtol(optarg,&endptr,10);
                if (endptr == optarg) usage("Value not parsable.");
                if (value < 0 || value > 127) usage("VALUE ∈ [0,127]");
                arguments.value = value;
                break;
            case 1:
                if (id_appeared) usage("Only one positional argument (ID).");
                id_appeared = true;
                int id = strtol(optarg,&endptr,10);
                if (endptr == optarg) usage("ID not parsable.");
                if (id < 0 || id > 63) usage("ID ∈ [0,63]");
                arguments.id = id;
                break;
            default:
                usage("(getopt printed the message)");
        }
    }
    if (!gs_appeared) usage("Specify -g XOR -s.");
    if (!id_appeared) usage("Specify id.");

    
    printf("pn: %d\n",arguments.portnum);
    printf("pns: %s\n",arguments.portstr);
    printf("cmd: %d\n",arguments.cmd);
    printf("value: %d\n",arguments.value);
    printf("id: %d\n",arguments.id);

    return 0;
}


/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message){
    fprintf(stderr,"%s\n",message);
    fprintf(stderr,"Usage: %s [-p PORT] {-g|-s VALUE} ID\n",program_name);
    exit(EXIT_FAILURE);
}


