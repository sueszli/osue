/**
 * @file searcher.c
 * @author Lukas Thurner MatrNr.: 01427205 <lukas.thurner@tuwien.ac.at> * @date 2.11.2021
 *
 * @brief Implementation of the searcher module
 *
 * The searcher module contains functions for searching a keyword in a string.
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * Convert content of the string to lower letters
 * @brief Convert all char from a string to lower letters
 * @details Read all char from string, convert it to lower case and save it to a temp string. The temp string is used, because the string itself must not be change before the output is written.
 * @param string String of which sould convert to lower letters
 * @param temp_string String with only lower letters 
 */
static void convertToLowerCase(char* string, char* temp_string){

	for(int i = 0; string[i]; i++){
		temp_string[i] = tolower(string[i]);

	}
    temp_string[strlen(string)] = '\0';
	
}

/**
 * Search keyword in a string
 * @brief Checks if the string contains the  keyword either in case sensitive or not
 * @details Search keyword in the string. If the it should not differentiate between lower and upper case letters all char in the keyword and the text are convert into lower case letters.
 * @param keyword This word will be searched in the text
 * @param text The keyword will be searched in this text
 * @return Retruns true if the text contains the keyword otherwise false    
 */
bool search_string(char *keyword, char *text, bool dif_up_low){
	
	char* result;
	
	if(dif_up_low){
	/*! search in case-sensitve way */ 
		 result = strstr(text, keyword);
	}else{
	/*! search in case-insensitve way */
        char temp_string[strlen(text) + 1];
        char temp_key[strlen(keyword) + 1];
        convertToLowerCase(text, temp_string);
        convertToLowerCase(keyword, temp_key);
        result = strstr(temp_string, temp_key);
	}
	if(result != NULL){
		return true;
	}	

	return false;

}

