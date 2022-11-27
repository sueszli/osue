/**
 * @author Maximilian Maresch
 */ 

/**
 * @brief Adds two integers in string representation
 * @detail This function is used to add two integers in string representation. Returns the result as a string.
 * @param x - term 1 of the addition
 * @param y - term 2 of the addition
 * @return x + y as a string
 */ 
char* add(char* x, char* y);

/**
 * @brief Converts a character to an integer
 * @detail This function is used to convert the character character to an integer.
 * @param character - the character to convert
 * @return an integer representing character
 */ 
int hexCharToInt(char character);

/**
 * @brief Adds 0s to an integer in string representation (in the sense of multiplication by 10)
 * @detail This function is used to add numZeros 0s to the end of an integer in string representation (in the sense of multiplication by 10). 
 * Returns the result as a string.
 * @param str - the integer in string representation to which to add 0s to
 * @param numZeros - the number of 0s to add
 * @return str with numZeros 0s at the end
 */ 
char* withZeros(char* str, int numZeros);