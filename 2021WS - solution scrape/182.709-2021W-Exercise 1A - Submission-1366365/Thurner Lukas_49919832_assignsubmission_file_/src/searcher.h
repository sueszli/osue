/**
 * @file searcher.h
 * @author Lukas Thurner MatrNr.: 01427205 <lukas.thurner@tuwien.ac.at>
 * @date 2.11.2021
 *
 * @brief Provides function to search string 
 *
 * The searcher module contains functions for searching a keyword in a string.
 */
 

#ifndef SEARCHER_FILE
#define SEARCHER_FILE

#include<string.h>
#include<stdbool.h>

/**
 * Search a keyword in a string
 * @brief This function search a keyword in a string 
 * @details The function search a keyword in a string. If it detects the keyword it returns true, otherwise false.
 * @param keyword Pointer to the keyword
 * @param text Pointer to the text in which the keyword should detect
 * @return Returns if keyword is found or not
 *
 */
bool search_string(char *keyword, char *text, bool dif_up_low);

#endif
