/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module  mygrep                                                                      *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief   Reimplements the functionality of the grep commandline tool                 *
 * @details This reimplementation of the grep commandline tool supports an input file,  *
 *          an output file as well as case insensitivity                                *
 *                                                                                      *
 * @date    14.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

char *programName;

/**
 * Prints the usage message and exits the program
 */
void usage( void ) {
    fprintf( stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", programName );
    exit( EXIT_FAILURE );
}

/**
 * @brief Converts a String of characters to lowercase
 *
 * @details
 *
 * @param str  the string to be converted
 * @param dest the destination in memory where the converted string will be copied
 * @param len  the length of the string to be converted
 * @return
 *
 */
char *strToLower( char *str, char *dest, size_t len ) {

    for ( size_t i = 0; i < len; ++i ) {
        dest[ i ] = tolower(( unsigned char ) str[ i ] );
    }

    return dest;
}

int main( int argc, char *argv[] ) {

    // Save the name of the programme in a variable
    programName = argv[ 0 ];
    int caseInsensitive = 0;
    int useOutfile = 0;
    char *outfileName = NULL;
    char *keyword = NULL;

    int c;

    while (( c = getopt( argc, argv, "io:" )) != -1 ) {
        switch ( c ) {
            case 'o':
                useOutfile = 1;
                outfileName = optarg;
                break;
            case 'i':
                caseInsensitive = 1;
                break;
            case '?': /* invalid option */
                usage();
                break;
            default:
                assert( 0 );
        }
    }

    if ( useOutfile == 1 && outfileName == NULL ) {
        usage();
    }

    // Check if the optargs have further arguments
    if ( optind < argc ) {

        // Get the number of extra arguments
        int numExtraArgs = argc - optind;

        // We need the keyword
        // if there is an extra input, it will be treated as a file
        if ( numExtraArgs < 1 ) {
            usage();
        }

        keyword = argv[ optind ];
        // move the pointer one forward
        optind = optind + 1;
        // reduce the amount of extra args by one because the keyword has been taken out
        numExtraArgs = numExtraArgs - 1;

        // if no more args are provided, parse from the stdin
        if ( numExtraArgs < 1 ) {

            char *line = NULL;
            size_t length = 0;
            getline( &line, &length, stdin );
            char *keyword2 = malloc( strlen( keyword ));
            memcpy( keyword2, keyword, strlen( keyword ));
            char *line2 = malloc( strlen( line ));
            memcpy( line2, line, strlen( line ));
            char *comparison = NULL;

            if ( caseInsensitive == 1 ) {
                keyword2 = strToLower( keyword, keyword2, strlen( keyword ));
                //keyword2 = toLower( keyword, strlen( keyword ));
                line2 = strToLower( line, line2, strlen( line ));
                //line2 = toLower( line, strlen( line ));
                comparison = strstr( line2, keyword2 );
            } else {
                comparison = strstr( line, keyword );
            }

            free( keyword2 );
            free( line2 );

            if ( comparison != NULL ) {
                if ( useOutfile == 1 ) {
                    FILE *outfile;
                    if (( outfile = fopen( outfileName, "a" )) == NULL ) {
                        fprintf( stderr, "[%s] ERROR: fopen failed: %s\n", programName, strerror( errno ));
                        fclose( outfile );
                        free( line );
                        exit( EXIT_FAILURE );
                    }
                    fputs( line, outfile );
                    fclose( outfile );
                } else {
                    fprintf( stdout, "%s", line );
                }
            }

            free( line );

        } else {

            // If there are more arguments, this should be our input
            // If there are files, read the files
            FILE **files = ( FILE ** ) malloc( sizeof( FILE * ) * numExtraArgs );
            if ( files == NULL ) {
                fprintf( stderr, "[%s] ERROR: Unable to allocate memory!\n", programName );
                free( files );
                exit( EXIT_FAILURE );
            }

            for ( int i = 0; i < numExtraArgs; i++ ) {

                files[ i ] = fopen( argv[ optind + i ], "r" );

                if ( files[ i ] == NULL ) {
                    fprintf( stderr, "[%s] ERROR: fopen failed: %s\n", programName, strerror( errno ));
                    free( files );
                    exit( EXIT_FAILURE );
                }

                char *line = NULL;
                size_t length = 0;
                char *keyword2 = malloc( strlen( keyword ));
                memcpy( keyword2, keyword, strlen( keyword ));
                while ( getline( &line, &length, files[ i ] ) != -1 ) {

                    char *line2 = malloc( strlen( line ));
                    char *comparison = NULL;

                    if ( caseInsensitive == 1 ) {
                        keyword2 = strToLower( keyword, keyword2, strlen( keyword ));
                        //keyword2 = toLower( keyword, strlen( keyword ));
                        line2 = strToLower( line, line2, strlen( line ));
                        //line2 = toLower( line, strlen( line ));
                        comparison = strstr( line2, keyword2 );
                    } else {
                        comparison = strstr( line, keyword );
                    }

                    free( line2 );

                    if ( comparison != NULL ) {
                        if ( useOutfile == 1 ) {
                            FILE *outfile;
                            if (( outfile = fopen( outfileName, "a" )) == NULL ) {
                                fprintf( stderr, "[%s] ERROR: fopen failed: %s\n", programName, strerror( errno ));
                                fclose( outfile );
                                free( files );
                                free( keyword2 );
                                free( line );
                                exit( EXIT_FAILURE );
                            }
                            fputs( line, outfile );
                            fclose( outfile );
                        } else {
                            fprintf( stdout, "%s", line );
                        }
                    }

                }
                fprintf( stdout, "\n" );

                fclose( files[ i ] );

                free( line );
                free( keyword2 );

            }

            // Release allocated memory
            free( files );

        }

    }
        // If there are no further arguments, then no input was provided
    else {
        usage();
    }

    exit(EXIT_SUCCESS);

}
