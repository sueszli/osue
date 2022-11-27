/**
 * @brief shuffle inplace
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */
//
// Created by mb on 01.11.21.
//

#ifndef B1_SHUFFLE_H
#define B1_SHUFFLE_H

/**
 * @brief Fisher Yates - inplace shuffle (Durstenfeld's version)
 * https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Modern_method
 * @param numbers
 * @param size
 */
void shuffle_array(long *numbers, int size);

#endif //B1_SHUFFLE_H
