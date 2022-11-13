/**
 * @file isPalindrom.h
 * @author Oliver Mayer, MatNr: 12023147
 * @brief This header includes a methode to check wether a string is a palindrom or not
 * @version 0.1
 * @date 2021-11-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * @brief   Checks if the given string is a palindrom. 
 * 
 * @details This method accepts two arguments to define how whitespaces or lower/upper case letters are treated.
 * 
 * @param string string which should be checked
 * @param ignoreWhitespaces Values 1 means ignore whitespaces; != 1 keep whitespaces
 * @param caseSensitive Value 0 means case sensitive; != 0 equals case insensitive
 * @return char* "is a palindrom" if string is a palindrom
 *               "is not a palindrom" if string is not a palindrom
 *               NULL if the function has an error
 */
char *isPalindrom(char *string, int ignoreWhitespaces, int caseSensitive);