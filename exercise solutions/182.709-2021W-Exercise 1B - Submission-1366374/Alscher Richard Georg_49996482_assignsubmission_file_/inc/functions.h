/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * @module  functions.h                                                                 *
 *                                                                                      *
 * @author  Richard Alscher - 11775285                                                  *
 *                                                                                      *
 * @brief                                                                               *
 * @details                                                                             *
 *                                                                                      *
 * @date    14.11.2021                                                                  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/


/**
 * @brief   implementation of the inArray function found in other programming languages
 *
 * @details loops over the haystack and checks if the current element is equivalent to the needle.
 *          returns needle in case they match, returns 0 otherwise.
 *
 * @param   haystack
 * @param   haystackSize
 * @param   needle
 * @return
 */
int inArray( int haystack[], int haystackSize, int needle ) {

    for ( int i = 0; i < haystackSize; i++ ) {

        if ( haystack[ i ] == -1 ) {
            return 0;
        }

        if ( haystack[ i ] == needle ) {
            return i;
        }

    }

    return 0;
}
