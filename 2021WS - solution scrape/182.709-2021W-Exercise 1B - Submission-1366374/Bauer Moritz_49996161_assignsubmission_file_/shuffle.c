/**
 * @brief shuffle inplace
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */
#include <stdlib.h>    /* for exit */
#include "shuffle.h"

/**
 * @brief returns a random in from 0 to max (exclusive)
 * @param max
 * @return
 */
static int rand_index(int max) {
    return random() % max;
}

/**
 * @brief swap inplace
 * @param numbers
 * @param a
 * @param b
 */
static void swap(long *numbers, int a, int b) {
    int temp = numbers[a];
    numbers[a] = numbers[b];
    numbers[b] = temp;
}

/**
 * Fisher Yates - inplace shuffle (Durstenfeld's version)
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Modern_method
 * @param numbers
 * @param size
 */
void shuffle_array(long *numbers, int size) {
    for (int i = size - 1; i > 0; i--) {
        swap(numbers, i, rand_index(i+1));
    }
}
